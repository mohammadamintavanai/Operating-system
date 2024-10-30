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

const char* SERVER_LAUNCHED = "server launched\n";
const char* NEW_CONNECTION = "New connection\n";
const char* NUMBER_OF_ROOM = "The number of room is:";
const char* INITIAL_ROOM_SUCCESS = "initial room success";
const char* ENTER_NAME = "enter your name\n";
const char* MY_NAME_IS = "my name is:";
const char* AVAILABLE_ROOMS = "available room is:";
const char* check="check";
int stringToNumber(const char* str) {
    std::stringstream ss(str);
    int number;
    ss >> number; // Convert string to integer
    return ss.fail() ? 0 : number; // Return 0 if conversion failed
}
void splitArray(const char* input, char splitChar, char*& firstPart, char*& secondPart) {
    size_t inputLength = strlen(input);
    size_t firstPartLength = 0;
    size_t secondPartLength = 0;
    bool splitOccurred = false;

    // First pass: Determine lengths of both parts
    for (size_t i = 0; i < inputLength; ++i) {
        if (input[i] == splitChar) {
            splitOccurred = true;
            continue; // Skip the split character
        }
        
        if (!splitOccurred) {
            firstPartLength++;
        } else {
            secondPartLength++;
        }
    }

    // Allocate memory for the new arrays
    firstPart = new char[firstPartLength + 1]; // +1 for null terminator
    secondPart = new char[secondPartLength + 1]; // +1 for null terminator

    // Second pass: Fill the new arrays
    size_t firstIndex = 0;
    size_t secondIndex = 0;
    
    splitOccurred = false; // Reset the flag for the second pass

    for (size_t i = 0; i < inputLength; ++i) {
        if (input[i] == splitChar) {
            splitOccurred = true; // Mark that we have passed the split character
            continue; // Skip the split character
        }
        
        if (!splitOccurred) {
            firstPart[firstIndex++] = input[i];
        } else {
            secondPart[secondIndex++] = input[i];
        }
    }

    // Null terminate the new arrays
    firstPart[firstPartLength] = '\0';
    secondPart[secondPartLength] = '\0';
}
bool areStringsEqual(const char* str1, const char* str2) {
    // Compare characters until the end of one string is reached
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return false; // Characters are not equal
        }
        str1++;
        str2++;
    }
    // Check if both strings reached the end (i.e., both are of the same length)
    return *str1 == *str2;
}
char* concatenateStringAndNumber(const char* str, int number) {
    // Convert number to string using stringstream
    std::stringstream ss;
    ss << number;
    std::string numberStr = ss.str();

    // Calculate the lengths of the input string and the number string
    size_t strLength = strlen(str);
    size_t numberStrLength = numberStr.length();

    // Allocate memory for the new concatenated string
    char* result = new char[strLength + numberStrLength + 1]; // +1 for null terminator

    // Copy the original string and the number string into the result
    strcpy(result, str);
    strcat(result, numberStr.c_str());

    return result;
}
char* concat_two_string(char* s1,char* s2){
    size_t s1_lengh = strlen(s1);
    size_t s2_lengh = strlen(s2);
    char* result = new char[s1_lengh+s2_lengh+1];
    strcpy(result,s1);
    strcat(result,s2);

    return result;
}

bool startsWith(const char* str, const char* prefix) {
    // Get lengths of both strings
    size_t prefixLength = strlen(prefix);
    size_t strLength = strlen(str);

    // If the prefix is longer than the string, it can't start with it
    if (prefixLength > strLength) {
        return false;
    }

    // Compare the beginning of str with the prefix
    return strncmp(str, prefix, prefixLength) == 0;
}

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
class Room {
    private:
    public:
    int no;
    ROOM_STATE state;
    
    char* player_1;
    char* player_2;
    int player_1_fd;
    int player_2_fd;
    char player_1_choice[1024];
    char player_2_choice[1024];

    char* ipaddr;
    long port;
    struct sockaddr_in room_addr;
    int room_fd, opt = 1;

    

    struct sockaddr_in br_room_addr;
    int br_room_fd, br_opt = 1,br_en =1;

     
    
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
    
        if (listen(room_fd, 3) == -1)
            perror("FAILED: Listen unsuccessfull");   

        br_room_fd =  socket(PF_INET, SOCK_DGRAM, 0);
        setsockopt(br_room_fd, SOL_SOCKET, SO_REUSEADDR, &br_en, sizeof(br_en));
        setsockopt(br_room_fd, SOL_SOCKET, SO_REUSEPORT, &br_opt, sizeof(br_opt));
        br_room_addr.sin_family = AF_INET;
        br_room_addr.sin_port = htons(port*2);
        br_room_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
        bind(br_room_fd,(const struct sockaddr*)(&br_room_addr), sizeof(br_room_addr));

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

    long br_port;
    sockaddr_in br_addr;
    int br_server_fd , br_opt =1 , br_en =1;

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

        br_server_fd = socket(PF_INET, SOCK_DGRAM, 0);
        setsockopt(br_server_fd, SOL_SOCKET, SO_REUSEADDR, &br_en, sizeof(br_en)) ;
        setsockopt(br_server_fd, SOL_SOCKET, SO_REUSEPORT, &br_opt, sizeof(br_opt));
            
