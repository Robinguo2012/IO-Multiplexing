//
//  socket_wrap.c
//  Socket demo 01
//
//  Created by robin on 2019/4/4.
//  Copyright © 2019 robin. All rights reserved.
//

#include "socket_wrap.h"

int open_listenfd(char *port) {
    /* hints 可选参数提供对列表更好的控制*/
    struct addrinfo *p,*listp, hints;
    int listenfd,optval = 1;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    
    getaddrinfo(NULL, port, &hints, &listp);
    
    for (p = listp; p; p = listp->ai_next) {
        
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) continue;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(int));
        
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
            printf("bind success\n");
            break;
        }
        close(listenfd);
    }
    
    freeaddrinfo(listp);
    
    if (!p) { // No Address worked
        return -1;
    }
    
    if (listen(listenfd, BACKLOG) < 0) {
        printf("listen success\n");
        close(listenfd);
        return -1;
    }
    
    return listenfd;
}


/*和服务器建立连接的函数，可重入和协议无关*/
int open_clientfd(char *hostname,char *port) {
    
    int client_fd;
    
    /* Get a list of pontential server address list*/
    struct addrinfo *p, *listp, hints;
    //    char buf[MAXLINE];
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; /*open a connection, 长连接*/
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_flags |= AI_ADDRCONFIG;
    
    getaddrinfo(hostname, port, &hints, &listp);
    
    /*Walk the list the one that we can successfully connect to */
    for (p = listp; p; p = listp->ai_next) {
        if ((client_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))<0) continue; /*socket failed, try next*/
        
        if (connect(client_fd, p->ai_addr, p->ai_addrlen) != -1) {
            break; /* success */
        }
        close(client_fd);
    }
    
    /*clean up*/
    freeaddrinfo(listp);
    
    if (!p) {
        return -1; // All connection failed;
    } else
        return client_fd;
}

