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
pthread_t recv_thread;

char brokers[MAX_BROKERS][BUF_SIZE];
int broker_sockets[MAX_BROKERS];
int broker_cnt = 0;

void handle_sigint(int sig) {
  printf("\nSIGINT received. Sending exit message to the server...\n");
  char message[BUF_SIZE] = "exit";
  for (int i = 0; i < MAX_BROKERS; i++) {
    send(broker_sockets[i], message, strlen(message) + 1, 0);
    close(broker_sockets[i]);
  }
  close(sock);
  exit(0);
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Usage: %s <load-balancer_ip> <load-balancer_port> <seed> "
           "<number_of_messages>\n",
           argv[0]);
    return -1;
  }

  char *ip = argv[1];
  int port = atoi(argv[2]);
  int seed = atoi(argv[3]);
  int messages_to_publish = atoi(argv[4]);

  srand(seed);

  int num_topics = sizeof(topics) / sizeof(topics[0]);
  int num_messages = sizeof(messages) / sizeof(messages[0]);

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

  char message[BUF_SIZE] = "P"; // 'P' for Publisher
  if (send(sock, message, strlen(message) + 1, 0) < 0) {
    printf("Failed to send publisher role\n");
    return -1;
  }

  printf("Publisher started. Press Ctrl+C to exit.\n");
  int topic_index_to_socket[num_topics];
  memset(topic_index_to_socket, -1, sizeof(topic_index_to_socket));
  while (messages_to_publish-- > 0) {
    sleep(rand() % 2); // Random delay between messages as think time

    // Select a random topic and message
    int topic_index = rand() % num_topics;
    const char *random_topic = topics[topic_index];
    const char *random_message = messages[rand() % num_messages];

    // check if socket already exists for a topic_index
    int broker_sock = topic_index_to_socket[topic_index];
    if (broker_sock == -1) {

      // Send topic index to server to get broker ip
      char buffer[BUF_SIZE];
      sprintf(buffer, "%d", topic_index);
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
      int broker_index;
      for (broker_index = 0; broker_index < MAX_BROKERS; broker_index++) {
        if (strcmp(brokers[broker_index], buffer) == 0) {
          broker_sock = broker_sockets[broker_index];
          topic_index_to_socket[topic_index] = broker_sock;
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
        // send 'P' to let it know that you are a publisher
        char message[BUF_SIZE] = "P"; // 'P' for publisher
        if (send(broker_sock, message, strlen(message) + 1, 0) < 0) {
          printf("Failed to send subscriber role\n");
          return -1;
        }

        // save this broker
        strcpy(brokers[broker_cnt], buffer);
        broker_sockets[broker_cnt] = broker_sock;
        broker_cnt++;
        topic_index_to_socket[topic_index] = broker_sock;
      }
    }

    // Construct and send the message in the format "topic:message"
    snprintf(message, BUF_SIZE, "%s:%s", random_topic, random_message);
    if (send(broker_sock, message, strlen(message) + 1, 0) < 0) {
      printf("Failed to send message: %s\n", message);
      continue;
    }
    printf("Published message on topic '%s': %s\n", random_topic,
           random_message);
  }
  for (int i = 0; i < MAX_BROKERS; i++) {
    close(broker_sockets[i]);
  }
  close(sock);
  return 0;
}
