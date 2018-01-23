#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <termios.h>
#include "input_manager.h"
#include "debug.h"


static int input_stdio_open(void);
static int input_stdio_close(void);
static int get_input_stdio_event(struct input_event *pevent);

static struct input_operation input_stdio_ops =
{
    .name            =   "stdio",
    .fd              =   STDIN_FILENO,
    .open            =   input_stdio_open,
    .get_input_event =   get_input_stdio_event,
    .close           =   input_stdio_close,
    .list            =   LIST_HEAD_INIT(input_stdio_ops.list),
};

static struct termios stdin_attr;


/*****************************************************************************
* Function     : input_stdio_open
* Description  : 打开标准输入
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月15日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int input_stdio_open(void)
{
    struct termios attr;

    //获取标准输入的属性
    if (-1 == tcgetattr(STDIN_FILENO, &attr) )
    {
        DBG_ERROR("tcgetattr error\n");
        return -1;
    }
    //备份标准输入属性
    stdin_attr = attr;

    //关闭标准模式
    attr.c_lflag &= ~ICANON;
    //收到一个字符就返回
    attr.c_cc[VMIN] = 1;

    //设置串口属性，并立即生效
    if (-1 == tcsetattr(STDIN_FILENO, TCSANOW, &attr) )
    {
        DBG_ERROR("tcsetattr error\n");
        return -1;
    }
    return 0;
}

/*****************************************************************************
* Function     : input_stdio_close
* Description  : 关闭标准输入
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月15日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int input_stdio_close(void)
{
    if (-1 == tcsetattr(STDIN_FILENO, TCSANOW, &stdin_attr) )
    {
        DBG_ERROR("tcsetattr error\n");
        return -1;
    }
    return 0;
}

/*****************************************************************************
* Function     : get_input_stdio_event
* Description  : 获取标准输入事件
* Input        : struct input_event *pevent  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月18日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int get_input_stdio_event(struct input_event *pevent)
{
    struct timeval curtime;
    
    char c;

    //获取输入，直到输入一个字符才会返回
    c = getchar();
    c = tolower(c);
    //获取当前时间
    if (-1 == gettimeofday(&curtime, NULL) )
    {
        DBG_ERROR("gettimeofday error\n");
        return -1;
    }
    switch (c)
    {
        case 'n':
            //下一页
            pevent->val.value = INPUT_EVENT_KEY_DOWN;
            break;
        case 'u':
            pevent->val.value = INPUT_EVENT_KEY_UP;
            break;
        case 'q':
            pevent->val.value = INPUT_EVENT_KEY_QUIT;
            break;
        default:
            pevent->val.value = INPUT_EVENT_KEY_UNKOWN;
            break;
    }
    //事件类型为标准输入
    pevent->type = INPUT_TYPE_STDIO;
    pevent->time = curtime;
    return 0;
}

/*****************************************************************************
* Function     : input_stdio_init
* Description  : stdio初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月15日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int input_stdio_init(void)
{
    return register_input_operation(&input_stdio_ops);
}

