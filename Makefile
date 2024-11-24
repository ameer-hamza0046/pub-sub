CC = gcc
CFLAGS = -lpthread

make all:
	$(CC) server.c -o ./server.out $(CFLAGS)
	$(CC) load-balancer.c -o ./load-balancer.out $(CFLAGS)

server: server.c
	$(CC) server.c -o ./server.out $(CFLAGS)
	
load-balancer: load-balancer.c
	$(CC) load-balancer.c -o ./load-balancer.out $(CFLAGS)

clean:
	rm -f *.out