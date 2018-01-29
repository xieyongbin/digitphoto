#include <stdio.h>
#include <stdlib.h>
#include "libthreadpro.h"

/*****************************************************************************
* Function     : sem_data_timedwait
* Description  : 等待一个信号量
* Input        : sem_t *sem         ：信号量指针
*                unsigned int msec  ：超时时间毫秒，0为无限期等待
* Output       ：
* Return       : -1---出错   0---成功
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月20日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int sem_data_timedwait(sem_t *sem, unsigned int msec)
{
    struct timespec ts;
    unsigned int nsec, ssec;

    if (sem == NULL)
    {
        return -1;
    }
    //一直等待
    if (msec == 0)
    {
        return sem_wait(sem);
    }
    
    //获取实时时间
    if (-1 == clock_gettime(CLOCK_REALTIME, &ts) )
    {
        return sem_wait(sem);
    }
    ssec = msec / 1000;
    nsec = (msec - ssec * 1000) * 1000000;
    
    if (ts.tv_nsec + nsec > 999999999)
    {
        ts.tv_nsec = ts.tv_nsec + nsec - 999999999;
        ts.tv_sec += ssec + 1;
    }
    else
    {
        ts.tv_nsec += nsec;
        ts.tv_sec += ssec;
    }
    return sem_timedwait(sem, &ts);
}

/*****************************************************************************
* Function     : cond_data_timedwait
* Description  : 等待一个条件变量
* Input        : pthread_cond_t *cond    
*                pthread_mutex_t *mutex  
*                unsigned int msec       :超时毫秒数，0为无限期等待
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月21日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int cond_data_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, unsigned int msec)
{
    struct timespec ts;
    unsigned int nsec, ssec;

    if ( (cond == NULL) || (mutex == NULL) )
    {
        return -1;
    }
    //一直等待
    if (msec == 0)
    {
        return pthread_cond_wait(cond, mutex);
    }
    
    //获取实时时间
    if (-1 == clock_gettime(CLOCK_REALTIME, &ts) )
    {
        return pthread_cond_wait(cond, mutex);
    }
    ssec = msec / 1000;
    nsec = (msec - ssec * 1000) * 1000000;
    
    if (ts.tv_nsec + nsec > 999999999)
    {
        ts.tv_nsec = ts.tv_nsec + nsec - 999999999;
        ts.tv_sec += ssec + 1;
    }
    else
    {
        ts.tv_nsec += nsec;
        ts.tv_sec += ssec;
    }
    return pthread_cond_timedwait(cond, mutex, &ts);
}

/*****************************************************************************
* Function     : pthread_get_priority_scope
* Description  : 获取线程指定策略支持的最大最小优先级
* Input        : int policy      : 要获取的策略
*                int *pmin_prio  ：该策略支持的最小优先级
*                int *pmax_prio  ：该策略支持的最大优先级
* Output       ：
* Return       : 0 ： 成功      -1 : 失败
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int pthread_get_priority_scope(int policy, int *pmin_prio, int *pmax_prio)
{
    int ret;
    
    if (pmin_prio)
    {
        //获取该调度策略的最大优先级
        if ( (ret = sched_get_priority_min(policy) ) == -1)
        {
            perror("sched_get_priority_min error\n");
            return ret;
        }
        *pmin_prio = ret;
    }
    if (pmax_prio)
    {
        if ( (ret = sched_get_priority_max(policy) ) == -1)
        {
            perror("shced_get_priority_max error\n")
;
            return ret;
        }
        *pmax_prio = ret;
    }
    return 0;
}

/*****************************************************************************
* Function     : pthread_set_attr
* Description  : 设置线程属性
* Input        : pthread_attr_t *pattr    :
*                int detach   
*                int prio                 
*                unsigned long stacksize  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int pthread_set_attr(pthread_attr_t *attr, int detach, int prio, unsigned long stacksize)
{
    int ret;
    int priority_min, priority_max;
    struct sched_param param;  //调度参数

    //初始化属性
    ret = pthread_attr_init(attr);
    if (ret)
    {
        fprintf(stderr, "pthread_attr_init error %d\n", ret);
        return ret;
    }

    //设置线程分离属性
    if (PTHREAD_CREATE_DETACHED == detach) //默认是 PTHREAD_CREATE_JOINABLE
    {
        ret = pthread_attr_setdetachstate(attr, detach);
        if (ret)
        {
            fprintf(stderr, "pthread_att_setdetachstate error %d\n", ret);
            goto goto_err;
        }
    }

    //设置线程调度策略为时间片轮转策略 SCHED_RR
    ret = pthread_attr_setschedpolicy(attr, SCHED_RR);
    if (ret) //默认为 SCHED_OTHER
    {
        fprintf(stderr, "pthread_attr_setschedpolicy error %d\n", ret);
        goto goto_err;
    }

    //获取时间片轮转调度策略支持的最小、最大优先级
    ret = pthread_get_priority_scope(SCHED_RR, &priority_min, &priority_max);
    if (ret)
    {
        fprintf(stderr, "pthread_get_priority_scope error %d\n", ret);
        goto goto_err;
    }

#if 0
    //获取调度参数
    ret = pthread_attr_getschedparam(attr, &param)
    if (ret)
    {
        fprintf(stderr, "pthread_getschedparam error %d\n", ret);
        goto goto_err;
    }
#endif    

    //调整调度参数的优先级
    if (prio < priority_min) 
    {
        param.sched_priority = priority_min;
    }
    else if (prio > priority_max)
    {
        param.sched_priority = priority_max;
    }
    else
    {
        param.sched_priority = prio;
    }

    //设置调度参数,就一个参数，线程优先级，在SCHED_OTHER调度策略，优先级始终为0
    ret = pthread_attr_setschedparam(attr, &param);
    if (ret)
    {
        fprintf(stderr, "pthread_attr_setschedparam error %d\n", ret);
        goto goto_err;
    }

//    if (stacksize < PTHREAD_STACK_MIN)
//    {
//        stacksize = PTHREAD_STACK_MIN;
//    }
//    
    //设置线程堆栈大小
    ret = pthread_attr_setstacksize(attr, stacksize);
    if (ret)
    {
        fprintf(stderr, "pthread_attr_setstacksize error %d\n", ret);
        goto goto_err;
    }
    
goto_err:
    pthread_attr_destroy(attr);
    return ret;
}

/*****************************************************************************
* Function     : pthread_spawn
* Description  : 创建一个线程
* Input        : pthread_t *thread               ：返回建立成功的线程ID
*                int detach                      : 线程分离状态，取值为 PTHREAD_CREATE_DETACHED 、PTHREAD_CREATE_JOINABLE
*                int prio                        : 线程优先级
*                unsigned long stacksize         : 线程堆栈字节数
*                void *(*start_routine)(void *)  ：线程主体函数
*                void *arg                       ：线程参数
* Output       ：
* Return       : 0 ： 成功     其他值：错误
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int pthread_spawn(pthread_t *thread, int detach, int prio, unsigned long stacksize, 
                  void *(*start_routine)(void *), void *arg)
{
    int ret;
    pthread_t tid;
    pthread_attr_t attr;

    //设置线程属性
    ret = pthread_set_attr(&attr, detach, prio, stacksize);
    if (ret)
    {
        fprintf(stderr, "pthread_set_attr error %d\n", ret);
        return ret;
    }

    //使用attr属性创建一个线程
    ret = pthread_create(&tid, &attr, start_routine, arg);
    if (ret)
    {
        fprintf(stderr, "pthread_create error %d\n", ret);
        return ret;
    }

    //返回线程id
    if (thread)
    {
        *thread = tid;
    }
    return ret;
}


