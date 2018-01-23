#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <termios.h>
#include <tslib.h>
#include "input_manager.h"
#include "input_screentouch.h"
#include "debug.h"

static int input_screentouch_open(void);
static int input_screentouch_close(void);
static int get_input_screentouch_event(struct input_event *pevent);

static struct input_operation input_screentouch_ops =
{
    .name            =   "screentouch",
    .open            =   input_screentouch_open,
    .get_input_event =   get_input_screentouch_event,
    .close           =   input_screentouch_close,
    .list            =   LIST_HEAD_INIT(input_screentouch_ops.list),
};

static struct tsdev *ts;;
static int lcd_xres, lcd_yres;  //lcd显示x\y分辨率

/*****************************************************************************
* Function     : is_timetout_ms
* Description  : 毫秒延时
* Input        : struct timeval *pcurtime     : 当前时间
*                struct timeval *pprevtime    ：要延时的开始时间
*                const unsigned int delay_ms  ：要延时的ms
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月18日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int is_timetout_ms(struct timeval *pcurtime, struct timeval *pprevtime, const unsigned int delay_ms)
{
    unsigned int curms;
    unsigned int prevms;
    //判断参数
    if ( (pcurtime == NULL) || (pprevtime == NULL) )
    {
        return -1;
    }
    if (delay_ms == 0)
    {
        return 1;
    }
    curms = pcurtime->tv_sec * 1000 + pcurtime->tv_usec / 1000;
    prevms = pprevtime->tv_sec * 1000 + pprevtime->tv_usec / 1000; 
    return ( (curms > prevms) ? (curms - prevms >= delay_ms) : 0);
}
/*****************************************************************************
* Function     : input_screentouch_open
* Description  : 打开触摸屏设备
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月15日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int input_screentouch_open(void)
{
    char *tsdevice = NULL;

    //根据环境变量 TSLIB_TSDEVICE 来决定打开的设备
    if ( (tsdevice = getenv("TSLIB_TSDEVICE") ) != NULL) 
    {
        ts = ts_open(tsdevice, 0);
    }
    else
    {
        ts = ts_open("/dev/input/event0", 0);
    }
    if (!ts) 
    {
		DBG_ERROR("ts_open error\n");
		return -1;
	}
    if (ts_config(ts) )
    {
		DBG_ERROR("ts_config error\n");
		return -1;
	}
    //获取ts文件描述符
    input_screentouch_ops.fd = ts_fd(ts);
    return 0;
}

/*****************************************************************************
* Function     : input_screentouch_close
* Description  : 关闭触摸屏设备
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月15日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int input_screentouch_close(void)
{
    //关闭此描述符
    ts_close(ts);
    return 0;
}

/*****************************************************************************
* Function     : get_input_screentouch_event
* Description  : 获取触摸屏输入事件
* Input        : struct input_event *pevent  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月18日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int get_input_screentouch_event(struct input_event *pevent)
{
    static struct timeval pretime;
    struct ts_sample samp;
    int ret;

    //获取输入，直到输入一个字符才会返回
	if ( (ret = ts_read(ts, &samp, 1) ) < 0)
	{
  		DBG_ERROR("ts_read error\n");
        return -1;
	}

    if (is_timetout_ms(&samp.tv, &pretime, 500) )
    {
#if 0        
        //屏幕x方向左边三分之一为上一页
        if (samp.x < lcd_xres / 3)
        {
            pevent->val.value = INPUT_EVENT_KEY_UP;
        }
        //中间三分之一为退出
        else if ( (samp.x >= lcd_xres / 3) && (samp.x <= lcd_xres * 2 / 3) )
        {
            pevent->val.value = INPUT_EVENT_KEY_QUIT;
        }
        //x方向的最右边为下一页
        else
        {
            pevent->val.value = INPUT_EVENT_KEY_DOWN;
        }
#else
        pevent->val.abs.x = samp.x;
        pevent->val.abs.y = samp.y;
        pevent->val.abs.press = samp.pressure;
#endif
        //事件类型为标准输入
        pevent->type = INPUT_TYPE_TOUCHSCREEN;
        pevent->time = samp.tv;
        pretime = samp.tv;
        return 0;
    }
    else
    {
        return -1;
    }
}

/*****************************************************************************
* Function     : input_screentouch_init
* Description  : screentouch初始化
* Input        : int xres  :显示x分辨率
*                int yres  :显示y分辨率
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月18日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int input_screentouch_init(int xres, int yres)
{
    if ( (xres <= 0) || (yres <= 0) )
    {
        return -1;
    }
    lcd_xres = xres;
    lcd_yres = yres;
    return register_input_operation(&input_screentouch_ops);
}


