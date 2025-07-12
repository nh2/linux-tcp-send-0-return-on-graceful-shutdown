#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define PORT 8888

int main() {
    int listen_fd, conn_fd;
    struct sockaddr_in serv_addr;
    ssize_t bytes_sent;

    // 1. Create a listening socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Allow reusing the address to avoid "Address already in use" errors
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Bind the socket to an IP and port
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind failed");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Listen for incoming connections
    if (listen(listen_fd, 5) < 0) {
        perror("listen failed");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);
    printf("Waiting for a client to connect...\n");

    // 4. Accept a connection (this is a blocking call)
    conn_fd = accept(listen_fd, NULL, NULL);
    if (conn_fd < 0) {
        perror("accept failed");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    // Enable TCP_NODELAY, so our `send()` actually happens when we call it.
    printf("Setting TCP_NODELAY...\n");
    int one = 1;
    if (setsockopt(conn_fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one)) == -1) {
        perror("setsockopt(TCP_NODELAY) failed");
        close(conn_fd);
        exit(EXIT_FAILURE);
    }

    printf("The server will now wait for 1 second.\n");

    // 5. CRITICAL STEP: Wait for the client to close the connection.
    // This gives the client's FIN packet time to arrive and be processed.
    sleep(1);

    // 6. Attempt to send data to the client that has already closed its side.
    const char* msg = "This message will not be sent.";
    printf("Attempting to send data to the client...\n");

    bytes_sent = send(conn_fd, msg, strlen(msg), 0);

    // 7. Check the return value of send()
    if (bytes_sent == -1) {
        // This would indicate an error like EPIPE (Broken Pipe)
        perror("send");
    } else {
        printf("\n--- Result ---\n");
        printf("send() returned: %zd\n", bytes_sent);
        if (bytes_sent == 0) {
            printf("This return value of 0 indicates the peer (client) has performed a graceful shutdown.\n");
            printf("The connection is in the CLOSE_WAIT state on the server side.\n");
        } else {
            printf("Unexpectedly sent %zd bytes.\n", bytes_sent);
        }
        printf("--------------\n");
    }

    // 8. Clean up
    printf("Closing connection.\n");
    close(conn_fd);
    close(listen_fd);

    return 0;
}
