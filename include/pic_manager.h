#ifndef __PIC_MANAGER_H__
#define __PIC_MANAGER_H__

#include <list.h>
#include "config.h"

struct file_desc
{
    int fd;         /* 文件描述符 */
    unsigned int filelen;    /* 文件大小 */ 
    unsigned char *pfilebuf; /* 使用mmap函数映射文件得到的内存 */
};

/* 图片的象素数据 */
struct pic_data
{
	unsigned int width;                  /* 宽度: 一行有多少个象素 */
	unsigned int height;                 /* 高度: 一列有多少个象素 */
	unsigned int bpp;                    /* 一个象素用多少位来表示 */
	unsigned int linebytes;              /* 一行数据有多少字节 */
	unsigned int totalbytes;             /* 所有字节数 */ 
	unsigned char *pixeldata;   /* 象素数据存储的地方 */
};

struct pic_operations 
{
    const char* name;
    int (*is_support)(struct file_desc *pfile);
    int (*get_pic_data)(struct file_desc* pfile, struct pic_data *pic);
    void (*free_pic_data)(struct pic_data *pic);
    struct list_head list;
};

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
int open_one_pic(const char* name, struct file_desc *pfile_desc);

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
void close_one_pic(struct file_desc *pfile_desc);

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
struct pic_operations* get_pic_operations_by_name(const char *name);

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
struct pic_operations* select_encode_for_pic(struct file_desc* pfile);

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
int register_pic_operation(struct pic_operations *pops);

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
int pic_init(void);

#endif //__PIC_MANAGER_H__

