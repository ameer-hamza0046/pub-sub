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

#define BUF_SIZE 1024
char buffer[BUF_SIZE] = {0};
#define PORT 10001

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

  // first ask whether client is a publisher or a subscriber
  // client will send 1 character
  // if it is publisher, the char will be 'P' else for subscriber it will be 'S'
  // otherwise invalid
  ret = recv(client_fd, client_buffer, BUF_SIZE, 0);
  if (ret < 0) {
    printf("recv error()\n");
    close(client_fd);
    pthread_exit(NULL);
  }
  
  if (strcmp(client_buffer, "P") == 0) {
    // to publish something, the client will send the message
    // in the following format
    // <topic>:<message>
    // If the colon(:) is missing then the message format is invalid
    // And when the client sends "exit" then it means the he want the
    // connection to be closed.
    while (true) {
      ret = recv(client_fd, client_buffer, BUF_SIZE, 0);
      if (ret < 0) {
        printf("recv() error", client_fd);
        break;
      }
      if (ret == 0) {
        printf("conn_closed - client_fd: %d\n", client_fd);
        break;
      }
      if (strcmp(client_buffer, "exit") == 0) {
        printf("client_fd %d is exiting...\n", client_fd);
        break;
      }
      // find if colon in present in the string
      bool found = false;
      for (int i = 0; i < ret; i++) {
        if (client_buffer[i] == ':') {
          found = true;
          break;
        }
      }
      if (!found) {
        printf("Message format is invalid... usage: <topic>:<message>\n");
        continue;
      }

      // seperate the topic and the message
      char topic_buffer[BUF_SIZE], message_buffer[BUF_SIZE];
      found = false;
      int j = 0;
      for (int i = 0; i < ret; i++) {
        if (client_buffer[i] == ':') {
          found = true;
          topic_buffer[j] = '\0';
          j = 0;
        } else if (!found) {
          topic_buffer[j++] = client_buffer[i];
        } else {
          message_buffer[j++] = client_buffer[i];
        }
      }
      message_buffer[j] = '\0';
      printf("P: <Topic>:<Message>: %s:%s\n", topic_buffer, message_buffer);

      // now we have to publish the message to the topic
      // TODO
    }
  } else if (strcmp(client_buffer, "S") == 0) {
    // Subscriber
    // subscriber will send topic names they would like to subscribe
    // if subscriber want to close the connection, it would send "exit"
    while (true) {
      ret = recv(client_fd, client_buffer, BUF_SIZE, 0);
      if (ret < 0) {
        printf("recv() error\n", client_fd);
        break;
      }

      if (ret == 0) {
        printf("conn_closed - client_fd: %d\n", client_fd);
        break;
      }

      if (strcmp(client_buffer, "exit") == 0) {
        printf("client_fd %d is exiting...\n", client_fd);
        break;
      }
      printf("S: Topic: %s\n", client_buffer);
      // now we have to subscribe to the topic
      // TODO
    }
  }
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
