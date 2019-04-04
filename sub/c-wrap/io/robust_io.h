//
//  robust_io.h
//  UNIX IO
//
//  Created by robin on 2019/4/1.
//  Copyright © 2019 robin. All rights reserved.
//

#ifndef robust_io_h
#define robust_io_h

#include <stdio.h>

#define RIO_BUFSIZE 8192

typedef struct {
    int rio_fd;
    int rio_cnt;  // 缓存中剩余未读的
    char *rio_bufptr;
    char rio_buf[RIO_BUFSIZE];
}rio_t;

void rio_readinitb(rio_t *rio,int fd);

ssize_t rio_writen(int fd,void *usrbuf,size_t n);

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);

/*按行读取*/
ssize_t rio_readlineb(rio_t *rp,void *usrbuf, size_t maxlen);

ssize_t rio_readnb(rio_t *rp,void *usrbuf, size_t n);

#endif /* robust_io_h */
