#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "libthreadpro.h"
#include "file.h"
#include "debug.h"

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
int is_dir(const char *path, const char* filename)
{
    struct stat filetstat;
    char buf[256];
    
    if ( (path == NULL) || (filename == NULL) )
    {
        return 0;
    }
    //构造文件的绝对路径
    strncpy(buf, path, sizeof(buf) - 1);  //此函数不一定会以\0结束
    buf[255] = '\0';
    strncat(buf, filename, sizeof(buf) - (strlen(buf) + 1) );

    if (stat(buf, &filetstat) == -1)
    {
        DBG_ERROR("get file %s error\n", buf);
        return 0;
    }
    //判断是否是一个目录
    if (S_ISDIR(filetstat.st_mode) )
    {
        return 1;
    }
    return 0;
}

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
int is_regfile(const char *path, const char* filename)
{
    struct stat filetstat;
    char buf[256];
    
    if (filename == NULL)
    {
        return 0;
    }

    strncpy(buf, path, sizeof(buf) - 1);
    buf[255] = '\0';
    strncat(buf, filename, sizeof(buf) - strlen(buf) - 1);
    if (stat(buf, &filetstat) == -1)
    {
        DBG_ERROR("get file %s error\n", buf);
        return 0;
    }
    //判断是否是一个常规文件
    if (S_ISREG(filetstat.st_mode) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

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
int get_dir_context(const char* dirp, struct file_dirent **ppdirent, unsigned int *pnum)
{
    struct dirent **namelist;
    struct file_dirent *pdirent;
    int n, i, j;
    
    if ( (dirp == NULL) || (ppdirent == NULL) )
    {
        return -1;
    }
    
    //获取指定目录的内容，使用alphasort进行排序
    if ( (n = scandir(dirp, &namelist, NULL, alphasort) ) == -1)
    {
        return -1;
    }
    if (pnum)
    {
        *pnum = 0;
    }
    
    if (n == 2)
    {
        for (i = 0; i < n; i++)
        {
            free_memory(namelist[i]);
        }
        free_memory(namelist);
        return 0;
    }

    //申请n-2个内存，过滤.跟..目录
    if ( (pdirent = (struct file_dirent *)malloc(sizeof(struct file_dirent) * (n - 2) ) ) == NULL)
    {
        for (i = 0; i < n; i++)
        {
            free_memory(namelist[i]);
        }
        free_memory(namelist);
        return -1;
    }

    //根据读取的文件内容进行分类，目前只分目录跟普通文件，非目录的文件都算普通文件
    //先整理目录后再整理文件
    for (i = 0, j = 0; i < n; i++)
    {
        //过滤.跟..目录
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..") )
        {
            free_memory(namelist[i]);
        }
        else
        {
            //其他文件，按照目录、常规文件进行分类
            if (is_dir(dirp, namelist[i]->d_name) )
            {
                //复制目录名字
                strcpy(pdirent[j].filename, namelist[i]->d_name);
                pdirent[j].filetype = FILE_TYPE_DIR; //目录属性
                free_memory(namelist[i]);
                j++;
            }
        }
    }
    //整理常规文件
    for (i = 0; i < n; i++)
    {
        if (namelist[i] == NULL)
            continue;

        //其他文件，按照目录、常规文件进行分类
        //整理常规文件
        if (is_regfile(dirp, namelist[i]->d_name) )
        {
            //复制常规文件名字
            strcpy(pdirent[j].filename, namelist[i]->d_name);
            pdirent[j].filetype = FILE_TYPE_FILE; //常规文件属性
            j++;
        }
        //其他非常规文件直接释放
        free_memory(namelist[i]);
    }
    //释放namelist的内存
    free_memory(namelist);

    *ppdirent = pdirent; //返回存储内容得地址
    
    if (pnum)
    {
        *pnum = j;   //返回目录、常规文件个数
    }
    return 0;
}

