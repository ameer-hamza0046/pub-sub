#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#define BUF_SIZE 1024
#define MAX_TOPICS 5
#define MAX_MESSAGES 5

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

  char *topics[MAX_TOPICS] = {"Sports", "Technology", "Weather", "News", "Finance"};
  char *messages[MAX_MESSAGES] = {
      "Breaking news!",
      "Big updates in tech!",
      "Rain expected tomorrow.",
      "Stock market hits record highs.",
      "The game was fantastic!"};

  char buffer[BUF_SIZE] = {0};
  char message[BUF_SIZE];

  // Send "P" to indicate this is a publisher
  strcpy(message, "P");
  ret = send(sock, message, strlen(message) + 1, 0);
  if (ret == -1) {
    printf("Failed to send message\n");
    close(sock);
    return -1;
  }
  printf("Sent: %s\n", message);

  // Publish messages
  int publish_count = rand() % 5 + 1; // Random number of times to publish
  for (int i = 0; i < publish_count; i++) {
    sleep(rand() % 4 + 1); // Random sleep between 1 to 4 seconds

    // Pick a random topic and message
    const char *topic = topics[rand() % MAX_TOPICS];
    const char *msg = messages[rand() % MAX_MESSAGES];

    snprintf(buffer, BUF_SIZE, "%s:%s", topic, msg);
    ret = send(sock, buffer, strlen(buffer) + 1, 0);
    if (ret == -1) {
      printf("Failed to send message\n");
      break;
    }
    printf("Sent: %s\n", buffer);
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
