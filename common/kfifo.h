/*
 * A simple kernel FIFO implementation.
 *
 * Copyright (C) 2004 Stelian Pop <stelian@popies.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#ifndef __KFIFO_H__
#define __KFIFO_H__

#include "pthread.h"

//判断x是否是2的倍数
#ifndef is_power_if_2
#define is_power_if_2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

struct kfifo {
	unsigned char *buffer;	/* the buffer holding the data 保存数据的buf*/
	unsigned int size;	    /* the size of the allocated buffer 分配的buf大小*/
	unsigned int in;	    /* data is added at offset (in % size) 写入地址*/
	unsigned int out;	    /* data is extracted from off. (out % size) 读出地址*/
	pthread_mutex_t mutex;	/* protects concurrent modifications 互斥锁*/
};

unsigned int roundup_pow_of_two(const unsigned int x);

struct kfifo *kfifo_init(unsigned char *buffer, unsigned int size);

struct kfifo *kfifo_alloc(unsigned int size);

void kfifo_free(struct kfifo *fifo);

unsigned int kfifo_put(struct kfifo *fifo, unsigned char *buffer, unsigned int len);

unsigned int kfifo_get(struct kfifo *fifo, unsigned char *buffer, unsigned int len);

void kfifo_reset(struct kfifo *fifo);

unsigned int kfifo_len(struct kfifo *fifo);

#endif //__KFIFO_H__
