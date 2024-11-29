#include <iostream>
#include <cstring>
#include <string>
#include <chrono>
#include <ctime>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int timeOffset = 0;

#define SERVER_PORT 12345
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE] = {0};

int server_port = SERVER_PORT;
std::string server_ip = SERVER_IP;
std::chrono::time_point<std::chrono::high_resolution_clock> then;

clock_t currentDelay;
clock_t currentOffset;

int sock;

clock_t clock() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> duration = now - then;
    return duration.count();
}

inline long long pseudoTime() {
    return clock() + timeOffset;
}

int establishSocket() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        return 1;
    }

    std::cout << "Connected to the server at " << server_ip << ":" << server_port << std::endl;
    return 0;
}

void sayHelloToServer() {
    const char* helloRequest = "HELLO";
    clock_t t1, t2, t3;
    t1 = clock();
    send(sock, helloRequest, strlen(helloRequest), 0);
    t2 = clock();
    std::cerr << t2 - t1 << std::endl;
    std::cerr << t3 - t2 << std::endl;

    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive response from server");
        close(sock);
        return;
    }

    buffer[bytes_received] = '\0';
    std::string greetings(buffer);
    std::cout << "Server said: " << greetings << std::endl;
}

void sendSyncRequest() {
    const char* confirmation = "X";

    const char* syncRequest = "SYNC";
    send(sock, syncRequest, strlen(syncRequest), 0);

    // INIT
    ssize_t bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    clock_t tInitReceive = pseudoTime();
    if (bytes_received <= 0) {
        perror("Failed to receive response from server");
        close(sock);
        return;
    }
    send(sock, confirmation, strlen(confirmation), 0);
    std::cerr << "INIT done" << std::endl;

    // FLUP
    memset(buffer, 0, sizeof buffer);
    bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive response from server");
        close(sock);
        return;
    }
    buffer[bytes_received] = '\0';
    int tInitSend = atoi(buffer);
    int timeOffset_pre = timeOffset;
    currentOffset = tInitSend - tInitReceive;
    timeOffset += currentOffset;

    // Send DELY
    int tDelySend;
    const char* delayRequest = "DELY";
    send(sock, delayRequest, strlen(delayRequest), 0);
    tDelySend = pseudoTime();

    // Receive DELY
    memset(buffer, 0, sizeof buffer);
    bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive response from server");
        close(sock);
        return;
    }
    buffer[bytes_received] = '\0';
    int tDelyReceive = atoi(buffer);
    currentDelay = (tDelyReceive - tDelySend) / 2;
    timeOffset += currentDelay;
    std::cerr << "Offset: " << currentOffset << ", Delay: " << currentDelay << std::endl;
}

/*
void syncWithServer(const std::string& server_ip, int server_port) {

    // Send sync request
    const char* syncRequest = "SYNC_REQUEST";
    send(sock, syncRequest, strlen(syncRequest), 0);

    // Receive the server's time
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive response from server");
        close(sock);
        return;
    }

    buffer[bytes_received] = '\0';
    std::string serverTime(buffer);
    std::cout << "Server time: " << serverTime << std::endl;

    // Close the connection
    close(sock);
}
*/

void sendTestRequest() {
    clock_t t1, t2, t3, t4;

    const char* msg = "TEST";
    t1 = clock();
    send(sock, msg, strlen(msg), 0);
    t2 = clock();

    sleep(5);
    char buffer[BUFFER_SIZE] = {0};
    t3 = clock();
    ssize_t bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    t4 = clock();

    if (bytes_received <= 0) {
        perror("Failed to receive response from server");
        close(sock);
        return;
    }
    std::cerr << t2 - t1 << ' ' << t4 - t3 << std::endl;
}

int main() {
    then = std::chrono::high_resolution_clock::now();
    establishSocket();
    freopen("benchmark.csv", "w", stdout);
    while (true) {
        sendSyncRequest();
        sleep(1);
        std::cout << currentOffset << ',' << currentDelay << '\n';
    }
    close(sock);
    return 0;
}
