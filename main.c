#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <execinfo.h>
#include <signal.h>
#include "config.h"
#include "debug.h"
#include "encode_manager.h"
#include "font_manager.h"
#include "disp_manager.h"
#include "input_manager.h"
#include "log_manager.h"
#include "page_manager.h"
#include "draw.h"


#define FILE_MAX_LEN (127)

struct bookset
{
    char hzkfile[FILE_MAX_LEN + 1];         //汉字库名字
    char freetypefile[FILE_MAX_LEN + 1];    //freetype字体名字
    char displayobj[FILE_MAX_LEN + 1];      //输出对象名字
    char showfile[FILE_MAX_LEN + 1];        //显示文件名
    unsigned int fontsize;     //字体大小
    unsigned char listflag;    //显示支持的列表
};

static struct bookset booksetting;

/* SIGSEGV信号的处理函数，回溯栈，打印函数的调用关系 */
void DebugBacktrace(int signum)
{
#define SIZE 100
    void *array[SIZE];
    int size, i;
    char **strings;

    fprintf (stderr, "\nSegmentation fault\n");
    size = backtrace (array, SIZE);    
    fprintf (stderr, "Backtrace (%d deep):\n", size);
    strings = backtrace_symbols (array, size);
    for (i = 0; i < size; i++)
        fprintf (stderr, "%d: %s\n", i, strings[i]);
    free (strings);
    exit(-1);
}

