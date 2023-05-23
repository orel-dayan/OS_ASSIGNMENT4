#ifndef ST_REACTOR_H
#define ST_REACTOR_H

#include <sys/poll.h>
#include <pthread.h>

typedef struct st_reactor *p_reactor_t;

/**
 * @brief Struct handler_t is a handler struct that contains a handler function pointer and an argument.
 * The handler function pointer is a function pointer to a handler function.
 * The argument is an argument to the handler function.
 *
 */

typedef struct
{
	void (*handler)(p_reactor_t reactor, int fd, void *arg);
	void *arg;
} handler_t, *p_handler_t;

/**
 * @brief Struct st_reactor is a reactor struct that contains all the information needed to run a reactor.
 * It contains a handler array, a fd array, a counter, a size, a status, a listener fd and a thread.
 * The handler array is an array of handler_t pointers. Each handler_t contains a handler function pointer and an argument.
 * The fd array is an array of pollfd structs. Each pollfd struct contains a fd and a revents.
 * The counter is the number of fds in the reactor.
 * The size is the size of the handler array and the fd array.
 * The status is the status of the reactor. 1 means running, 0 means stopped.
 * The listener fd is the fd of the listener.
 * The thread is the reactor thread.
 */

typedef struct st_reactor
{
	p_handler_t *handlers; // handler array
	struct pollfd *fds;	   // fd array
	int counter;
	int size;		  // array size
	int isRunning;	  // reactor status
	int listenFD;	  // listener fd
	pthread_t thread; // reactor thread. mutex to make it thread safe
} reactor_t, *p_reactor_t;

p_reactor_t createReactor(int, int);	 // create reactor
void stopReactor(p_reactor_t);			 // stop reactor
void startReactor(p_reactor_t);			 // start reactor
void *runReactor(void *);				 // run reactor
void addFd(p_reactor_t, int, handler_t); // add fd to reactor
void waitFor(p_reactor_t);				 // wait for reactor to stop
void deleteReactor(p_reactor_t);		 // free memory and stop reactor
void deleteFd(p_reactor_t, int);		 // delete fd from reactor
int findFdIndex(p_reactor_t, int);		 // return fd index in reactor

#endif // ST_REACTOR_H
