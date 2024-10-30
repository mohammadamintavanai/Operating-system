#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <iostream>
#define STDIN 0
#define STDOUT 1
#define BUFFER_SIZE 1024
using namespace std;
int main(int argc, char *argv[])
{
    
    
    if (argc != 3)
        perror("Invalid Arguments");

    char *ipaddr = argv[1];
    struct sockaddr_in server_addr;
    struct sockaddr_in old_server_addr;
    int old_server_fd;
    int server_fd, opt = 1;

    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, ipaddr, &(server_addr.sin_addr)) == -1)
        perror("FAILED: Input ipv4 address invalid");

    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        perror("FAILED: Socket was not created");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        perror("FAILED: Making socket reusable failed");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        perror("FAILED: Making socket reusable port failed");

    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    server_addr.sin_port = htons(strtol(argv[2], NULL, 10));

    if (connect(server_fd, (sockaddr *)(&server_addr), sizeof(server_addr)))
        perror("FAILED: Connect");

    while (1)
    {
        char recieeve_massage[BUFFER_SIZE];
        // Receive messages
        memset(recieeve_massage, 0, BUFFER_SIZE);
        recv(server_fd, recieeve_massage, BUFFER_SIZE, 0);
        if (recieeve_massage[0] == '$')
        {
            
            char *start = strchr(recieeve_massage, '$') + 1;
            char *end = strchr(start, '$');

            int portLength = end - start;

            char *port = new char[portLength + 1];

            strncpy(port, start, portLength);
            write(1,port,sizeof(port));
            
            old_server_addr = server_addr;
            old_server_fd = server_fd;
            server_addr.sin_family = AF_INET;
            if (inet_pton(AF_INET, ipaddr, &(server_addr.sin_addr)) == -1)
                perror("FAILED: Input ipv4 address invalid");

            if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
                perror("FAILED: Socket was not created");

            if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
                perror("FAILED: Making socket reusable failed");

            if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
                perror("FAILED: Making socket reusable port failed");

            memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

            server_addr.sin_port = htons(strtol(port, NULL, 10));

            if (connect(server_fd, (sockaddr *)(&server_addr), sizeof(server_addr)))
                perror("FAILED: Connect");
        }
        else{
        write(STDOUT, recieeve_massage, strlen(recieeve_massage));

        // Send messages
        char send_massage[BUFFER_SIZE];
        memset(send_massage, 0, BUFFER_SIZE);
        read(0, send_massage, sizeof(send_massage));
        send_massage[strcspn(send_massage, "\n")] = 0;
        send(server_fd, send_massage, strlen(send_massage), 0);
    }}

    return 0;
}