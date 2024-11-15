#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include <signal.h>

#define PORT 9010
#define BUFFER_SIZE 1024

using namespace std;

int sockfd;
void handle_sigint(int sig) {
    close(sockfd);
    printf("\nServer socket closed. Exiting.\n");
    exit(0);
}

void* handle_client(void* args) {
    int clientfd = *((int*)(args));
    delete (int*)(args);
    while(true) {
        char buffer[BUFFER_SIZE];
        int r = recv(clientfd, buffer, sizeof(BUFFER_SIZE)-1, 0);
        if(r <= 0) {
            cerr << "recv() error\n";
            return NULL;
        }
        buffer[r] = '\0';
        cout << "Received: " << buffer << endl;
        string str = buffer;
        if(str == "exit") {
            break;
        }
        sprintf(buffer,"hello client");
        r = send(clientfd, buffer, strlen(buffer), 0);
        if(r < 0) {
            cerr << "send() error\n";
            return NULL;
        }
        cout << "Sent: " << buffer << endl;
    }
    close(clientfd);
    return NULL;
}

int main() {
    int clientfd;
    signal(SIGINT, handle_sigint);
    struct sockaddr_in server_addr, client_addr;
    if (sockfd = socket(AF_INET, SOCK_STREAM, 0) < 0) {
        cerr << "socket() error\n";
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "bind() error\n";
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (listen(sockfd, SOMAXCONN) < 0) {
        cerr << "Listen failed";
        close(sockfd);
        return EXIT_FAILURE;
    }

    cout << "Server running at port " << PORT << "...\n";

    while(true) {
        socklen_t addrLen = sizeof(client_addr);
        if (clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrLen) < 0) {
            cerr << "accept() error\n";
            continue;
        }
        cout << "New client connected\n";

        pthread_t thread;
        int* ptr = new int(clientfd);
        if(pthread_create(&thread, NULL, handle_client, ptr) < 0) {
            cerr << "pthread_create() error\n";
            close(clientfd);
        }

        pthread_detach(thread);
    }
    close(sockfd);
    return 0;
}