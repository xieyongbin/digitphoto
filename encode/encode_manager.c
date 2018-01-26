#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "debug.h"
#include "encode_manager.h"
#include "font_manager.h"
#include "encode_utf8.h"
#include "encode_utf16le.h"
#include "encode_utf16be.h"
#include "encode_ascii.h"

//定义编码链表头
static struct list_head encode_list_head = LIST_HEAD_INIT(encode_list_head);
static pthread_rwlock_t list_head_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/*****************************************************************************
* Function     : register_encode_operation
* Description  : 注册编码方式
* Input        : struct encode_operation* pops  
* Output       ：
* Return       : 0：注册成功       -1:注册失败
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int register_encode_operation(struct encode_operation* pops)
{
    if (pops == NULL)
    {
        return -1;
    }
    //把该编码方式加入到链表尾部
    pthread_rwlock_wrlock(&list_head_rwlock);
    list_add_tail(&pops->list, &encode_list_head);
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : add_font_to_encode
* Description  : 添加一种字体pfont到pencode编码中
* Input        : struct font_operation* pfont     ：注册到pencode的字体
                 struct encode_operation* pencode ：解码方式
* Output       ：
* Return       : -1 --- 添加失败  0---添加成功
* Note(s)      : 此函数将会分配一个sizeof(struct font_list)空间
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int add_font_to_encode(struct font_operation* pfont, struct encode_operation* pencode)
{
    struct font_list* plist = NULL;
    
    if ( (pfont == NULL) || (pencode == NULL) )
    {
        return -1;
    }
    check_null_point(plist);
    plist = (struct font_list*)malloc(sizeof(struct font_list) );
    if (plist == NULL)
    {
        return -1;
    }
    plist->list.next = &plist->list;
    plist->list.prev = &plist->list;
    plist->pcontext = pfont;
    pthread_rwlock_wrlock(&list_head_rwlock);
    list_add_tail(&plist->list, &pencode->supportfontlist);
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : del_font_from_encode
* Description  : 从一个文字解码器中删除一个字体
* Input        : struct font_operation* pfont      
*                struct encode_operation* pencode  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月5日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int del_font_from_encode(struct font_operation* pfont, struct encode_operation* pencode, unsigned char needlock)
{
    struct list_head *plist, *pnext;
    struct font_list* pfontlist;
    struct font_operation* pdelfont;
    
    if ( (pfont == NULL) || (pencode == NULL) )
    {
        return -1;
    }

    //遍历该文字解码支持的显示字体
    if (needlock)
    {
        pthread_rwlock_wrlock(&list_head_rwlock);
    }
    list_for_each_safe(plist, pnext, &pencode->supportfontlist)
    {
        pfontlist = list_entry(plist, struct font_list, list);
        if (pfontlist && pfontlist->pcontext)
        {
            pdelfont = (struct font_operation*)pfontlist->pcontext;
            if (!strcmp(pdelfont->name, pfont->name) ) //名字相同
            {
                list_del(plist);
                free_memory(pfontlist);
            }
        }
    }
    if (needlock)
    {
        pthread_rwlock_unlock(&list_head_rwlock);
    }
    return 0;
}

/*****************************************************************************
* Function     : del_font_from_all_encode
* Description  : 删除所有解码里的支持的该字体
* Input        : struct font_operation* pfont  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月5日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int del_font_from_all_encode(struct font_operation* pfont)
{
    struct list_head *pencodelist;
    struct encode_operation* pencode;
    int error = 0;
    
    if (pfont == NULL)
    {
        return -1;
    }
    
    pthread_rwlock_wrlock(&list_head_rwlock);
    //遍历文字解码链表
    list_for_each(pencodelist, &encode_list_head)
    {
        pencode = list_entry(pencodelist, struct encode_operation, list);
        error |= del_font_from_encode(pfont, pencode, 0);
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    
    return error;
}

/*****************************************************************************
* Function     : del_all_font_from_encode
* Description  : 删除解码字体
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月12日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int del_all_font_from_encode(void)
{
    struct list_head *pencodelist, *pfontlist, *pnext;
    struct encode_operation* pencode;

    struct font_list* pfontlistcontext;

    pthread_rwlock_wrlock(&list_head_rwlock);
    //遍历文字解码链表
    list_for_each(pencodelist, &encode_list_head)
    {
        pencode = list_entry(pencodelist, struct encode_operation, list);
        //遍历该解码的支持的字体链表
        list_for_each_safe(pfontlist, pnext, &pencode->supportfontlist)
        {
            pfontlistcontext = list_entry(pfontlist, struct font_list, list);
            free_memory(pfontlistcontext); //释放先前分配的 struct font_list
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : selectencodeforfile
* Description  : 给指定文件选择一个编码
* Input        : unsigned char *filebuf  
*                int buflen              
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct encode_operation* selectencodeforfile(unsigned char *filebuf, int buflen)
{
    struct encode_operation *ptemp = NULL;
    struct list_head *plist = NULL;
    
    if ( (filebuf == NULL) || !buflen)
    {
        return NULL;
    }

    pthread_rwlock_rdlock(&list_head_rwlock);
    //遍历文件编码链表
    list_for_each(plist, &encode_list_head)
    {
        //转化为struct encode_operation结构体地址
        ptemp = list_entry(plist ,struct encode_operation, list); 
        if ( (ptemp->issupport) && (ptemp->issupport(filebuf, buflen) ) )
        {
            pthread_rwlock_unlock(&list_head_rwlock);
            return ptemp;
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return NULL;
}

/*****************************************************************************
* Function     : show_support_encode
* Description  : 显示支持的编码方式
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_support_encode(void)
{
    unsigned int num = 0;
    struct list_head *plist;
    struct encode_operation* ptemp;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &encode_list_head)
    {
        ptemp = list_entry(plist, struct encode_operation, list);
        if (ptemp)
        {
            printf("%02d %s\n", ++num, ptemp->name);
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : show_encode_support_font
* Description  : 显示编译支持的字体
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_encode_support_font(void)
{
    unsigned int num = 0;
    struct list_head *pencodelist;
    struct list_head *pfontlist;
    struct font_list *ptemp;
    struct encode_operation* pencode;
    struct font_operation* pfont;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(pencodelist, &encode_list_head)
    {
        pencode = list_entry(pencodelist, struct encode_operation, list);          //获得编码
        if (pencode)
        {
            num = 0;
            printf("\nencode %s suppored:\n", pencode->name);
            list_for_each(pfontlist, &pencode->supportfontlist)
            {
                ptemp = list_entry(pfontlist, struct font_list, list);
                pfont = (struct font_operation*)ptemp->pcontext; //获得编码支持的字体
                if (pfont)
                {
                    printf("%02d font %s\n", ++num, pfont->name);
                }
            }
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : encode_init
* Description  : 编码初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int encode_init(void)
{
    //ascii编码初始化
    if (ascii_encode_init() )
    {
        DBG_ERROR("ascii_encode_init error\n");
        return -1;
    }
    //utf16le编码初始化
    if (utf16le_encode_init() )
    {
        DBG_ERROR("utf16le_encode_init error\n");
        return -1;
    }
    //utf16be编码初始化
    if (utf16be_encode_init() )
    {
        DBG_ERROR("utf16be_encode_init error\n");
        return -1;
    }
    //utf8编码初始化
    if (utf8_encode_init() )
    {
        DBG_ERROR("utf8_encode_init error\n");
        return -1;
    }
    return 0;
}

/*****************************************************************************
* Function     : encode_exit
* Description  : 释放字体解码占用的资源，通过 add_font_to_encode() 添加一个支持的字体
                 会分配一个sizeof(struct font_list)空间，在此处进行释放
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
void encode_exit(void)
{
    //删除所有字体解码的所支持的字体
    del_all_font_from_encode();
}

