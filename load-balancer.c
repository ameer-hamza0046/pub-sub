#include "topicsAndMessages.c"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif
#define MAX_BROKERS 100
char brokers[MAX_BROKERS][BUF_SIZE];

int num_brokers, num_topics;

//============== CLIENT HANDLER AND MAIN FUNCTION =================
void *client_handler(void *arg) {
  int client_fd = *(int *)arg;
  free(arg);

  char client_buffer[BUF_SIZE];
  int ret;
  // client will send topic_index,
  // we will return broker_ip:broker_port
  while (true) {
    ret = recv(client_fd, client_buffer, BUF_SIZE, 0);
    if (ret < 0) {
      printf("recv() error\n");
      break;
    }
    if (ret == 0 || strcmp(client_buffer, "exit") == 0) {
      printf("conn_closed - client_fd: %d\n", client_fd);
      break;
    }
    int topic_index = atoi(client_buffer);
    sprintf(client_buffer, "%s", brokers[topic_index % num_brokers]);
    send(client_fd, client_buffer, strlen(client_buffer) + 1, 0);
  }
  close(client_fd);
  pthread_exit(NULL);
}

int serv_sockfd;
void handle_sigint(int sig) {
  close(serv_sockfd);
  printf("\nServer socket closed. Exiting.\n");
  exit(0);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf(
        "Usage: ./load-balancer.out <port> <ip1>:<port1> <ip2>:<port2> ...\n");
    printf("At least one IP:Port is required\n");
    return EXIT_FAILURE;
  }
  num_brokers = argc - 2;
  for (int i = 0; i < num_brokers; i++) {
    sprintf(brokers[i], "%s", argv[i + 2]);
  }
  num_topics = sizeof(topics) / sizeof(topics[0]);
  // We will divide num_topics in num_brokers
  // topic with index i will be handled by broker (i % num_brokers)

  // Now we will create a server so that the pub-sub-clients can connect here
  // They will send the topic-index
  // We will send back the ip:port
  int port = atoi(argv[1]);
  signal(SIGINT, handle_sigint);

  if ((serv_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("socket() error\n");
    return -1;
  }

  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  /*======================BIND()============================*/
  if (bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("bind() error\n");
    close(serv_sockfd);
    return -1;
  }

  // Convert IP to human-readable format
  char server_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &serv_addr.sin_addr, server_ip, INET_ADDRSTRLEN);
  printf("Server is running on IP: %s and port: %d\n", server_ip,
         ntohs(serv_addr.sin_port));

  /*======================LISTEN()============================*/
  if (listen(serv_sockfd, SOMAXCONN) < 0) {
    printf("listen() error\n");
    close(serv_sockfd);
    return -1;
  }

  printf("Server is listening...\n");

  /*======================ACCEPT()============================*/
  while (1) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd =
        accept(serv_sockfd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd == -1) {
      continue;
    }
    pthread_t thread;
    int *client_fd_ptr = malloc(sizeof(int));
    *client_fd_ptr = client_fd;
    if (pthread_create(&thread, NULL, client_handler, client_fd_ptr) != 0) {
      printf("pthread_create() error\n");
      close(client_fd);
      free(client_fd_ptr);
      continue;
    }
    pthread_detach(thread);
  }
  close(serv_sockfd);
  return 0;
}