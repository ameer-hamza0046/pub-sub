#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

//============= SOME CONSTANTS =====================================

#define BUF_SIZE 1024
#define MAX_TOPICS 1000
#define MAX_SUBSCRIBERS 100000

typedef struct {
  char topic_name[BUF_SIZE];
  int subscribers[MAX_SUBSCRIBERS];
} Topic;

Topic topics[MAX_TOPICS];
int topic_count = 0;
pthread_mutex_t topics_lock;


//============= HANDLING THE DATA STRUCTURE FOR PUB-SUB ===============

int add_topic(const char *topic_name) {
  pthread_mutex_lock(&topics_lock);
  // Check if topic already exists
  for (int i = 0; i < topic_count; i++) {
    if (strcmp(topics[i].topic_name, topic_name) == 0) {
      pthread_mutex_unlock(&topics_lock);
      return i; // Topic already exists, return index
    }
  }
  // check for overflow
  if (topic_count >= MAX_TOPICS) {
    pthread_mutex_unlock(&topics_lock);
    return -1;
  }
  // Add new topic
  Topic *new_topic = &topics[topic_count++];
  strncpy(new_topic->topic_name, topic_name, BUF_SIZE);
  memset(new_topic->subscribers, -1, sizeof(new_topic->subscribers));
  pthread_mutex_unlock(&topics_lock);
  // return index of new topic
  return topic_count - 1;
}

Topic *get_topic(const char *topic_name) {
  pthread_mutex_lock(&topics_lock);
  for (int i = 0; i < topic_count; i++) {
    if (strcmp(topics[i].topic_name, topic_name) == 0) {
      pthread_mutex_unlock(&topics_lock);
      return &topics[i];
    }
  }
  pthread_mutex_unlock(&topics_lock);
  return NULL;
}


//================== PUBLISH() AND SUBSCRIBE() FUNCTIONS ================
void publish(const char *topic_name, const char *message) {
  Topic *topic = get_topic(topic_name);
  if (!topic) {
    int topic_idx = add_topic(topic_name);
    if (topic_idx < 0) {
      printf("Failed to add topic: %s\n", topic_name);
      return;
    }
    topic = &topics[topic_idx];
  }
  char buffer[BUF_SIZE];
  int n = snprintf(buffer, BUF_SIZE-1, "%s:%s", topic_name, message);
  buffer[n] = '\0';
  pthread_mutex_lock(&topics_lock);
  // ---
  for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
    if (topic->subscribers[i] != -1) {
      printf("Sending to subscriber %d\n", topic->subscribers[i]);
      send(topic->subscribers[i], buffer, strlen(buffer) + 1, 0);
    }
  }
  pthread_mutex_unlock(&topics_lock);
}

void subscribe(int client_fd, const char *topic_name) {
  Topic *topic = get_topic(topic_name);
  if (!topic) {
    int topic_idx = add_topic(topic_name);
    if (topic_idx < 0) {
      printf("Failed to add topic: %s\n", topic_name);
      return;
    }
    topic = &topics[topic_idx];
  }
  pthread_mutex_lock(&topics_lock);
  // check if subscriber already exists in the topic
  bool sub = false;
  for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
    if (topic->subscribers[i] == client_fd) {
      printf("Already subscribed to this topic\n");
      sub = true;
      break;
    }
  }
  // Add subscriber to that topic!!!!!!
  if (!sub) {
    bool added = false;
    for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
      if (topic->subscribers[i] == -1) {
        topic->subscribers[i] = client_fd;
        added = true;
        break;
      }
    }
    if (added) {
      printf("Subscriber added\n");
    } else {
      printf("Subscriber overflow: %s\n", topic_name);
    }
  }

  pthread_mutex_unlock(&topics_lock);
}

// ============== HANDLING PUBLISHER AND SUBSCRIBER CLIENTS ============

void remove_subscriber(int client_fd) {
  pthread_mutex_lock(&topics_lock);
  for (int i = 0; i < topic_count; i++) {
    for (int j = 0; j < MAX_SUBSCRIBERS; j++) {
      if (topics[i].subscribers[j] == client_fd) {
        topics[i].subscribers[j] = -1;
        break;
      }
    }
  }
  pthread_mutex_unlock(&topics_lock);
}

