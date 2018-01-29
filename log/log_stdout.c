#include <stdarg.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <kfifo.h>
#include <list.h>
#include "libthreadpro.h"
#include "log_manager.h"
#include "debug.h"

#define THREAD_LOG_STDOUT_STACK_SIZE    (1024 * 20)  //堆栈大小
#define THREAD_LOG_STDOUT_PRI           (0)          //任务优先级
static char send_buf[LOG_BUF_SIZE];

static pthread_t stdout_snd_tid;
static struct kfifo *psnd_fifo = NULL;

#define USE_SEM
#ifdef USE_SEM
    static sem_t sem_send;
#else
    static pthread_mutex_t mutex_send = PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t cond_send = PTHREAD_COND_INITIALIZER;
#endif


static int log_stdout_open(void);
static void log_stdout_close(void);
static int log_stdout_print(const char *buf, unsigned int len);
static void *thread_stdout_send(void* arg);


static struct log_operations log_stdout_ops =
{
    .name       = "stdout",
    .can_use    = 0,
    .open       = log_stdout_open,
    .close      = log_stdout_close,
    .print      = log_stdout_print,
    .list       = LIST_HEAD_INIT(log_stdout_ops.list),
};


/*****************************************************************************
* Function     : log_stdout_open
* Description  : 打开标准输出
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月19日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int log_stdout_open(void)
{
    if (!log_stdout_ops.can_use)
    {
        DBG_WARN("log stdout is forbidded\n");
        return -1;
    }
    //创建stdout发送线程
//    if (-1 == pthread_create(&stdout_snd_tid, &attr, thread_stdout_send, NULL) )
    if (pthread_spawn(&stdout_snd_tid, PTHREAD_CREATE_DETACHED, THREAD_LOG_STDOUT_PRI, THREAD_LOG_STDOUT_STACK_SIZE, thread_stdout_send, NULL) )    
    {
        DBG_ERROR("create stdout send error\n");
        return -1;
    }
    return 0;
}

/*****************************************************************************
* Function     : log_stdout_close
* Description  : 关闭标准输出
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月19日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void log_stdout_close(void)
{
    if (!pthread_kill(stdout_snd_tid, 0) ) //检查stdout发送线程是否存在
        pthread_cancel(stdout_snd_tid);
}

/*****************************************************************************
* Function     : log_stdout_print
* Description  : 标准输出打印函数
* Input        : const char *format  
*                ...                 
* Output       ：
* Return       : static
* Note(s)      : 此函数不是线程安全函数,且有可能休眠
* Histroy      : 
* 1.Date       : 2017年12月19日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int log_stdout_print(const char *buf, unsigned int len)
{    
    if (!log_stdout_ops.can_use)
    {
        DBG_WARN("log stdout is forbidded\n");
        return 0;
    }
    if ( (buf == NULL) || !len)
    {
        return 0;
    }
    len = kfifo_put(psnd_fifo, (unsigned char *)buf, len);
    if (len)
    {
#ifdef USE_SEM           
        sem_post(&sem_send);
#else
        pthread_mutex_lock(&mutex_send);
        pthread_cond_signal(&cond_send);
        pthread_mutex_unlock(&mutex_send);
#endif
    }
    return len; 
}

/*****************************************************************************
* Function     : free_kfifo
* Description  : 释放kfifo
* Input        : void *arg  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void free_kfifo(void *arg)
{
    if (arg == NULL)
        return;
    printf("thread log stdout send free %p kfifo\n", arg);
    free(arg);
}

/*****************************************************************************
* Function     : thread_stdout_send
* Description  : 标准输出发送线程
* Input        : void* arg  
* Output       ：
* Return       : void
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月21日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void *thread_stdout_send(void* arg)
{
    unsigned int fifo_len;
    
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);  //不允许被取消
    //设置为分离属性
//    pthread_detach(pthread_self() );
    //申请一个kfifo
    psnd_fifo = kfifo_alloc(LOG_BUF_SIZE * 10);
    if (psnd_fifo == NULL)
    {
        DBG_ERROR("alloc kfifo error\n");
        pthread_exit((void *)-1);
    }
#ifdef USE_SEM    
    //创建一个初始化值为信号量
    if (-1 == sem_init(&sem_send, 0, 0) )
    {
        kfifo_free(psnd_fifo);  //释放kfifo
        DBG_ERROR("sem init error\n");
        pthread_exit((void *)-1);
    }
#else
    
#endif
    pthread_cleanup_push(free_kfifo, psnd_fifo);            //设置释放kfifo清理函数
#ifdef USE_SEM
    pthread_cleanup_push((void (*)(void *arg))sem_post, (void *)&sem_send);              //设置释放信号量清理函数
#else
    pthread_cleanup_push((void (*)(void *arg))pthread_mutex_unlock, (void *)&mutex_send);//设置释放mutex清理函数
#endif    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);    //允许被取消
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);   //延迟取消
    printf("thread_stdout_send ready\n");
    
    while (1)
    {
#ifdef USE_SEM           
        //等待信号量
        sem_data_timedwait(&sem_send, 100);                 //等待100ms
        if (!(fifo_len = kfifo_len(psnd_fifo) ) )
        {
            //kfifo里没有数据，继续休眠
            continue;
        }
        //从kfifo读取数据
        if (kfifo_get(psnd_fifo, (unsigned char *)send_buf, sizeof(send_buf) - 1) )
        {
            printf("%s", send_buf);
        }
#else
        pthread_mutex_lock(&mutex_send);
        while (!(fifo_len = kfifo_len(psnd_fifo) ) )
        {
            pthread_cond_wait(&cond_send, &mutex_send);
        }
        //从kfifo读取数据
        if (kfifo_get(psnd_fifo, (unsigned char *)send_buf, sizeof(send_buf) - 1) )
        {
            pthread_mutex_unlock(&mutex_send);
            printf("%s", send_buf);
        }
        else
        {
            pthread_mutex_unlock(&mutex_send);
        }
#endif        
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0); //为了跟 pthread_cleanup_push成对出现
}

/*****************************************************************************
* Function     : log_stdout_init
* Description  : 初始化标准输出
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月19日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int log_stdout_init(void)
{
    return (register_log_operation(&log_stdout_ops) );
}

