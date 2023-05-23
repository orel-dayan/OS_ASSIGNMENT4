#include "reactor.h"
#include <stdlib.h>

p_reactor_t createReactor(int size, int listen)
{
    p_reactor_t reactor = (p_reactor_t)malloc(sizeof(reactor_t)); // allocate memory for the reactor
    reactor->handlers = (p_handler_t *)malloc(size * sizeof(p_handler_t)); // allocate memory for the handlers
    reactor->fds = (struct pollfd *)malloc(size * sizeof(struct pollfd)); // allocate memory for the pollfd
    reactor->counter = 0; // set the counter to 0
    reactor->size = size; // set the size
    reactor->isRunning = 0; // set the isRunning to 0
    reactor->listenFD = listen; // set the listenFD
    return reactor;
}

void startReactor(p_reactor_t reactor)
{
    if (reactor != NULL) // if the reactor is not null
    {
        reactor->isRunning = 1;
        pthread_create(&reactor->thread, NULL, runReactor, reactor);
    }
}

void stopReactor(p_reactor_t reactor)
{
    if (reactor != NULL)
    {
        reactor->isRunning = 0; // set the isRunning to 0
        pthread_cancel(reactor->thread); // cancel the thread
        pthread_join(reactor->thread, NULL); // join the thread
        pthread_detach(reactor->thread); // detach the thread

    }
}


void *runReactor(void *arg)
{
    p_reactor_t reactor = (p_reactor_t)arg; // cast the arg to a reactor
    while (reactor->isRunning) // while the reactor is running
    {
        int events = poll(reactor->fds, reactor->counter, -1);
        if (events > 0) // if there are events
        {
            int count = reactor->counter; // get the counter from the reactor
            for (int i = 0; i < count; i++) // loop through the handlers
            {
                if (reactor->fds[i].revents & POLLIN) // if there is data to read
                    reactor->handlers[i]->handler(reactor, reactor->fds[i].fd, reactor->handlers[i]->arg);
                
            }
        }
    }
}

void addFd(p_reactor_t reactor, int fd, handler_t handler)
{
    if (reactor->counter < reactor->size) // if the counter is less than the size
    {
        reactor->handlers[reactor->counter] = (p_handler_t)malloc(sizeof(handler_t)); // allocate memory for the handler
        reactor->handlers[reactor->counter]->handler = handler.handler; // set the handler
        reactor->handlers[reactor->counter]->arg = handler.arg; // set the arg
        reactor->fds[reactor->counter].fd = fd; // set the fd
        reactor->fds[reactor->counter].events = POLLIN; // set the events
        reactor->counter += 1; // increment the counter by 1 because we added a handler
    }
    else // if the counter is greater than the size
    {
        size_t newSize = reactor->size * 2; // double the size
        reactor->handlers = (p_handler_t *)realloc(reactor->handlers, newSize * sizeof(p_handler_t)); // reallocate memory for the handlers
        reactor->fds = (struct pollfd *)realloc(reactor->fds, newSize * sizeof(struct pollfd)); // reallocate memory for the pollfd
        reactor->size = newSize; // set the size to the new size
        addFd(reactor, fd, handler); // recursively call addFd to add the handler
    }
}

void waitFor(p_reactor_t reactor)
{
    if (reactor && !reactor->isRunning)
    {
        pthread_join(reactor->thread, NULL); // join the thread

    }
}

int findFdIndex(p_reactor_t reactor, int fd)
{
    if(reactor == NULL) // handle invalid input
    {
        return -1;
    }

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
    int index = findFdIndex(reactor, fd);
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
        if (reactor->isRunning) // if the reactor is running
            stopReactor(reactor);

        if (reactor->handlers != NULL)
        {
            for (int i = 0; i < reactor->counter; i++)
                free(reactor->handlers[i]);

            free(reactor->handlers);
        }
        if (reactor->fds != NULL) // if the reactor is not null
            free(reactor->fds); // free the reactor

        free(reactor); // free the reactor
    }
}
