#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "config.h"
#include "debug.h"
#include "encode_manager.h"
#include "font_manager.h"
#include "disp_manager.h"
#include "draw.h"


//页描述
struct page_desc
{
    unsigned char *pstart;                  //页开始地址(含)
    unsigned char *pend;                    //页结束地址(不含)
    struct list_head page_list;             //链表
};

struct disp_page_manager
{
    struct list_head list_head;        //页面链表头
    struct list_head *pcur_page;            //当前显示页面
};

struct disp_file_info
{
    unsigned char *pfilemem;                //显示文件虚拟内存
    unsigned char *pstart;                  //文件显示内容的开始地址(含)
    unsigned char *pend;                    //文件显示内容的结束地址(不含)
    struct stat filestat;                   //文件属性
    unsigned int fontsize;                  //显示字体大小
    struct encode_operation* pencode_ops;   //文件支持的编码
};

//显示页面管理
static struct disp_page_manager page_manager =
{
    .list_head  = LIST_HEAD_INIT(page_manager.list_head),
};

//显示文件
static struct disp_file_info file_info;

static struct disp_operation* pdisp_ops;            //显示操作集指针

/*****************************************************************************
* Function     : opentextfile
* Description  : 打开一个文件
* Input        : const char *filename  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int opentextfile(const char *filename)
{
    int filefd;

    //以只读打开文件
    filefd = open(filename, O_RDONLY);
    if (filefd < 0)
    {
        DBG_ERROR("open %s error\n", filename);
        return -1;
    }
    //获取文件信息
    if (-1 == fstat(filefd, &file_info.filestat) )
    {
        DBG_ERROR("fstat %s error\n", filename);
        return -1;        
    }
    //建立虚拟内存映射
    if ( (file_info.pfilemem = (unsigned char *)mmap(NULL, file_info.filestat.st_size, PROT_READ, MAP_SHARED, filefd, 0) ) == (unsigned char *)-1)
    {
        DBG_ERROR("mmap %s error\n", filename);
        return -1;                
    }
    //查找一种支持的编码方式
    if ( (file_info.pencode_ops = selectencodeforfile(file_info.pfilemem, file_info.filestat.st_size) ) == NULL)
    {
        DBG_ERROR("selectencodeforfile %s error\n", filename);
        munmap(file_info.pfilemem, file_info.filestat.st_size);
        return -1;
    }
    file_info.pstart = file_info.pfilemem + file_info.pencode_ops->headlen;  //计算lcd当前显示的在文件的偏移值
    file_info.pend = file_info.pfilemem + file_info.filestat.st_size;           //文件结尾
    return 0;
}


/*****************************************************************************
* Function     : set_font_details
* Description  : 设置字体的参数
* Input        : const char *fontfile         
*                const unsigned int fontsize  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int set_font_details(const char *hzkfile, const char *freetypefile, const unsigned int fontsize)
{
    //以哪种字体显示
    struct list_head *plist, *pnext;
    struct font_list *ptemp;
    struct font_operation *pops;
    unsigned char onefont = 0;

    if (file_info.pencode_ops == NULL)
    {
        return -1;
    }
    list_for_each_safe(plist, pnext, &file_info.pencode_ops->supportfontlist) //plis后续会被释放，不能使用list_for_each
    {
        ptemp = list_entry(plist, struct font_list, list);
      
        if (ptemp && ptemp->pcontext)
        {
            pops = (struct font_operation *)ptemp->pcontext;
            if (pops->open)            //执行字体打开操作
            {
                if (!strcmp(pops->name, "ascii") )
                {
                    if (pops->open(NULL, fontsize) )//打开失败
                    {
                        DBG_WARN("open encode %s error\n", pops->name);
                        list_del(plist);    //删除此字体
                        free_memory(ptemp); //释放struct font_list *ptemp;
                    }
                    else
                    {
                        DBG_INFO("open ascii ok\n");
                        onefont++;
                    }
                }
                else if (!strcmp(pops->name, "gbk") )
                {
                    if (pops->open(hzkfile, fontsize) )//打开失败
                    {
                        DBG_WARN("open encode %s error\n", pops->name);
                        list_del(plist);    //删除此字体
                        free_memory(ptemp); //释放struct font_list *ptemp;
                    }
                    else
                    {
                        DBG_INFO("open gbk ok\n");
                        onefont++;
                    }
                }
                else if (!strcmp(pops->name, "freetype") )
                {
                    if (pops->open(freetypefile, fontsize) )//打开失败
                    {
                        DBG_WARN("open encode %s error\n", pops->name);
                        list_del(plist);    //删除此字体
                        free_memory(ptemp); //释放struct font_list *ptemp;
                    }
                    else
                    {
                        DBG_INFO("open freetype ok\n");
                        onefont++;
                    }
                }
            }
        }
    }
    if (!onefont)                   //没有一个字体可支持
    {
        DBG_ERROR("no font support to display\n");
        return -1;
    }
    file_info.fontsize = fontsize;           //保存字体大小
    return 0;
}

/*****************************************************************************
* Function     : open_display_by_name
* Description  : 通过名字打开一个display设备
* Input        : const char* name  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int open_display_by_name(const char* name)
{
    struct disp_operation *ptemp;
    
    if (name == NULL)
        return -1;
    if ( (ptemp = get_display_operation_by_name(name) ) == NULL)
        return -1;
    if (ptemp->open)    //提供了open函数，调用open函数
    {
        pdisp_ops = ptemp;
        return ptemp->open();
    }
    return 0;
}

/*****************************************************************************
* Function     : get_display_operations
* Description  :  获取显示操作集指针
* Input        : void  
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月18日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct disp_operation* get_display_operations(void)
{
    return pdisp_ops;
}

/*****************************************************************************
* Function     : inc_lcd_x
* Description  : 增加x坐标
* Input        : int x  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月9日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int inc_lcd_x(int x)
{
	if ( (x + 1) < pdisp_ops->xres)
		return (x + 1);
	else
		return 0;
}

/*****************************************************************************
* Function     : inc_lcd_y
* Description  : y坐标加1
* Input        : int y  
* Output       ：
* Return       : 0--回到第一行
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月9日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int inc_lcd_y(int y)
{
	if ( (y + file_info.fontsize) < pdisp_ops->yres)
		return (y + file_info.fontsize);
	else
		return 0;
}

/*****************************************************************************
* Function     : relocate_font_pos
* Description  : 定位字体显示位置
* Input        : struct font_bitmap* pfont_bitmap  
* Output       ：
* Return       : -1 ：满页了 0 ：可以正显示下一页
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月9日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int relocate_font_pos(struct font_bitmap* pfont_bitmap)
{
    unsigned int x, y;
    int delta_x;
	int delta_y;
    
    if (pfont_bitmap->bitmap_var.y_max > pdisp_ops->yres) //该字体的y坐标超过了显示的y坐标
    {
        //满页了
        return -1;
    }

    if (pfont_bitmap->bitmap_var.x_max > pdisp_ops->xres)//该字体的x坐标超过了显示的x坐标，需要换行
    {
        //需要换行,计算新坐标的x、y
        x = 0;
        y = inc_lcd_y(pfont_bitmap->cur_disp.y);        //y坐标加1
        if (0 == y)
        {
            //满页了
            return -1;
        }
        else
        {
            /* 没满页 */
			delta_x = pfont_bitmap->cur_disp.x - x;     //计算新坐标跟原坐标的x差值
			delta_y = y - pfont_bitmap->cur_disp.y;     //计算新坐标跟原坐标的y差值

			pfont_bitmap->cur_disp.x  = x;              //更改当前显示位置
			pfont_bitmap->cur_disp.y  = y;

			pfont_bitmap->next_disp.x -= delta_x;       //更改下一个显示位置
			pfont_bitmap->next_disp.y += delta_y;

			pfont_bitmap->bitmap_var.left -= delta_x;   //字体显示的x坐标
			pfont_bitmap->bitmap_var.x_max -= delta_x;  //字体x方向最大坐标

			pfont_bitmap->bitmap_var.top += delta_y;    //字体显示的y坐标
			pfont_bitmap->bitmap_var.y_max += delta_y;; //字体y方向的最大坐标
        }
    }
    return 0;
}


