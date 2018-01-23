#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "font_manager.h"
#include "debug.h"

static int freetype_open(const char *fontfile, const unsigned int fontsize);
static int freetype_get_font_bitmap(const unsigned int code, struct font_bitmap *pfont_bitmap);
static int freetype_set_font_size(const unsigned int fontsize);
static int freetype_get_font_size(void);

static struct font_operation freetype_font_ops =
{
    .name               = "freetype",
    .open               = freetype_open,
    .set_font_size      = freetype_set_font_size,
    .get_font_size      = freetype_get_font_size,
    .get_font_bitmap    = freetype_get_font_bitmap,
};

static FT_GlyphSlot slot;
static FT_Face face;
static int face_font_size;


/*****************************************************************************
* Function     : freetype_open
* Description  : freetype打开函数
* Input        : const char *fontfile         
*                const unsigned int fontsize  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int freetype_open(const char *fontfile, const unsigned int fontsize)
{
    FT_Library library;

    if ( (fontfile == NULL) || (*fontfile == '\0') )
    {
        return -1;
    }
    
    if (FT_Init_FreeType(&library) )                //初始化Freetype
    {
        DBG_ERROR("FT_Init_Freetype error\n");
        return -1;
    }

    if (FT_New_Face(library, fontfile, 0, &face) )  //打开一个字体
    {
        DBG_ERROR("FT_New_Face error\n");
        return -1;   
    }
    
    slot = face->glyph;

    //设置字体大小
    if (FT_Set_Pixel_Sizes(face, fontsize, 0) )
    {
        DBG_ERROR("FT_Set_Pixel_Sizes error\n");
        return -1;   
    }
    face_font_size = fontsize;
    return 0;
}

/*****************************************************************************
* Function     : freetype_set_font_size
* Description  : 设置freetype字体大小
* Input        : const unsigned int fontsize  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月9日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int freetype_set_font_size(const unsigned int fontsize)
{
    //设置字体大小
    if (FT_Set_Pixel_Sizes(face, fontsize, 0) )
    {
        DBG_ERROR("FT_Set_Pixel_Sizes error\n");
        return -1;   
    }
    face_font_size = fontsize;
    return 0;
}

/*****************************************************************************
* Function     : freetype_get_font_size
* Description  : 获取freetype字体大小
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月9日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int freetype_get_font_size(void)
{
    return face_font_size;
}

/*****************************************************************************
* Function     : freeype_get_font_bitmap
* Description  : 根据编码获取字体位图
* Input        : unsigned int code  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int freetype_get_font_bitmap(const unsigned int code, struct font_bitmap *pfont_bitmap)
{
    FT_Vector pen;

    if (pfont_bitmap == NULL)
    {
        return -1;
    }
    //设置显示坐标
    pen.x = pfont_bitmap->cur_disp.x;
    pen.y = pfont_bitmap->cur_disp.y;
    
    //设置字体位子、旋转角度
    FT_Set_Transform(face, NULL, &pen);
   
    //获得字体bitmap
    if (FT_Load_Char(face, code, FT_LOAD_RENDER | FT_LOAD_MONOCHROME) )
    {
        DBG_ERROR("FT_Load_Char error\n");
        return -1;
    }
    //计算下一个位置
    pfont_bitmap->next_disp.x = pen.x + slot->advance.x / 64;   //下一个字体的x坐标
    pfont_bitmap->next_disp.y = pen.y;                          //下一个字体的y坐标
    //返回bitmap参数
    //真正需要开始描点的坐标
    pfont_bitmap->bitmap_var.left = pen.x + slot->bitmap_left;  //slot->bitmap_left是相对于pen.x的偏移值
    pfont_bitmap->bitmap_var.top = pen.y - slot->bitmap_top;    //slot->bitmap_top是相对于pen.y的偏移值
    pfont_bitmap->bitmap_var.x_max = pfont_bitmap->bitmap_var.left + slot->bitmap.width; //该字体的x方向最大值
    pfont_bitmap->bitmap_var.y_max = pfont_bitmap->bitmap_var.top + slot->bitmap.rows;   //该字体的y方向最大值
    pfont_bitmap->bitmap_var.bpp = 1;
    pfont_bitmap->bitmap_var.pitch = slot->bitmap.pitch;
    pfont_bitmap->bitmap_var.pbuf = slot->bitmap.buffer;        //该字体的位图信息
    return 0;
}
/*****************************************************************************
* Function     : freetype_font_init
* Description  : freetype字体初始化
* Input        : void  
* Output       ：
* Return       : 0--注册成功       -1--注册失败
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int freetype_font_init(void)
{
    return register_font_operation(&freetype_font_ops);
}


