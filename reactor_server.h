#ifndef REACTOR_SERVER_H
#define REACTOR_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <netdb.h>


#include "reactor.h"

#define PORT "9034" // beej's guide port
#define MAX_CLIENTS 1024*8 // max number of clients
#define BUFFER 256   // max buffer size

void signalHandler(int);
void *get_in_addr(struct sockaddr *);
int getListenerSocket(void);
void connectHandler(p_reactor_t, void *);
void clientHandler(p_reactor_t, int, void *);

#endif // REACTOR_SERVER_H
