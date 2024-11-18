#include "signal.h"
#include "topicsAndMessages.c"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 1024

int sock;
pthread_t recv_thread;

void handle_sigint(int sig) {
  printf("\nSIGINT received. Sending exit message to the server...\n");
  char message[BUF_SIZE] = "exit";
  send(sock, message, strlen(message) + 1, 0);
  close(sock);
  // pthread_cancel(recv_thread);
  pthread_join(recv_thread, NULL);
  exit(0);
}

void *receive_messages(void *arg) {
  int sock = *(int *)arg;
  char buffer[BUF_SIZE];
  while (1) {
    int ret = recv(sock, buffer, BUF_SIZE, 0);
    if (ret <= 0) {
      printf("Connection closed or error\n");
      break;
    }
    buffer[ret] = '\0';
    printf("Received message: %s\n", buffer);
  }
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s <server_ip> <server_port> <seed>\n", argv[0]);
    return -1;
  }

  char *ip = argv[1];
  int port = atoi(argv[2]);
  int seed = atoi(argv[3]);

  srand(seed);

  int num_topics = sizeof(topics) / sizeof(topics[0]);

  int ret;
  struct sockaddr_in serv_addr;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Socket creation error\n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
    printf("Invalid address/ Address not supported\n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("Connection Failed\n");
    return -1;
  }

  signal(SIGINT, handle_sigint);

  char message[BUF_SIZE] = "S"; // 'S' for Subscriber
  if (send(sock, message, strlen(message) + 1, 0) < 0) {
    printf("Failed to send subscriber role\n");
    return -1;
  }

  int selected_count = rand() % num_topics + 1;
  printf("Subscribing to %d topics:\n", selected_count);

  int selected_indices[num_topics];
  memset(selected_indices, 0, sizeof(selected_indices));

  for (int i = 0; i < selected_count; ++i) {
    sleep(1); // think time
    int idx;
    do {
      idx = rand() % num_topics; // Random index
    } while (selected_indices[idx]); // Ensure no repetition
    selected_indices[idx] = 1; // Mark index as selected

    // Send topic to server
    if (send(sock, topics[idx], strlen(topics[idx]) + 1, 0) < 0) {
      printf("Failed to send topic: %s\n", topics[idx]);
      return -1;
    }
    printf("Subscribed to topic: %s\n", topics[idx]);
  }

  pthread_create(&recv_thread, NULL, receive_messages, &sock);

  pthread_join(recv_thread, NULL);

  close(sock);
  return 0;
}
