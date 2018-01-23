#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include "log_manager.h"
#include "log_stdout.h"
#include "log_udp.h"
#include "debug.h"

static struct list_head log_list_head = LIST_HEAD_INIT(log_list_head);
static pthread_rwlock_t list_head_rwlock = PTHREAD_RWLOCK_INITIALIZER;  //读写锁


/*****************************************************************************
* Function     : register_log_operation
* Description  : 注册log操作集
* Input        : struct log_operation * pops  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月19日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int register_log_operation(struct log_operations *pops)
{
    if (pops == NULL)
    {
        return -1;
    }
    pthread_rwlock_wrlock(&list_head_rwlock);  //获取写锁
    list_add_tail(&pops->list, &log_list_head);
    pthread_rwlock_unlock(&list_head_rwlock);  //释放写锁
    return 0;
}

/*****************************************************************************
* Function     : log_print
* Description  : log输出
* Input        : const char *format  
*                ...                 
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月20日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
void log_print(const char *format, ...)
{
    static char output[LOG_BUF_SIZE];
    struct list_head *plist;
    struct log_operations *ptemp;
    va_list arg;
    unsigned int done;
    
    va_start(arg, format);
    done = vsnprintf(output, sizeof(output), format, arg);
    va_end(arg);
    
    pthread_rwlock_rdlock(&list_head_rwlock);  //获取读锁
    list_for_each(plist, &log_list_head)
    {
        ptemp = list_entry(plist, struct log_operations, list);
        if (ptemp->can_use) //如果允许使用
        {
            if (ptemp->print)
            {
                ptemp->print(output, done);     //执行打印函数
            }
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);   //释放读锁
}

/*****************************************************************************
* Function     : get_log_operation_by_name
* Description  : 通过名字获取log设备操作集
* Input        : const char* name  
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct log_operations *get_log_operation_by_name(const char* name)
{
    struct list_head *plist;
    struct log_operations *ptemp;
    
    if (name == NULL)
        return NULL;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &log_list_head)
    {
        ptemp = list_entry(plist, struct log_operations, list);
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
* Function     : set_log_device_use
* Description  : 设置log设备开关
* Input        : const char* name   
*                unsigned char val  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int set_log_device_use(const char* name, unsigned char val)
{
    struct log_operations *pops;
    
    if (name == NULL)
    {
        return -1;
    }
    //获取log设备操作集
    if ( (pops = get_log_operation_by_name(name) ) == NULL)
    {
        return -1;
    }
    //是否需要关闭/打开设备
    if (pops->can_use && !val)
    {
        pops->can_use = val;
        //关闭设备
        if (pops->close)
        {
            pops->close();
        }
    }
    else if (!pops->can_use && val)
    {
        pops->can_use = val;
        //打开设备
        if (pops->open)
        {
            pops->open();
        }
    }
    return 0;
}
/*****************************************************************************
* Function     : show_support_log
* Description  : 显示支持的log
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月20日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_support_log(void)
{
    unsigned char num = 0;
    struct list_head* plist;
    struct log_operations* ptemp;
    
    pthread_rwlock_rdlock(&list_head_rwlock);  //获取读锁
    list_for_each(plist, &log_list_head)
    {
        ptemp = list_entry(plist, struct log_operations, list);
        printf("%02d log %s\n", ++num, ptemp->name);
    }
    pthread_rwlock_unlock(&list_head_rwlock);   //释放读锁
    return 0;
}

/*****************************************************************************
* Function     : log_open
* Description  : 打开log设备
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月21日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int log_open(void)
{
    int error = 0;
    struct list_head* plist;
    struct log_operations* ptemp;
    
    pthread_rwlock_rdlock(&list_head_rwlock);  //获取读锁
    list_for_each(plist, &log_list_head)
    {
        ptemp = list_entry(plist, struct log_operations, list);
        if (ptemp || ptemp->can_use || ptemp->open)
            error |= ptemp->open();
    }
    pthread_rwlock_unlock(&list_head_rwlock);   //释放读锁
    return error;
}

/*****************************************************************************
* Function     : log_close
* Description  : 关闭log设备
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月21日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
void log_close(void)
{
    struct list_head* plist;
    struct log_operations* ptemp;
    
    pthread_rwlock_rdlock(&list_head_rwlock);  //获取读锁
    list_for_each(plist, &log_list_head)
    {
        ptemp = list_entry(plist, struct log_operations, list);
        if (ptemp || ptemp->close)
             ptemp->close();
    }
    pthread_rwlock_unlock(&list_head_rwlock);   //释放读锁
}

/*****************************************************************************
* Function     : log_init
* Description  : log初始化
* Input        : void  
* Output       ：
* Return       : -1 --- 错误    0---成功
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月20日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int log_init(void)
{
    //初始化标准输出
//    if (log_stdout_init())
//    {
//        DBG_ERROR("log stdout init error\n");
//        return -1;
//    }
    if (log_udp_init() )
    {
        DBG_ERROR("log stdout init error\n");
        return -1;
    }
    //打开log设备
    if (log_open() )
    {
        DBG_ERROR("log stdout open error\n");
        return -1;
    }
    return 0;
}


