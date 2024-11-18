#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define MAX_TOPICS 100
#define MAX_SUBSCRIBERS 100

typedef struct {
  char topic_name[BUF_SIZE];
  int subscribers[MAX_SUBSCRIBERS];
} Topic;

Topic topics[MAX_TOPICS];
int topic_count = 0;
pthread_mutex_t topics_lock;

