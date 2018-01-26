#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "config.h"
#include "encode_manager.h"
#include "font_manager.h"
#include "font_freetype.h"
#include "font_ascii.h"
#include "font_gbk.h"
#include "debug.h"

static struct list_head font_list_head = LIST_HEAD_INIT(font_list_head);
static pthread_rwlock_t list_head_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/*****************************************************************************
* Function     : register_font_operation
* Description  : 注册字体操作函数
* Input        : struct font_operation* pops  
* Output       ：
* Return       : 
* Note(s)      : 0--注册成功       -1--注册失败
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int register_font_operation(struct font_operation* pops)
{
    struct font_list *plist = NULL;
    
    if (pops == NULL)
    {
        return -1;
    }
    check_null_point(plist);
    plist = (struct font_list *)malloc(sizeof(struct font_list) );
    if (plist == NULL)
    {
        return -1;
    }
    plist->list.next = &plist->list;
    plist->list.prev = &plist->list;
    plist->pcontext = pops;
    
    //把字体加入到链表尾部
    pthread_rwlock_wrlock(&list_head_rwlock);
    list_add_tail(&plist->list, &font_list_head);
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : unregister_font_operation
* Description  : 注销一个字体
* Input        : struct font_operation* pops  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月12日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int unregister_font_operation(struct font_operation* pops)
{
    struct list_head *plist, *pnext;
    struct font_list *pfontlist;
    struct font_operation *pfont;
    
    if (pops == NULL)
    {
        return -1;
    }

    pthread_rwlock_wrlock(&list_head_rwlock);
    list_for_each_safe(plist, pnext, &font_list_head)
    {
        pfontlist = list_entry(plist, struct font_list, list);
        pfont = (struct font_operation *)pfontlist->pcontext;
        if (!strcmp(pfont->name, pops->name) ) //比较名字
        {
            list_del(plist);
            free_memory(pfontlist);
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : unregister_all_font_operation
* Description  : 注销所有的字体
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月12日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int unregister_all_font_operation(void)
{
    struct list_head *plist, *pnext;
    struct font_list *pfontlist;

    pthread_rwlock_wrlock(&list_head_rwlock);
    list_for_each_safe(plist, pnext, &font_list_head)
    {
        pfontlist = list_entry(plist, struct font_list, list);
        list_del(plist);
        free_memory(pfontlist);
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}


/*****************************************************************************
* Function     : get_font_operation_by_name
* Description  : 通过名字获取字体操作集
* Input        : const char *name  
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct font_operation* get_font_operation_by_name(const char *name)
{ 
    struct list_head *plist;
    struct font_list* ptemp;
    struct font_operation* pops;
    
    if (name == NULL)
    {
        return NULL;
    }
    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &font_list_head)
    {
        ptemp = list_entry(plist, struct font_list, list);
        if (ptemp && ptemp->pcontext)
        {
            pops = (struct font_operation*)ptemp->pcontext;
            if (!strcmp(pops->name, name) )
            {
                pthread_rwlock_unlock(&list_head_rwlock);
                return pops;
            }
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return NULL;
}

/*****************************************************************************
* Function     : open_all_font
* Description  : 打开所有的字体，有一个字体打开成功就算成功
* Input        : const char *hzkfile          
*                const char *freetypefile     
*                const unsigned int fontsize  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月2日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int open_all_font(const char *hzkfile, const char *freetypefile, const unsigned int fontsize)
{
    struct list_head *plist, *pnext;
    struct font_list* ptemp;
    struct font_operation* pops;
    unsigned char onefont = 0;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each_safe(plist, pnext, &font_list_head) //plis后续会被释放，不能使用list_for_each
    {
        ptemp = list_entry(plist, struct font_list, list);
        if (ptemp && ptemp->pcontext)
        {
            pops = (struct font_operation*)ptemp->pcontext;
            if (pops && pops->open)
            {
                if (!strcmp(pops->name, "ascii") )
                {
                    if (pops->open(NULL, fontsize) )//打开失败
                    {
                        DBG_WARN("open font %s error\n", pops->name);
                        del_font_from_all_encode(pops);
                        list_del(plist);    //删除此字体
                        free_memory(ptemp); //释放struct font_list *ptemp;
                    }
                    else
                    {
                        DBG_INFO("open ascii ok\n");
                        onefont++;
                    }
                }
                else if (!strcmp(pops->name, "gbk") )
                {
                    if (pops->open(hzkfile, fontsize) )//打开失败
                    {
                        DBG_WARN("open font %s error\n", pops->name);
                        list_del(plist);    //删除此字体
                        free_memory(ptemp); //释放struct font_list *ptemp;
                    }
                    else
                    {
                        DBG_INFO("open gbk ok\n");
                        onefont++;
                    }
                }
                else if (!strcmp(pops->name, "freetype") )
                {
                    if (pops->open(freetypefile, fontsize) )//打开失败
                    {
                        DBG_WARN("open font %s error\n", pops->name);
                        list_del(plist);    //删除此字体
                        free_memory(ptemp); //释放struct font_list *ptemp;
                    }
                    else
                    {
                        DBG_INFO("open freetype ok\n");
                        onefont++;
                    }
                }
            }
            
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    if (!onefont)                   //没有一个字体可支持
    {
        DBG_ERROR("no font support to display\n");
        return -1;
    }
    return 0;
}

/*****************************************************************************
* Function     : show_support_font
* Description  : 显示支持的字体
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_support_font(void)
{
    unsigned int num = 0;
    struct list_head *plist;
    struct font_list* ptemp;
    struct font_operation* pops;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &font_list_head)
    {
        ptemp = list_entry(plist, struct font_list, list);
        if (ptemp && ptemp->pcontext)
        {
            pops = (struct font_operation*)ptemp->pcontext;
            printf("%02d %s\n", ++num, pops->name);
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}
/*****************************************************************************
* Function     : font_init
* Description  : 字体初始化
* Input        : void  
* Output       ：
* Return       : 0--注册成功       -1--注册失败
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int font_init(void)
{
    //ascii字体初始化
    if (ascii_font_init() )
    {
        DBG_ERROR("ascii_font_init error\n");
        return -1;
    }
    //gbk字体初始化
    if (gbk_font_init() )
    {
        DBG_ERROR("gbk_font_init error\n");
        return -1;        
    }
    //freetype字体初始化
    if (freetype_font_init() )
    {
        DBG_ERROR("freetype_font_init error\n");
        return -1;
    }
    return 0;
}


