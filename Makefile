CC = gcc

all: reactor server 

server: reactor_server.c reactor_server.h
	$(CC) -o react_server reactor_server.c -L. ./st_reactor.so -lpthread 

reactor: reactor.c reactor.h
	$(CC) -c -fPIC reactor.c -o reactor.o
	$(CC) -shared -o st_reactor.so reactor.o

clean:
	rm -rf *.o react_server *.so 
