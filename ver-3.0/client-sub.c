#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define MAX_TOPICS 5

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s <server_ip> <server_port> <seed>\n", argv[0]);
    return -1;
  }

  char *ip = argv[1];
  int port = atoi(argv[2]);
  int seed = atoi(argv[3]);

  srand(seed);

  int sock, ret;
  struct sockaddr_in serv_addr;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Socket creation error\n");
    return -1;
  }

  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) <
      0) {
    printf("Failed to set socket timeout\n");
    close(sock);
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

  char *topics[MAX_TOPICS] = {"Sports", "Technology", "Weather", "News",
                              "Finance"};
  char buffer[BUF_SIZE] = {0};
  char message[BUF_SIZE];

  // Send "S" to indicate this is a subscriber
  strcpy(message, "S");
  ret = send(sock, message, strlen(message) + 1, 0);
  if (ret == -1) {
    printf("Failed to send message\n");
    close(sock);
    return -1;
  }
  printf("Sent: %s\n", message);

  // Subscribe to random topics
  int subscribe_count =
      rand() % 3 + 1; // Random number of topics to subscribe to (1 to 3)
  for (int i = 0; i < subscribe_count; i++) {
    const char *topic = topics[rand() % MAX_TOPICS];
    ret = send(sock, topic, strlen(topic) + 1, 0);
    if (ret == -1) {
      printf("Failed to subscribe to topic: %s\n", topic);
      continue;
    }
    printf("Subscribed to: %s\n", topic);
  }

  // Define how long to run before exiting
  int runtime = rand() % 10 + 5; // Random runtime between 5 and 15 seconds
  printf("Subscriber will run for %d seconds.\n", runtime);

  time_t start_time = time(NULL);
  while (1) {
    ret = recv(sock, buffer, BUF_SIZE, 0);
    if (ret == -1) {
      printf("Failed to receive message\n");
      break;
    }
    if (ret == 0) {
      printf("Server closed connection\n");
      break;
    }
    buffer[ret] = '\0';
    printf("Received: %s\n", buffer);

    // Check runtime
    if (time(NULL) - start_time >= runtime) {
      printf("Time's up! Subscriber exiting.\n");
      break;
    }
  }

  // Send "exit" to close the connection
  strcpy(message, "exit");
  ret = send(sock, message, strlen(message) + 1, 0);
  if (ret == -1) {
    printf("Failed to send 'exit' message\n");
  } else {
    printf("Sent: %s\n", message);
  }

  close(sock);
  return 0;
}
