#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <istream>
#include <sstream>
#include <iostream>
#include <string>

//mine includes
#include <cstddef>


using namespace std;

struct Message {
    //string command;
    //string name; //for user
    string subject;//new for the assignment
    int length;
    string value;
    //bool needed;
};

//place to store it--------------------------------------------------
//user object to store messages
struct User {
    string name;
    vector<Message> messages;
};

class Server {
public:
    Server(int port);
    ~Server();

    void run();
    void create();
    void close_socket();
    void serve();
    void handle(int);
    string get_request(int);
    //bool handle_message(int, Message& message);
    bool send_response(int client, string response);
    void process_request(int client, string request);
	void put_message(int client, istringstream &ss);
	//very much different
    bool get_value(int client, Message& message);
    void print_message_object(Message& message);
    //new functions-----------------------------------------------
    void process_list(int client, string userName);
    void get_message(int client, string userName, int index);

    int port_;
    int server_;
    int buflen_;
    char* buf_;
    //for saving values
    string cache_;
    //place to store users------------------------------------------------------
    vector<User> users;
};