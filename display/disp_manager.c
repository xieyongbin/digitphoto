#include <string.h>
#include <pthread.h>
#include "disp_manager.h"
#include "disp_lcd.h"

static struct list_head disp_list_head = LIST_HEAD_INIT(disp_list_head);
static pthread_rwlock_t list_head_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/*****************************************************************************
* Function     : register_disp_operation
* Description  : 注册一个显示操作集
* Input        : struct disp_operation * pops  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int register_disp_operation(struct disp_operation *pops)
{
    if (pops == NULL)
    {
        return -1;
    }
    pthread_rwlock_wrlock(&list_head_rwlock);
    list_add_tail(&pops->list, &disp_list_head);
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : get_display_operation_by_name
* Description  : 通过名字获取显示操作函数
* Input        : const char* name  
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct disp_operation *get_display_operation_by_name(const char* name)
{
    struct list_head *plist;
    struct disp_operation *ptemp;
    
    if (name == NULL)
        return NULL;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &disp_list_head)
    {
        ptemp = list_entry(plist, struct disp_operation, list);
        if (ptemp && !strcmp(ptemp->name, name) )
        {
            pthread_rwlock_unlock(&list_head_rwlock);
            return ptemp;
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return NULL;
}

/*****************************************************************************
* Function     : show_support_display
* Description  : 显示所有支持的display
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月8日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_support_display(void)
{
    unsigned int num = 0;
    struct list_head *plist;
    struct disp_operation *ptemp;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &disp_list_head)
    {
        ptemp = list_entry(plist, struct disp_operation, list);
        if (ptemp)
        {
            printf("%02d %s\n", ++num, ptemp->name);
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : disp_close_all_device
* Description  : 关闭所有显示设备
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月12日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int disp_close_all_device(void)
{
    struct list_head *plist;
    struct disp_operation *ptemp;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &disp_list_head)
    {
        ptemp = list_entry(plist, struct disp_operation, list);
        if (ptemp && ptemp->close)
        {
            ptemp->close();
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : disp_init
* Description  : disp初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月7日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int disp_init(void)
{
    //LCD初始化
    if (disp_lcd_init() )
    {
        return -1;
    }
    return 0;
}



