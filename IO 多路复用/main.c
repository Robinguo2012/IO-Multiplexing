//
//  main.c
//  IO 多路复用
//
//  Created by robin on 2019/4/3.
//  Copyright © 2019 robin. All rights reserved.
//

#include <stdio.h>
#include "robust_io.h"
#include "sock_func.h"

#define MAXLINE 1024

typedef struct  {
    int nready;
    fd_set read_set;
    fd_set ready_set;
    int maxi;
    int maxfd;
    rio_t clientrio[__DARWIN_FD_SETSIZE];
    int clientfd[__DARWIN_FD_SETSIZE];
}pool;

int byte_cnt = 0;

void init_pool(int fd,pool *pool);
void add_client(int connfd,pool *p);
void check_client(pool *p);
void apperror(char *err);

int main(int argc, const char * argv[]) {
    
    /*IO多路复用是做并发事件驱动（event-driven）程序的基础。在事件驱动程序中，某些事件会导致流向前推进，一般的思路是将逻辑流模型化为状态机。状态机就是一组状态（state），输入事件（input-event）和转移(transition)，其中转移是将输入事件和状态映射到状态。每一个转移是将一个（输入事件，状态）映射到一个输出状态。*/
    int listenfd,connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    
    static pool pool;
    
    listenfd = open_listenfd("8088");
    init_pool(listenfd, &pool);
    
//    __DARWIN_FD_SET(STDIN_FILENO, &pool.read_set);
    
    while (1) {
        pool.ready_set = pool.read_set;
        pool.nready = select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);
        if (__DARWIN_FD_ISSET(listenfd, &pool.ready_set)) {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
            printf("accetp connection from fd %d\n",connfd);
            add_client(connfd, &pool);
        }
        
        // echo message
        check_client(&pool);
    }
    
    return 0;
}

void add_client(int connfd,pool *p) {
    int i=0;
    p->nready--;
    for (i=0; i<__DARWIN_FD_SETSIZE; i++) {
        if (p->clientfd[i] < 0 && connfd > 0) {
            p->clientfd[i] = connfd;
            rio_readinitb(&p->clientrio[i], connfd);
            __DARWIN_FD_SET(connfd, &p->read_set);
            
            // update the max file descriptor
            if (p->maxfd < connfd) {
                p->maxfd = connfd;
            }
            
            if (i>p->maxi) {
                p->maxi = i;
            }
            break;
        }
        
        if (i == __DARWIN_FD_SETSIZE) {
            apperror("Could not find a solt");
        }
    }
}

void init_pool(int listenfd,pool *pool)
{
    pool->maxi = -1;
    int i;
    for (i=0; i<__DARWIN_FD_SETSIZE; i++) {
        pool->clientfd[i] = -1;
    }
    pool->maxfd = listenfd;
    __DARWIN_FD_ZERO(&pool->read_set);
    __DARWIN_FD_SET(listenfd, &pool->read_set);
}

void check_client(pool *p) {
    int i,n,connfd;
    rio_t r;
    char buf[MAXLINE];
    
    for (i=0; (i <= p->maxi) && (p->nready > 0); i++) {
        connfd = p->clientfd[i];
        r = p->clientrio[i];
        
        if (connfd > 0 && (__DARWIN_FD_ISSET(connfd, &p->ready_set))) {
            // the file descriptor can read
            p->nready--;
            rio_readinitb(&r, connfd);
            if ((n = rio_readlineb(&r, buf, MAXLINE)) != 0) {
                byte_cnt += n;
                printf("Server received bytes %d (%d total) from fd %d\n",n,byte_cnt,connfd);
                // echo a text line
                rio_writen(connfd, buf, n);
            } else {
                // EOF
                close(connfd);
                __DARWIN_FD_CLR(connfd, &p->ready_set);
                p->clientfd[i] = -1;
            }
            
        }
        
    }
}

void apperror(char *err) {
    fprintf(stderr, "%s", err);
    exit(0);
}
