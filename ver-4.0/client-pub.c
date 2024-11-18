#include "signal.h"
#include "topicsAndMessages.c"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 1024

int sock;

void handle_sigint(int sig) {
  printf("\nSIGINT received. Sending exit message to the server...\n");
  char message[BUF_SIZE] = "exit";
  send(sock, message, strlen(message) + 1, 0);
  close(sock);
  exit(0);
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

  while (1) {
    sleep(rand() % 3 + 1); // Random delay between messages as think time

    // Select a random topic and message
    const char *random_topic = topics[rand() % num_topics];
    const char *random_message = messages[rand() % num_messages];

    // Construct and send the message in the format "topic:message"
    snprintf(message, BUF_SIZE, "%s:%s", random_topic, random_message);
    if (send(sock, message, strlen(message) + 1, 0) < 0) {
      printf("Failed to send message: %s\n", message);
      continue;
    }
    printf("Published message on topic '%s': %s\n", random_topic, random_message);
  }

  close(sock);
  return 0;
}