/*****************************************************************************
* Function     : print_usage
* Description  : 打印Usage
* Input        : char* argv  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
void print_usage(char* argv)
{
    if (argv == NULL)
    {
        return;
    }
    printf("\nUsage:\n\t%s <freetype_font_file>\n", argv);
}

/*****************************************************************************
* Function     : booksetinit
* Description  : 初始参数
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static void booksetinit(void)
{
    memset(&booksetting, 0, sizeof(booksetting) );

    //默认字体为16
    booksetting.fontsize = 48;
    //默认显示参数为LCD
    strncpy(booksetting.displayobj, "lcd", FILE_MAX_LEN);
    booksetting.displayobj[FILE_MAX_LEN] = '\0';
}

/*****************************************************************************
* Function     : parsecmdline
* Description  : 解析命令行参数
* Input        : int argc               
*                char *const *argv      
*                const char *optstring  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int parsecmdline(int argc, char *const *argv, const char *optstring)
{
    int ret;
    
    if ( (argc == 1) || (argv == NULL) || (optstring == NULL) )
    {
        return -1;
    }
    while ( (ret = getopt(argc, argv, "ls:f:h:") ) != -1)
    {
        switch (ret)
        {
            case 'l':
                booksetting.listflag = 1;
                break;
            case 's':
                booksetting.fontsize = strtol(optarg, NULL, 0);         //设置字体大小
                break;
            case 'f':
                strncpy(booksetting.freetypefile, optarg, FILE_MAX_LEN);//拷贝freetype字体库名字
                booksetting.freetypefile[FILE_MAX_LEN] = '\0';
                break;
            case 'h':
                strncpy(booksetting.hzkfile, optarg, FILE_MAX_LEN);     //拷贝汉字库字体库名字
                booksetting.hzkfile[FILE_MAX_LEN] = '\0';
                break;
            default:
                print_usage(argv[0]);
                return -1;
        }
    }
    return 0;
}

/*****************************************************************************
* Function     : handlecmdline
* Description  : 处理命令行参数
* Input        : void  
* Output       ：
* Return       : static
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int handlecmdline(const char* pshowfile)
{
    if (booksetting.listflag)  //列出支持的选项
    {
        printf("supported display:\n");
        show_support_display();
        printf("\nsupported font:\n");
        show_support_font();
        printf("\nsupported encode:\n");
        show_support_encode();
        show_encode_support_font();
        printf("\nsupported inuput:\n");
        show_support_input();
        printf("\nsupported log:\n");
        show_support_log();
        return 1;
    }
    else
    {
        if (pshowfile == NULL)
            return -1;
        //拷贝要显示的文件名
        strncpy(booksetting.showfile, pshowfile, FILE_MAX_LEN);
        booksetting.showfile[FILE_MAX_LEN] = '\0';
    }
    return 0;
}

/*****************************************************************************
* Function     : handleinputchar
* Description  : 处理输入的字符
* Input        : char c  
* Output       ：
* Return       : 1:收到'q' 0：其他字符
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
static int handleinputchar(char c)
{
    switch (c)
    {
        case INPUT_EVENT_KEY_DOWN:
            show_next_page();
            break;
        case INPUT_EVENT_KEY_UP:
            show_prev_page();
            break;
        case INPUT_EVENT_KEY_QUIT:
            return 1;
        default:
            break;
    }
    return 0;
}

/*****************************************************************************
* Function     : target_init
* Description  : 各模块初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月22日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int target_init(struct disp_operation **ppdisp)
{
    struct disp_operation *pdisp;
    struct timeval time;
    int ret = -1;
    
    if (log_init() ) //log初始化
    {
        DBG_ERROR("log init error\n");
        return ret;
    }
    
    //使用select延时500ms，等待其他线程就绪
    time.tv_sec = 0;
    time.tv_usec = 500;
    select(0, NULL, NULL, NULL, &time);
    log_print("log init ok\n");
    
    //显示字体初始化
    if (font_init() )
    {
        DBG_ERROR("font init error\n");
        return ret;
    }

    //初始化encode解码
    if (encode_init() ) 
    {
        //注销字体
        unregister_all_font_operation();
        DBG_ERROR("encode init error\n");
        return ret;
    }

    //显示初始化
    if (disp_init() )
    {
        DBG_ERROR("disp init error\n");
        goto goto_error2;
    }

    //图片解码初始化
    if (pic_init() )
    {       
        DBG_ERROR("pic init error\n");
        goto goto_error2;
    }
    
    //打开指定的显示设备
    if (open_display_by_name(booksetting.displayobj) )
    {
        DBG_ERROR("open_display_by_name error\n");
        goto goto_error2;
    }
    
    //获得显示操作集指针
    pdisp = get_display_operations();
    if (pdisp == NULL)
    {
        DBG_ERROR("cur display operations is null\n");
        goto goto_error1;
    }
    
    //输入事件初始化
    if (input_init(pdisp->xres, pdisp->yres) )
    {
        DBG_ERROR("input init error\n");
        goto goto_error1;
    }

    //输入设备打开
    if (input_open() )
    {
        DBG_ERROR("input open error\n");
        goto goto_error1;
    }

    //页面初始化
    if (page_init() )
    {
        DBG_ERROR("page init error\n");
        goto goto_error1;
    }
    //如果给定了参数，返回显示设备
    if (ppdisp)
    {
        *ppdisp = pdisp;
    }
    return 0;
    
goto_error1:
    disp_close_all_device();
goto_error2:
    //注销文字解码
    del_all_font_from_encode();
    //注销字体
    unregister_all_font_operation();
    return ret;
}

/*****************************************************************************
* Function     : thread_control
* Description  : 创建控制线程
* Input        : void* arg  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
void* thread_control(void* arg)
{
    struct input_event in_event;
    struct disp_operation* pdisp_ops = (struct disp_operation *)arg;

    //显示设备必须要存在
    if (pdisp_ops == NULL)
    {
        DBG_ERROR("must exit one display device\n");
        pthread_exit((void *)-1);
    }

    //清屏幕
    if ( (pdisp_ops->clean_screen) && (pdisp_ops->show_flush) )
    {
        pdisp_ops->clean_screen(COLOR_BACKGROUND, NULL, 0);
        pdisp_ops->show_flush(NULL, 0);
    }
#if 0    
    //显示首页
    if ( (pmain_ops = get_page_operations_by_name("main") ) == NULL)
    {
        DBG_ERROR("Should exit main page\n");
        pthread_exit((void *)-1);
    }
    
    //显示主界面
    show_specify_page(pmain_ops, pdisp_ops);
#endif    
    if (show_specify_page_by_name("main", pdisp_ops) == -1)
    {
        DBG_ERROR("show main page error\n");
        pthread_exit((void *)-1);
    }
    
    
    while (1)
    {
        get_input_event(&in_event);   //获取输入事件
        deal_input_event(&in_event, pdisp_ops);  //处理输入事件
    }
    pthread_exit((void *)0);
}

// ./digitphoto <freetype_font_file>
int main(int argc, char **argv)
{
    int ret = -1;
    void *tret = NULL;
    struct disp_operation* pdisp;
    struct timeval time;
    pthread_t control_tid;

    /* 设置SIGSEGV信号的处理函数 */
    signal(SIGSEGV, DebugBacktrace);
    
    if (argc != 2)
    {
        print_usage(argv[0]);
        return ret;
    }
    
    booksetinit();  //参数初始化

    //拷贝freetype字库
    strncpy(booksetting.freetypefile, argv[1], FILE_MAX_LEN - 1);
    booksetting.freetypefile[FILE_MAX_LEN] = '\0';
    
    //解析命令行参数
    if (parsecmdline(argc, argv, "ls:f:h:") )
    {
        print_usage(argv[0]);
        return ret;
    }
 
    if (target_init(&pdisp) == -1)
    {
        DBG_ERROR("target init error\n");
        return -1;
    }
    //处理命令行参数
    ret = handlecmdline(argv[optind]);
    if (ret)
    {
        goto goto_done;
    }

//    //打开显示的文件
//    if (opentextfile(booksetting.showfile) )
//        return -1;

    //设置显示字体属性
    printf("hzkfile %s,freetype %s, fontsize %d\n", booksetting.hzkfile, booksetting.freetypefile, booksetting.fontsize);
//    if (set_font_details(booksetting.hzkfile, booksetting.freetypefile, booksetting.fontsize) )
//        return -1;

    if (open_all_font(booksetting.hzkfile, booksetting.freetypefile, booksetting.fontsize) )
        goto goto_error1;
    
    //创建主控线程
    if (pthread_create(&control_tid, NULL, thread_control, (void *)pdisp) == -1)
        goto goto_error1;
    
    while (1)
    {
        pthread_join(control_tid, &tret);
        printf("thread control exit code %d\n", (int)tret);
        exit(0);
    }
    return 0;

goto_done:     
goto_error1:
    disp_close_all_device();
goto_error2:
    //注销文字解码
    del_all_font_from_encode();
    //注销字体
    unregister_all_font_operation();
    return ret;
}

