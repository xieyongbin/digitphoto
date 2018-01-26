#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "libthreadpro.h"
#include "page_manager.h"
#include "page_browse.h"
#include "pic_merge.h"
#include "pic_zoom.h"
#include "file.h"
#include "config.h"
#include "debug.h"


#define DIR_FILE_ICON_WIDTH   (40u)
#define DIR_FILE_ICON_HEIGHT  (40u)

#define DIR_FILE_NAME_HEIGHT  (20u)
#define DIR_FILE_NAME_WIDTH   (20u)

#define DIR_DELTA_X           (40u)
#define DIR_DELTA_Y           (20u)

#define ICON_DIR_NAME         "fold_closed.bmp"
#define ICON_FILE_NAME        "file.bmp"

#define DIR_TOP_PATH          "/"       //顶层目录

static struct pic_data* get_browse_page_data(const struct disp_operation * const pdisp);
static int deal_browse_page_event(struct input_event* pevent, struct disp_operation* pdisp);
static int browse_page_sync_source(struct pic_data *ppic_data);
/*****************************************************************************
* Function     : browse_page_free_source
* Description  : 释放当前界面的资源，切换到别的页面时调用
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月25日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void browse_page_free_source(void);

/*****************************************************************************
* Function     : __browse_page_free_source
* Description  : 释放当前接资源，仅内部使用，
* Input        : int free_background  ： 0 --- 不释放背景内存
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月25日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void __browse_page_free_source(int free_background);

//浏览界面显示的坐标
static const struct disp_layout browse_page_icon_layout[] =
{
    {0, 0,  68, 68,  'u', "up.bmp"},          //向上
       
	{0, 69, 68, 136, 's', "select.bmp"},      //选择
	
	{0, 137, 68, 204, 'p', "pre_page.bmp"},   //上一页

    {0, 205, 68, 271, 'n', "next_page.bmp"},  //下一页
   
	{0, 0, 0, 0, '\0', NULL},
};

//浏览界面选择操作目录/文件的操作
static const struct disp_layout browse_page_select_layout[] =
{
    {0, 0, 0, 0, 'd', NULL},    //选择目录

    {0, 0, 0, 0, 'f', NULL},    //选择常规文件
};

static struct pic_data *pcur_disp_data;        //当前显示界面
static unsigned char need_generate_background = 1; //需要生成背景图

static char dir_name_select[256] = DIR_TOP_PATH;            //选择显示的目录名

//当前显示内容
static struct page_context_desc cur_page_context, next_page_context;

static struct page_operations page_browse_ops =
{
    .name = "browse",
    .ppage_father = NULL,
    .page_kid = LIST_HEAD_INIT(page_browse_ops.page_kid),
    .ppage_data = NULL,
    .get_page_data = get_browse_page_data,
    .deal_event = deal_browse_page_event,
    .sync_source = browse_page_sync_source,
    .free_source = browse_page_free_source,
};

/*****************************************************************************
* Function     : calc_file_coordinate
* Description  : 计算各文件的坐标
* Input        : struct disp_layout *pshowdev  
*                unsigned int startx           
*                unsigned int starty           
* Output       ：
* Return       : static
* Note(s)      : 此函数将会分配一块内存，
* Histroy      : 
* 1.Date       : 2018年1月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int calc_file_coordinate(struct disp_layout *pshowdev, unsigned int startx, unsigned int starty, struct page_browse *pbrowsepage)
{
    unsigned int i;
    unsigned int col, row;
    unsigned int file_icon_width, file_icon_height;
    unsigned int num_per_row, num_per_col;
    
    struct disp_layout **pfile_layout;
    
    if ( (pshowdev == NULL) || (pbrowsepage == NULL) )
    {
        return -1;
    }
    if ( (startx < pshowdev->topleftx) || (starty < pshowdev->toplefty) \
        ||(startx > pshowdev->botrightx) || (starty > pshowdev->botrighty) )
    {
        return -1;
    }

    //确定一行显示多少个"目录或文件", 显示多少行
    file_icon_width = DIR_FILE_ICON_WIDTH;
    file_icon_height =  file_icon_width;

    //计算每行图标个数，每个图标间隔为10
//    num_per_row * file_icon_width + (num_per_row - 1) * 10 <= (pshowdev->botrightx - pshowdev->topleftx + 1)
    num_per_row = (pshowdev->botrightx - startx + 1 + 10) / (file_icon_width + DIR_DELTA_X);

    //计算每列的图标个数，每个图标间隔为10
//    num_per_col * file_icon_height + (num_per_col - 1) * 10 <= (pshowdev->botrighty - pshowdev->toplefty + 1)
    num_per_col = (pshowdev->botrighty - starty + 1 + 10) / (file_icon_height + DIR_FILE_NAME_HEIGHT + DIR_DELTA_Y);

    DBG_INFO("num_per_row %d, num_per_col %d\n", num_per_row, num_per_col);

    //申请num_per_row * num_per_col个struct disp_layout
    if ( (pfile_layout = calloc(num_per_col, sizeof(struct disp_layout *) ) ) == NULL)
    {
        DBG_ERROR("calloc %d struct disp_layout * error\n", num_per_col + 1);
        return -1;
    }
    for (col = 0; col < num_per_col; col++)
    {
        if ( (pfile_layout[col] = calloc(num_per_row, sizeof(struct disp_layout) ) ) == NULL)
        {
            for (i = 0; i < col; i++)
            {
                free_memory(pfile_layout[i]);
            }
            free_memory(pfile_layout);
            DBG_ERROR("calloc %d struct disp_layout error\n", num_per_row);
            return -1;
        }
    }
    
    for (col = 0; col < num_per_col; col++)
    {
        for (row = 0; row < num_per_row; row++)
        {
            pfile_layout[col][row].toplefty = starty + (file_icon_height + DIR_FILE_NAME_HEIGHT+ DIR_DELTA_Y) * col;
            pfile_layout[col][row].botrighty = pfile_layout[col][row].toplefty + file_icon_height;

            pfile_layout[col][row].topleftx = startx + (file_icon_width + DIR_DELTA_X) * row;
            pfile_layout[col][row].botrightx = pfile_layout[col][row].topleftx + file_icon_width;
        } 
    }
    
    pbrowsepage->pagerow = num_per_row;                                         //每行
    pbrowsepage->pagecol = num_per_col;                                         //每页
    pbrowsepage->numperpage = num_per_row * num_per_col;                        //一页的个数
    pbrowsepage->pagenum = pbrowsepage->totalnum / pbrowsepage->numperpage;     //总共页数
    if (pbrowsepage->totalnum % pbrowsepage->numperpage)
    {
        pbrowsepage->pagenum++;
    }
    pbrowsepage->pagedata = pfile_layout;
    
    return 0;
}

/*****************************************************************************
* Function     : get_dir_one_page_context
* Description  : 获取一个目录下的一页内容
* Input        : unsigned int page                
*                struct page_browse *pbrowsepage  
*                struct pic_data *pbigpic         
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月10日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int get_dir_one_page_context(unsigned int page, struct page_browse *pbrowsepage, struct pic_data *pbigpic)
{
    char path[128];
    int retval = -1;
    unsigned int row, col;
    unsigned int start;
    struct pic_data file_zoom_pic, dir_zoom_pic; 
    struct file_dirent *pdirent;
    
    file_zoom_pic.pixeldata = NULL;
    dir_zoom_pic.pixeldata = NULL;
    
    if (pbigpic == NULL)
    {
        return retval;
    }
    start = page * pbrowsepage->numperpage;
    if (start > pbrowsepage->totalnum)
    {
        DBG_ERROR("start %d is too larger than totalnum %d\n", start, pbrowsepage->totalnum);
        return retval;
    }
    //构建文件路径
    strncpy(path, ICON_PATH, sizeof(path) - 1);
    strncat(path, ICON_FILE_NAME, sizeof(path) - 1);
    //提取缩放后的文件数据
    if (page_get_pic_data(path, pbrowsepage->pagedata[0], pbigpic->bpp, &file_zoom_pic) )
    {
        DBG_ERROR("get icon file error\n");
        return retval;
    }

    //构建目录路径
    strncpy(path, ICON_PATH, sizeof(path) - 1);
    strncat(path, ICON_DIR_NAME, sizeof(path) - 1);
    //提取缩放后的文件数据
    if (page_get_pic_data(path, pbrowsepage->pagedata[0], pbigpic->bpp, &dir_zoom_pic) )
    {
        free_memory(file_zoom_pic.pixeldata);
        DBG_ERROR("get icon dir error\n");
        return retval;
    }
    
    pdirent = pbrowsepage->pdirfiledirent + start;
    for (col = 0; col < pbrowsepage->pagecol; col++)
    {
        for (row = 0; row < pbrowsepage->pagerow; row++)
        {
            //是否已经结尾了
            if (col * pbrowsepage->pagerow + row + start >= pbrowsepage->totalnum)
            {
                retval = 0;
                goto goto_done;
            }
            
            switch (pdirent->filetype)
            {
                case FILE_TYPE_DIR:  //是目录
                    if (pic_merge(pbrowsepage->pagedata[col][row].topleftx, pbrowsepage->pagedata[col][row].toplefty, &dir_zoom_pic, pbigpic) )
                    {
                        DBG_ERROR("pic merger error\n");
                        goto goto_err;
                    }
                    //显示目录名字
                    if (string_merge(pbrowsepage->pagedata[col][row].topleftx, pbrowsepage->pagedata[col][row].toplefty + DIR_FILE_ICON_HEIGHT + 16,\
                        (unsigned char*)pdirent->filename, pbigpic, 16) )
                    {
                        DBG_ERROR("string_merge error\n");
                        goto goto_err;
                    }
                    break;
                case FILE_TYPE_FILE: //是文件
                    if (pic_merge(pbrowsepage->pagedata[col][row].topleftx, pbrowsepage->pagedata[col][row].toplefty, &file_zoom_pic, pbigpic) )
                    {
                        DBG_ERROR("pic merger error\n");
                        goto goto_err;
                    }
                    //显示文件名字
                    if (string_merge(pbrowsepage->pagedata[col][row].topleftx, pbrowsepage->pagedata[col][row].toplefty + DIR_FILE_ICON_HEIGHT + 16,\
                        (unsigned char*)pdirent->filename, pbigpic, 16) )
                    {
                        DBG_ERROR("string_merge error\n");
                        goto goto_err;
                    }
                    break;
                default:
                    break;
            }
            pdirent++;
        }
    }
    retval = 0;
    
goto_done:
goto_err:
    free_memory(dir_zoom_pic.pixeldata);
    free_memory(file_zoom_pic.pixeldata);
    return retval;
}
/*****************************************************************************
* Function     : get_browse_page_data
* Description  : 获取browse界面的位图数据
* Input        : struct disp_operation * pdisp  
* Output       ：
* Return       : NULL --- 失败   否则返回描绘好的一页数据
* Note(s)      : 返回成功后，此函数内部申请了一个显示设备大小的内存。外部用完后需要释放
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static struct pic_data *get_browse_page_data(const struct disp_operation * const pdisp)
{
    char path[128];
    unsigned int i;
    struct pic_data zoom_pic, *disp_data = NULL;
    struct disp_layout big_layout;
    const struct disp_layout* playout;
    struct page_context_desc *ppage_context = &next_page_context;
    
    if ( (pdisp == NULL) )
    {
        return NULL;
    }

    if (need_generate_background)
    {
        ppage_context = &cur_page_context;
        //分配另外一块内存，用来保存当前界面的背景数据，避免后面重复计算背景数据
        check_null_point(ppage_context->pback_ground);
        if ( (ppage_context->pback_ground = (unsigned char *)calloc(1, pdisp->dev_mem_size) ) == NULL)
        {
            DBG_ERROR("malloc background memory error\n");
            return NULL;
        }
        need_generate_background = 0;
        //第一次进入的是选择的目录名，不再是根目录
        strncpy(ppage_context->dir_name, dir_name_select, sizeof(ppage_context->dir_name) - 1);
        ppage_context->dir_name[sizeof(ppage_context->dir_name) - 1] = '\0';
    }  
    //获取当前目录的内容
    if (get_dir_context(ppage_context->dir_name, &ppage_context->context.pdirfiledirent, &ppage_context->context.totalnum) )
    {
        free_memory(ppage_context->pback_ground);
        return NULL;
    }
    DBG_INFO("get %d file\n", ppage_context->context.totalnum);
    big_layout.topleftx = 0;
    big_layout.toplefty = 0;
    big_layout.botrightx = pdisp->xres;
    big_layout.botrighty = pdisp->yres;
    //计算一个目录一页内容的坐标
    if (calc_file_coordinate(&big_layout, 80, 20, &ppage_context->context) )
    {
        free_memory(ppage_context->pback_ground);
        DBG_ERROR("calc_file_coordinate error\n");
        return NULL;
    }
    //构造显示设备的pic_data结构体
    check_null_point(disp_data);
    if ( (disp_data = (struct pic_data *)calloc(1, sizeof(struct pic_data) ) ) == NULL)
    {
        for (i = 0; i < ppage_context->context.pagerow; i++)
        {
            free_memory(ppage_context->context.pagedata[i]);
        }
        free_memory(ppage_context->context.pagedata);
        free_memory(ppage_context->pback_ground);
        return NULL;
    }
    disp_data->width = pdisp->xres;
    disp_data->height = pdisp->yres;
    disp_data->bpp = pdisp->bpp;
    disp_data->linebytes = disp_data->width * (disp_data->bpp >> 3);
    disp_data->totalbytes = pdisp->dev_mem_size;
    check_null_point(disp_data->pixeldata);
    if ( (disp_data->pixeldata = (unsigned char *)calloc(1, disp_data->totalbytes) ) == NULL)
    {
        free_memory(disp_data);
        for (i = 0; i < ppage_context->context.pagerow; i++)
        {
            free_memory(ppage_context->context.pagedata[i]);
        }
        free_memory(ppage_context->context.pagedata);
        free_memory(ppage_context->pback_ground);
        return NULL;
    }
    
    //清除当前界面的背景为COLOR_BACKGROUND
    if (pdisp->clean_screen)
    {
        pdisp->clean_screen(COLOR_BACKGROUND, disp_data->pixeldata, disp_data->totalbytes);
    }

    //显示背景图
    for (playout = browse_page_icon_layout; playout->iconname; playout++)
    {
        strncpy(path, ICON_PATH, sizeof(path) - 1);
        strncat(path, playout->iconname, sizeof(path) - 1);
        DBG_INFO("parse %s pic\n",  playout->iconname);     
        if (page_get_pic_data(path, playout, disp_data->bpp, &zoom_pic) ) //获取缩放后的数据
        {
            DBG_ERROR("page_get_pic_data error\n");
            continue;
        }
        //合并图像
        if (pic_merge(playout->topleftx, playout->toplefty, &zoom_pic, disp_data) == -1)
        {
            DBG_ERROR("pic merge error\n");
        }
        free_memory(zoom_pic.pixeldata);
    }
    
    //备份背景数据内容
    memcpy(cur_page_context.pback_ground, disp_data->pixeldata, pdisp->dev_mem_size);
    //合并第一页目录数据
    ppage_context->index = 0;
    if (get_dir_one_page_context(ppage_context->index, &ppage_context->context, disp_data) )
    {
        free_memory(disp_data);
        for (i = 0; i < ppage_context->context.pagerow; i++)
        {
            free_memory(ppage_context->context.pagedata[i]);
        }
        free_memory(ppage_context->context.pagedata);
        free_memory(ppage_context->pback_ground);
        DBG_ERROR("get_dir_one_page_context error\n");
        return NULL;
    }
    return disp_data;    
}

/*****************************************************************************
* Function     : deal_browse_page_key
* Description  : 处理按键值的松开/按下
* Input        : int key    
*                int press  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int deal_browse_page_key(int key, int press, struct disp_operation* pdisp)
{
    char buf[256];
    unsigned int i, n;
    int ret;
    FILE* pfather_dir_stream;
    struct pic_data* pdisp_data_tmp;
    
    switch (key)
    {
        case 'u':       //向上
            printf("\ncur_page_context.dir_name %s\n", cur_page_context.dir_name);
            printf("cur browse ops name %s\n", page_browse_ops.ppage_father->name);
            //判断是否是顶层目录
            if (!strcmp(cur_page_context.dir_name, DIR_TOP_PATH) )
            {
                //是顶层目录，返回到上一层
                //如果存在没有存在上一层目录，返回上一个界面
                if (page_browse_ops.ppage_father)
                {
                    if (show_specify_page(page_browse_ops.ppage_father, pdisp) )
                    {
                        DBG_ERROR("show dir %s error\n", page_browse_ops.ppage_father->name);
                        return -1;
                    }
                    need_generate_background = 1;
                    printf("top dir %s\n", page_browse_ops.ppage_father->name);
                    return 0;
                }
                else
                {
                    DBG_ERROR("has no father page\n");
                    return -1;
                }
            }
            else
            {
                //不是顶层目录，显示上一级目录
                ret = snprintf(buf, sizeof(buf), "dirname %s", cur_page_context.dir_name);
                if ( (ret == -1) || (ret >= sizeof(buf) ) )
                {
                    DBG_ERROR("snprintf error\n");
                    return -1;
                }
                //执行脚本命令dirname 
                if ( (pfather_dir_stream = popen(buf, "r") )== NULL)
                {
                    DBG_ERROR("popen error:%s\n", strerror(errno) );
                    return -1;
                }
                //从文件流中读取数据到buf
                memset(buf, 0, sizeof(buf));
                n = fread(buf, sizeof(char), sizeof(buf) - 1, pfather_dir_stream);
                if (!feof(pfather_dir_stream) || !n) //判断是否是文件结束，返回非0表示文件结束
                {
                    pclose(pfather_dir_stream);
                    DBG_ERROR("fread error\n");
                    return -1;
                }
                for (i = 0; i < n; i++)
                {
                    if (buf[i] == 0x0a) //LF换行符
                    {
                        buf[i] ='\0';
                    }
                }
                if (buf[strlen(buf) - 1] != '/')
                {
                    buf[strlen(buf)]= '/';
                }
                //关闭文件流
                pclose(pfather_dir_stream);
                printf("dir %s father dir is %s\n", cur_page_context.dir_name, buf);
                
                strncpy(next_page_context.dir_name, buf, sizeof(next_page_context.dir_name) - 1);
                next_page_context.dir_name[sizeof(next_page_context.dir_name) - 1] = '\0';
                if (page_browse_ops.get_page_data)
                {
                    if ( (pdisp_data_tmp = page_browse_ops.get_page_data(pdisp) ) == NULL)//显示上一层目录
                    {
                        return -1;
                    }
                    if (pdisp->show_flush(pdisp_data_tmp->pixeldata, 0) )
                    {
                        return -1;
                    }
                    //清除上一个目录的目录、坐标资源
                    __browse_page_free_source(2);
                    //把下一个要显示的路径拷贝给当前进行同步，页面未释放的资源将通过 cur_page_context来释放
                    //拷贝目录路径
                    strncpy(cur_page_context.dir_name, next_page_context.dir_name, sizeof(cur_page_context.dir_name) - 1);
                    cur_page_context.dir_name[sizeof(cur_page_context.dir_name) - 1] = '\0';
                    cur_page_context.index = next_page_context.index;
                    cur_page_context.context = next_page_context.context;
                    //清除下一次的数据
                    memset(&next_page_context, 0, sizeof(next_page_context) );
                    pcur_disp_data = pdisp_data_tmp;
                    return 0;
                }
            }
            return -1;
            
        case 's':       //选择
            strncpy(dir_name_select, cur_page_context.dir_name, sizeof(dir_name_select) - 1);
            dir_name_select[sizeof(dir_name_select) - 1] = '\0';
            printf("select dir name %s\n", dir_name_select);
            break;
            
        case 'p':       //上一页
            if (cur_page_context.index--)
            {
                memcpy(pcur_disp_data->pixeldata, cur_page_context.pback_ground, pdisp->dev_mem_size);
                if (get_dir_one_page_context(cur_page_context.index, &cur_page_context.context, pcur_disp_data) )
                {
                    cur_page_context.index++;
                    DBG_ERROR("get_dir_one_page_context error\n");
                    return -1;
                }
                pdisp->show_flush(pcur_disp_data->pixeldata, 0);
            }
            break;
            
        case 'n':       //下一页
            //刷新背景图片
            memcpy(pcur_disp_data->pixeldata, cur_page_context.pback_ground, pdisp->dev_mem_size);
            if (get_dir_one_page_context(++cur_page_context.index, &cur_page_context.context, pcur_disp_data) )
            {
                cur_page_context.index--;
                DBG_ERROR("get_dir_one_page_context error\n");
                return -1;
            }
            //刷新到显示设备
            pdisp->show_flush(pcur_disp_data->pixeldata, 0);
            break;

        case 'd':       //选择了目录
            if (page_browse_ops.get_page_data)
            {
                if ( (pdisp_data_tmp = page_browse_ops.get_page_data(pdisp) ) == NULL)//下一级目录
                {
                    DBG_ERROR("get dir %s error\n", next_page_context.dir_name);
                    return -1;
                }
                if (pdisp->show_flush(pdisp_data_tmp->pixeldata, 0) )
                {
                    DBG_ERROR("flush dir %s error\n", next_page_context.dir_name);
                    return -1;
                }
                //清除上一个目录的目录、坐标资源
                __browse_page_free_source(2);

                //把下一个要显示的路径拷贝给当前进行同步，页面未释放的资源将通过 cur_page_context来释放
                //拷贝目录路径
                strncpy(cur_page_context.dir_name, next_page_context.dir_name, sizeof(cur_page_context.dir_name) - 1);
                cur_page_context.dir_name[sizeof(cur_page_context.dir_name) - 1] = '\0';
                cur_page_context.index = next_page_context.index;
                cur_page_context.context = next_page_context.context;
                //清除下一次的数据
                memset(&next_page_context, 0, sizeof(next_page_context) );
                
                pcur_disp_data = pdisp_data_tmp;
                printf("cur_browse_dir %s\n", next_page_context.dir_name);
                return 0;
            }
            return -1;
            
        case 'f':       //选择了文件
            break;
            
            
        default:
            return -1;
    }
    return 0;
}

/*****************************************************************************
* Function     : deal_browse_page_event
* Description  : 处理browse页面的输入事件
* Input        : struct input_event* pevent  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int deal_browse_page_event(struct input_event* pevent, struct disp_operation* pdisp)
{
    const struct disp_layout* picon;
    unsigned int start;
    unsigned int col, row;
    struct file_dirent *pdirent;
    
    if (pevent == NULL)
    {
        return -1;
    }

    picon = &browse_page_icon_layout[0];
    while (picon->iconname)
    {
        //判断触摸坐标是否在该icon显示区域内
        if ( (pevent->val.abs.x >= picon->topleftx) && (pevent->val.abs.x <= picon->botrightx) \
            && (pevent->val.abs.y >= picon->toplefty) && (pevent->val.abs.y <= picon->botrighty) )
        {
            return deal_browse_page_key(picon->keyvalue, pevent->val.abs.press, pdisp); //处理该按键
        }
        picon++;
    }
    //进入此处说明光标不在4个icons上，判断是否在文件上

    //计算开始该页的开始地址
    start = cur_page_context.index * cur_page_context.context.numperpage;
    if (start > cur_page_context.context.totalnum)
    {
        return -1;
    }
    //偏移到当前页的起始地址
    for (col = 0; col < cur_page_context.context.pagecol; col++)
    {
        for (row = 0; row < cur_page_context.context.pagerow; row++)
        {
            if (col * cur_page_context.context.pagerow + row >= cur_page_context.context.totalnum)
            {
                return -1;
            }
            //判断是否在目录内容icon上，不含显示名字区域
            if ( (pevent->val.abs.x >= cur_page_context.context.pagedata[col][row].topleftx) && (pevent->val.abs.x <= cur_page_context.context.pagedata[col][row].botrightx) \
                && (pevent->val.abs.y >= cur_page_context.context.pagedata[col][row].toplefty) && (pevent->val.abs.y <= cur_page_context.context.pagedata[col][row].botrighty) )
            {
                pdirent = cur_page_context.context.pdirfiledirent + start + col * cur_page_context.context.pagerow + row;
                switch (pdirent->filetype)
                {
                    case FILE_TYPE_DIR:
                        printf("select dir %s\n", pdirent->filename);
                        //保存要显示的目录名称
                        //读取当前显示的路径
                        strncpy(next_page_context.dir_name, cur_page_context.dir_name, sizeof(next_page_context.dir_name) - 1);
                        //拼接选择的目录名
                        strncat(next_page_context.dir_name, pdirent->filename, sizeof(next_page_context.dir_name) - strlen(next_page_context.dir_name) - 2);
                        //拼接/
                        strncat(next_page_context.dir_name, "/", sizeof(next_page_context.dir_name) - strlen(next_page_context.dir_name) - 1);
                        return deal_browse_page_key(browse_page_select_layout[0].keyvalue, pevent->val.abs.press, pdisp); //处理该按键
                    
                    case FILE_TYPE_FILE:
                        printf("select file %s\n", pdirent->filename);
                        //打开该文件
//                        return deal_browse_page_key(browse_page_select_layout[1].keyvalue, pevent->val.abs.press, pdisp); //处理该按键
                        return 0;
                }
            }
        }
    }
        
    return -1;
}

/*****************************************************************************
* Function     : browse_page_sync_source
* Description  : 同步界面
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int browse_page_sync_source(struct pic_data *ppic_data)
{
    if (ppic_data == NULL)
    {
        return -1;
    }
//    cur_page_context = next_page_context;
    pcur_disp_data = ppic_data;
    printf("page browse sync source\n");
    return 0;
}

/*****************************************************************************
* Function     : __browse_page_free_source
* Description  : 释放当前接资源，仅内部使用，
* Input        : int free_background  ： 0 --- 不释放背景内存
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月25日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void __browse_page_free_source(int free_background)
{
    int i;
    
    if (pcur_disp_data)
    {
        //释放图片内容
        if (pcur_disp_data->pixeldata)
        {
            free_memory(pcur_disp_data->pixeldata);
        }
        //释放本身
        free_memory(pcur_disp_data);
    }
    if (free_background == 1)
    {
        //释放界面背景内存
        if (cur_page_context.pback_ground)
        {
            free_memory(cur_page_context.pback_ground);
        }
        //释放一页坐标的数据,是一个二维数组
        for (i = 0; i < cur_page_context.context.pagecol; i++)
        {
            free_memory(cur_page_context.context.pagedata[i]);
        }
        free_memory(cur_page_context.context.pagedata);

        //释放文件名数据
        for (i = 0; i < cur_page_context.context.totalnum; i++)
        {
            free_memory(cur_page_context.context.pdirfiledirent);
        }
    }
    else if (free_background == 2)
    {
        //释放一页坐标的数据,是一个二维数组
        for (i = 0; i < cur_page_context.context.pagecol; i++)
        {
            free_memory(cur_page_context.context.pagedata[i]);
        }
        free_memory(cur_page_context.context.pagedata);

        //释放文件名数据
        for (i = 0; i < cur_page_context.context.totalnum; i++)
        {
            free_memory(cur_page_context.context.pdirfiledirent);
        }
    }
}


/*****************************************************************************
* Function     : browse_page_free_source
* Description  : 释放当前界面的资源，切换到别的页面时调用
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月25日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void browse_page_free_source(void)
{
    __browse_page_free_source(1);
}

/*****************************************************************************
* Function     : page_browse_init
* Description  : 浏览模式界面初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int page_browse_init(void)
{
    struct page_operations* ppage_ops;
    
    if (register_page_operation(&page_browse_ops) == -1)
    {
        DBG_ERROR("register page browse error\n");
        return -1;
    }

    if ( (ppage_ops = get_page_operations_by_name("main") ) == NULL)
    {
        DBG_ERROR("register father page main kid browse error\n");
        return -1;
    }
    return register_kid_page(ppage_ops, &page_browse_ops);
}

