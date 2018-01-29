#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "encode_manager.h"
#include "debug.h"

struct font_file_info
{
    unsigned char *pfilemem;                //文件虚拟内存
    unsigned char *pstart;                  //文件显示内容的开始地址(含)
    unsigned char *pend;                    //文件显示内容的结束地址(不含)
    struct stat filestat;                   //文件属性
    unsigned int fontsize;                  //显示字体大小
};

struct font_file_info fileinfo;

static int gbk_open(const char *fontfile, const unsigned int fontsize);
static int gbk_get_font_bitmap(const unsigned int code, struct font_bitmap *pfont_bitmap);

//字体gbk操作集
static struct font_operation gbk_font_ops =
{
    .name               = "gbk",
    .open               = gbk_open,
    .get_font_bitmap    = gbk_get_font_bitmap,
};


/*****************************************************************************
* Function     : gbk_open
* Description  : gbk打开函数
* Input        : const char *fontfile         
*                const unsigned int fontsize  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int gbk_open(const char *fontfile, const unsigned int fontsize)
{
    int gbk_fd;

    //目前只支持16*16的汉字库
    if (fontsize != 16)
    {
        return -1;
    }
    //对于gbk字体，需要提供一个汉字库
    if ( (fontfile == NULL) || (*fontfile == '\0') || (fontsize == 0) )
    {
        return -1;
    }
    //打开汉字库文件
    gbk_fd = open(fontfile, O_RDONLY);
    if (gbk_fd < 0)
    {
        DBG_ERROR("open %s error\n", fontfile);
        return -1;
    }
    //获取字库文件信息
    if (-1 == fstat(gbk_fd, &fileinfo.filestat) )
    {
        DBG_ERROR("fstat %s error\n", fontfile);
        return -1;        
    }
    //内存映射
    fileinfo.pfilemem = (unsigned char *)mmap(NULL, fileinfo.filestat.st_size, PROT_READ, MAP_SHARED, gbk_fd, 0);
    if ((unsigned char *)-1 == fileinfo.pfilemem)
    {
        DBG_ERROR("mmap %s error\n", fontfile);
        return -1;
    }
    //计算内存结束地址(不含)
    fileinfo.pend = fileinfo.pfilemem + fileinfo.filestat.st_size;    
    return 0;
}

/*****************************************************************************
* Function     : freeype_get_font_bitmap
* Description  : 根据编码获取字体位图
* Input        : unsigned int code  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int gbk_get_font_bitmap(const unsigned int code, struct font_bitmap *pfont_bitmap)
{
#if 0
    一个GB2312汉字是由两个字节编码的，范围为0xA1A1~0xFEFE。A1-A9为符号区，B0-F7为汉字区。每一个区有94个字符.
    区码:汉字的第一个字节-0xA0 (因为汉字编码是从0xA0区开始的， 所以文件最前面就是从0xA0区开始， 要算出相对区码)
    位码:汉字的第二个字节-0xA0
    得到汉字在HZK16中的绝对偏移位置: offset=(94*(区码-1)+(位码-1))*32

    注意:区码减1是因为数组是以0为开始而区号位号是以1为开始的
         (94*(区号-1)+位号-1)是一个汉字字模占用的字节数
#endif
    int area, bit;  //区码、位码
    unsigned int pen_x = pfont_bitmap->cur_disp.x;
	unsigned int pen_y = pfont_bitmap->cur_disp.y;

    //一个GBK码用两个字节表示
    if (code & 0xffff0000)
    {
        DBG_ERROR("gbk code bigger than 0xffff0000\n");
        return -1;
    }
    //计算区码
    area = (int)(code & 0xff) - 0xa1;
    //计算位码
    bit  = (int)( (code >> 8) & 0xff)- 0xa1;
    if ( (area < 0) || (bit < 0) )
    {
        DBG_ERROR("gbk code area=%d bit=%d\n", area, bit);
        return -1;
    }
	pfont_bitmap->bitmap_var.left    = pen_x;
	pfont_bitmap->bitmap_var.top     = pen_y - 16;
	pfont_bitmap->bitmap_var.x_max   = pen_x + 16;
	pfont_bitmap->bitmap_var.y_max   = pen_y;
	pfont_bitmap->bitmap_var.bpp      = 1;
	pfont_bitmap->bitmap_var.pitch    = 2;
	pfont_bitmap->bitmap_var.pbuf = fileinfo.pfilemem + ( (area * 94 + bit) << 5);	
    if (pfont_bitmap->bitmap_var.pbuf >= fileinfo.pend)
    {
        DBG_ERROR("gbk bitmap buf overflow, pbuf=%p, fileinfo.pend=%p\n", pfont_bitmap->bitmap_var.pbuf, pfont_bitmap->bitmap_var.pbuf);
        return -1;
    }
	pfont_bitmap->next_disp.x = pen_x + 16;
	pfont_bitmap->next_disp.y = pen_y;

    return 0;
}
/*****************************************************************************
* Function     : gbk_font_init
* Description  : gbk字体初始化
* Input        : void  
* Output       ：
* Return       : 0--注册成功       -1--注册失败
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int gbk_font_init(void)
{
    return register_font_operation(&gbk_font_ops);
}



