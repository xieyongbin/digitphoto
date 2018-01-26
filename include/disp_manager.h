#ifndef __DISP_MANAGER_H__
#define __DISP_MANAGER_H__

#include <list.h>
#include "config.h"

struct disp_operation 
{   
    char* name;
    unsigned int xres;
    unsigned int yres;
    unsigned int bpp;
    unsigned int dev_mem_size;
    int (*open)(void);
    int (*close)(void);
    int (*show_pixel)(int x, int y, unsigned int color);
    int (*clean_screen)(unsigned int back_color, unsigned char *pback_buf, unsigned int back_size);
    int (*show_flush)(unsigned char *wrbuf, unsigned int wrnum);
    struct list_head list;
};

struct disp_layout 
{
	int topleftx;
	int toplefty;
	int botrightx;
	int botrighty;
    int keyvalue;
	char *iconname;
};

/*****************************************************************************
* Function     : register_disp_operation
* Description  : 注册一个显示操作集
* Input        : struct disp_operation * pops  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int register_disp_operation(struct disp_operation * pops);

/*****************************************************************************
* Function     : get_display_operation_by_name
* Description  : 通过名字获取显示操作函数
* Input        : const char* name  
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct disp_operation *get_display_operation_by_name(const char* name);

/*****************************************************************************
* Function     : show_support_display
* Description  : 显示所有支持的display
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_support_display(void);

/*****************************************************************************
* Function     : disp_close_all_device
* Description  : 关闭所有显示设备
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月12日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int disp_close_all_device(void);

/*****************************************************************************
* Function     : disp_init
* Description  : disp初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int disp_init(void);

/*****************************************************************************
* Function     : disp_exit
* Description  : 释放所有显示设备的资源
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
void disp_exit(void);

#endif //__DISP_MANAGER_H__

