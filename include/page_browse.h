#ifndef __PAGE_BROWSE_H__
#define __PAGE_BROWSE_H__

struct page_browse
{
    unsigned int totalnum;      //该目录总共的内容个数
    unsigned int numperpage;    //每页的内容个数
    unsigned int pagenum;       //总页数
    unsigned int pagecol;       //一页的列数
    unsigned int pagerow;       //一页的行数
    struct disp_layout **pagedata;   //一页坐标的数据
    struct file_dirent *pdirfiledirent; //一个目录的文件信息
};

struct page_context_desc 
{
    char dir_name[256];             //路径名
    unsigned int index;             //当前目录的索引值，标识第几页
    unsigned char* pback_ground;    //当前目录的背景图片数据，不含任何的目录内容
    struct page_browse context;     //当前目录的内容
};

/*****************************************************************************
* Function     : page_browse_init
* Description  : 浏览模式界面初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int page_browse_init(void);

#endif //__PAGE_BROWSE_H__

