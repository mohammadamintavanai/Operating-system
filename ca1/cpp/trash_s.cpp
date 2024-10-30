#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <map>
#include <sstream>
#include <iostream>
using namespace std;
typedef struct pollfd pollfd;

#define STDOUT 1
#define BUFFER_SIZE 1024

const char* SERVER_LAUNCHED = "Server Launched!\n";
const char* NEW_CONNECTION = "New Connection!\n";
const char* ENTER_NAME = "Enter your name: ";
const char* ROOM_LIST = "Available Rooms:";


const char* JOIN_ERROR = "Invalid room selection\n";

struct user_inf{
    int state; // 0: not in room, 1: in room
    string name;
};

struct room_inf{
    int state; // 0: empty, 1: one player, 2: full
    int no;
    int player_1; // -1: empty, otherwise fd of player 1
    int player_2; // -1: empty, otherwise fd of player 2
    int fd;
    sockaddr_in sock_address;
};

int main(int argc, char* argv[])
{
    if (argc != 4)
        perror("Invalid Arguments");

    char* ipaddr = argv[1];
    struct sockaddr_in server_addr;
    int server_fd, opt = 1;
    vector<pollfd> pfds;
    map<int, string> fd_to_name; 
    map<int, user_inf> users_inf;
    vector<room_inf> room_infs;
   
    // Initialize rooms
    for(int j = 0; j < stoi(argv[3]); j++) {
        room_infs.push_back({0, j, -1, -1,-1,{}});

    }

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

    if (bind(server_fd, (const struct sockaddr*)(&server_addr), sizeof(server_addr)) == -1)
        perror("FAILED: Bind unsuccessfull");

    if (listen(server_fd, 20) == -1)
        perror("FAILED: Listen unsuccessfull");

    write(1, SERVER_LAUNCHED, strlen(SERVER_LAUNCHED));

    pfds.push_back(pollfd{server_fd, POLLIN, 0});
    for(int j = 0; j < stoi(argv[3]); j++) {
        room_infs[j].sock_address.sin_family = AF_INET;
    if (inet_pton(AF_INET, ipaddr, &(room_infs[j].sock_address.sin_addr)) == -1)
        perror("FAILED: Input ipv4 address invalid");

    if ((room_infs[j].fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        perror("FAILED: Socket was not created");

    if (setsockopt(room_infs[j].fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        perror("FAILED: Making socket reusable failed");

    if (setsockopt(room_infs[j].fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        perror("FAILED: Making socket reusable port failed");

    memset(room_infs[j].sock_address.sin_zero, 0, sizeof(room_infs[j].sock_address.sin_zero));

    room_infs[j].sock_address.sin_port = htons(1+j+strtol(argv[2], NULL, 10));

    if (bind(room_infs[j].fd, (const struct sockaddr*)(&room_infs[j].sock_address), sizeof(room_infs[j].sock_address)) == -1)
        perror("FAILED: Bind unsuccessfull");

    if (listen(server_fd, 2) == -1)
        perror("FAILED: Listen unsuccessfull");
    }
    
    for (size_t i = 0; i < room_infs.size(); ++i) {
        pfds.push_back(pollfd{room_infs[i].fd, POLLIN, 0});
    }
    while (1)
    {
        if (poll(pfds.data(), (nfds_t)(pfds.size()), -1) == -1)
            perror("FAILED: Poll");

        for (size_t i = 0; i < pfds.size(); ++i)
        {
            if (pfds[i].revents & POLLIN)
            {
                if (pfds[i].fd == server_fd) // new user
                {
                    struct sockaddr_in new_addr;
                    socklen_t new_size = sizeof(new_addr);
                    int new_fd = accept(server_fd, (struct sockaddr*)(&new_addr), &new_size);
                    write(1, NEW_CONNECTION, strlen(NEW_CONNECTION));

                    // Send "Enter your name" prompt
                    send(new_fd, ENTER_NAME, strlen(ENTER_NAME), 0);

                    pfds.push_back(pollfd{new_fd, POLLIN, 0});
                    
                    users_inf[new_fd] = user_inf{0,""};

                    // Send available rooms
                    
                }
                else // message from user
                {
                    
                    
                    // Handle messages based on user state
                    if (users_inf[pfds[i].fd].state == 0) {
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    recv(pfds[i].fd, buffer, BUFFER_SIZE, 0);
                        // User is not in a room (get name)
                        users_inf[pfds[i].fd].name = buffer;
                        users_inf[pfds[i].fd].state = 1;
                        write(1, "Name: ", strlen("Name: "));
                        write(1, buffer, strlen(buffer));
                        write(1, "\n", 1);

                        // Send available rooms
                        stringstream room_list;
                        room_list << ROOM_LIST;
                        for (size_t j = 0; j < room_infs.size(); ++j) {
                            if (room_infs[j].state == 0 || room_infs[j].state == 1) {
                                room_list <<  room_infs[j].no << " ";
                            }
                        }
                        room_list << "\n";
                        send(pfds[i].fd, room_list.str().c_str(), room_list.str().length(), 0);
                        

                    } else  if(users_inf[pfds[i].fd].state == 1){
                        
                        char buffer[BUFFER_SIZE];
                        memset(buffer, 0, BUFFER_SIZE);
                        recv(pfds[i].fd, buffer, BUFFER_SIZE, 0);
                        string bufferString(buffer);
                        int number;
                        stringstream ss(bufferString);
                        int found = 0;
                        if (ss >> number && ss.eof()) {

                            for (size_t j = 0; j < room_infs.size(); ++j) {
                                if (number == room_infs[j].no && room_infs[j].state == 0){
                                    
                                    room_infs[j].player_1 = pfds[i].fd;
                                    room_infs[j].state = 1;
                                    stringstream ss;
                                    ss << "$" << ntohs(room_infs[j].sock_address.sin_port) << "$";
                                    string port = ss.str();
                                    const char* port_c = port.c_str();
                                    send(pfds[i].fd,port_c,sizeof(port_c),0);
                                    found = 1;

                                }
                                else if(number == room_infs[j].no && room_infs[j].state == 1){
                                    room_infs[j].player_2 = pfds[i].fd;
                                    room_infs[j].state = 2;
                                    
                                    found = 1;   
                                }
                                
                            }
                            if(found==0){
                                send(pfds[i].fd,JOIN_ERROR,strlen(JOIN_ERROR),0);
                            }
                            
                        
                    }
                    else{
                        send(pfds[i].fd,JOIN_ERROR,strlen(JOIN_ERROR),0);
                    }

                }
            }
        }
    }
    }
    return 0;
}