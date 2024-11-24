CC = gcc
CFLAGS = -lpthread

server: server.c
	$(CC) server.c -o ./server.out $(CFLAGS)
	
load-balancer: load-balancer.c
	$(CC) load-balancer.c -o ./load-balancer.out $(CFLAGS)

clean:
	rm -f *.out