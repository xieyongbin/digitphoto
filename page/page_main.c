#include <stdlib.h>
#include <string.h>
#include "page_manager.h"
#include "pic_zoom.h"
#include "pic_merge.h"
#include "debug.h"
#include "config.h"

static struct pic_data * get_main_page_data(const struct disp_operation * const pdisp);
static int deal_main_page_event(struct input_event* pevent, struct disp_operation* pdisp);

//主界面显示的坐标
static const struct disp_layout main_page_icon_layout[] =
{
    {180, 23,  300, 83,  'b', "browse_mode.bmp"},   //浏览模式
        
	{180, 106, 300, 166, 'c', "continue_mod.bmp"},  //连播模式
	
	{180, 189, 300, 249, 's', "setting.bmp"},       //设置
	
	{0, 0, 0, 0, '\0', NULL},
};

static struct page_operations page_main_ops =
{
    .name = "main",
    .ppage_father = NULL,
    .page_kid = LIST_HEAD_INIT(page_main_ops.page_kid),
    .ppage_data = NULL,
    .get_page_data = get_main_page_data,
    .deal_event = deal_main_page_event,
};

/*****************************************************************************
* Function     : get_main_page_data
* Description  : 获取main界面的位图数据
* Input        : struct disp_operation * pdisp  
* Output       ：
* Return       : NULL --- 失败   否则返回描绘好的一页数据
* Note(s)      : 返回成功后，此函数内部申请了一个显示设备大小的内存。外部用完后需要释放
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static struct pic_data *get_main_page_data(const struct disp_operation * const pdisp)
{
    char path[128];
    struct file_desc pic_desc;
    struct pic_operations* ppic;
    struct pic_data pic, zoom_pic, *disp_data;
    const struct disp_layout* playout = main_page_icon_layout;
    if (pdisp == NULL)
    {
        return NULL;
    }

    //构造显示设备的pic_data结构体
    if ( (disp_data = (struct pic_data *)malloc(sizeof(struct pic_data) ) ) == NULL)
    {
        return NULL;
    }
    disp_data->width = pdisp->xres;
    disp_data->height = pdisp->yres;
    disp_data->bpp = pdisp->bpp;
    disp_data->linebytes = disp_data->width * (disp_data->bpp >> 3);
    disp_data->totalbytes = pdisp->dev_mem_size;
    if ( (disp_data->pixeldata = (unsigned char *)malloc(disp_data->totalbytes) ) == NULL)
    {
        return NULL;
    }

    //清除当前界面的背景为COLOR_BACKGROUND
    if (pdisp->clean_screen)
    {
        pdisp->clean_screen(COLOR_BACKGROUND, disp_data->pixeldata, disp_data->totalbytes);
    }

    for (playout = main_page_icon_layout; playout->iconname; playout++)
    {
        strncpy(path, ICON_PATH, sizeof(path) - 1);
        strncat(path, playout->iconname, sizeof(path) - 1);
        DBG_INFO("parse %s pic\n",  playout->iconname);

        if (open_one_pic(path, &pic_desc) == -1)//打开图片浏览模式
        {
            DBG_ERROR("get % error\n");
            continue;
        }
        
        //为该图片选择合适的解码器
        if ( (ppic = select_encode_for_pic(&pic_desc) ) == NULL)
        {
            close_one_pic(&pic_desc);
            DBG_ERROR("no encode can parse %s\n", path);
            continue;
        }
        
        //获取图片数据
        pic.bpp = disp_data->bpp;  //设置要转换后的图片bpp,LCD是16位
        if (ppic->get_pic_data(&pic_desc, &pic) == -1)
        {
            close_one_pic(&pic_desc);
            DBG_ERROR("can not get % data\n", path);
            continue;
        }
 
        //构造缩小后的图片数据
        zoom_pic.height = playout->botrighty - playout->toplefty + 1;
        zoom_pic.width = playout->botrightx - playout->topleftx + 1;
        zoom_pic.bpp = disp_data->bpp;
        zoom_pic.linebytes = zoom_pic.width * (zoom_pic.bpp >> 3);
        zoom_pic.totalbytes = zoom_pic.linebytes * zoom_pic.height;
        
        if ( (zoom_pic.pixeldata = (unsigned char *)malloc(zoom_pic.totalbytes) ) == NULL)
        {
            close_one_pic(&pic_desc);
            DBG_ERROR("malloc %s %d memory error\n", path, zoom_pic.totalbytes);
            continue;
        }
        memset(zoom_pic.pixeldata, COLOR_BACKGROUND, sizeof(zoom_pic.totalbytes) );
        //进行图片缩放
        if (pic_zoom(&zoom_pic, &pic) == -1)
        {
            free_memory(zoom_pic.pixeldata);
            DBG_ERROR("pic zoom error\n");
            continue;
        }

        //合并图像
        if (pic_merge(playout->topleftx, playout->toplefty, &zoom_pic, disp_data) == -1)
        {
            free_memory(zoom_pic.pixeldata);
            DBG_ERROR("pic merge error\n");
            continue;
        }
      
        free_memory(zoom_pic.pixeldata);
           
        //关闭文件数据
        close_one_pic(&pic_desc);
    }
 
    return disp_data;
}

/*****************************************************************************
* Function     : deal_main_page_key
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
static int deal_main_page_key(int key, int press, struct disp_operation* pdisp)
{
    struct page_operations* ppage_ops;
    
    switch (key)
    {
        case 'b':       //浏览模式
            if ( (ppage_ops = get_kid_page("browse", &page_main_ops) ) == NULL)
            {
                DBG_ERROR("page main has no kid browse page\n");
                return -1;
            }
            return show_specify_page(ppage_ops, pdisp);
            
        case 'c':       //连播模式
            printf("entry auto mode\n");
            break;
        
        case 's':       //设置
            if ( (ppage_ops = get_kid_page("setting", &page_main_ops) ) == NULL)
            {
                DBG_ERROR("page main has no kid setting page\n");
                return -1;
            }
            return show_specify_page(ppage_ops, pdisp);
            
        default:
            return -1;
    }
    return 0;
}

/*****************************************************************************
* Function     : deal_main_page_event
* Description  : 处理main页面的输入事件
* Input        : struct input_event* pevent  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int deal_main_page_event(struct input_event* pevent, struct disp_operation* pdisp)
{
    const struct disp_layout* picon;
    
    if (pevent == NULL)
    {
        return -1;
    }

    picon = &main_page_icon_layout[0];
    while (picon->iconname)
    {
        //判断触摸坐标是否在该icon显示区域内
        if ( (pevent->val.abs.x >= picon->topleftx) && (pevent->val.abs.x <= picon->botrightx) \
            && (pevent->val.abs.y >= picon->toplefty) && (pevent->val.abs.y <= picon->botrighty) )
        {
            return deal_main_page_key(picon->keyvalue, pevent->val.abs.press, pdisp); //处理该按键
        }
        picon++;
    }
    return -1;
}

/*****************************************************************************
* Function     : page_main_init
* Description  : main界面初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int page_main_init(void)
{
    return register_page_operation(&page_main_ops);
}

