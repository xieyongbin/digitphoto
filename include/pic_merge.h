#ifndef __PIC_MERGE_H__
#define __PIC_MERGE_H__

/*****************************************************************************
* Function     : pic_merge
* Description  : 图片合并，将一张小图合并到大图的指定位置
* Input        : unsigned int x              :大图的起始x坐标
*                unsigned int y              :大图的起始y坐标
*                struct pic_data* psmallpic  
*                struct pic_data* pbigpic    
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int pic_merge(unsigned int x, unsigned int y, struct pic_data* psmallpic, struct pic_data* pbigpic);

int string_merge(unsigned int x, unsigned int y, unsigned char* str, struct pic_data* pbigpic, const unsigned int font_size);

#endif //__PIC_MERGE_H__
