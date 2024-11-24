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
#define MAX_BROKERS 100

int sock;

char brokers[MAX_BROKERS][BUF_SIZE];
int broker_sockets[MAX_BROKERS];
int broker_cnt = 0;

pthread_t broker_threads[MAX_BROKERS];

void handle_sigint(int sig) {
  printf("\nSIGINT received. Sending exit message to the server...\n");
  char message[BUF_SIZE] = "exit";
  for (int i = 0; i < MAX_BROKERS; i++) {
    send(broker_sockets[i], message, strlen(message) + 1, 0);
    close(broker_sockets[i]);
  }
  close(sock);
  for (int i = 0; i < MAX_BROKERS; i++) {
    pthread_cancel(broker_threads[i]);
    pthread_join(broker_threads[i], NULL);
  }
  exit(0);
}

void *receive_messages(void *arg) {
  int sock = *(int *)arg;
  free(arg);
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
    printf("Usage: %s <load_balancer_ip> <load_balancer_port> <seed>\n",
           argv[0]);
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
  //////////////////////////////////////////////////////////////

  int selected_count = rand() % num_topics + 1;
  printf("Subscribing to %d topics:\n", selected_count);

  int selected_indices[num_topics];
  memset(selected_indices, 0, sizeof(selected_indices));

  for (int i = 0; i < selected_count; ++i) {
    // sleep(1); // think time
    int idx;
    do {
      idx = rand() % num_topics; // Random index
    } while (selected_indices[idx]); // Ensure no repetition
    selected_indices[idx] = 1; // Mark index as selected

    // check if idx already has a socket assigned

    // Send topic index to server to get broker ip
    char buffer[BUF_SIZE];
    sprintf(buffer, "%d", idx);
    if (send(sock, buffer, strlen(buffer) + 1, 0) < 0) {
      printf("Failed to send topic index\n");
      return -1;
    }
    // receive ip and port
    if (recv(sock, buffer, BUF_SIZE, 0) <= 0) {
      printf("Failed to get ip and port\n");
      return -1;
    }
    // connect to this broker if not already connected
    int broker_sock = -1, broker_index;
    for (broker_index = 0; broker_index < MAX_BROKERS; broker_index++) {
      if (strcmp(brokers[broker_index], buffer) == 0) {
        broker_sock = broker_sockets[broker_index];
        break;
      }
    }
    if (broker_sock == -1) {
      // connect to this broker
      char ip[BUF_SIZE], port_str[BUF_SIZE];
      int idx1 = 0, idx2 = 0;
      int flag = 0;
      for (int i = 0; i < BUF_SIZE && buffer[i] != '\0'; i++) {
        if (buffer[i] == ':') {
          flag = 1;
          continue;
        }
        if (!flag) {
          ip[idx1++] = buffer[i];
        } else {
          port_str[idx2++] = buffer[i];
        }
      }

      char broker_ip_address[BUF_SIZE];
      sprintf(broker_ip_address, "%s", ip);
      int broker_port = atoi(port_str);
      ////////////////////////////////
      int ret;
      struct sockaddr_in serv_addr;

      if ((broker_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
      }

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(broker_port);

      // Convert IPv4 and IPv6 addresses from text to binary form
      if (inet_pton(AF_INET, broker_ip_address, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported\n");
        return -1;
      }

      if (connect(broker_sock, (struct sockaddr *)&serv_addr,
                  sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
      }
      // send 'S' to let it know that you are a subscriber
      char message[BUF_SIZE] = "S"; // 'S' for Subscriber
      if (send(broker_sock, message, strlen(message) + 1, 0) < 0) {
        printf("Failed to send subscriber role\n");
        return -1;
      }
      // save this broker
      strcpy(brokers[broker_cnt], buffer);
      printf("IP in pub: %s\n", buffer);
      broker_sockets[broker_cnt] = broker_sock;
      broker_cnt++;

      // create thread for broker
      int *ptr = malloc(sizeof(int));
      *ptr = broker_sock;
      pthread_create(&broker_threads[broker_cnt - 1], NULL, receive_messages,
                     ptr);
    }
    // now send the topic to this broker
    if (send(broker_sock, topics[idx], strlen(topics[idx]) + 1, 0) < 0) {
      printf("Failed to send topic: %s\n", topics[idx]);
      return -1;
    }
    printf("Subscribed to topic: %s\n", topics[idx]);
  }
  for (int i = 0; i < MAX_BROKERS; i++) {
    pthread_join(broker_threads[i], NULL);
    close(broker_sockets[i]);
  }
  close(sock);
  return 0;
}
