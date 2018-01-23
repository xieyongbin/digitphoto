#ifndef __PIC_BMP_H__
#define __PIC_BMP_H__

//bmp文件头
struct bitmapfileheader 
{  
    unsigned short bftype;    
    unsigned int   bfsize; 
    unsigned short bfreserved1; 
    unsigned short bfreserved2; 
    unsigned int   bfoffbits;
}__attribute__((packed));

//bmp位图信息头
struct bitmapinfoheader
{
    unsigned int   bisize; 
    unsigned long  biwidth; 
    unsigned long  biheight; 
    unsigned short biplanes; 
    unsigned short bibitcount; 
    unsigned int   bicompression; 
    unsigned int   bisizeimage; 
    unsigned long  bixpelspermeter; 
    unsigned long  biypelspermeter; 
    unsigned int   biclrused; 
    unsigned int   biclrimportant;
}__attribute__((packed));


/*****************************************************************************
* Function     : pic_bmp_init
* Description  : 注册bmp图片解码
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int pic_bmp_init(void);

#endif //__PIC_BMP_H__

