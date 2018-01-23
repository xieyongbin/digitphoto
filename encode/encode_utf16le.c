 #include <string.h>
 #include "encode_manager.h"

//utf-16le文件头
static const unsigned char utf16le_head[] = {0xFF, 0xFE};

static int is_support_utf16le(unsigned char *pfilecontext, unsigned int filelen);
static int utf16le_get_code(unsigned char *pstart, unsigned char *pend, unsigned int *pcode);


static struct encode_operation utf16le_encode_ops =
{
    .name               = "utf-16le",
    .headlen            = 2,
    .headbuf            = utf16le_head,
    .issupport          = is_support_utf16le,
    .getcodefrmbuf      = utf16le_get_code,
    .supportfontlist    = LIST_HEAD_INIT(utf16le_encode_ops.supportfontlist),
    .list               = LIST_HEAD_INIT(utf16le_encode_ops.list),
};

/*****************************************************************************
* Function     : is_support_utf16le
* Description  : 判断是否支持utf16le来解码
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
static int is_support_utf16le(unsigned char *pfilecontext, unsigned int filelen)
{
    if ( (pfilecontext == NULL) || (filelen <= utf16le_encode_ops.headlen) )
    {
        return 0;
    }
    //比较文件头两个字节
    if (!strncmp((const char*)pfilecontext, (const char*)utf16le_encode_ops.headbuf, utf16le_encode_ops.headlen) )
    {
        return 1; //支持
    }
    return 0;
}

/*****************************************************************************
* Function     : utf16le_get_code
* Description  : 给定一个buf，转出对应的编码值
* Input        : unsigned char *pstart  ：要解码的开始地址
*                unsigned char *pend    ：要解码的结束地址(不含)
*                unsigned int *pcode    ：解码后的存储的地址
* Output       ：
* Return       : -1 --- 参数错    0---达到文件尾 正值---以几个字节表示一个utf16le
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月11日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int utf16le_get_code(unsigned char *pstart, unsigned char *pend, unsigned int *pcode)
{
    //对于utf16le来说，用两个自己来表示一个编码值，并以小端形式存储
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
* Function     : utf16le_encode_init
* Description  : utf16le编码初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月11日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int utf16le_encode_init(void)
{
    //给utf16le编码添加支持的字体
    add_font_to_encode(get_font_operation_by_name("ascii"), &utf16le_encode_ops);
    add_font_to_encode(get_font_operation_by_name("freetype"), &utf16le_encode_ops);    
    return register_encode_operation(&utf16le_encode_ops); //注册utf16le解码方式
}

