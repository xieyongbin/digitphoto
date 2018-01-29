#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include "input_manager.h"
#include "input_stdio.h"
#include "input_screentouch.h"
#include "debug.h"

#define THREAD_INPUT_STACK_SIZE     (1024 * 20)  // 堆栈大小20K
#define THREAD_INPUT_PRI            (0)          // 优先级

static struct list_head input_list_head = LIST_HEAD_INIT(input_list_head);
static pthread_rwlock_t list_head_rwlock = PTHREAD_RWLOCK_INITIALIZER;

static struct input_event inputevent;

//初始化一个线程互斥锁
static pthread_mutex_t event_mutex = PTHREAD_MUTEX_INITIALIZER;
//初始化一个线程条件变量
static pthread_cond_t event_cond = PTHREAD_COND_INITIALIZER;

/*****************************************************************************
* Function     : get_input_event_thread
* Description  : 获取输入事件线程
* Input        : void *arg  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月18日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void *get_input_event_thread(void *arg)
{
    int (*pevent)(struct input_event *pevent);
    pevent = (int (*)(struct input_event *pevent) )arg;
    struct input_event event;
    
    //设置为分离属性
//    pthread_detach(pthread_self());
    
    if (pevent == NULL)
    {
        DBG_ERROR("thread arg is null\n");
        pthread_exit( (void *)(-1) );
    }
    
    while (1)
    {
        if (0 == pevent(&event) )
        {
            //获取输入事件成功，返回事件
            pthread_mutex_lock(&event_mutex);   //获得事件互斥锁
            inputevent = event;
            pthread_cond_signal(&event_cond);   //唤醒等待该条件变量的线程
            pthread_mutex_unlock(&event_mutex); //获得事件互斥锁
        }
    }
    return NULL;
}

/*****************************************************************************
* Function     : get_input_event
* Description  : 获取输入事件
* Input        : struct input_event *pevent  
* Output       ：
* Return       : -1 ---参数错误或select错误或超时            0---获得一个输入事件
* Note(s)      : 此函数会休眠直到有事件输入
* Histroy      : 
* 1.Date       : 2017年12月18日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int get_input_event(struct input_event *pevent)
{
    if (pevent == NULL)
    {
        return -1;
    }
    pthread_mutex_lock(&event_mutex);               //获得输入事件锁
    pthread_cond_wait(&event_cond, &event_mutex);   //等待输入事件条件变量
    *pevent = inputevent;
    pthread_mutex_unlock(&event_mutex);             //释放输入事件锁
    return 0;
}

/*****************************************************************************
* Function     : register_input_operation
* Description  : 注册输入操作函数集
* Input        : struct input_operation * pops  
* Output       ：
* Return       : -1 --- 失败     0 ---成功
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月15日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int register_input_operation(struct input_operation * pops)
{
    if (pops == NULL)
    {
        return -1;
    }

    pthread_rwlock_wrlock(&list_head_rwlock);
    list_add_tail(&pops->list, &input_list_head);
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

/*****************************************************************************
* Function     : input_init
* Description  : 输入事件初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月18日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int input_init(int xres, int yres)
{
    //标准输入初始化
    if (input_stdio_init() )
    {
        DBG_ERROR("input stdio init error\n");
        return -1;
    }
    //触摸屏初始化
    if (input_screentouch_init(xres, yres) )
    {
        DBG_ERROR("input screentouch init error\n");
        return -1;        
    }
    return 0;
}


/*****************************************************************************
* Function     : input_open
* Description  : 输入事件打开
* Input        : void  
* Output       ：
* Return       : -1 ：有设备打开失败          0：全部输入设备打开成功
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月18日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int input_open(void)
{
    struct list_head *plist;
    struct input_operation *pops;
    int error = 0;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &input_list_head)
    {
        pops = list_entry(plist, struct input_operation, list);
        if (pops && pops->open)
        {
            error |= pops->open();
            if (!error)
            {
                //打开成功，创建一个线程
                if (pthread_spawn(&pops->tid, PTHREAD_CREATE_DETACHED, THREAD_INPUT_PRI, THREAD_INPUT_STACK_SIZE, get_input_event_thread, pops->get_input_event) )
                {
                    pthread_rwlock_unlock(&list_head_rwlock);
                    DBG_ERROR("pthread_spawn get_input_event_thread error\n");
                    return -1;
                }
            }
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return error;
}

/*****************************************************************************
* Function     : input_close
* Description  : 输入事件关闭
* Input        : void  
* Output       ：
* Return       : -1 ：有设备关闭失败          0：全部输入设备关闭成功
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月18日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int input_close(void)
{
    struct list_head *plist;
    struct input_operation *pops;
    int error = 0;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &input_list_head)
    {
        pops = list_entry(plist, struct input_operation, list);
        if (pops && pops->close)
        {
            error |= pops->close();
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return error;
}

/*****************************************************************************
* Function     : show_support_input
* Description  : 显示支持的输入
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月18日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int show_support_input(void)
{
    unsigned char num = 0;
    struct list_head* plist;
    struct input_operation* ptemp;

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &input_list_head)
    {
        ptemp = list_entry(plist, struct input_operation, list);
        printf("%02d input %s\n", ++num, ptemp->name);
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return 0;
}

