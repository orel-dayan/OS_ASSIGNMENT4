#include "reactor.h"
#include <stdlib.h>

p_reactor_t createReactor(int size, int listen)
{
    p_reactor_t reactor = (p_reactor_t)malloc(sizeof(reactor_t));
    reactor->handlers = (p_handler_t *)malloc(size * sizeof(p_handler_t));
    reactor->fds = (struct pollfd *)malloc(size * sizeof(struct pollfd));
    reactor->counter = 0;
    reactor->size = size;
    reactor->isRunning = 0;
    reactor->listenFD = listen;
    return reactor;
}

void startReactor(p_reactor_t reactor)
{
    if (reactor != NULL)
    {
        reactor->isRunning = 1;
        pthread_create(&reactor->thread, NULL, runReactor, reactor);
    }
}

void stopReactor(p_reactor_t reactor)
{
    if (reactor != NULL)
    {
        reactor->isRunning = 0;
        pthread_cancel(reactor->thread);
        pthread_join(reactor->thread, NULL);
        pthread_detach(reactor->thread);

    }
}


void *runReactor(void *arg)
{
    p_reactor_t reactor = (p_reactor_t)arg;
    while (reactor->isRunning)
    {
        int events = poll(reactor->fds, reactor->counter, -1);
        if (events > 0)
        {
            int currCount = reactor->counter;
            for (int i = 0; i < currCount; i++)
            {
                if (reactor->fds[i].revents & POLLIN)
                {
                    reactor->handlers[i]->handler(reactor, reactor->fds[i].fd, reactor->handlers[i]->arg);
                }
            }
        }
    }
}

void addFd(p_reactor_t reactor, int fd, handler_t handler)
{
    if (reactor->counter < reactor->size)
    {
        reactor->handlers[reactor->counter] = (p_handler_t)malloc(sizeof(handler_t));
        reactor->handlers[reactor->counter]->handler = handler.handler;
        reactor->handlers[reactor->counter]->arg = handler.arg;
        reactor->fds[reactor->counter].fd = fd;
        reactor->fds[reactor->counter].events = POLLIN;
        reactor->counter += 1;
    }
    else
    {
        size_t newSize = reactor->size * 2;
        reactor->handlers = (p_handler_t *)realloc(reactor->handlers, newSize * sizeof(p_handler_t));
        reactor->fds = (struct pollfd *)realloc(reactor->fds, newSize * sizeof(struct pollfd));
        reactor->size = newSize;
        addFd(reactor, fd, handler);
    }
}

void waitFor(p_reactor_t reactor)
{
    if (reactor && !reactor->isRunning)
    {
        pthread_join(reactor->thread, NULL);
    }
}

int findFd(p_reactor_t reactor, int fd)
{
    for (int i = 0; i < reactor->counter; i++)
    {
        if (reactor->fds[i].fd == fd)
        {
            return i;
        }
    }
    return -1;
}

void deleteFd(p_reactor_t reactor, int fd)
{
    int index = findFd(reactor, fd);
    if (index != -1)
    {
        free(reactor->handlers[index]);
        for (int i = index; i < reactor->counter - 1; i++)
        {
            reactor->handlers[i] = reactor->handlers[i + 1];
            reactor->fds[i] = reactor->fds[i + 1];
        }
        reactor->counter -= 1;
    }
}

void deleteReactor(p_reactor_t reactor)
{
    if (reactor != NULL)
    {
        if (reactor->isRunning)
            stopReactor(reactor);

        if (reactor->handlers != NULL)
        {
            for (int i = 0; i < reactor->counter; i++)
                free(reactor->handlers[i]);

            free(reactor->handlers);
        }
        if (reactor->fds != NULL)
            free(reactor->fds);

        free(reactor);
    }
}
