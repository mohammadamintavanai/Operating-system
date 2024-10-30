
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
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
const char* INVALID_ROOM = "you choose invalid room\n";
const char* HICHI = "hich\n";
enum Client_state{
    CHOOSE_NAME,
    CHOOSE_ROOM,
    TRANSFER_TO_ROOM,
    CHOOSE_FOR_GAME,
    FIRST_RECIEVE,
    SECOND_RECIEVE
};

void alarm_handler(int sig) {
    printf("tick tock\n");
}
class Client{
    private:
    public:
    char* ipaddr;
    long port;

    char* name;

    long server_br_port;
    sockaddr_in br_sever;
    int server_br_en;
    int br_server_fd;

    vector<long> rooms_br_port;
    vector<sockaddr_in> br_rooms;
    vector<int> br_rooms_en;
    vector<int>  br_rooms_fd;

    vector<pollfd> pfds;

    struct sockaddr_in server_addr;
    int server_fd, opt = 1;
    
    vector<char*> room_ip;
    vector<long> room_port;
    vector<int> room_fd;
    vector<sockaddr_in> room_sock;
    
    Client_state state;

    Client(char* _ip,char* _port){
        ipaddr = _ip;
        state = CHOOSE_NAME;
        server_addr.sin_family = AF_INET;
        if(inet_pton(AF_INET, ipaddr, &(server_addr.sin_addr)) == -1)
            perror("FAILED: Input ipv4 address invalid");

        if((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
            perror("FAILED: Socket was not created");

        if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
            perror("FAILED: Making socket reusable failed");

        if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
            perror("FAILED: Making socket reusable port failed");

        memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

        server_addr.sin_port = htons(strtol(_port, NULL, 10));
        port = strtol(_port, NULL, 10);
        pfds.push_back({server_fd,POLLIN,0});
        

        if(connect(server_fd, (sockaddr*)(&server_addr), sizeof(server_addr)))
            perror("FAILED: Connect");
    }
    void handle_choose_room(){
        char buffer[1024];
        char sended_massage[1024];
        memset(sended_massage, 0, sizeof(sended_massage));
        memset(buffer, 0, sizeof(buffer));
        recv(server_fd,buffer,sizeof(buffer),0);
        state = TRANSFER_TO_ROOM;
        write(1,buffer,strlen(buffer));
        read(1,sended_massage,sizeof(sended_massage));
        sended_massage[strcspn(sended_massage, "\n")] = 0;
        send(server_fd,sended_massage,strlen(sended_massage),0);
    }
    void connect_to_room(char *buffer){
        string bufferString(buffer);
        long number;
        stringstream ss(bufferString);
        ss >> number;
        room_port.push_back(number);
        room_ip.push_back(ipaddr);
        struct sockaddr_in room_socket;
        room_sock.push_back(room_socket);
        int new_room_fd;
        int opt = 1;
        room_sock[room_sock.size()-1].sin_family = AF_INET;
        if(inet_pton(AF_INET, room_ip[room_ip.size()-1], &(room_sock[room_sock.size()-1].sin_addr)) == -1)
            perror("FAILED: Input ipv4 address invalid");
        if((new_room_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
            perror("FAILED: Socket was not created");  
        if(setsockopt(new_room_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
            perror("FAILED: Making socket reusable failed");       
        if(setsockopt(new_room_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
            perror("FAILED: Making socket reusable port failed");   
        memset(room_sock[room_sock.size()-1].sin_zero, 0, sizeof(room_sock[room_sock.size()-1].sin_zero));   
        room_sock[room_sock.size()-1].sin_port = htons(number);
        room_fd.push_back(new_room_fd);
        if(connect(new_room_fd, (sockaddr*)(&room_sock[room_sock.size()-1]), sizeof(room_sock[room_sock.size()-1]))){

            perror("FAILED: Connect");}
        state = CHOOSE_FOR_GAME;
    }
    void handle_transfer(){
        char buffer[1024];
        memset(buffer,0,sizeof(buffer));
        read(server_fd,buffer,sizeof(buffer));
        if(buffer[0]=='J'){
            write(1,INVALID_ROOM,strlen(INVALID_ROOM));
            send(server_fd,name,strlen(name),0);
            state = CHOOSE_ROOM;
        }
        else{
            connect_to_room(buffer);
        }
    }
    void handle_events(){
        while(1){
            char buffer[1024];
            char sended_massage[1024];
            memset(buffer,0,1024);
            memset(sended_massage,0,1024);
            if(state == CHOOSE_NAME){
                handle_choose_name(buffer,sended_massage);
            }
            if(state == CHOOSE_ROOM){
                handle_choose_room();   
            }
            if(state == TRANSFER_TO_ROOM){
                handle_transfer();
            }
            if(state == CHOOSE_FOR_GAME){
                char buffer[1024];
                memset(buffer,0,sizeof(buffer));
                recv(room_fd[room_fd.size()-1],buffer,sizeof(buffer),0);
                write(1,buffer,strlen(buffer));
                char buff[1024];
                signal(SIGALRM, alarm_handler);
                siginterrupt(SIGALRM, 1);

                alarm(10);
                int read_ret = read(0, buff, 1024);
                alarm(0);
                if(read_ret == -1){
                    send(room_fd[room_fd.size()-1],HICHI,strlen(HICHI),0);
                    
                }
                else{
                    send(room_fd[room_fd.size()-1],buff,strlen(buff),0);
                }

            }
        }
    }
    void handle_choose_name(char* buffer,char* sended_massage){
                recv(server_fd,buffer,1024,0);
                write(1,buffer,strlen(buffer));
                state = CHOOSE_ROOM;
                read(1,sended_massage,1024);
                send(server_fd,sended_massage,strlen(sended_massage),0);

    }
    

};
int main(int argc,char* argv[]){
    Client client(argv[1],argv[2]);
    client.handle_events();
}