#ifndef __LOG_MANAGER_H__
#define __LOG_MANAGER_H__

#include <list.h>

#define LOG_BUF_SIZE  (1024u)    //标准输出kfifo大小

struct log_operations
{
    char *name;
    unsigned char can_use;                     //1：可以使用      0:不能使用
    int (*open)(void);
    void (*close)(void);
    int (*print)(const char *buf, unsigned int len);
    struct list_head list;
};

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
void log_print(const char *format, ...);

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
struct log_operations *get_log_operation_by_name(const char* name);

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
int set_log_device_use(const char* name, unsigned char val);

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
int show_support_log(void);

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
int register_log_operation(struct log_operations * pops);

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
int log_open(void);

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
void log_close(void);

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
int log_init(void);

#endif //__LOG_MANAGER_H__