void handlePublisher(int client_fd) {
  printf("New publisher with client_fd: %d\n", client_fd);
  char client_buffer[BUF_SIZE];
  int ret;
  // to publish something, the client will send the message
  // in the following format
  // <topic>:<message>
  // If the colon(:) is missing then the message format is invalid
  // And when the client sends "exit" then it means the he want the
  // connection to be closed.
  while (true) {
    ret = recv(client_fd, client_buffer, BUF_SIZE, 0);
    if (ret < 0) {
      printf("recv() error");
      break;
    }
    if (ret == 0 || strcmp(client_buffer, "exit") == 0) {
      printf("pub_conn_closed - client_fd: %d\n", client_fd);
      break;
    }

    char topic_buffer[BUF_SIZE], message_buffer[BUF_SIZE];
    // Find the first occurrence of ':'
    char *colon_pos = strchr(client_buffer, ':');
    if (!colon_pos) {
      // if colon not present, hence message is invalid
      printf("Message format is invalid... usage: <topic>:<message>\n");
      continue;
    }

    // Separate topic and message
    int topic_len = colon_pos - client_buffer;
    strncpy(topic_buffer, client_buffer, topic_len);
    topic_buffer[topic_len] = '\0';
    strcpy(message_buffer, colon_pos + 1);

    printf("P-%d: %s:%s\n", client_fd, topic_buffer, message_buffer);

    // now we have to publish the message to the topic
    publish(topic_buffer, message_buffer);
  }
  close(client_fd);
  pthread_exit(NULL);
}

void handleSubscriber(int client_fd) {
  printf("New subscriber with client_fd: %d\n", client_fd);
  // Subscriber
  // subscriber will send topic names they would like to subscribe
  // if subscriber want to close the connection, it would send "exit"
  char client_buffer[BUF_SIZE];
  int ret;
  while (true) {
    ret = recv(client_fd, client_buffer, BUF_SIZE, 0);
    if (ret < 0) {
      printf("recv() error\n");
      remove_subscriber(client_fd);
      break;
    }

    if (ret == 0 || strcmp(client_buffer, "exit") == 0) {
      printf("sub_conn_closed - client_fd: %d\n", client_fd);
      remove_subscriber(client_fd);
      break;
    }

    printf("S-%d: %s\n", client_fd, client_buffer);
    // now we have to subscribe to the topic
    subscribe(client_fd, client_buffer);
  }
  close(client_fd);
  pthread_exit(NULL);
}

//============== CLIENT HANDLER AND MAIN FUNCTION =================
void *client_handler(void *arg) {
  int client_fd = *(int *)arg;
  free(arg);

  char client_buffer[BUF_SIZE];
  int ret;

  /* first ask whether client is a publisher or a subscriber
   * client will send 1 character
   * if it is publisher, the char will be 'P'
   * else for subscriber it will be 'S'
   * otherwise invalid
   */
  ret = recv(client_fd, client_buffer, BUF_SIZE, 0);
  if (ret <= 0) {
    printf("recv error()\n");
    close(client_fd);
    pthread_exit(NULL);
  }

  if (strcmp(client_buffer, "P") == 0) {
    handlePublisher(client_fd);
  } else if (strcmp(client_buffer, "S") == 0) {
    handleSubscriber(client_fd);
  } else {
    printf("Error: Invalid role %s\n", client_buffer);
    close(client_fd);
    pthread_exit(NULL);
  }
  return NULL;
}

/*=================HANDLING CTRL+C SIGNAL=====================*/
int serv_sockfd;
void handle_sigint(int sig) {
  close(serv_sockfd);
  printf("\nServer socket closed. Exiting.\n");
  exit(0);
}

int main(int argc, char *argv[]) {
  if(argc != 2) {
    printf("Usage: ./server.out <port>\n");
    return EXIT_FAILURE;
  }
  int port = atoi(argv[1]);

  signal(SIGINT, handle_sigint);
  pthread_mutex_init(&topics_lock, NULL);

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
  pthread_mutex_destroy(&topics_lock);
  return 0;
}
