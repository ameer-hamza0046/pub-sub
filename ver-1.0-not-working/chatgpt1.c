#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOPICS 10
#define MAX_SUBSCRIBERS 10
#define MAX_MESSAGE_LENGTH 100

// Subscriber structure
typedef struct Subscriber {
    char id[20];
    void (*receive)(const char *topic, const char *message);
} Subscriber;

// Topic structure
typedef struct Topic {
    char name[20];
    Subscriber *subscribers[MAX_SUBSCRIBERS];
    int subscriber_count;
} Topic;

// Broker structure
typedef struct Broker {
    Topic topics[MAX_TOPICS];
    int topic_count;
} Broker;

// Function to initialize a broker
void initBroker(Broker *broker) {
    broker->topic_count = 0;
}

// Function to create a topic
void addTopic(Broker *broker, const char *topic_name) {
    if (broker->topic_count >= MAX_TOPICS) {
        printf("Error: Maximum topic limit reached.\n");
        return;
    }
    strcpy(broker->topics[broker->topic_count].name, topic_name);
    broker->topics[broker->topic_count].subscriber_count = 0;
    broker->topic_count++;
    printf("Topic '%s' added.\n", topic_name);
}

// Function to find a topic
Topic *findTopic(Broker *broker, const char *topic_name) {
    for (int i = 0; i < broker->topic_count; i++) {
        if (strcmp(broker->topics[i].name, topic_name) == 0) {
            return &broker->topics[i];
        }
    }
    return NULL;
}

// Function to add a subscriber to a topic
void subscribe(Broker *broker, const char *topic_name, Subscriber *subscriber) {
    Topic *topic = findTopic(broker, topic_name);
    if (topic == NULL) {
        printf("Error: Topic '%s' not found.\n", topic_name);
        return;
    }
    if (topic->subscriber_count >= MAX_SUBSCRIBERS) {
        printf("Error: Maximum subscribers for topic '%s' reached.\n", topic_name);
        return;
    }
    topic->subscribers[topic->subscriber_count++] = subscriber;
    printf("Subscriber '%s' subscribed to topic '%s'.\n", subscriber->id, topic_name);
}

// Function to publish a message to a topic
void publish(Broker *broker, const char *topic_name, const char *message) {
    Topic *topic = findTopic(broker, topic_name);
    if (topic == NULL) {
        printf("Error: Topic '%s' not found.\n", topic_name);
        return;
    }
    printf("Publishing message '%s' to topic '%s'.\n", message, topic_name);
    for (int i = 0; i < topic->subscriber_count; i++) {
        topic->subscribers[i]->receive(topic_name, message);
    }
}

// Example receive function for a subscriber
void receiveMessage(const char *topic, const char *message) {
    printf("Received on topic '%s': %s\n", topic, message);
}

// Main function to demonstrate pub-sub
int main() {
    Broker broker;
    initBroker(&broker);

    // Create topics
    addTopic(&broker, "Topic1");
    addTopic(&broker, "Topic2");

    // Create subscribers
    Subscriber subscriber1 = { "Subscriber1", receiveMessage };
    Subscriber subscriber2 = { "Subscriber2", receiveMessage };

    // Subscribe to topics
    subscribe(&broker, "Topic1", &subscriber1);
    subscribe(&broker, "Topic1", &subscriber2);
    subscribe(&broker, "Topic2", &subscriber2);

    // Publish messages
    publish(&broker, "Topic1", "Hello, Topic1!");
    publish(&broker, "Topic2", "Hello, Topic2!");
    publish(&broker, "Topic3", "This topic doesn't exist.");

    return 0;
}
