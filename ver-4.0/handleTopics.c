#include "constants.c"

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
