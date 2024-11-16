#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 1024
char buffer[BUF_SIZE] = {0};
#define PORT 10000

int serv_sockfd;
void handle_sigint(int sig) {
  close(serv_sockfd);
  printf("\nServer socket closed. Exiting.\n");
  exit(0);
}

void *client_handler(void *arg) {
  int client_fd = *(int *)arg;
  free(arg);

  char client_buffer[BUF_SIZE];
  int ret;

  ret = recv(client_fd, client_buffer, BUF_SIZE, 0);
  if (ret == -1) {
    printf("recv() error\n");
    close(client_fd);
    pthread_exit(NULL);
  }
  printf("Received: %s\n", client_buffer);
  char *message = "world";
  ret = send(client_fd, message, strlen(message), 0);
  if (ret == -1) {
    printf("send() error\n");
    close(client_fd);
    pthread_exit(NULL);
  }
  printf("Sent: %s\n", message);
  close(client_fd);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  signal(SIGINT, handle_sigint);

  int port = PORT;
  /*======================SOCKET()============================*/
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