        br_addr.sin_family = AF_INET;
        br_addr.sin_port = htons(port*2);
        br_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
        bind(br_server_fd, (const struct sockaddr*)(&br_addr), sizeof(br_addr));
        pfds.push_back({server_fd,POLLIN,0});
        pfds.push_back({br_server_fd,POLLIN,0});
        initial_rooms();

        

    }
    void initial_rooms(){
        for(int i =0;i<rooms_n;i++){
            rooms.push_back(new Room(ipaddr,port+i+1,i));
            pfds.push_back(pollfd{rooms[i]->room_fd, POLLIN, 0});
        }
    }
    void handle_event(){
        while(1){
            for(size_t i = 0; i < pfds.size(); ++i)
            {
                if(poll(pfds.data(), (nfds_t)(pfds.size()), -1) == -1){
                    perror("FAILED: Poll");}
                if(pfds[i].revents & POLLIN){
                if(pfds[i].fd == 0){

                }
                else if(pfds[i].fd == server_fd){
                    handle_new_connection();                }
                else  if(pfds[i].fd == br_server_fd){
            
                }
                else if(is_room_fd(pfds[i].fd)!=-1){
                    int index = is_room_fd(pfds[i].fd);
                }
                else if(is_client_in_room_fd(pfds[i].fd)[0] != -1){
                    vector<int> index = is_client_in_room_fd(pfds[i].fd);
                }
                else if(is_client_fd(pfds[i].fd)!=-1){
                    int index = is_client_fd(pfds[i].fd);
                    char rec_msg[1024];
                    memset(rec_msg,0,sizeof(rec_msg));
                    recv(clients_inf[index].fd,rec_msg,sizeof(rec_msg),0);
                    handle_client_massage_to_server(rec_msg,index);
                    
                }
            }
        }
    }
}
    void handle_client_massage_to_server(char* rec_msg,int index){
        if(startsWith(rec_msg,INITIAL_ROOM_SUCCESS)){
            rec_initial_room_and_send_name_msg(rec_msg,index);
        } 
        else if(startsWith(rec_msg,MY_NAME_IS)){
            write(1,check,strlen(check));
            initial_the_name(rec_msg,index);
        }
    }

    void initial_the_name(char* rec_msg,int index){

    char* name;
    char* temp;
    char send_msg[1024];
    memset(send_msg,0,sizeof(send_msg));
    splitArray(rec_msg,':',temp,name);

    clients_inf[index].name = new char[strlen(name) + 1]; 
    strcpy(clients_inf[index].name, name);

    clients_inf[index].state = CHOOSE_ROOM;

    stringstream ss;
    ss<<AVAILABLE_ROOMS;
    for (int i = 0; i < rooms.size(); i++) {
        if(rooms[i]->state == EMPTY || rooms[i]->state == ONE_GAMER_JOINED)
            ss << rooms[i]->no;
        if (i < rooms.size() - 1) {
            ss << " ";
        }
    }
    ss << "\n";

    // Copy the stringstream content to sended_massage
    strcpy(send_msg, ss.str().c_str()); 
    write(1,send_msg, strlen(send_msg));
    // Send the room list
    send(clients_inf[index].fd, send_msg, strlen(send_msg), 0);
}



    void handle_new_connection(){
        struct sockaddr_in new_addr;
        socklen_t new_size = sizeof(new_addr);
        int new_fd = accept(server_fd, (struct sockaddr*)(&new_addr), &new_size);
        write(1, NEW_CONNECTION, strlen(NEW_CONNECTION));

        pfds.push_back(pollfd{new_fd, POLLIN, 0});
        clients_inf.push_back({nullptr,new_fd,CHOOSE_NAME,0});
        
        char* number_of_room_msg = concatenateStringAndNumber(NUMBER_OF_ROOM,rooms_n);
        send(new_fd,number_of_room_msg,strlen(number_of_room_msg),0);
    }

    void rec_initial_room_and_send_name_msg(char* rec_msg,int index){
            send(clients_inf[index].fd,ENTER_NAME,strlen(ENTER_NAME),0);    
    }



    int is_room_fd(int fd){
        for(int i = 0;i<rooms.size();i++){
            if(rooms[i]->room_fd == fd){
                return i;
            }
        }
        return -1;
    }
    vector<int> is_client_in_room_fd(int fd){
        vector<int> index;
        for(int i=0;i<rooms.size();i++){
            if(rooms[i]->player_1_fd == fd){
                index.push_back(i);
                index.push_back(1);
                return index;
            }
            else if(rooms[i]->player_2_fd == fd){
                index.push_back(i);
                index.push_back(2);
                return index;                
            }
        }
        index.push_back(-1);
        return index;
    }
    int is_br_room_fd(int fd){
        for(int i = 0;i<rooms.size();i++){
            if(rooms[i]->br_room_fd == fd){
                return i;
            }
        }
        return -1;
    }
    int is_client_fd(int fd){
        for(int i = 0;i<clients_inf.size();i++){
            if(clients_inf[i].fd == fd){
                return i;
            }
        }
        return -1;
    }


};
int main(int argc,char* argv[]){
    stringstream ss;
    int number;
    ss << argv[3];
    ss >> number;
    Server server(argv[1],argv[2],number);
    server.handle_event();
    return 0;
}