static int show_one_font(struct font_bitmap* pfont_bitmap)
{
	unsigned int x, y, i = 0;
	unsigned char byte = 0;
	int bit;

	if (pfont_bitmap->bitmap_var.bpp == 1)
	{
		for (y = pfont_bitmap->bitmap_var.top; y < pfont_bitmap->bitmap_var.y_max; y++)
		{
			i = (y - pfont_bitmap->bitmap_var.top) * pfont_bitmap->bitmap_var.pitch;
			for (x = pfont_bitmap->bitmap_var.left, bit = 7; x < pfont_bitmap->bitmap_var.x_max; x++)
			{
				if (bit == 7)
				{
					byte = pfont_bitmap->bitmap_var.pbuf[i++];
				}
				
				if (byte & (1<<bit))
				{
					pdisp_ops->show_pixel(x, y, COLOR_FOREGROUND);
				}
				else
				{
					/* 使用背景色, 不用描画 */
					// g_ptDispOpr->ShowPixel(x, y, 0); /* 黑 */
				}
				bit--;
				if (bit == -1)
				{
					bit = 7;
				}
			}
		}
	}
	else if (pfont_bitmap->bitmap_var.bpp == 8)
	{
		for (y = pfont_bitmap->bitmap_var.top; y < pfont_bitmap->bitmap_var.y_max; y++)
			for (x = pfont_bitmap->bitmap_var.left; x < pfont_bitmap->bitmap_var.x_max; x++)
			{
				//g_ptDispOpr->ShowPixel(x, y, ptFontBitMap->pucBuffer[i++]);
				if (pfont_bitmap->bitmap_var.pbuf[i++])
					pdisp_ops->show_pixel(x, y, COLOR_FOREGROUND);
			}
	}
	else
	{
		DBG_ERROR("ShowOneFont error, can't support %d bpp\n", pfont_bitmap->bitmap_var.bpp);
		return -1;
	}
	return 0;
}


