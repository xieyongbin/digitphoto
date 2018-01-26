#include <stdlib.h>
#include <string.h>
#include <list.h>
#include "page_manager.h"
#include "pic_merge.h"
#include "pic_zoom.h"
#include "page_main.h"
#include "page_setting.h"
#include "page_interval.h"
#include "page_browse.h"
#include "debug.h"


static struct list_head page_list_head = LIST_HEAD_INIT(page_list_head);
static pthread_rwlock_t list_head_rwlock = PTHREAD_RWLOCK_INITIALIZER;

static struct page_operations *pcur_page = NULL;  //当前页面

/*****************************************************************************
* Function     : get_page_operations_by_name
* Description  : 根据页面名字获取对应的操作集
* Input        : const char *name  
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct page_operations* get_page_operations_by_name(const char *name)
{
    struct list_head *plist;
    struct page_operations *ptemp;
    
    if (name == NULL)
        return NULL;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &page_list_head)
    {
        ptemp = list_entry(plist, struct page_operations, list);
        if (ptemp && ptemp->name)
        {
            if (!strcmp(ptemp->name, name))
            {
                pthread_rwlock_unlock(&list_head_rwlock);
                return ptemp;
            }
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return NULL;
}

/*****************************************************************************
* Function     : get_kid_page
* Description  : 查找指定界面的指定名字的子界面
* Input        : const char *kid_name          
*                struct page_operations *pfather  
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct page_operations* get_kid_page(const char *kid_name, struct page_operations *pfather)
{
    struct list_head *plist;
    struct page_list *ptemp;
    struct page_operations *ppage;

    
    if ( (kid_name == NULL) || (pfather == NULL) )
        return NULL;

    //判断是否有子页面
    if (list_empty(&pfather->page_kid) )
        return NULL;
    
    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &pfather->page_kid)
    {
        ptemp = list_entry(plist, struct page_list, list);
        if (ptemp->pcontext)
        {
            ppage = (struct page_operations *)ptemp->pcontext;
            if (!strcmp(ppage->name, kid_name) ) //找到了子界面
            {
                pthread_rwlock_unlock(&list_head_rwlock);
                return ppage; //返回找到的子界面结构体地址
            }
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return NULL;
}

/*****************************************************************************
* Function     : show_father_page
* Description  : 显示父菜单
* Input        : struct page_operations *pops  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_father_page(struct page_operations *pops, struct disp_operation *pdisp)
{
    if ( (pops == NULL) || (pdisp == NULL) )
        return -1;

    if (pops->ppage_father && pops->ppage_father->get_page_data)
    {
        //获取父菜单界面数据，返回获取成功的字节数
        return show_specify_page(pops->ppage_father, pdisp);
    }
    return 0;
}

/*****************************************************************************
* Function     : show_specify_page
* Description  : 显示指定页面
* Input        : struct page_operations *pops  
* Output       ：
* Return       : 
* Note(s)      : 此函数将自动释放先前页的数据
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_specify_page(struct page_operations *pops, struct disp_operation* pdisp)
{
    struct pic_data *page_data;
    
    if ( (pops == NULL) || (pops->get_page_data == NULL) \
        || (pdisp == NULL) || (pdisp->show_pixel == NULL) || (pdisp->show_flush == NULL) )
    {
        return -1;
    }

    //获得指定页得图像数据
    if ( (page_data = pops->get_page_data(pdisp) ) == NULL)
    {
        return -1;
    }
    
    //刷新到显示设备
    pdisp->show_flush(page_data->pixeldata, 0);

    //同步资源
    if (pops->sync_source)
    {
        pops->sync_source(page_data);
    }
    if (pcur_page) //释放当前页面的资源
    {
        printf("page %s free source\n", pcur_page->name);
        if (pcur_page->free_source)
        {
            pcur_page->free_source();
        }
    }
    else
    {
        if (pcur_page) //当前界面不为NULL
        {
            if (pcur_page->ppage_data) //没有释放当前页面的数据
            {
                if (pcur_page->ppage_data->pixeldata)
                {
                    //释放该页内存
                    free_memory(pcur_page->ppage_data->pixeldata);
                }
                //释放该页内存
                free_memory(pcur_page->ppage_data);
            }
        }
    }
    
    //同步当前界面指针
    pcur_page = pops;
    pcur_page->ppage_data = page_data; 
    return 0;
}

/*****************************************************************************
* Function     : get_cur_page_operations
* Description  : 获取当前界面的操作集
* Input        : void  
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月4日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct page_operations *get_cur_page_operations(void)
{
    return pcur_page;
}

/*****************************************************************************
* Function     : show_specify_page_by_name
* Description  : 指定名字显示对应的界面
* Input        : const char *name              
*                struct disp_operation* pdisp  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月3日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_specify_page_by_name(const char *name, struct disp_operation* pdisp)
{
    struct page_operations * pops;
    
    if ( (name == NULL) || (pdisp == NULL) || (pdisp->show_pixel == NULL) || (pdisp->show_flush == NULL) )
    {
        return -1;
    }
    
    if ( (pops = get_page_operations_by_name(name) ) == NULL)
    {
        return -1;
    }
    return show_specify_page(pops, pdisp);
}

/*****************************************************************************
* Function     : deal_input_event
* Description  : 处理输入事件
* Input        : struct input_event *pevent  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int deal_input_event(struct input_event *pevent, struct disp_operation* pdisp)
{
    if (pevent == NULL)
        return -1;
    
    //目前只处理触摸屏事件
    if (pevent->type == INPUT_TYPE_TOUCHSCREEN)
    {
        if (pcur_page && pcur_page->deal_event)
            return pcur_page->deal_event(pevent, pdisp); //交给下一层去完成
    }
    else
    {
        if (pevent->val.value == 'q')
        {
            //释放当前界面的内存
            if (pcur_page)
            {
                if (pcur_page->free_source)
                {
                    pcur_page->free_source();
                }
            }
            //释放字体内存
            font_exit();
            //释放所有字体解码所支持的字体空间
            encode_exit();
            //释放显示设备的资源
            disp_exit();
            //释放page的资源
            page_exit();
            pthread_exit(0);
        }
    }
    return -1;
}

/*****************************************************************************
* Function     : register_page_operation
* Description  : 注册一个界面
* Input        : struct page_operations *pops  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int register_page_operation(struct page_operations *pops)
{
    if (pops == NULL)
    {
        return -1;
    }
    pthread_rwlock_wrlock(&list_head_rwlock);  //获取写锁
    list_add_tail(&pops->list, &page_list_head);
    pthread_rwlock_unlock(&list_head_rwlock);  //释放写锁
    return 0;
}

/*****************************************************************************
* Function     : register_kid_page
* Description  : 往父页上注册一个子页
* Input        : struct page_operations* pfather  
*                struct page_operations* pkid     
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月5日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int register_kid_page(struct page_operations* pfather, struct page_operations* pkid)
{
    struct page_list* ppage_list = NULL;
    
    if ( (pfather == NULL) || (pkid == NULL) )
    {
        return -1;
    }

    check_null_point(ppage_list);
    if ( (ppage_list = (struct page_list*)malloc(sizeof(struct page_list) ) ) == NULL)
    {
        return -1;
    }
    ppage_list->pcontext = pkid; 
    pthread_rwlock_wrlock(&list_head_rwlock);
    list_add_tail(&ppage_list->list, &pfather->page_kid);
    pkid->ppage_father = pfather;       //指定父菜单
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : unregister_kid_page
* Description  : 把一个子页从父页中删除
* Input        : struct page_operations* pfather  
*                struct page_operations* pkid     
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月5日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int unregister_kid_page(struct page_operations* pfather, struct page_operations* pkid)
{
    struct list_head *plist, *pnext;
    struct page_list *ppage_list;
    struct page_operations *ppage_temp;
    
    if ( (pfather == NULL) || (pkid == NULL) )
    {
        return -1;
    }
    pthread_rwlock_wrlock(&list_head_rwlock);
    list_for_each_safe(plist, pnext, &pfather->page_kid)
    {
        ppage_list = list_entry(plist, struct page_list, list);
        if (ppage_list && ppage_list->pcontext)
        {
            ppage_temp = (struct page_operations *)(ppage_list->pcontext);
            if (!strcmp(pkid->name, ppage_temp->name) )
            {
                list_del(&ppage_list->list);
                free_memory(ppage_list);
                pkid->ppage_father = NULL;
                break;
            }
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : page_init
* Description  : 页面初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月30日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int page_init(void)
{
    int error = 0;
    
    error |= page_main_init();     //main页
    error |= page_setting_init();  //setting页
    error |= page_interval_init(); //interval页

    error |= page_browse_init();   //browse页
    
    return error;
}

/*****************************************************************************
* Function     : page_exit
* Description  : 注销所有的page，并释放子page的内存
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int page_exit(void)
{
    struct list_head *plist, *pnext;
    struct list_head *pkid_list, *pkid_next;
    struct page_operations *ppage_ops;
    struct page_list *ppage;

    pthread_rwlock_wrlock(&list_head_rwlock);
    //遍历整个page链表
    list_for_each_safe(plist, pnext, &page_list_head)
    {
        ppage_ops = list_entry(plist, struct page_operations, list);
        //是否有子页
        if (!list_empty(&ppage_ops->page_kid) )
        {
            //有子页
            //遍历该page的子page
            list_for_each_safe(pkid_list, pkid_next, &ppage_ops->page_kid)
            {
                //子page是以struct page_list的list作为节点的
                ppage = list_entry(pkid_list, struct page_list, list);
                //删除子page节点
                list_del(pkid_list);
                //释放 strcut page_list结构体
                free_memory(ppage);
            }
        }
        //删除节点
        list_del(plist);
    }
    pthread_rwlock_unlock(&list_head_rwlock);
}

/*****************************************************************************
* Function     : page_get_pic_data
* Description  : 把一张图片按照指定比例合并到一张大图上
* Input        : char* path                         
*                const struct disp_layout* playout  
*                struct pic_data *pbig_pic          
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int page_get_pic_data(char* path, const struct disp_layout* playout, unsigned int bpp, struct pic_data *pzoom_data)
{
    struct file_desc pic_desc;
    struct pic_operations* ppic;
    struct pic_data pic, zoom_pic;

    if ( (path == NULL) || (playout == NULL) || (pzoom_data == NULL) )
    {
        return -1;
    }
    if (open_one_pic(path, &pic_desc) == -1)//打开图片浏览模式
    {
        DBG_ERROR("get % error\n");
        return -1;
    }
    
    //为该图片选择合适的解码器
    if ( (ppic = select_encode_for_pic(&pic_desc) ) == NULL)
    {
        close_one_pic(&pic_desc);
        DBG_ERROR("no encode can parse %s\n", path);
        return -1;
    }
    
    //获取图片数据
    pic.bpp = bpp;  //设置要转换后的图片bpp,LCD是16位
    if (ppic->get_pic_data(&pic_desc, &pic) == -1)
    {
        close_one_pic(&pic_desc);
        DBG_ERROR("can not get % data\n", path);
        return -1;
    }
    //关闭打开的图片文件
    close_one_pic(&pic_desc);
    
    //构造缩小后的图片数据
    zoom_pic.height = playout->botrighty - playout->toplefty + 1;
    zoom_pic.width = playout->botrightx - playout->topleftx + 1;
    zoom_pic.bpp = bpp;
    zoom_pic.linebytes = zoom_pic.width * (zoom_pic.bpp >> 3);
    zoom_pic.totalbytes = zoom_pic.linebytes * zoom_pic.height;
    zoom_pic.pixeldata = NULL;
    check_null_point(zoom_pic.pixeldata);
    if ( (zoom_pic.pixeldata = (unsigned char *)calloc(1, zoom_pic.totalbytes) ) == NULL)
    {
        //释放图片原数据
        free_memory(pic.pixeldata);
        DBG_ERROR("malloc %s %d memory error\n", path, zoom_pic.totalbytes);
        return -1;
    }
    
    //进行图片缩放
    if (pic_zoom(&zoom_pic, &pic) == -1)
    {
        free_memory(zoom_pic.pixeldata);
        DBG_ERROR("pic zoom error\n");
        return -1;
    }
    //释放图片原数据
    free_memory(pic.pixeldata);
    *pzoom_data = zoom_pic; //保存缩放后的数据
    return 0;
}

