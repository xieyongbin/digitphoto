#ifndef __INPUT_MANAGER_H__
#define __INPUT_MANAGER_H__

#include <pthread.h>
#include <sys/time.h>
#include <list.h>
#include "config.h"

#define INPUT_TYPE_STDIO          (1u)   //标准输入类型
#define INPUT_TYPE_TOUCHSCREEN    (2u)   //触摸屏输入类型

#define INPUT_EVENT_KEY_UNKOWN    (0u)
#define INPUT_EVENT_KEY_DOWN      (1u)
#define INPUT_EVENT_KEY_UP        (2u)
#define INPUT_EVENT_KEY_QUIT      (3u)

struct abs_value 
{
	int x;     /* X/Y座标 */
	int y;
	int press; /* 压力值 */
};


//输入事件结构体
struct input_event
{
    struct timeval time;    //发生事件的时间
	unsigned short type;    //事件类型
	unsigned short code;    //事件编码
	union
	{
	    int value;                 //事件值
	    struct abs_value abs;      //相对位置坐标
	}val;
};

struct input_operation                   //输入事件操作函数
{
    char* name;
    int fd;
    pthread_t tid;          //线程id
    int (*open)(void);
    int (*get_input_event)(struct input_event *pevent);
    int (*close)(void);
    struct list_head list;
};

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
int get_input_event(struct input_event *pevent);

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
int register_input_operation(struct input_operation * pops);

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
int input_init(int xres, int yres);

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
int input_open(void);

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
int input_close(void);

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
int show_support_input(void);


#endif //__INPUT_MANAGER_H__

