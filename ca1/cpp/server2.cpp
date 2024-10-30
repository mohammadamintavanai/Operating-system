

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
const char* NEW_CONNECTION = "New connection\n";
const char* SERVER_LAUNCHED = "Server launched!\n";
const char* ENTERE_NAME = "Please entere your name\n";
const char* JOIN_ERROR = "Joining failed\n";
const char* GET_CHOICE = "Please entere your choice\nsang kaghaz gheychi\n";

enum ROOM_STATE {
    EMPTY,
    ONE_GAMER_JOINED,
    FULL,
    FIRST_RECIEVE,
    SECOND_RECIEVE
};
enum Client_state{
    CHOOSE_NAME,
    CHOOSE_ROOM,
    TRANSFER_TO_ROOM,
    CHOOSE_FOR_GAME
};
struct Client_inf{
    char* name;
    int fd;
    Client_state state;
    int number_of_win;

};
char* deep_copy_char(const char* source) {
    if (source == nullptr) {
        return nullptr;
    }
    size_t length = strlen(source) + 1;
    char* copy = new char[length];
    strcpy(copy, source); 
    return copy;
}
class Room { // Move this declaration above Server
    private:
    public:
    int no;
    
    ROOM_STATE state;
    char* player_1;
    char* player_2;
    int player_1_fd;
    int player_2_fd;
    char* ipaddr;
    struct sockaddr_in room_addr;
    int room_fd, opt = 1;
    long port; 
    char player_1_choice[1024];
    char player_2_choice[1024];
    
