#ifndef __DRAW_H__
#define __DRAW_H__

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
int opentextfile(const char *filename);

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
int set_font_details(const char *hzkfile, const char *freetypefile, const unsigned int fontsize);

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
int open_display_by_name(const char* name);

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
struct disp_operation* get_display_operations(void);

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
int show_next_page(void);

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
int show_prev_page(void);

#endif //__DRAW_H__

