#include "handleTopics.c"

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
