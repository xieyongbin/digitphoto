#include <string.h>
#include "config.h"
#include "pic_manager.h"
#include "encode_manager.h"
#include "font_manager.h"
#include "debug.h"

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
int pic_merge(unsigned int x, unsigned int y, struct pic_data* psmallpic, struct pic_data* pbigpic)
{
    int i;
    unsigned char *psrc, *pdest;
    
    if ( (psmallpic == NULL) || (pbigpic == NULL) || (psmallpic->pixeldata == NULL) || (pbigpic->pixeldata == NULL) )
    {
        return -1;
    }
    if ( (x > pbigpic->width) || (y > pbigpic->height) )
    {
        return -1;
    }

    if ( (psmallpic->width > pbigpic->width) || (psmallpic->height > pbigpic->height) )
    {
        return -1;
    }

    //计算出大图的源地址
    pdest = pbigpic->pixeldata + pbigpic->linebytes * y + (pbigpic->bpp * x >> 3);
    psrc = psmallpic->pixeldata;

    for (i = 0; i < psmallpic->height; i++)
    {
        memcpy(pdest, psrc, psmallpic->linebytes);
        psrc += psmallpic->linebytes;
        pdest += pbigpic->linebytes;
    }
    return 0;
}

static int SetColorForPixelInVideoMem(int iX, int iY, struct pic_data* ppic_data, unsigned int dwColor)
{
	unsigned char *pucVideoMem;
	unsigned short *pwVideoMem16bpp;
	unsigned int *pdwVideoMem32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;

    if ( (ppic_data == NULL) || (ppic_data->pixeldata == NULL) )
    {
        return -1;
    }
    
    pucVideoMem      = ppic_data->pixeldata;
	pucVideoMem      += iY * ppic_data->linebytes + iX * ppic_data->bpp / 8;
	pwVideoMem16bpp  = (unsigned short *)pucVideoMem;
	pdwVideoMem32bpp = (unsigned int *)pucVideoMem;


    switch (ppic_data->bpp)
	{
		case 8:
		{
			*pucVideoMem = (unsigned char)dwColor;
			return 1;
			break;
		}
		case 16:
		{
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			*pwVideoMem16bpp	= wColor16bpp;
			return 2;
			break;
		}
		case 32:
		{
			*pdwVideoMem32bpp = dwColor;
			return 4;
			break;
		}
		default :
		{			
			return -1;
		}
	}

	return -1;
}

void ClearRectangleInVideoMem(unsigned int iTopLeftX, unsigned int iTopLeftY, unsigned int iBotRightX, unsigned int iBotRightY, struct pic_data* ppic_data, unsigned int dwColor)
{
	int x, y;
	for (y = iTopLeftY; y <= iBotRightY; y++)
	{
		for (x = iTopLeftX; x <= iBotRightX; x++)
		{
			SetColorForPixelInVideoMem(x, y, ppic_data, dwColor);
		}
	}
}

int MergeOneFontToVideoMem(struct font_bitmap* ptFontBitMap, struct pic_data* ppic_data)
{
	int i;
	int x, y;
	int bit;
	int iNum;
	unsigned char ucByte;

    if ( (ptFontBitMap == NULL) || (ptFontBitMap->bitmap_var.pbuf == NULL) \
        || (ppic_data == NULL) || (ppic_data->pixeldata == NULL) )
    {
        return -1;
    }
    
	if (ptFontBitMap->bitmap_var.bpp == 1)
	{
		for (y = ptFontBitMap->bitmap_var.top; y < ptFontBitMap->bitmap_var.y_max; y++)
		{
			i = (y - ptFontBitMap->bitmap_var.top) * ptFontBitMap->bitmap_var.pitch;
			for (x = ptFontBitMap->bitmap_var.left, bit = 7; x < ptFontBitMap->bitmap_var.x_max; x++)
			{
				if (bit == 7)
				{
					ucByte = ptFontBitMap->bitmap_var.pbuf[i++];
				}
				
				if (ucByte & (1<<bit))
				{
					iNum = SetColorForPixelInVideoMem(x, y, ppic_data, COLOR_FOREGROUND);
				}
				else
				{
					/* 使用背景色 */
					// g_ptDispOpr->ShowPixel(x, y, 0); /* 黑 */
					iNum = SetColorForPixelInVideoMem(x, y, ppic_data, COLOR_BACKGROUND);
				}
				if (iNum == -1)
				{
					return -1;
				}
				bit--;
				if (bit == -1)
				{
					bit = 7;
				}
			}
		}
	}
	else if (ptFontBitMap->bitmap_var.bpp == 8)
	{
		for (y = ptFontBitMap->bitmap_var.top; y < ptFontBitMap->bitmap_var.y_max; y++)
		{
			for (x = ptFontBitMap->bitmap_var.left; x < ptFontBitMap->bitmap_var.x_max; x++)
			{
				//g_ptDispOpr->ShowPixel(x, y, ptFontBitMap->pucBuffer[i++]);
				if (ptFontBitMap->bitmap_var.pbuf[i++])
				{
					iNum = SetColorForPixelInVideoMem(x, y, ppic_data, COLOR_FOREGROUND);
				}
				else
				{
					iNum = SetColorForPixelInVideoMem(x, y, ppic_data, COLOR_BACKGROUND);
				}
				
				if (iNum == -1)
				{
					return -1;
				}
			}
		}
	}
	else
	{
		return -1;
	}
	return 0;
}


