#include <stdio.h>
#include <string.h>
#include "encode_manager.h"
#include "font_manager.h"

//utf-8编码的文件头
static const unsigned char utf8_head[] = {0xEF, 0xBB, 0xBF};

static int is_support_utf8(unsigned char *pfilecontext, unsigned int filelen);
static int utf8_get_code(unsigned char *pstart, unsigned char *pend, unsigned int *pcode);


static struct encode_operation utf8_encode_ops =
{
    .name            = "utf-8",
    .headlen         = 3,
    .headbuf         = utf8_head,
    .issupport       = is_support_utf8,
    .getcodefrmbuf   = utf8_get_code,
    .supportfontlist = LIST_HEAD_INIT(utf8_encode_ops.supportfontlist),
    .list            = LIST_HEAD_INIT(utf8_encode_ops.list),
};

/*****************************************************************************
* Function     : get_lead_one_num
* Description  : 计算一个unsigned char 值的前导1个数
* Input        : unsigned char val  
* Output       ：
* Return       : 前导1个数
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static unsigned char get_lead_one_num(unsigned char val)
{
    unsigned char num = 0;
    
    while (val & 0x80)
    {
        val <<= 1;
        num++;
    }
    return num;
}
/*****************************************************************************
* Function     : is_support_utf8
* Description  :  判断一个文件是否支持utf8编码
* Input        : unsigned char *pfilecontext  
*                unsigned int filelen         
* Output       ：
* Return       : 0---不支持    1---支持
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int is_support_utf8(unsigned char *pfilecontext, unsigned int filelen)
{
    if ( (pfilecontext == NULL) || (filelen <= utf8_encode_ops.headlen) )
    {
        return 0;
    }
    if (!strncmp((const char *)utf8_head, (const char *)pfilecontext, utf8_encode_ops.headlen) )
    {
        return 1;   //支持
    }
    return 0;
}

/*****************************************************************************
* Function     : utf8_get_code
* Description  : 给定一个buf，以utf8进行解码，解码后的值存-
                 储到pcode中，返回用几个字节表示此utf8
* Input        : unsigned char *pbuf   :要以utf8格式进行解码的开始buf
*                unsigned int buflen   :要解码的结束buf,不含
*                unsigned char *pcode  :解码后的值
* Output       ：
* Return       : -1 --- 参数错    0---达到文件尾 正值---以几个字节表示一个utf8
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int utf8_get_code(unsigned char *pstart, unsigned char *pend, unsigned int *pcode)
{
    unsigned char onenum, val, i;
    int sum = 0;
    
    if ( (pstart == NULL) || (pend == NULL) || (pcode == NULL) )
    {
        return -1;
    }
#if 0
    对于UTF-8编码中的任意字节B，如果B的第一位为0，则B为ASCII码，并且B独立的表示一个字符;
    如果B的第一位为1，第二位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且不为字符的第一个字节编码;
    如果B的前两位为1，第三位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由两个字节表示;
    如果B的前三位为1，第四位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由三个字节表示;
    如果B的前四位为1，第五位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由四个字节表示;

    因此，对UTF-8编码中的任意字节，根据第一位，可判断是否为ASCII字符;
    根据前二位，可判断该字节是否为一个字符编码的第一个字节; 
    根据前四位（如果前两位均为1），可确定该字节为字符编码的第一个字节，并且可判断对应的字符由几个字节表示;
    根据前五位（如果前四位为1），可判断编码是否有错误或数据传输过程中是否有错误。
#endif
    //从utf-8编码描述来看，需要计算前导1个数
    onenum = get_lead_one_num(pstart[0]);

    if (onenum >= (unsigned int)pend - (unsigned int)pstart)
    {
        return 0;
    }
    if (onenum)
    {
        val = pstart[0] << onenum;   //去掉pbuf[0]的高onenum个1
        val >>= onenum;            //把val剩余的8-onenum保留到原位
        sum += val;
        for (i = 1; i < onenum; i++)
        {
            val = pstart[i] & 0x3f;  //去掉后面onenum-1字节的高两位，此两位固定位10
            sum <<= 6;
            sum += val;
        }
        *pcode = sum;
        return onenum;
    }
    else //是一个ascii
    {
        *pcode = pstart[0];
        return 1;
    }
}

/*****************************************************************************
* Function     : utf8_encode_init
* Description  : utf8编码初始化
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int utf8_encode_init(void)
{
    //freetype字库支持显示utf-8的内容
    add_font_to_encode(get_font_operation_by_name("freetype"), &utf8_encode_ops); 
//    add_font_to_encode(get_font_operation_by_name("ascii") );
    return register_encode_operation(&utf8_encode_ops);
}

