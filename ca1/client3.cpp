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
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <map>
#include <sstream>
using namespace std;
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
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <map>
const char* NUMBER_OF_ROOM = "The number of room is:";
const char* INITIAL_ROOM_SUCCESS = "initial room success";
const char* ENTER_NAME = "enter your name\n";
const char* MY_NAME_IS = "my name is:";

using namespace std;
int stringToNumber(const char* str) {
    std::stringstream ss(str);
    int number;
    ss >> number; // Convert string to integer
    return ss.fail() ? 0 : number; // Return 0 if conversion failed
}

// Split the input string into two parts based on a delimiter
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

// Check if two strings are equal
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

// Concatenate a string and an integer
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


    return result; // Return the concatenated string
}

// Concatenate two strings
char* concat_two_string(const char* s1, const char* s2) {
    size_t s1_length = strlen(s1);
    size_t s2_length = strlen(s2);
    char* result = new char[s1_length + s2_length + 1]; // +1 for null terminator

    strcpy(result, s1);
    strcat(result, s2);
    result[s2_length + s1_length] = '\0';
    return result; // Return the concatenated result
}

// Check if a string starts with a given prefix
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


class Client{
    private:
    public:
    char* ipaddr;
    long server_port;
    struct sockaddr_in server_addr;
    int server_fd, opt = 1;

    char* name;
    int rooms;

    long server_br_port;
    sockaddr_in br_sever;
    int server_br_en=1,server_br_opt=1;
    int br_server_fd;

    vector<long> rooms_br_port;
    vector<sockaddr_in> br_rooms;
    vector<int> br_rooms_en;
    vector<int> br_rooms_opt;
    vector<int>  br_rooms_fd;

    vector<pollfd> pfds;

    
    vector<char*> room_ip;
    vector<long> room_port;
    vector<int> room_fd;
    vector<sockaddr_in> room_sock;
    
    Client_state state;
    Client(char* _ip,char* _port){
        state = CHOOSE_NAME;

        ipaddr = _ip;
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
        server_port = strtol(_port, NULL, 10);
        pfds.push_back({server_fd,POLLIN,0});
        

        if(connect(server_fd, (sockaddr*)(&server_addr), sizeof(server_addr)))
            perror("FAILED: Connect");
       
        br_server_fd = socket(AF_INET,SOCK_DGRAM,0);
        setsockopt(br_server_fd,SOL_SOCKET,SO_BROADCAST,&server_br_en,sizeof(server_br_en));
        setsockopt(br_server_fd,SOL_SOCKET,SO_REUSEPORT,&server_br_port,sizeof(server_br_opt));

        br_sever.sin_family = AF_INET;
        br_sever.sin_port = htons(server_port*2);
        br_sever.sin_addr.s_addr = inet_addr(ipaddr);

        bind(br_server_fd,(struct sockaddr *)&br_sever,sizeof(br_sever));
        pfds.push_back({br_server_fd,POLL_IN,0});
        
    }
    void handle_event(){
        while(1){
            for(size_t i = 0; i < pfds.size(); ++i)
            {
                if(poll(pfds.data(), (nfds_t)(pfds.size()), -1) == -1){
                    perror("FAILED: Poll");}
                 if(pfds[i].revents & POLLIN){
                    if(pfds[i].fd == server_fd){
                        handle_server_msg();
                    }
                    else if(pfds[i].fd == br_server_fd){

                    }
                    else if(int index = is_fd_in_br_room(pfds[i].fd)){

                    }
                    else{

                    }
                }
            }
        }
    

    }

    void handle_server_msg(){
        char rec_msg[1024];
        memset(rec_msg,0,sizeof(rec_msg));
        recv(server_fd,rec_msg,sizeof(rec_msg),0);
        if(startsWith(rec_msg,NUMBER_OF_ROOM)){
            handle_initialing_room_inf_and_send_success(rec_msg);
        }
        if(startsWith(rec_msg,ENTER_NAME)){
            handle_enter_name_andsend_name(rec_msg);
        }


    }



    void handle_initialing_room_inf_and_send_success(char *rec_msg){
        char* n;
        char* temp;
        int room_n;
        splitArray(rec_msg,':',temp,n);
        room_n = stringToNumber(n);
        rooms = room_n;
        for(int i = 0;i<room_n;i++){
            br_rooms_fd.push_back(socket(PF_INET, SOCK_DGRAM, 0));
            br_rooms_en.push_back(1);
            br_rooms_opt.push_back(1);
            setsockopt(br_rooms_fd[i],SOL_SOCKET,SO_BROADCAST,&br_rooms_en[i],sizeof(br_rooms_en[i]));
            setsockopt(br_rooms_fd[i],SOL_SOCKET,SO_REUSEPORT,&br_rooms_opt[i],sizeof(br_rooms_opt[i]));
            br_rooms.push_back({});
            br_rooms[i].sin_family = AF_INET;
            br_rooms[i].sin_port = htons((server_port+i+1)*2);
            br_rooms[i].sin_addr.s_addr = inet_addr("255.255.255.255");
            bind(br_rooms_fd[i],(struct sockaddr *)&br_rooms_fd[i],sizeof(br_rooms_fd[i]));
        }
        send(server_fd,INITIAL_ROOM_SUCCESS,strlen(INITIAL_ROOM_SUCCESS),0);        
    }


    void handle_enter_name_andsend_name(char* rec_msg){
        char name[1024];
        char* send_msg;

        memset(name,0,sizeof(name));

        write(1,rec_msg,strlen(rec_msg));
        read(0,name,sizeof(name));
        send_msg = concat_two_string(MY_NAME_IS,name);
        write(1,send_msg,strlen(send_msg));
        send(server_fd,send_msg,strlen(send_msg),0);
    }
    bool is_fd_in_br_room(int fd){

    }
};
int main(int argc,char* argv[]){
    Client client(argv[1],argv[2]);
    client.handle_event();
    return 0;
}
