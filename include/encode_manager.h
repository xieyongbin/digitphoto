#ifndef __ENCODE_MANAGE_H__
#define __ENCODE_MANAGE_H__

#include <list.h>
#include "config.h"
#include "font_manager.h"
#include "pic_manager.h"

struct encode_operation
{
    char* name;                                                                             //名字
    unsigned char headlen;                                                                  //编码支持的文件头长度
    const unsigned char *headbuf;                                                           //编码支持的头
    int (*issupport)(unsigned char *pfilecontext, unsigned int filelen);                    //是否支持该编码来解码此文件
    int (*getcodefrmbuf)(unsigned char *pstart, unsigned char *pend, unsigned int *pcode);  //对文件已经进行解码
    struct list_head supportfontlist;                                                       //支持以哪种font显示
    struct list_head list;                                                                  //双链表
};

/*****************************************************************************
* Function     : register_encode_operation
* Description  : 注册编码方式
* Input        : struct encode_operation* pops  
* Output       ：
* Return       : 0：注册成功       -1:注册失败* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int register_encode_operation(struct encode_operation* pops);

/*****************************************************************************
* Function     : add_font_to_encode
* Description  : 添加一种字体pfont到pencode编码中
* Input        : struct font_operation* pfont     ：注册到pencode的字体
                 struct encode_operation* pencode ：解码方式
* Output       ：
* Return       : -1 --- 添加失败  0---添加成功
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int add_font_to_encode(struct font_operation* pfont, struct encode_operation* pencode);

/*****************************************************************************
* Function     : del_font_from_encode
* Description  : 从一个文字解码器中删除一个字体
* Input        : struct font_operation* pfont      
*                struct encode_operation* pencode  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月5日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int del_font_from_encode(struct font_operation* pfont, struct encode_operation* pencode, unsigned char needlock);

/*****************************************************************************
* Function     : del_font_from_all_encode
* Description  : 删除所有解码里的支持的该字体
* Input        : struct font_operation* pfont  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月5日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int del_font_from_all_encode(struct font_operation* pfont);

/*****************************************************************************
* Function     : del_all_font_from_encode
* Description  : 删除解码字体
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月12日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int del_all_font_from_encode(void);

/*****************************************************************************
* Function     : selectencodeforfile
* Description  : 给指定文件选择一个编码
* Input        : unsigned char *filebuf  
*                int buflen              
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct encode_operation* selectencodeforfile(unsigned char *filebuf, int buflen);

/*****************************************************************************
* Function     : show_support_encode
* Description  : 显示支持的编码方式
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_support_encode(void);

/*****************************************************************************
* Function     : show_encode_support_font
* Description  : 显示编译支持的字体
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_encode_support_font(void);

/*****************************************************************************
* Function     : encode_init
* Description  : 编码初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int encode_init(void);

/*****************************************************************************
* Function     : encode_exit
* Description  : 释放字体解码占用的资源，通过 add_font_to_encode() 添加一个支持的字体
                 会分配一个sizeof(struct font_list)空间，在此处进行释放
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
void encode_exit(void);

void ClearRectangleInVideoMem(unsigned int iTopLeftX, unsigned int iTopLeftY, unsigned int iBotRightX, unsigned int iBotRightY, struct pic_data* ppic_data, unsigned int dwColor);


#endif // __ENCODE_MANAGE_H__
