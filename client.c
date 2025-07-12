#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888

int main() {
    int sock_fd;
    struct sockaddr_in serv_addr;

    // 1. Create a socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Set up the server address to connect to
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Connect to the server
    printf("Connecting to server at %s:%d...\n", SERVER_IP, PORT);
    if (connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("Successfully connected to the server.\n");

    // 4. CRITICAL STEP: Immediately gracefully close the connection.
    // This sends a FIN packet to the server, starting the graceful shutdown.
    //
    // Note we have to use `SHUT_RDWR` (value 2) instead of `SHUT_RD` (value `0`),
    // because on Linux, `SHUT_RD` has no effect and does not send TCP FIN,
    // see e.g.: https://github.com/WebAssembly/WASI/issues/547#issuecomment-2004647912
    // (I cannot find this documented anywhere, but checked it in Wireshark on Linux 6.8.)
    printf("Immediately doing graceful close (shutdown()) on the connection.\n");
    shutdown(sock_fd, SHUT_RDWR);

    sleep(2); // during this time, the server code attempts the send()

    printf("Fully close the socket.\n");
    close(sock_fd);

    printf("Client finished.\n");
    return 0;
}
