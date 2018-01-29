#ifndef __FONT_MANAGER_H__
#define __FONT_MANAGER_H__

#include <list.h>
#include "config.h"

//此文件说的坐标都是笛卡尔坐标，而非lcd坐标
//坐标信息
struct coordinate
{
    unsigned int x;                         //x坐标
    unsigned int y;                         //y坐标
};

//位图信息
struct bitmap_param
{
    unsigned x_max;                     //该字体的x方向最大坐标
    unsigned y_max;                     //该字体的y方向最大坐标
    unsigned int left;                  //字体真正显示的起点x坐标
    unsigned top;                       //字体真正显示的起点y坐标
    unsigned char bpp;
    int pitch;                          //
    unsigned char *pbuf;
};

struct font_bitmap
{
    struct coordinate cur_disp;              //输入:当前显示坐标
    struct coordinate next_disp;             //输出:下一个显示坐标
    struct bitmap_param bitmap_var;          //输出:位图参数
};

//字体操作函数,供下级具体字体调用
struct font_operation
{
    char* name;                                                                         //字体名字
    int (*open)(const char *fontfile, const unsigned int fontsize);                     //打开字体
    void (*close)(void);                                                                //关闭字体
    int (*set_font_size)(const unsigned int fontsize);                                  //设置字体大小
    int (*get_font_size)(void);                                                         //获取字体大小
    int (*get_font_bitmap)(const unsigned int code, struct font_bitmap *pfont_bitmap);  //获取字体位图
};

struct font_list                        //字体链表节点
{
    struct list_head list;
    void *pcontext;                     //指向struct font_operation 
};

/*****************************************************************************
* Function     : register_font_operation
* Description  : 注册字体操作函数
* Input        : struct font_operation* pops  
* Output       ：
* Return       : 
* Note(s)      : 0--注册成功       -1--注册失败
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int register_font_operation(struct font_operation* pops);

/*****************************************************************************
* Function     : unregister_font_operation
* Description  : 注销一个字体
* Input        : struct font_operation* pops  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月12日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int unregister_font_operation(struct font_operation* pops);

/*****************************************************************************
* Function     : unregister_all_font_operation
* Description  : 注销所有的字体
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月12日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int unregister_all_font_operation(void);

/*****************************************************************************
* Function     : open_all_font
* Description  : 打开所有的字体，有一个字体打开成功就算成功
* Input        : const char *hzkfile          
*                const char *freetypefile     
*                const unsigned int fontsize  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月2日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int open_all_font(const char *hzkfile, const char *freetypefile, const unsigned int fontsize);

/*****************************************************************************
* Function     : get_font_operation_by_name
* Description  : 通过名字获取字体操作集
* Input        : const char *name  
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct font_operation* get_font_operation_by_name(const char *name);

/*****************************************************************************
* Function     : show_support_font
* Description  : 显示支持的字体
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_support_font(void);

/*****************************************************************************
* Function     : font_init
* Description  : 字体初始化
* Input        : void  
* Output       ：
* Return       : 0--注册成功       -1--注册失败
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int font_init(void);

/*****************************************************************************
* Function     : font_exit
* Description  : 释放字体的资源，调用 register_font_operation() 会分配一个 sizeof(struct font_list)空间
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
void font_exit(void);


#endif //__FONT_MANAGER_H__

