 #include <string.h>
 #include "encode_manager.h"

//utf-16Be文件头
static const unsigned char utf16be_head[] = {0xFE, 0xFF};

static int is_support_utf16be(unsigned char *pfilecontext, unsigned int filelen);
static int utf16be_get_code(unsigned char *pstart, unsigned char *pend, unsigned int *pcode);


static struct encode_operation utf16be_encode_ops =
{
    .name               = "utf-16be",
    .headlen            = 2,
    .headbuf            = utf16be_head,
    .issupport          = is_support_utf16be,
    .getcodefrmbuf      = utf16be_get_code,
    .supportfontlist    = LIST_HEAD_INIT(utf16be_encode_ops.supportfontlist),
    .list               = LIST_HEAD_INIT(utf16be_encode_ops.list),
};


/*****************************************************************************
* Function     : is_support_utf16be
* Description  : 判断是否支持utf16be来解码
* Input        : unsigned char *pfilecontext  ：文件指针
*                unsigned int filelen         ：文件长度
* Output       ：
* Return       : 0---不支持    1---支持
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月11日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int is_support_utf16be(unsigned char *pfilecontext, unsigned int filelen)
{
    if ( (pfilecontext == NULL) || (filelen <= utf16be_encode_ops.headlen) )
    {
        return 0;
    }
    //比较文件头两个字节
    if (!strncmp((const char*)pfilecontext, (const char*)utf16be_encode_ops.headbuf, utf16be_encode_ops.headlen) )
    {
        return 1; //支持
    }

    return 0;
}

/*****************************************************************************
* Function     : utf16be_get_code
* Description  : 给定一个buf，转出对应的编码值
* Input        : unsigned char *pstart  ：要解码的开始地址
*                unsigned char *pend    ：要解码的结束地址(不含)
*                unsigned int *pcode    ：解码后的存储的地址
* Output       ：
* Return       : -1 --- 参数错    0---达到文件尾 正值---以几个字节表示一个utf16be
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月11日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int utf16be_get_code(unsigned char *pstart, unsigned char *pend, unsigned int *pcode)
{
    //对于utf16be来说，用两个自己来表示一个编码值，并以大端形式存储
    if ( (pstart == NULL) || (pend == NULL) || (pcode == NULL) )
    {
        return -1;
    }
    if ((unsigned int)pend - (unsigned int)pstart < 2)
    {
        return -1;
    }
    *pcode = *( (unsigned short*)pstart);
    return 2;
}

/*****************************************************************************
* Function     : utf16be_encode_init
* Description  : utf16be编码初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月11日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int utf16be_encode_init(void)
{
    //给utf16be编码添加支持的字体
    add_font_to_encode(get_font_operation_by_name("ascii"), &utf16be_encode_ops);
    add_font_to_encode(get_font_operation_by_name("freetype"), &utf16be_encode_ops);    
    return register_encode_operation(&utf16be_encode_ops); //注册utf16be解码方式
}


