
#include "reactor_server.h"

p_reactor_t p_reactor;

int main(void)
{
	// set signal handler
	signal(SIGINT, signalHandler); 								// set the signal handler for SIGINT (Ctrl-C)
	signal(SIGTERM, signalHandler); 							// set the signal handler for SIGTERM (kill) 
	int listener;												// Listening socket descriptor
	int new_fd;													// new accepted socket descriptor
	struct sockaddr_storage remoteaddr; // Client address
	socklen_t addrlen;
	char ip[INET6_ADDRSTRLEN]; // Client IP address

	// Set up and get a listening socket
	printf("[INFO] Setting up listening socket on port %s\n", PORT);
	listener = getListenerSocket();

	if (listener == -1)
	{
		fprintf(stderr, "error getting listening socket\n");
		exit(EXIT_FAILURE);
	}

	// Create reactor
	p_reactor = createReactor(5, listener); // 5 is the max number of clients , can be changed

	handler_t cHandler;
	cHandler.arg = NULL;
	cHandler.handler = (void (*)(struct st_reactor *, int, void *)) & connectHandler;
	addFd(p_reactor, listener, cHandler);

	startReactor(p_reactor);
	printf("[INFO] Reactor started\n");

	while (p_reactor->isRunning)
	{
		sleep(1); // sleep for 1 second
	}

	return 0;
}

// signal handler
void signalHandler(int signal)
{
	if (signal == SIGINT || signal == SIGTERM) // if signal is SIGINT or SIGTERM
	{
		deleteReactor(p_reactor); // delete the reactor
		exit(EXIT_SUCCESS); 			// exit the program
	}
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// from Beej's Guide to Network Programming
int getListenerSocket(void) 
{
	int listener; // Listening socket descriptor
	int yes = 1;	// For setsockopt() SO_REUSEADDR, below
	int rv;

	struct addrinfo hints, *ai, *p;

	// Get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
	{
		fprintf(stderr, "server: %s\n", gai_strerror(rv));
		exit(EXIT_FAILURE);
	}

	for (p = ai; p != NULL; p = p->ai_next)
	{
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0)
		{
			continue;
		}

		// Lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(listener);
			continue;
		}

		break;
	}

	freeaddrinfo(ai); // All done with this

	if (p == NULL) // If we got here, it means we didn't get bound
	{
		return -1;
	}

	// Listen
	if (listen(listener, MAX_CLIENTS) == -1)
	{
		return -1;
	}

	return listener;
}

void connectHandler(p_reactor_t reactor, void *arg) 
{
	struct sockaddr_storage remote_addr; // Client address
	socklen_t addr_len;
	char IP[INET6_ADDRSTRLEN];
	int new_fd; // Newly accepted socket descriptor

	addr_len = sizeof remote_addr;
	new_fd = accept(reactor->listenFD, (struct sockaddr *)&remote_addr, &addr_len); // Accept the incoming connection
	if (new_fd == -1)
	{
		printf("Error accepting new connection\n");
	}
	else
	{
		printf("[INFO] new connection from %s on "
					 "socket %d\n",
					 inet_ntop(remote_addr.ss_family,
										 get_in_addr((struct sockaddr *)&remote_addr),
										 IP, INET6_ADDRSTRLEN),
					 new_fd);
		handler_t cHandler;
		cHandler.arg = NULL;
		cHandler.handler = &clientHandler;
		addFd(reactor, new_fd, cHandler);
	}
}
void clientHandler(p_reactor_t reactor, int client_fd, void *arg) 
{
	char buf[BUFFER] = {0};
	int nbytes;

	nbytes = recv(client_fd, buf, BUFFER, 0);
	if (nbytes <= 0)
	{
		if (nbytes == 0)
		{
			printf("[INFO] socket %d hung up\n", client_fd);
		}
		else
		{
			printf("error receiving message from socket %d\n", client_fd);
		}
		close(client_fd);
		deleteFd(reactor, client_fd);
	}
	else
	{

		printf("[INFO] socket %d got message: %s", client_fd, buf);
		for (int i = 0; i < reactor->counter; i++)
		{
			if (reactor->fds[i].fd != client_fd && reactor->fds[i].fd != reactor->listenFD)
			{
				if (send(reactor->fds[i].fd, buf, nbytes, 0) == -1)
				{
					printf("error sending message to socket %d\n", reactor->fds[i].fd);
				}
			}
		}
		printf("[INFO] message sent to all clients \n");
	}
}
