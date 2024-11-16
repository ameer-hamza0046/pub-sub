#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 1024

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

  sleep(rand() % 4);

  char buffer[BUF_SIZE] = {0};
  char message[BUF_SIZE] = "S";
  ret = send(sock, message, strlen(message)+1, 0);
  if (ret == -1) {
    printf("Failed to send message\n");
    return -1;
  }
  printf("Sent: %s\n", message);


  strcpy(message, "World");
  ret = send(sock, message, strlen(message)+1, 0);
  if (ret == -1) {
    printf("Failed to send message\n");
    return -1;
  }
  printf("Sent: %s\n", message);


  strcpy(message, "exit");
  ret = send(sock, message, strlen(message)+1, 0);
  if (ret == -1) {
    printf("Failed to send message\n");
    return -1;
  }
  printf("Sent: %s\n", message);
  
  sleep(rand() % 4);

  close(sock);
  return 0;
}
