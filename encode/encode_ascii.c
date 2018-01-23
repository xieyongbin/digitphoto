 #include <string.h>
 #include "encode_manager.h"

//utf-16be文件头
static const unsigned char utf16be_head[] = {0xFE, 0xFF};
//utf-16le文件头
static const unsigned char utf16le_head[] = {0xFF, 0xFE};
//utf-8编码的文件头
static const unsigned char utf8_head[] = {0xEF, 0xBB, 0xBF};

static int is_support_ascii(unsigned char *pfilecontext, unsigned int filelen);
static int ascii_get_code(unsigned char *pstart, unsigned char *pend, unsigned int *pcode);


static struct encode_operation ascii_encode_ops =
{
    .name               = "ascii",
    .headlen            = 0,
    .headbuf            = NULL,
    .issupport          = is_support_ascii,
    .getcodefrmbuf      = ascii_get_code,
    .supportfontlist    = LIST_HEAD_INIT(ascii_encode_ops.supportfontlist),
    .list               = LIST_HEAD_INIT(ascii_encode_ops.list),
};


/*****************************************************************************
* Function     : is_support_ascii
* Description  : 判断是否支持ascii来解码
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
static int is_support_ascii(unsigned char *pfilecontext, unsigned int filelen)
{
    if ( (pfilecontext == NULL) || (filelen <= ascii_encode_ops.headlen) )
    {
        return 0;
    }
    //对ascii来说，没有固定的文件头，目前就以不是utf8\utf16be\utf16le的文件都算支持ascii
    //比较utf16be文件头两个字节
    if (!strncmp((const char*)pfilecontext, (const char*)utf16be_head, 2) )
    {
        return 0; //支持
    }
    //比较utf16le文件头两个字节
    else if (!strncmp((const char*)pfilecontext, (const char*)utf16le_head, 2) )
    {
        return 0; //支持
    }
    //比较utf8文件头三个字节
    else if (!strncmp((const char*)pfilecontext, (const char*)utf8_head, 3) )
    {
        return 0; //支持
    }

    return 1;
}

/*****************************************************************************
* Function     : ascii_get_code
* Description  : 给定一个buf，转出对应的编码值
* Input        : unsigned char *pstart  ：要解码的开始地址
*                unsigned char *pend    ：要解码的结束地址(不含)
*                unsigned int *pcode    ：解码后的存储的地址
* Output       ：
* Return       : -1 --- 参数错    0---达到文件尾 正值---以几个字节表示一个ascii
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月11日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int ascii_get_code(unsigned char *pstart, unsigned char *pend, unsigned int *pcode)
{
    //对于ascii来说，支持ascii和GBK
    //ascii用一个字节表示，且不能大于127
    //gbk用两个字节表示，且第一个字节不许大于127
    char c;
    
    if ( (pstart == NULL) || (pend == NULL) || (pcode == NULL) )
    {
        return -1;
    }
    if (pstart >= pend)
    {
        return 0;
    }
    c = pstart[0];
    if (c < 0x80) //ascii
    {
        /* 返回ASCII码 */
        *pcode = c;
        return 1;
    }
    else if (pstart + 2 <= pend)
    {
        /* 返回GBK码 */
        *pcode = pstart[0] | ( (pstart[1])<<8);
        return 2;
    }
    return -1;
}

/*****************************************************************************
* Function     : ascii_encode_init
* Description  : ascii编码初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月11日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int ascii_encode_init(void)
{
    //给ascii编码添加支持的字体
    add_font_to_encode(get_font_operation_by_name("ascii"), &ascii_encode_ops);
    add_font_to_encode(get_font_operation_by_name("freetype"), &ascii_encode_ops);  
    add_font_to_encode(get_font_operation_by_name("gbk"), &ascii_encode_ops);
    return register_encode_operation(&ascii_encode_ops); //注册ascii解码方式
}



