#ifndef __FILE_H__
#define __FILE_H__

#include <dirent.h>

typedef enum 
{
    FILE_TYPE_UNKOWN,   // 0:未知类型
    FILE_TYPE_DIR,      // 1:目录类型
    FILE_TYPE_FILE,     // 2:普通文件
}FILE_TYPE;

struct file_dirent
{
    char filename[256]; 
    unsigned char filetype;   
};

/*****************************************************************************
* Function     : is_dir
* Description  : 判断是否是一个目录
* Input        : const char* filename  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月5日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int is_dir(const char *path, const char* filename);

/*****************************************************************************
* Function     : is_regfile
* Description  : 是否是一个常规文件
* Input        : const char* filename  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月5日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int is_regfile(const char *path, const char* filename);

/*****************************************************************************
* Function     : get_dir_context
* Description  : 获取一个目录的内容
* Input        : const char* dirp               ：目录名
*                struct file_dirent **ppdirent  ：存储目录的内容
*                unsigned int *pnum             ：内容个数
* Output       ：
* Return       : 
* Note(s)      : 此函数会分配目录的内容，使用完后需要释放
* Histroy      : 
* 1.Date       : 2018年1月6日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int get_dir_context(const char* dirp, struct file_dirent **ppdirent, unsigned int *pnum);

#endif //__FILE_H__