/*****************************************************************************
* Function     : show_one_page
* Description  : 显示一页
* Input        : void  
* Output       ：
* Return       : 0---显示完一页或者到达文件尾 -1---出错
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int show_one_page(unsigned char *pstart, unsigned char *pend, unsigned char **const parse)
{
    struct list_head *plist;
    struct font_list* ptemp;
    struct font_operation *pfontops;
    struct font_bitmap font_bitmap;
    unsigned int code;
    int codenum;
    unsigned char has_not_clr_sceen = 1;
    
    if ( (pstart == NULL) || (pend == NULL) || (pstart >= pend) )
    {
        return -1;
    }
    
    font_bitmap.cur_disp.x = 0;
    font_bitmap.cur_disp.y = file_info.fontsize;
    
    while (1)
    {
        //1、使用支持的encode进行解码
        codenum = file_info.pencode_ops->getcodefrmbuf(pstart, pend, &code);
        DBG_INFO("num = 0x%x, code=0x%x\n", codenum, code);
        if (codenum == -1) //解码失败
        {
            DBG_ERROR("use %s encode file error\n", file_info.pencode_ops->name);
            pdisp_ops->show_flush(NULL, 0);
            return -1;
        }
        else if (codenum == 0) //达到文件尾巴
        {
            DBG_INFO("file display end\n");
            pdisp_ops->show_flush(NULL, 0);
            return 0;   
        }
        pstart += codenum;  //计算下一个编码
        //有些文本以\n\r两个一起才便是回车换行，这种只处理\n
        if (code == '\n')
        {
            // 回车换行,切换到下一行
			font_bitmap.cur_disp.x = 0;   //x坐标为0
			font_bitmap.cur_disp.y = inc_lcd_y(font_bitmap.cur_disp.y); //切换到下一行
            if (0 == font_bitmap.cur_disp.y) 
			{
				/* 显示完当前一屏了 */
                *parse = pstart - codenum;
                pdisp_ops->show_flush(NULL, 0);
				return 0;
			}
			else
			{
				continue;
			}
        }
        else if (code == '\r')
        {
            continue;
        }
        else if (code == '\t') 
        {
            //tab用4个空格代替
            code = ' ';
        }
        //2、使用encode支持的font进行显示
        //遍历该编码支持的font
        list_for_each(plist, &file_info.pencode_ops->supportfontlist)
        {
            ptemp = list_entry(plist, struct font_list, list);
            if (ptemp && ptemp->pcontext)
            {
                pfontops = (struct font_operation*)ptemp->pcontext;
                DBG_INFO("pfontops->name %s\n", pfontops->name);
                if (!pfontops->get_font_bitmap(code, &font_bitmap) ) //提取bitmap成功
                {
                    if (relocate_font_pos(&font_bitmap) )            //根据位图定位显示的位置
                    {
                        if (parse)
                            *parse = pstart - codenum;
                        DBG_INFO("show one page over\n");
                        pdisp_ops->show_flush(NULL, 0);
                        //剩余空间不能显示这个字符了
                        return 0;
                    }
                    if (has_not_clr_sceen)
                    {
                        pdisp_ops->clean_screen(COLOR_BACKGROUND, NULL, 0);
                        pdisp_ops->show_flush(NULL, 0);
                        has_not_clr_sceen = 0;
                    }
                    break;
                }
                else
                {
                    DBG_ERROR("get_font_bitmap error\n");
                }
            }
        }
        
        //3、使用指定的disp进行显示
        if (show_one_font(&font_bitmap)) //显示一个字符
		{
            DBG_ERROR("show font error\n");
            pdisp_ops->show_flush(NULL, 0);
			return -1;
		}
        //指定下一个要显示的坐标
        font_bitmap.cur_disp = font_bitmap.next_disp;
    }
}


