#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024

void runPublisher(int clientSocket) {
    cout << "Enter messages in the format <topic>:<message> (or type 'exit' to quit):\n";
    string input;

    while (true) {
        getline(cin, input);
        if (input == "exit") break;
        send(clientSocket, input.c_str(), input.size(), 0);
    }
}

void runSubscriber(int clientSocket) {
    cout << "Enter the topic to subscribe to: ";
    string topic;
    getline(cin, topic);
    send(clientSocket, topic.c_str(), topic.size(), 0);

    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesRead = read(clientSocket, buffer, BUFFER_SIZE - 1);
        if (bytesRead <= 0) {
            cout << "Disconnected from server.\n";
            break;
        }
        cout << "Received message: " << buffer << endl;
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection failed");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    cout << "Connected to broker. Are you a Publisher (P) or Subscriber (S)? ";
    char role;
    cin >> role;
    cin.ignore(); // Clear input buffer

    send(clientSocket, &role, 1, 0);

    if (role == 'P') {
        runPublisher(clientSocket);
    } else if (role == 'S') {
        runSubscriber(clientSocket);
    } else {
        cout << "Invalid role. Disconnecting.\n";
    }

    close(clientSocket);
    return 0;
}
