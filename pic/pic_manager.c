#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include "pic_manager.h"
#include "pic_bmp.h"
#include "debug.h"

static struct list_head pic_list_head = LIST_HEAD_INIT(pic_list_head);
static pthread_rwlock_t list_head_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/*****************************************************************************
* Function     : open_one_pic
* Description  : 打开一个图片，成功后返回一个虚拟内存
* Input        : const char* name       :图片名字
*                void **const ppic_map  :保存图片的虚拟内存地址
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int open_one_pic(const char* name, struct file_desc *pfile_desc)
{
    int file_fd;
    struct stat file_stat;
    
    if ( (name == NULL) || (pfile_desc == NULL) )
    {
        return -1;
    }

    //以只读模块打开图片文件
    if  ( (file_fd = open(name, O_RDONLY) ) == -1)
    {
        DBG_ERROR("open pic %s error\n", name);
        return -1;
    }
    //获取文件系统
    if (fstat(file_fd, &file_stat) == -1)
    {
        close(file_fd);
        DBG_ERROR("fstat pic %s error\n", name);
        return -1;
    }
    //文件虚拟映射
    if ( (pfile_desc->pfilebuf = mmap(NULL, file_stat.st_size, PROT_READ, MAP_SHARED, file_fd, 0) )== (void*)-1)
    {
        close(file_fd);
        DBG_ERROR("fstat pic %s error\n", name);
        return -1;
    }
    pfile_desc->fd = file_fd;
    pfile_desc->filelen = file_stat.st_size; 
    return 0;
}

/*****************************************************************************
* Function     : close_one_pic
* Description  : 关闭一张图片
* Input        : struct file_desc *pfile_desc  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
void close_one_pic(struct file_desc *pfile_desc)
{
    if ( (pfile_desc == NULL) || (pfile_desc->fd < 0) || !pfile_desc->filelen || (pfile_desc->pfilebuf == NULL) )
    {
        return;
    }
    //释放虚拟内存
    munmap(pfile_desc->pfilebuf, pfile_desc->filelen);
    close(pfile_desc->fd);
}

/*****************************************************************************
* Function     : get_pic_operations_by_name
* Description  : 指定名字获取图片解码器
* Input        : const char *name  
* Output       ：
* Return       : struct
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct pic_operations* get_pic_operations_by_name(const char *name)
{
    struct list_head *plist;
    struct pic_operations *ptemp;
    
    if (name == NULL)
        return NULL;
    
    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &pic_list_head)
    {
        ptemp = list_entry(plist, struct pic_operations, list);
        if (ptemp && ptemp->name)
        {
            if (!strcmp(ptemp->name, name))
            {
                pthread_rwlock_unlock(&list_head_rwlock);
                return ptemp;
            }
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return NULL;
}

/*****************************************************************************
* Function     : select_encode_for_pic
* Description  : 为一张图片选择合适的解码器
* Input        : struct file_desc* pfile  
* Output       ：
* Return       : NULL ：没有找到合适的解码器  。 其他值：返回对应的解码器地址
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月28日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
struct pic_operations* select_encode_for_pic(struct file_desc* pfile)
{
    struct list_head *plist;
    struct pic_operations *ptemp;
    
    if ( (pfile == NULL) || (pfile->pfilebuf == NULL) || !pfile->filelen)
    {
        DBG_ERROR("param is error\n");
        return NULL;
    }

    pthread_rwlock_rdlock(&list_head_rwlock);
    list_for_each(plist, &pic_list_head)
    {
        ptemp = list_entry(plist, struct pic_operations, list);
        if (ptemp && ptemp->is_support)
        {
            //判断是否支持
            if (ptemp->is_support(pfile) )
            {
                DBG_INFO("pic encode name %s\n", ptemp->name);
                pthread_rwlock_unlock(&list_head_rwlock);
                return ptemp;
            }
        }
    }
    pthread_rwlock_unlock(&list_head_rwlock);
    return NULL;
}

/*****************************************************************************
* Function     : register_pic_operation
* Description  : 注册一个图片解码
* Input        : struct pic_operations *pops  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2017年12月27日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int register_pic_operation(struct pic_operations *pops)
{
    if (pops == NULL)
    {
        return -1;
    }
    pthread_rwlock_wrlock(&list_head_rwlock);  //获取写锁
    list_add_tail(&pops->list, &pic_list_head);
    pthread_rwlock_unlock(&list_head_rwlock);  //释放写锁
    return 0;
}

/*****************************************************************************
* Function     : pic_init
* Description  : 图片解码初始化
* Input        : void  
* Output       ：
* Return       : 
* Note(s)      : 
* Histroy      : 
* 1.Date       : 2018年1月2日
*   Author     : Xieyb
*   Modify     : Create Function
*****************************************************************************/
int pic_init(void)
{
    int error = 0;
    
    error |= pic_bmp_init();
    return error;
}

