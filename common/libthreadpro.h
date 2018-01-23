#ifndef __LIB_THREAD_PRO_H__
#define __LIB_THREAD_PRO_H__

#include <pthread.h>
#include <semaphore.h>
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
int sem_data_timedwait(sem_t *sem, unsigned int msec);

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
int cond_data_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, unsigned int msec);

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
//void free_memory(void *ptr);

#define free_memory(ptr) do {free(ptr); ptr = NULL;} while(0);
#endif //__LIB_THREAD_PRO_H__

