#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/fb.h>
#include "config.h"
#include "disp_manager.h"
#include "disp_lcd.h"
#include "debug.h"

static struct fb_var_screeninfo var;	/* Current var */
//static struct fb_fix_screeninfo fix;	/* Current fix */
static unsigned int screen_size;
static unsigned char *fbmem, *lcd_temp_buf;
static unsigned int line_width;
static unsigned int pixel_width;
static int lcd_fd;

static int disp_lcd_open(void);
static int disp_lcd_close(void);
static int disp_lcd_show_pixel(int x, int y, unsigned int color);
static int disp_lcd_clean_screen(unsigned int back_color, unsigned char *pback_buf, unsigned int back_size);
static int disp_lcd_show_flush(unsigned char *wrbuf, unsigned int wrnum);


static struct disp_operation lcd_ops =
{
    .name           = "lcd",
    .open           = disp_lcd_open,
    .close          = disp_lcd_close, 
    .show_pixel     = disp_lcd_show_pixel,
    .clean_screen   = disp_lcd_clean_screen,
    .show_flush     = disp_lcd_show_flush,
    .list           = LIST_HEAD_INIT(lcd_ops.list),
};

/*****************************************************************************
* Function     : disp_lcd_init
* Description  : lcd初始化
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int disp_lcd_open(void)
{
    //打开/dev/fb0
    lcd_fd = open("/dev/fb0", O_RDWR);
    if (lcd_fd < 0)
    {
        DBG_ERROR("open /dev/fb0 error\n");
        return -1;
    }

    //获取lcd的可变参数
    if (-1 == ioctl(lcd_fd, FBIOGET_VSCREENINFO, &var) )
    {
        DBG_ERROR("get /dev/fb0 var error\n");
        return -1;        
    }

    //lcd freambuffer字节数 : x分辨率 * y分辨率 * 每个像素的位数 / 8
    line_width  = var.xres * var.bits_per_pixel / 8;
	pixel_width = var.bits_per_pixel / 8;
    screen_size = var.xres * var.yres * var.bits_per_pixel / 8;

    //虚拟内存映射
    fbmem = (unsigned char *)mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    if ( (unsigned char *)(-1) == fbmem)
    {
        DBG_ERROR("mmap /dev/fb0 error\n");
        return -1;        
    }
    //分配一个中间内存，避免一个一个字刷新
    check_null_point(lcd_temp_buf);
    lcd_temp_buf = (unsigned char *)malloc(screen_size);
    if ( (unsigned char *)(NULL) == lcd_temp_buf)
    {
        DBG_ERROR("malloc lcd_temp_buf error\n");
        return -1;        
    } 
    //记录lcd的大小
    lcd_ops.xres = var.xres;
    lcd_ops.yres = var.yres;
    lcd_ops.bpp = var.bits_per_pixel;
    lcd_ops.dev_mem_size = screen_size;
    //清屏
    memset(fbmem, COLOR_BACKGROUND, screen_size);
    memset(lcd_temp_buf, COLOR_BACKGROUND, screen_size);
    return 0;
}

/*****************************************************************************
* Function     : disp_lcd_close
* Description  : 关闭lcd设备
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月12日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int disp_lcd_close(void)
{
    //释放临时内存
    free_memory(lcd_temp_buf);
    munmap(fbmem, screen_size);
    close(lcd_fd);
    return 0;
}

/*****************************************************************************
* Function     : disp_lcd_show_pixel
* Description  : lcd显示一个像素
* Input        : int x               
*                int y               
*                unsigned int color  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int disp_lcd_show_pixel(int x, int y, unsigned int color)
{
    unsigned char *pen_8 = lcd_temp_buf + y * line_width + x * pixel_width;
    unsigned short *pen_16 = (unsigned short *) pen_8;
    unsigned int *pen_32 = (unsigned int *)pen_8;
    unsigned int red, green, blue;

    if ( (x >= var.xres) || (y >= var.yres) )
	{
		return -1;
	}

    switch (var.bits_per_pixel)
    {
        case 8:
            *pen_8 = (unsigned char)color;
            break;
        case 16:
            /* 888转565 */
			red   = (color >> 16) & 0xff;  
			green = (color >> 8) & 0xff;
			blue  = (color >> 0) & 0xff;
			color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3); //把24色转成16色
			*pen_16 = (unsigned short)color;
            break;
        case 32:
            *pen_32 = color;
            break;
        default:
            return -1;
    }
    return 0;
}

/*****************************************************************************
* Function     : disp_lcd_show_flush
* Description  :  把数据写入真正的lcd buf中
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月13日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int disp_lcd_show_flush(unsigned char *wrbuf, unsigned int wrnum)
{
    if (wrbuf == NULL)
    {
        memcpy(fbmem, lcd_temp_buf, screen_size);
    }
    else
    {
        if (wrnum == 0)
        {
            wrnum = screen_size;
        }
        memcpy(fbmem, wrbuf, wrnum > screen_size ? screen_size : wrnum);
    }
    return 0;
}
/*****************************************************************************
* Function     : disp_lcd_clean_screen
* Description  : lcd清屏函数
* Input        : unsigned int back_color  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int disp_lcd_clean_screen(unsigned int back_color, unsigned char *pback_buf, unsigned int back_size)
{
    unsigned int red, green, blue, i = 0;
    unsigned int clear_size;
    unsigned char *ptr;

    ptr = pback_buf ? pback_buf : lcd_temp_buf;
    clear_size = pback_buf ? back_size : screen_size;
        
    switch (var.bits_per_pixel)
    {
        case 8:
            memset(ptr, back_color, clear_size);
            break;
        case 16:
            /* 888转565 */
			red   = (back_color >> 16) & 0xff;  
			green = (back_color >> 8) & 0xff;
			blue  = (back_color >> 0) & 0xff;
			back_color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3); //把24色转成16色
            while (i < clear_size)
            {
                *( (unsigned short*)ptr) = back_color;
                ptr += 2;
                i += 2;
            }
            break;
        case 32:
            while (i < clear_size)
			{
				*( (unsigned int*)ptr) = back_color;
				ptr += 4;
				i += 4;
			}
            break;
        default:
            DBG_ERROR("lcd bpp %d, can not support\n", var.bits_per_pixel);
            return -1;
    }
    return 0;
}

/*****************************************************************************
* Function     : disp_lcd_init
* Description  : lcd初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int disp_lcd_init(void)
{
    return register_disp_operation(&lcd_ops);
}

