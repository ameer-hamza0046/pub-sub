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
#define PORT 10001
#define MAX_TOPICS 100
#define MAX_SUBSCRIBERS 100

typedef struct {
  int client_fd;
  bool is_active; // To handle disconnections
} Subscriber;

typedef struct {
  char topic_name[BUF_SIZE];
  // messages will be stored for each topic
  char messages[BUF_SIZE][BUF_SIZE];
  int msg_count;
  Subscriber subscribers[MAX_SUBSCRIBERS];
  int sub_count;
  pthread_mutex_t lock;
} Topic;

Topic topics[MAX_TOPICS];
int topic_count = 0;
pthread_mutex_t topics_lock;

int add_topic(const char *topic_name) {
  pthread_mutex_lock(&topics_lock);
  // check for overflow
  if (topic_count >= MAX_TOPICS) {
    pthread_mutex_unlock(&topics_lock);
    return -1;
  }
  // Check if topic already exists
  for (int i = 0; i < topic_count; i++) {
    if (strcmp(topics[i].topic_name, topic_name) == 0) {
      pthread_mutex_unlock(&topics_lock);
      return i; // Topic already exists, return index
    }
  }
  // Add new topic
  Topic *new_topic = &topics[topic_count++];
  strncpy(new_topic->topic_name, topic_name, BUF_SIZE);
  new_topic->msg_count = 0;
  new_topic->sub_count = 0;
  pthread_mutex_init(&new_topic->lock, NULL);
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

void publish(const char *topic_name, const char *message) {
  // printf("%s=%s\n",topic_name, message);
  Topic *topic = get_topic(topic_name);
  if (!topic) {
    int topic_idx = add_topic(topic_name);
    if (topic_idx < 0) {
      printf("Failed to add topic: %s\n", topic_name);
      return;
    }
    // printf("topic_idx = %d\n", topic_idx);
    topic = &topics[topic_idx];
  }

  pthread_mutex_lock(&topics_lock);
  if (topic->msg_count < BUF_SIZE) {
    strncpy(topic->messages[topic->msg_count++], message, BUF_SIZE);
  } else {
    printf("Overflow: Cannot publish more messages for topic: %s\n",
           topic_name);
  }

  // ---
  for (int i = 0; i < topic->sub_count; i++) {
    if (topic->subscribers[i].is_active) {
      printf("Sending to subscriber\n");
      send(topic->subscribers[i].client_fd, message, strlen(message) + 1, 0);
    }
  }
  pthread_mutex_unlock(&(topic->lock));
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
  pthread_mutex_lock(&topic->lock);

  // Add subscriber
  if (topic->sub_count < MAX_SUBSCRIBERS) {
    topic->subscribers[topic->sub_count].client_fd = client_fd;
    topic->subscribers[topic->sub_count].is_active = true;
    topic->sub_count++;
  } else {
    printf("Subscriber overflow: %s\n", topic_name);
  }
  printf("Subscriber added\n");

  // send older message to the subscriber
  for (int i = 0; i < topic->msg_count; i++) {
    send(client_fd, topic->messages[i], strlen(topic->messages[i])+1, 0);
  }
  pthread_mutex_unlock(&topic->lock);
}

void remove_subscriber(int client_fd) {
  pthread_mutex_lock(&topics_lock);
  for (int i = 0; i < topic_count; i++) {
    pthread_mutex_lock(&topics[i].lock);
    for (int j = 0; j < topics[i].sub_count; j++) {
      if (topics[i].subscribers[j].client_fd == client_fd) {
        topics[i].subscribers[j].is_active = false;
        break;
      }
    }
    pthread_mutex_unlock(&topics[i].lock);
  }
  pthread_mutex_unlock(&topics_lock);
}

void handlePublisher(int client_fd) {
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
    if (ret == 0) {
      printf("conn_closed - client_fd: %d\n", client_fd);
      break;
    }
    if (strcmp(client_buffer, "exit") == 0) {
      printf("client_fd %d is exiting...\n", client_fd);
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

    printf("P: %s:%s\n", topic_buffer, message_buffer);

    // now we have to publish the message to the topic
    // TODO
    publish(topic_buffer, message_buffer);
  }
  close(client_fd);
  pthread_exit(NULL);
}

void handleSubscriber(int client_fd) {
  // Subscriber
  // subscriber will send topic names they would like to subscribe
  // if subscriber want to close the connection, it would send "exit"
  char client_buffer[BUF_SIZE];
  int ret;
  while (true) {
    ret = recv(client_fd, client_buffer, BUF_SIZE, 0);
    printf("subs_client_fd: %d\n", client_fd);
    if (ret < 0) {
      printf("recv() error\n");
      remove_subscriber(client_fd);
      break;
    }

    if (ret == 0) {
      printf("conn_closed - client_fd: %d\n", client_fd);
      remove_subscriber(client_fd);
      break;
    }

    if (strcmp(client_buffer, "exit") == 0) {
      printf("client_fd %d is exiting...\n", client_fd);
      remove_subscriber(client_fd);
      break;
    }
    printf("S: Topic: %s\n", client_buffer);
    // now we have to subscribe to the topic
    // TODO
    subscribe(client_fd, client_buffer);
  }
  close(client_fd);
  pthread_exit(NULL);
}

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
  pthread_mutex_init(&topics_lock, NULL);
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
