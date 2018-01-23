#include <stdlib.h>
#include <string.h>
#include "pic_manager.h"
#include "pic_bmp.h"
#include "debug.h"

#define BMP_FORMAT_HEAD  (0x4d42)           //BMP图像最开头两个字节
#define BMP_MIN_SIZE     (54u)              //BMP前面54字节是文件信息，不是真的数据

static int is_bmp_format(struct file_desc *pfile);
static int get_bmp_data(struct file_desc* pfile, struct pic_data *pic);
static void free_bmp_data(struct pic_data *pic);

static struct pic_operations pic_bmp_ops =
{
    .name = "bmp",
    .is_support = is_bmp_format,
    .get_pic_data = get_bmp_data,
    .free_pic_data = free_bmp_data,
    .list = LIST_HEAD_INIT(pic_bmp_ops.list),
};


/*****************************************************************************
* Function     : is_bmp_format
* Description  : 是否是bmp文件
* Input        : struct file_desc *pfile  : 文件信息
* Output       ：
* Return       : 0 --- 不支持      1---支持
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int is_bmp_format(struct file_desc *pfile)
{
    if ( (pfile == NULL) || (pfile->pfilebuf == NULL) || (pfile->filelen <= BMP_MIN_SIZE) )
    {
        return 0;
    }
    //判断文件头
    if ( (pfile->pfilebuf[0] | (pfile->pfilebuf[1] << 8) ) == BMP_FORMAT_HEAD)
    {
        return 1;
    }
    return 0;
}

static int CovertOneLine(int iWidth, int iSrcBpp, int iDstBpp, unsigned char *pudSrcDatas, unsigned char *pudDstDatas)
{
	unsigned int dwRed;
	unsigned int dwGreen;
	unsigned int dwBlue;
	unsigned int dwColor;

	unsigned short *pwDstDatas16bpp = (unsigned short *)pudDstDatas;
	unsigned int   *pwDstDatas32bpp = (unsigned int *)pudDstDatas;

	int i;
	int pos = 0;

	if (iSrcBpp != 24)
	{
		return -1;
	}

	if (iDstBpp == 24)
	{
		memcpy(pudDstDatas, pudSrcDatas, iWidth*3);
	}
	else
	{
		for (i = 0; i < iWidth; i++)
		{
			dwBlue  = pudSrcDatas[pos++];
			dwGreen = pudSrcDatas[pos++];
			dwRed   = pudSrcDatas[pos++];
			if (iDstBpp == 32)
			{
				dwColor = (dwRed << 16) | (dwGreen << 8) | dwBlue;
				*pwDstDatas32bpp = dwColor;
				pwDstDatas32bpp++;
			}
			else if (iDstBpp == 16)
			{
				/* 565 */
				dwRed   = dwRed >> 3;
				dwGreen = dwGreen >> 2;
				dwBlue  = dwBlue >> 3;
				dwColor = (dwRed << 11) | (dwGreen << 5) | (dwBlue);
				*pwDstDatas16bpp = dwColor;
				pwDstDatas16bpp++;
			}
		}
	}
	return 0;
}

/*****************************************************************************
* Function     : get_bmp_data
* Description  : 提取bmp的位图信息
* Input        : struct file_desc* pfile  ：bmp源文件
*                struct pic_data *pic     ：提取成功后的bmp信息
* Output       ：
* Return       : static
* Note(s)      : bmp文件从左下角往右进行保存，并以行为单位，每行要求是4字节对齐，
                 提取成功后的bmp位图数据以左上角开始进行保存，并填充的字节.
                 注意，此函数成功后会申请一块内存作为bmp位图信息，使用完毕后
                 需要调用free_bmp_data进行释放
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int get_bmp_data(struct file_desc* pfile, struct pic_data *pic)
{
    struct pic_data pic_info;
    struct bitmapfileheader *pfilehead;
    struct bitmapinfoheader *pinfohead;
    unsigned char *psrc, *pdest;
    unsigned int i, line_align;
    
    if ( (pfile == NULL) || (pfile->pfilebuf == NULL) || (pfile->filelen <= BMP_MIN_SIZE) )
    {
        return -1;
    }
    //安全起见，再次判断是否是bmp格式
    if (!is_bmp_format(pfile) )
    {
        return -1;
    }
    pfilehead = (struct bitmapfileheader *)(pfile->pfilebuf);
    pinfohead = (struct bitmapinfoheader *)(pfile->pfilebuf + sizeof(struct bitmapfileheader) );
    if (pinfohead->bisize != sizeof(struct bitmapinfoheader) ) //目前只支持40字节的bmp位图信息头
    {
        return -1;
    }
    //目前只支持24bpp的bmp图片，LCD本身是16位色的
    if (pinfohead->bibitcount != 24)
    {
        DBG_ERROR("bmp bpp is %d, now only support 24 bpp\n", pinfohead->bibitcount);
        return -1;
    }
    pic_info.width = pinfohead->biwidth;
    pic_info.height = pinfohead->biheight;
    pic_info.bpp = pic->bpp;
    pic_info.linebytes = (pic_info.width * pinfohead->bibitcount) >> 3;
    pic_info.totalbytes = pic_info.linebytes * pic_info.height;
    pic_info.pixeldata = (unsigned char *)malloc(pic_info.totalbytes);
    if (pic_info.pixeldata == NULL)
    {
        DBG_ERROR("malloc %d memory error\n", pic_info.totalbytes);
        return -1;
    }

    //4字节对齐
    line_align = (pic_info.linebytes + 3) & (~0x03);
    psrc = pfile->pfilebuf + pfilehead->bfoffbits;              //偏移到真正的位图数据
    //bmp第一个数据是左下角的像素点，从左往右保存的，以行为单位
    psrc = psrc + (pic_info.height - 1) * line_align; 
    pdest = pic_info.pixeldata;
    for (i = 0; i < pic_info.height; i++)
    {
//        memcpy(pdest, psrc, pic_info.linebytes);
        CovertOneLine(pic_info.width, pinfohead->bibitcount, pic->bpp, psrc, pdest);
        psrc -= line_align;
        pdest += pic_info.linebytes;
    }
    *pic = pic_info;
    return 0;
}

/*****************************************************************************
* Function     : free_bmp_data
* Description  : 释放bmp位图数据
* Input        : struct pic_data *pic  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void free_bmp_data(struct pic_data *pic)
{
    if ( (pic == NULL) || (pic->pixeldata == NULL) )
    {
        return;
    }
    free_memory(pic->pixeldata);
}

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
int pic_bmp_init(void)
{
    return register_pic_operation(&pic_bmp_ops);
}


