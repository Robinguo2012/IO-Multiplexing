//
//  socket_wrap.h
//  Socket demo 01
//
//  Created by robin on 2019/4/4.
//  Copyright © 2019 robin. All rights reserved.
//

#ifndef socket_wrap_h
#define socket_wrap_h

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BACKLOG 1024

int open_listenfd(char *port);


#endif /* socket_wrap_h */