    sockaddr_in* player_1_sock;
    sockaddr_in* player_2_sock;
    Room(char* _ip,long _port,int _no){

        no = _no;
        state = EMPTY;
        
        ipaddr = _ip;
        room_addr.sin_family = AF_INET;
        if (inet_pton(AF_INET, ipaddr, &(room_addr.sin_addr)) == -1)
            perror("FAILED: Input ipv4 address invalid");
    
        if ((room_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
            perror("FAILED: Socket was not created");
    
        if (setsockopt(room_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
            perror("FAILED: Making socket reusable failed");
    
        if (setsockopt(room_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
            perror("FAILED: Making socket reusable port failed");
    
        memset(room_addr.sin_zero, 0, sizeof(room_addr.sin_zero));
    
        room_addr.sin_port = htons(_port);
        port = _port; 

        if (bind(room_fd, (const struct sockaddr*)(&room_addr), sizeof(room_addr)) == -1)
            perror("FAILED: Bind unsuccessfull");
    
        if (listen(room_fd, 10) == -1)
            perror("FAILED: Listen unsuccessfull");     
    }  

};

class Server {
    private:
    public:
    int rooms_n;
    char* ipaddr;
    struct sockaddr_in server_addr;
    int server_fd, opt = 1;
    long port;
    vector<pollfd> pfds;
    vector<Room*> rooms;
    vector<Client_inf> clients_inf;
    Server(char* _ip,char* _port,int _rooms_n){
        ipaddr = _ip;
        rooms_n = _rooms_n;
        server_addr.sin_family = AF_INET;
        if (inet_pton(AF_INET, ipaddr, &(server_addr.sin_addr)) == -1)
            perror("FAILED: Input ipv4 address invalid");
    
        if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
            perror("FAILED: Socket was buffernot created");
    
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
            perror("FAILED: Making socket reusable failed");
    
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
            perror("FAILED: Making socket reusable port failed");
    
        memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));
    
        server_addr.sin_port = htons(strtol(_port, NULL, 10));
        port = strtol(_port, NULL, 10); 
    

        
        if (bind(server_fd, (const struct sockaddr*)(&server_addr), sizeof(server_addr)) == -1)
            perror("FAILED: Bind unsuccessfull");
        if (listen(server_fd, 20) == -1)
            perror("FAILED: Listen unsuccessfull");
        write(1, SERVER_LAUNCHED, strlen(SERVER_LAUNCHED));
        pfds.push_back(pollfd{server_fd, POLLIN, 0});
        add_rooms();
        add_rooms_fd();
        
    }
    void add_rooms(){
        for (int i=0;i<rooms_n;i++){
            rooms.push_back(new Room(ipaddr,port+i+1,i));
        }
    }
    void add_rooms_fd(){
        for (int i=0;i<rooms_n;i++){
            pfds.push_back(pollfd{rooms[i]->room_fd, POLLIN, 0});
        }
    }

    void handle_new_connection(){
        struct sockaddr_in new_addr;
        socklen_t new_size = sizeof(new_addr);
        int new_fd = accept(server_fd, (struct sockaddr*)(&new_addr), &new_size);
        write(1, NEW_CONNECTION, strlen(NEW_CONNECTION));
        pfds.push_back(pollfd{new_fd, POLLIN, 0});
        clients_inf.push_back({nullptr,new_fd,CHOOSE_NAME,0});
        send(new_fd,ENTERE_NAME,strlen(ENTERE_NAME),0);
    }
    int is_room_fd(int fd){
        for(int i=0;i<rooms.size();i++){
            if (fd == rooms[i]->room_fd){
                return i;
            }
        }
        return -1;
    }
    int find_user(int fd){
        for(int i=0;i<clients_inf.size();i++){
            if(fd == clients_inf[i].fd){
                return i;
            }
        }
    }
void handle_name(int index) {
    char buffer[1024];
    char sended_massage[1024];
    memset(sended_massage, 0, sizeof(sended_massage));
    memset(buffer, 0, sizeof(buffer));

    recv(clients_inf[index].fd, buffer, sizeof(buffer), 0);

    // Deep copy of the name
    clients_inf[index].name = new char[strlen(buffer) + 1]; 
    strcpy(clients_inf[index].name, buffer);

    clients_inf[index].state = CHOOSE_ROOM;

    stringstream ss;
    for (int i = 0; i < rooms.size(); i++) {
        if(rooms[i]->state == EMPTY || rooms[i]->state == ONE_GAMER_JOINED)
            ss << rooms[i]->no;
        if (i < rooms.size() - 1) {
            ss << " ";
        }
    }
    ss << "\n";

    // Copy the stringstream content to sended_massage
    strcpy(sended_massage, ss.str().c_str()); 

    // Send the room list
    send(clients_inf[index].fd, sended_massage, strlen(sended_massage), 0);
}
    void handle_room(int index){
        char buffer[1024];
        memset(buffer, 0, 1024);
        recv(clients_inf[index].fd, buffer, 1024, 0);
        string bufferString(buffer);
        int number;
        stringstream ss(bufferString);
        int found = 0;
        if (ss >> number && ss.eof()) {

        for (size_t j = 0; j < rooms.size(); ++j) {
        if (number == rooms[j]->no && rooms[j]->state == EMPTY){
                                    
            rooms[j]->player_1 = clients_inf[index].name;
            rooms[j]->state = ONE_GAMER_JOINED;
            stringstream ss;
            ss << rooms[j]->port ;
            string port = ss.str();
            const char* port_c = port.c_str();
            send(clients_inf[index].fd,port_c,sizeof(port_c),0);
            found = 1;
            clients_inf[index].state = TRANSFER_TO_ROOM; 
            rooms[j]->player_1 = clients_inf[index].name;


            }
            else if(number == rooms[j]->no && rooms[j]->state == ONE_GAMER_JOINED){
                rooms[j]->player_2 = clients_inf[j].name;
                rooms[j]->state = FULL;
                stringstream ss;
                ss << (rooms[j]->port) ;
                string port = ss.str();
                const char* port_c = port.c_str();
                
                send(clients_inf[index].fd,port_c,sizeof(port_c),0);    
                found = 1;  
                clients_inf[index].state = TRANSFER_TO_ROOM; 
                rooms[j]->player_2 = clients_inf[index].name;
                


            }
                                
        }
        if(found==0){
            clients_inf[index].state = CHOOSE_NAME;
            send(clients_inf[index].fd,JOIN_ERROR,strlen(JOIN_ERROR),0);
        }
                            
                        
    }
    else{
            clients_inf[index].state = CHOOSE_NAME;
            send(clients_inf[index].fd,JOIN_ERROR,strlen(JOIN_ERROR),0);
        }

    }
    void handle_new_join(int index){

                    struct sockaddr_in* new_addr;
                    new_addr = new sockaddr_in();
                    socklen_t new_size = sizeof(*new_addr);
                    int new_fd = accept(rooms[index]->room_fd, (struct sockaddr*)(new_addr), &new_size);
                    if(rooms[index]->state == FULL){
                        rooms[index]->player_2_fd = new_fd;
                        rooms[index]->player_2_sock = new_addr;
                    send(rooms[index]->player_2_fd,GET_CHOICE,strlen(GET_CHOICE),0);
                    send(rooms[index]->player_1_fd,GET_CHOICE,strlen(GET_CHOICE),0);

                    }
                    else{
                        rooms[index]->player_1_fd = new_fd;
                        rooms[index]->player_1_sock = new_addr;
                    }
                    pfds.push_back(pollfd{new_fd, POLLIN, 0});
    }
    vector<int> is_client_room_fd(int fd){
        vector<int> result;
        for(int i = 0;i<rooms.size();i++){
            if(rooms[i]->player_1_fd == fd){
                result.push_back(i);
                result.push_back(1);
                return result;}
            else if(rooms[i]->player_2_fd == fd){
                result.push_back(i);
                result.push_back(2);
                return result;                
            }
        }
        result.push_back(-1);
        return result;
    }
    void handle_events(){
        while(1){
                
            if(poll(pfds.data(), (nfds_t)(pfds.size()), -1) == -1){
                perror("FAILED: Poll");}
              
            for(size_t i = 0; i < pfds.size(); i++){

                if(pfds[i].revents & POLLIN){
                    
                    if(pfds[i].fd == server_fd){

                        handle_new_connection();
                    }
                    else if(is_room_fd(pfds[i].fd)!= -1){
        
                        handle_new_join(is_room_fd(pfds[i].fd));
                    }
                    else if(is_client_room_fd(pfds[i].fd)[0]!= -1){
                        vector<int> result = is_client_room_fd(pfds[i].fd);
                        if(rooms[result[0]]->state == FULL){
                            if(result[1]==1){
                                memset(rooms[result[0]]->player_1_choice,0,1024);
                                recv(rooms[result[0]]->player_1_fd,rooms[result[0]]->player_1_choice,sizeof(rooms[result[0]]->player_1_choice),0);
    
                            }
                            else{
                                memset(rooms[result[0]]->player_2_choice,0,1024);
                                recv(rooms[result[0]]->player_2_fd,rooms[result[0]]->player_2_choice,sizeof(rooms[result[0]]->player_2_choice),0);    
                            }
                            rooms[result[0]]->state = FIRST_RECIEVE;
                        }
                        else if(rooms[result[0]]->state == FIRST_RECIEVE){
                            if(result[1]==1){
                                memset(rooms[result[0]]->player_1_choice,0,1024);
                                recv(rooms[result[0]]->player_1_fd,rooms[result[0]]->player_1_choice,sizeof(rooms[result[0]]->player_1_choice),0);
    
                            }
                            else{
                                memset(rooms[result[0]]->player_2_choice,0,1024);
                                recv(rooms[result[0]]->player_2_fd,rooms[result[0]]->player_2_choice,sizeof(rooms[result[0]]->player_2_choice),0);    
                            } 
                            rooms[result[0]]->state = SECOND_RECIEVE;
                            if(rooms[result[0]]->player_1_choice[0]=='h' && rooms[result[0]]->player_2_choice[0]=='h'){
                                stringstream ss;
                                ss<<"NO WINNER";
                                write(1,ss.str().c_str(),sizeof(ss.str().c_str()));
                            }
                            else if(rooms[result[0]]->player_1_choice[0]=='h'){
                                char* a = "2 bord";
                                write(1,a,strlen(a));

                            }
                            else if(rooms[result[0]]->player_2_choice[0]=='h'){
                                char *a = "1 bord";
                                write(1,a,strlen(a));
                            }

                        }
                        
                    }
                    else{
                        int index = find_user(pfds[i].fd);
                        if (clients_inf[index].state == CHOOSE_NAME){

                            handle_name(index); 
                            clients_inf[index].state = CHOOSE_ROOM;
                        }
                        else if(clients_inf[index].state == CHOOSE_ROOM){
                            handle_room(index);

                            
                        }

                    }
                }
            }
            
            
        }
    }
    
};


int main(int argc, char* argv[]){
    Server server(argv[1],argv[2],atoi(argv[3]));
    int numRooms = atoi(argv[3]);
    server.handle_events();
    
    
}
