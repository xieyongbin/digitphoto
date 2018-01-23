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
* Function     : free_memory
* Description  : 释放一段内存，并设置为NULL，避免重复释放
* Input        : void *ptr  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月10日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
//void free_memory(void *ptr)
//{
//    free(ptr);      //如果ptr为NULL，不执行任何操作
//    if (ptr)
//    {
//        ptr = NULL;
//    }
//}


