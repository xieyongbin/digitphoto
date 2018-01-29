#ifndef __LIB_THREAD_PRO_H__
#define __LIB_THREAD_PRO_H__

#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
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
                  void *(*start_routine)(void *), void *arg);

#define free_memory(ptr) do {free(ptr); ptr = NULL;} while(0);
#define check_null_point(ptr)  do { if(ptr) printf("[%s][%05d]ptr is not null point\n", __FILE__, __LINE__); }while(0);

                                    
#endif //__LIB_THREAD_PRO_H__