/*****************************************************************************
* Function     : show_next_page
* Description  : 显示下一页
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月9日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_next_page(void)
{
    struct page_desc *ppage_desc;
    unsigned char *cur_page_start = NULL, *cur_page_end = NULL;

    if (page_manager.pcur_page == NULL)
    {
        cur_page_start = file_info.pstart;
    }
    else
    {
        cur_page_start = ( (struct page_desc *)list_entry(page_manager.pcur_page, struct page_desc, page_list) )->pend;
    }
   
    //显示一页数据
    if (!show_one_page(cur_page_start, file_info.pend, &cur_page_end) )
    {
        if ( (page_manager.pcur_page == NULL) || page_manager.pcur_page->next == &page_manager.list_head) //已经到达链表的最后一项，需要把当前界面的内容加入链表
        {
            ppage_desc = (struct page_desc *)malloc(sizeof(struct page_desc) );
            if (ppage_desc == NULL)
            {
                return -1;
            }
            ppage_desc->pstart = cur_page_start;
            ppage_desc->pend = cur_page_end;
            ppage_desc->page_list.prev = &ppage_desc->page_list;
            ppage_desc->page_list.next = &ppage_desc->page_list;
            list_add_tail(&ppage_desc->page_list, &page_manager.list_head); //添加到链表尾部
            page_manager.pcur_page = &ppage_desc->page_list;
        }
        else
        {
            page_manager.pcur_page = page_manager.pcur_page->next;
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

/*****************************************************************************
* Function     : show_prev_page
* Description  : 显示上一页
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月11日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_prev_page(void)
{
    unsigned char *cur_page_start = NULL, *cur_page_end = NULL;

    cur_page_start = ( (struct page_desc *)list_entry(page_manager.list_head.prev, struct page_desc, page_list) )->pend;

    if (page_manager.pcur_page == NULL)
    {
        return -1;
    }
    else
    {
        if (page_manager.pcur_page->prev == &page_manager.list_head)
        {
            //达到第一页
            DBG_INFO("first page\n");
            return 0;
        }
        cur_page_start = ( (struct page_desc *)list_entry(page_manager.pcur_page->prev, struct page_desc, page_list) )->pstart;
    }
    if (!show_one_page(cur_page_start, file_info.pend, &cur_page_end) )
    {
//        printf("\npage_start %p, page_end %p\n", cur_page_start, cur_page_end);
        page_manager.pcur_page = page_manager.pcur_page->prev;
    }
    return 0;
}


