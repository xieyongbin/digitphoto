#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <kfifo.h>
#include <list.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "libthreadpro.h"
#include "log_manager.h"
#include "debug.h"

#define THREAD_LOG_UDP_SEND_STACK_SIZE       (1024 * 50)   //log udp发送线程堆栈大小
#define THREAD_LOG_UDP_SEND_PRI              (0)           //log udp发送线程优先级
#define THREAD_LOG_UDP_RECV_STACK_SIZE       (1024 * 100)  //log udp接收线程堆栈大小
#define THREAD_LOG_UDP_RECV_PRI              (0)           //log udp接收线程优先级
#define SERVER_PORT  (1234)

static pthread_mutex_t mutex_send = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_send = PTHREAD_COND_INITIALIZER;
static pthread_t snd_tid, rcv_tid;
static int server_fd;
static struct sockaddr_in client_addr;
static struct kfifo *pfifo;
static unsigned char client_exit = 0;

static char send_buf[LOG_BUF_SIZE];

static int log_udp_open(void);
static void log_udp_close(void);
static int log_udp_print(const char *buf, unsigned int len);

static struct log_operations log_udp_ops =
{
    .name       = "udp",
    .can_use    = 1,
    .open       = log_udp_open,
    .close      = log_udp_close,
    .print      = log_udp_print,
    .list       = LIST_HEAD_INIT(log_udp_ops.list),
};

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
    free(arg);
}

/*****************************************************************************
* Function     : thread_udp_send
* Description  : upd打印发送线程
* Input        : void *arg  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void * thread_udp_send(void *arg)
{
    unsigned int len, done;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);   //禁止取消线程
//    pthread_detach(pthread_self());
    if ( (pfifo = kfifo_alloc(LOG_BUF_SIZE * 10) ) == NULL)
    {
        pthread_exit( (void *)(-1));
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);   //允许取消线程
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);  //延迟取消
    
    pthread_cleanup_push(free_kfifo, pfifo);               //注册kfifo清零函数
    pthread_cleanup_push((void (*)(void *))pthread_mutex_unlock, &mutex_send);//注册mutex解锁函数

    while (1)
    {
        pthread_mutex_lock(&mutex_send);
        while (!kfifo_len(pfifo) || !client_exit)
        {
            cond_data_timedwait(&cond_send, &mutex_send, 500);  //每500ms醒来判断下,一个取消点
        }
        len = kfifo_get(pfifo, (unsigned char*)send_buf, sizeof(send_buf) - 1);
        pthread_mutex_unlock(&mutex_send);
        if (!len)
        {
            continue;
        }
        else
        {
            done = sendto(server_fd, send_buf, len, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
            if (done != len)
            {
                fprintf(stderr, "udp send %d data,should send %d data\n", done, len);
            }
        }
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
}

/*****************************************************************************
* Function     : thread_udp_rcv
* Description  : udp接收线程
* Input        : void *arg  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void * thread_udp_rcv(void *arg)
{
    struct sockaddr_in addr;
    int len;
    socklen_t client_addr_size = sizeof(struct sockaddr_in);
    char rcv_buf[LOG_BUF_SIZE];
    //设置为分离属性
//    pthread_detach(pthread_self());
    
    while (1)
    {
        len = recvfrom(server_fd, rcv_buf, sizeof(rcv_buf) - 1, 0, (struct sockaddr *)&addr, &client_addr_size);
        if (len > 0)
        {
            rcv_buf[len] = '\0';
            printf("udp get msg: %s\n", rcv_buf);
            /* 解析数据:
			 * setclient            : 设置接收打印信息的客户端
			 * dbglevel=0,1,2...    : 修改打印级别
			 * stdout=0             : 关闭stdout打印
			 * stdout=1             : 打开stdout打印
			 * netprint=0           : 关闭netprint打印
			 * netprint=1           : 打开netprint打印
			 */
			if (strncmp(rcv_buf, "setclient", strlen("setclient") ) == 0)
			{
				client_addr = addr;
				client_exit = 1;
			}
//			else if (strncmp(rcv_buf, "dbglevel=", 9) == 0)
//			{
//				SetDbgLevel(ucRecvBuf);
//			}
			else if (strncmp(rcv_buf, "stdout=", strlen("stdout=") ) == 0)
			{
                if (isdigit(rcv_buf[strlen("stdout=")] ) )
                {
    				if (rcv_buf[strlen("stdout=")] - '0')
    				{
                        set_log_device_use("stdout", 1);
    				}
                    else
                    {
                        set_log_device_use("stdout", 0);
                    }
                }
			}
            else if (strncmp(rcv_buf, "udp=", strlen("udp=")) == 0)
			{
                if (isdigit(rcv_buf[strlen("udp=")]) )
                {
    				if (rcv_buf[strlen("udp=")] - '0')
    				{
                        set_log_device_use("udp", 1);
    				}
                    else
                    {
                        set_log_device_use("udp", 0);
                    }
                }
			}
        }
        
    }
}

/*****************************************************************************
* Function     : log_udp_open
* Description  : 打开udp打印
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int log_udp_open(void)
{
    struct sockaddr_in server_addr; 

    if (!log_udp_ops.can_use)
    {
        DBG_WARN("log udp is forbidded\n");
        return -1;
    }
    //创建一个socket
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == server_fd)
    {
        DBG_ERROR("socket error\n");
        return -1;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero) );
    //绑定服务器IP地址端口到描述符
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr) ) )
    {
        DBG_ERROR("bind error\n");
        return -1;
    }
    //创建发送线程
//    pthread_create(&snd_tid, NULL, thread_udp_send, NULL);
    if (pthread_spawn(&snd_tid, PTHREAD_CREATE_DETACHED, THREAD_LOG_UDP_SEND_PRI, THREAD_LOG_UDP_SEND_STACK_SIZE, thread_udp_send, NULL) )
    {
        DBG_ERROR("pthread_spawn thread_udp_send error\n");
        return -1;
    }
    //创建接收线程
//    pthread_create(&rcv_tid, NULL, thread_udp_rcv, NULL);
    if (pthread_spawn(&rcv_tid, PTHREAD_CREATE_DETACHED, THREAD_LOG_UDP_RECV_PRI, THREAD_LOG_UDP_RECV_STACK_SIZE, thread_udp_rcv, NULL) )
    {
        DBG_ERROR("pthread_spawn thread_udp_rcv error\n");
        return -1;
    }
    return 0;
}

/*****************************************************************************
* Function     : log_udp_close
* Description  : upd关闭
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月21日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void log_udp_close(void)
{
    if (!pthread_kill(snd_tid, 0) )
        pthread_cancel(snd_tid);

    if (!pthread_kill(rcv_tid, 0) )
        pthread_cancel(rcv_tid);
}

/*****************************************************************************
* Function     : log_udp_print
* Description  : udp日志打印
* Input        : const char *buf   
*                unsigned int len  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月26日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int log_udp_print(const char *buf, unsigned int len)
{
    if (!log_udp_ops.can_use)
    {
        DBG_WARN("log upd is forbidded\n");
        return 0;
    }
    if ( (buf == NULL) || !len)
    {
        return 0;
    }
    len = kfifo_put(pfifo, (unsigned char *)buf, len);
    if (len)
    {
        pthread_mutex_lock(&mutex_send);
        pthread_cond_signal(&cond_send);
        pthread_mutex_unlock(&mutex_send);
    }
    return len; 
}


/*****************************************************************************
* Function     : log_udp_init
* Description  : udp打印初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月19日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int log_udp_init(void)
{
    return register_log_operation(&log_udp_ops);
}

