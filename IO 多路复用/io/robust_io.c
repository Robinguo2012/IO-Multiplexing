//
//  robust_io.c
//  UNIX IO
//
//  Created by robin on 2019/4/1.
//  Copyright © 2019 robin. All rights reserved.
//

#include "robust_io.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void rio_readinitb(rio_t *rio,int fd) {
    rio->rio_fd = fd;
    rio->rio_cnt = 0;
    rio->rio_bufptr = rio->rio_buf;
}

ssize_t rio_writen(int fd,void *usrbuf,size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;
    
    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) < 0) {
            // EINTR 此调用被信号所中断
            if (errno == EINTR) {
                nwritten = 0;
            } else
                return -1;
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

/*
 这里的大体原理：
 
 从未知文件尺寸中读取文件到 内置的缓冲区，一次读取 RIO_BUFSIZE 的内容
 如果返回为 -1 则出错。返回 0 表示文件读取完成。 返回大于 0，则表示读取到返回值的字节数。
 然后把这个返回的字节数和要求读取的字节数比较，如果小于要求，则表示待读取的文件还没有完成，返回已读然后更新读状态结构体即可。
 这个提高了读取文件函数的健壮性，兼顾了异常情况的处理。不过这是系统级IO函数，ANSI C的内置读写函数应该也考虑到了这些问题。
 */
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;
    
    while (rp->rio_cnt <= 0) {  /* refill if buf is empty */
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,
                           sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0) {
            if (errno != EINTR) /* interrupted by sig handler return */
                return -1;
        }
        else if (rp->rio_cnt == 0)  /* EOF */
            return 0;
        else
            rp->rio_bufptr = rp->rio_buf; /* reset buffer ptr */
    }
    
    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;
    if ((unsigned)rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

/*
 // 调用系统IO会切换到内核态，频繁切换到内核态会增大CPU占用
 ssize_t rio_readlineb(rio_t *rp,void *usrbuf, size_t maxlen) {
 int n, rc;
 char c, *bufp = usrbuf;
 
 for (n=1; n<maxlen; n++) {
 if ((rc = rio_read(rp, &c, 1)) == 1) {
 *bufp++ = c;
 if (c == '\n') {
 n++;
 break;
 }
 } else if(rc == 0) {
 if (n == 1) {
 return 0; // EOF,no data to read
 } else
 break; // some data was read
 } else
 return -1; // Error
 }
 
 *bufp = 0;
 return n-1;
 }
 */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    int rc;
    unsigned int n;
    char c, *bufp = usrbuf;
    
    for (n = 1; n < maxlen; n++) {
        if ((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1)
                return 0; /* EOF, no data read */
            else
                break;    /* EOF, some data was read */
        } else
            return -1;      /* error */
    }
    *bufp = 0;
    return n;
}

ssize_t rio_readnb(rio_t *rp,void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;
    
    while (nleft > 0) {
        if ((nread = rio_read(rp, bufp, nleft)) < 0) {
            return -1; // errno set by read()
        }else if (nread == 0)
            break;
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}
