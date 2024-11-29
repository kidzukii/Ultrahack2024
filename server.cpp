#include <iostream>
#include <cstring>
#include <string>
#include <chrono>
#include <ctime>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

int sock;
int fakeOffset = 0;

std::chrono::time_point<std::chrono::high_resolution_clock> then;

clock_t clock() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> duration = now - then;
    return duration.count() + fakeOffset;
}

int syncClock(const int &client_socket, char *buffer) {
    // Send INIT
    clock_t tInitSend;
    const char* syncRequest = "INIT";
    send(client_socket, syncRequest, strlen(syncRequest), 0);
    tInitSend = clock();

    // Wait for confirmation
    recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    memset(buffer, 0, sizeof buffer);
    std::cerr << "INIT confirmed" << std::endl;
    
    // Send FLUP
    char followUpRequest[20] = "";
    strcat(followUpRequest, std::to_string(tInitSend).c_str());
    send(client_socket, followUpRequest, strlen(followUpRequest), 0);
    std::cerr << "FLUP done" << std::endl;

    // DELY
    recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    clock_t tDelyReceive = clock();
    
    char delayRequest[20] = "";
    strcat(delayRequest, std::to_string(tDelyReceive).c_str());
    send(client_socket, delayRequest, strlen(delayRequest), 0);

    return 0;
}

int handleClient(int client_socket) {
    while (true) {
        char buffer[BUFFER_SIZE] = {0};
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            std::cerr << strerror(errno) << std::endl;
            std::cerr << "Failed to receive data from client" << std::endl;
            close(client_socket);
            return 1;
        }

        buffer[bytes_received] = '\0';
        std::string request(buffer);

        if (request == "HELLO") {
            std::cerr << "hoohoo" << std::endl;
            const char* helloRequest = "Dcmm";
            send(client_socket, helloRequest, strlen(helloRequest), 0);
        }
        else if (request == "SYNC") {
            syncClock(client_socket, buffer);
        }
        else if (request == "TEST") {
            const char* backRequest = "OKAY";
            send(client_socket, backRequest, strlen(backRequest), 0);
        }
        else {
            std::cerr << "Unknown request from client: " << request << std::endl;
        }
    }

    close(client_socket);
    return 0;
}

/*
void handleClient(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        std::cerr << "Failed to receive data from client" << std::endl;
        close(client_socket);
        return;
    }

    buffer[bytes_received] = '\0';
    std::string request(buffer);

    if (request == "SYNC_REQUEST") {
        // Get the current server time
        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        // Send the server time back to the client
        std::string response = std::ctime(&currentTime); // Convert time to string
        send(client_socket, response.c_str(), response.size(), 0);
        std::cout << "Sent server time to client: " << response;
    } else {
        std::cerr << "Unknown request from client: " << request << std::endl;
    }

    close(client_socket);
}
*/

int establishSocket() {
    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Bind the socket to an address
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock);
        return 1;
    }

    // Start listening for connections
    if (listen(sock, 5) < 0) {
        perror("Listen failed");
        close(sock);
        return 1;
    }

    std::cout << "Server listening on port " << SERVER_PORT << std::endl;

    return 1;
}

void alterClock() {
    while (true) {
        sleep(7);
        fakeOffset += (rand() % 100);
        std::cerr << "lmao" << '\n';
    }
}

int main() {
    then = std::chrono::high_resolution_clock::now();
    // std::thread(alterClock).detach();
    establishSocket();
    // Accept client connections
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        std::cout << "Client connected!" << std::endl;

        // Handle the client in a separate thread
        std::thread(handleClient, client_socket).detach();
    }

    close(sock);
    return 0;
}