int string_merge(unsigned int x, unsigned int y, unsigned char* str, struct pic_data* pbigpic, const unsigned int font_size)
{
    struct list_head *plist;
    struct font_list* ptemp;
    struct font_operation *pfontops;
    struct font_bitmap font_bitmap;
    struct encode_operation* pencode;
    unsigned int code;
    int codenum;
    int last_font_size;
    unsigned char *pstart, *pend;

    if ( (pbigpic == NULL) || (x >= pbigpic->width) || (y >= pbigpic->height) )
    {
        return -1;
    }
    pstart = str;
    pend = str + strlen( (const char*)str);
    
    if ( (pstart == NULL) || (pend == NULL) || (pstart >= pend) )
    {
        return -1;
    }
    
    font_bitmap.cur_disp.x = x;
    font_bitmap.cur_disp.y = y;
    
    //1、选择对应的文字解码工具
    if ( (pencode = selectencodeforfile(pstart, strlen( (const char*)str) ) ) == NULL)
    {
        DBG_ERROR("can not find one font encode\n");
        return -1;
    }
    if (pencode->getcodefrmbuf == NULL)
    {
        DBG_ERROR("%s font encode not provide getcodefrmbuf fun\n");
        return -1;
    }
    
    while (1)
    {
        //2、使用支持的encode进行解码
        codenum = pencode->getcodefrmbuf(pstart, pend, &code);
        DBG_INFO("num = 0x%x, code=0x%x\n", codenum, code);
        if (codenum == -1) //解码失败
        {
            DBG_ERROR("use %s encode file error\n", pencode->name);
            return -1;
        }
        else if (codenum == 0) //达到文件尾巴
        {
            DBG_INFO("file display end\n");
            return 0;   
        }
        pstart += codenum;  //计算下一个编码
        //2、使用encode支持的font进行显示
        //遍历该编码支持的font
        list_for_each(plist, &pencode->supportfontlist)
        {
            ptemp = list_entry(plist, struct font_list, list);
            if (ptemp && ptemp->pcontext)
            {
                pfontops = (struct font_operation*)ptemp->pcontext;
                DBG_INFO("pfontops->name %s\n", pfontops->name);
                if (pfontops->get_font_size)
                {
                    last_font_size = pfontops->get_font_size();
                    if ( (last_font_size != font_size) && pfontops->set_font_size)
                    {
                        pfontops->set_font_size(font_size);
                    }
                }
                if (!pfontops->get_font_bitmap(code, &font_bitmap) ) //提取bitmap成功
                {
                    if (MergeOneFontToVideoMem(&font_bitmap, pbigpic) )
                    {
                        DBG_ERROR("MergeOneFontToVideoMem error\n");
                        return -1;
                    }
                    break;
                }
                else
                {
                    DBG_ERROR("get_font_bitmap error\n");
                    return -1;
                }
            }
        }
       
        //3、指定下一个要显示的坐标
        font_bitmap.cur_disp = font_bitmap.next_disp;
    }
}


