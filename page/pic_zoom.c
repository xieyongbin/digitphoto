#include <stdlib.h>
#include "string.h"
#include "pic_manager.h"

/*****************************************************************************
* Function     : pic_zoom
* Description  : 图片缩放
* Input        : struct pic_data *pdest  
*                struct pic_data *psrc   
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int pic_zoom(struct pic_data *pdest, struct pic_data *psrc)
{
    unsigned int x, y, srcy;
    unsigned char *dest_one_line_begin = NULL, *src_one_line_begin = NULL;
    unsigned int *ptable_src = NULL;
    unsigned int pixelbytes;

    if ( (pdest == NULL) || (psrc == NULL) )
        return -1;
    
    if (!pdest->width || !pdest->height || !psrc->width || !psrc->height) 
        return -1;

    if (pdest->bpp != psrc->bpp)
        return -1;

    //申请一块内存，提前算好x，加快缩放速度
    check_null_point(ptable_src);
    if ( (ptable_src = (unsigned int *)malloc(sizeof(unsigned int) * pdest->width) ) == NULL)
        return -1;
    
    for (x = 0; x < pdest->width; x++)
        ptable_src[x] = x * psrc->width / pdest->width;

    //计算每个像素的字节数
    pixelbytes = psrc->bpp >> 3;
    
    for (y = 0; y < pdest->height; y++)
    {
        srcy = y * psrc->height / pdest->height;
        //目的一行的开始地址
        dest_one_line_begin = pdest->pixeldata + pdest->linebytes * y;
        //源一行的开始地址
        src_one_line_begin = psrc->pixeldata + psrc->linebytes * srcy;
        
        for (x = 0; x < pdest->width; x++)
        {
            memcpy(dest_one_line_begin + x * pixelbytes, src_one_line_begin + ptable_src[x] * pixelbytes, pixelbytes);
        }
    }
    free_memory(ptable_src);
    return 0;
}

int PicZoom(struct pic_data* ptOriginPic, struct pic_data* ptZoomPic)
{
    unsigned long dwDstWidth = ptZoomPic->width;
    unsigned long* pdwSrcXTable = NULL;
	unsigned long x;
	unsigned long y;
	unsigned long dwSrcY;
	unsigned char *pucDest;
	unsigned char *pucSrc;
	unsigned long dwPixelBytes = ptOriginPic->bpp/8;

	if (ptOriginPic->bpp != ptZoomPic->bpp)
	{
		return -1;
	}

    check_null_point(pdwSrcXTable);
    pdwSrcXTable = malloc(sizeof(unsigned long) * dwDstWidth);
    if (NULL == pdwSrcXTable)
    {
        return -1;
    }

    for (x = 0; x < dwDstWidth; x++)//���ɱ� pdwSrcXTable
    {
        pdwSrcXTable[x]=(x*ptOriginPic->width/ptZoomPic->width);
    }

    for (y = 0; y < ptZoomPic->height; y++)
    {			
        dwSrcY = (y * ptOriginPic->height / ptZoomPic->height);

		pucDest = ptZoomPic->pixeldata + y*ptZoomPic->linebytes;
		pucSrc  = ptOriginPic->pixeldata + dwSrcY*ptOriginPic->linebytes;
		
        for (x = 0; x <dwDstWidth; x++)
        {
            /* ԭͼ����: pdwSrcXTable[x]��srcy
             * ��������: x, y
			 */
			 memcpy(pucDest+x*dwPixelBytes, pucSrc+pdwSrcXTable[x]*dwPixelBytes, dwPixelBytes);
        }
    }

    free_memory(pdwSrcXTable);
	return 0;
}

