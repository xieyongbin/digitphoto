#ifndef __PAGE_MANAGER_H__
#define __PAGE_MANAGER_H__

#include <list.h>
#include "config.h"
#include "disp_manager.h"
#include "input_manager.h"
#include "pic_manager.h"

#define ICON_PATH "/mnt/app/digit/digitphoto/project/icons/"

struct page_list                        //字体链表节点
{
    struct list_head list;
    void *pcontext;                     //指向struct page_operations 
};

struct page_operations
{
    const char *name;
    struct page_operations *ppage_father; //父页
    struct list_head page_kid;            //子页
    struct pic_data* ppage_data;          //当前页面数据指针
    struct pic_data* (*get_page_data)(const struct disp_operation * const pdisp); //获取当前页面要显示的点阵数据
    int (*deal_event)(struct input_event* pevent, struct disp_operation* pdisp);  //处理事件
    int (*sync_source)(struct pic_data *ppic_data);            //同步资源
    void (*prepare)(void);
    struct list_head list;
};

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
struct page_operations* get_page_operations_by_name(const char *name);

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
int show_specify_page_by_name(const char *name, struct disp_operation* pdisp);

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
struct page_operations* get_kid_page(const char *kid_name, struct page_operations *pfather);

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
int show_father_page(struct page_operations *pops, struct disp_operation *pdisp);

/*****************************************************************************
* Function     : show_specify_page
* Description  : 显示指定页面
* Input        : struct page_operations *pops  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_specify_page(struct page_operations *pops, struct disp_operation* pdisp);

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
struct page_operations *get_cur_page_operations(void);

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
int deal_input_event(struct input_event *pevent, struct disp_operation* pdisp);

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
int register_page_operation(struct page_operations *pops);

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
int register_kid_page(struct page_operations* pfather, struct page_operations* pkid);

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
int unregister_kid_page(struct page_operations* pfather, struct page_operations* pkid);

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
int page_init(void);

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
int page_get_pic_data(char* path, const struct disp_layout* playout, unsigned int bpp, struct pic_data *pzoom_data);

#endif //__PAGE_MANAGER_H__


