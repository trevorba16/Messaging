#include "server.h"


Server::Server(int port) {
    // setup variables
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_ + 1];
    cache_ = "";

    //initialize users
    users.clear();
}

Server::~Server() {
    delete buf_;
}

void
Server::run() {
    // create and run the server
    create();
    serve();
}

void
Server::create() {
    struct sockaddr_in server_addr;

    // setup socket address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create socket
    server_ = socket(PF_INET, SOCK_STREAM, 0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // set socket to immediately reuse port when the application closes
    int reuse = 1;
    if (setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(-1);
    }

    // call bind to associate the socket with our local address and
    // port
    if (bind(server_, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    // convert the socket to listen for incoming connections
    if (listen(server_, SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }
}

void
Server::close_socket() {
    //cout << "CLOSING THE SOCKET" << endl;
    close(server_);
}

void
Server::serve() {
    //cout << "in server::serve" << endl;
    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

    // accept clients
    while ((client = accept(server_, (struct sockaddr *) &client_addr, &clientlen)) > 0) {

        handle(client);
    }
    close_socket();
}

void
Server::handle(int client) {
    //cout << "in server::handle. client is "<< client << endl;
    while (1) 
    {
        string request = get_request(client);
        if (request.empty()) 
        {
            cout << "REQUEST EMPTY, CLOSING CLIENT" << endl;
            break;
        }
        process_request(client, request);
    }
    cache_ = "";
    buf_ = new char[buflen_ + 1];
    close(client);
}

string
Server::get_request(int client) {
	string request = "";
	if (cache_.length() > 0) {
		request = cache_;
	}

	// read until we get a newline
	while (request.find("\n") == string::npos) {
		int nread = recv(client, buf_, 1024, 0);
		if (nread < 0) {
			if (errno == EINTR)
				// the socket call was interrupted -- try again
				continue;
			else {
				// an error occurred, so break out
				return "";
			}
		}
		else if (nread == 0) {
			// the socket is closed
			return "";
		}
		// be sure to use append in case we have binary data
		request.append(buf_, nread);
	}

	// a better server would cut off anything after the newline and
	for (int i = 0; i < request.length(); i++) {
		if (request[i] == '\n') {
			cache_ = request.substr(i + 1, request.length() - (i + 1));
			request = request.substr(0, i);
			break;
		}
	}
	return request;
}

void
Server::process_request(int client, string request) {
	istringstream ss(request);
	string command = "";
	ss >> command;
	if (command == "put")
	{
		put_message(client, ss);
		return;
	}
	else if (command == "list")
	{
		string userName;
		ss >> userName;
		process_list(client, userName);
		return;
	}
	else if (command == "get")
	{
		string userName;
		ss >> userName;
		int index;
		ss >> index;
		get_message(client, userName, index);
		return;
	}

	else if (command == "reset")
	{
		//cout << "we successfully reseted" << endl;
		for (int i = 0; i < users.size(); i++)
		{
			users[i].messages.clear();
		}

		//send to client and stuff------------------------------------------------------------------
		send_response(client, "OK\n");//--------------------------------------------------
		return;
	}
	else
	{
		//cout << "ERROR! UNRECOGNIZED COMMAND!" << endl;
		//     //return send_response(client, "BAD");------------------
		send_response(client, "error UNRECOGNIZED COMMAND/BAD INPUT!\n");//------------------
		return;
	}
}

void Server::put_message(int client, istringstream &ss) {
	User user;
	Message message;
	string userName;
	ss >> userName;
	for (int i = 0; i < users.size(); i++)
	{
		if (users[i].name == userName)
		{
			//cout << "Existing user" << endl;
			ss >> message.subject >> message.length;
			if (message.length == 0)
			{
				//cout << "We don't need this message" << endl;
				send_response(client, "error MESSAGE NEEDS CHARACTERS!\n");//------------------
				return;
			}
			print_message_object(message);

			bool out = get_value(client, message);
			print_message_object(message);
			users[i].messages.push_back(message);
			if (out == true)
			{
				send_response(client, "OK\n");//------------------
			}
			else
			{
				//     //return send_response(client, "BAD!"");--------------------
				send_response(client, "error CLIENT DIDN'T GIVE US CORRECT VALUES/NO VALUES RECIEVED!\n");//------------------
			}
			return;
		}
	}
	//cout << "We must make a user" << endl;
	user.name = userName;
	ss >> message.subject >> message.length;
	if (message.length == 0)
	{
		//cout << "message is too short" << endl;
		//     //return send_response(client, "BAD");-----------
		send_response(client, "error MESSAGE NEEDS CHARACTERS!\n");//------------------
		return;
	}
	//message.needed = true;
	//print_message_object(message);
	//call get_value
	bool out = get_value(client, message);
	//print_message_object(message);
	user.messages.push_back(message);
	users.push_back(user);
	if (out == true)
	{
		send_response(client, "OK\n");//------------------
	}
	else
	{
		//     //return send_response(client, "BAD!"");--------------------
		send_response(client, "error CLIENT DIDN'T GIVE US CORRECT VALUES/NO VALUES RECIEVED!\n");//------------------
	}
	return;
}

bool
Server::get_value(int client, Message &message) 
{
    int len = message.length;
    
    if (len <= cache_.length()) {
        message.value = cache_.substr(0, len);
        cache_ = cache_.substr(len);
        return true;
    }
    while (len > 0) {//do we still need more bytes?
        int nread = recv(client, buf_, 1024, 0);
        if (nread < 0) {
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return false;
        } else if (nread == 0) {
            // the socket is closed
            return false;
        } else {
            cache_.append(buf_, nread);

            if (len <= cache_.length()) {
                message.value = cache_.substr(0, len);
                cache_ = cache_.substr(len);
                len = 0;
            }
        }
    }
    //all done with the message
    return true;
}

void
Server::process_list(int client, string userName)//gets and sends the list to client.
{
    //cout << "in server::process_list. userName is  "<< userName << endl;
    for(int i = 0; i < users.size(); i++)
    {
        if(users[i].name == userName)
        {
            //found em
            ostringstream oss;
            //cout << "found " << userName << endl;
            //cout << endl;
            //cout << "list " << users[i].messages.size() << endl;
            oss  << "list " << users[i].messages.size() << endl;
            for(int j = 0; j < users[i].messages.size(); j++)
            {
                cout << j + 1 << " " << users[i].messages[j].subject << endl;
                oss << j + 1 << " " << users[i].messages[j].subject << endl;//because client is a &(%%&())----
            }
            send_response(client, oss.str());//--------------------------------------------------
            return;
        }
    }
    //     //return send_response(client, "BAD");------------------
    send_response(client, "error THIS USER DOESN'T EXIST/BAD INPUT!\n");//------------------
    return;
}

void 
Server::get_message(int client, string userName, int index)//gets and sends the one message to client
{
    //cout << "in server::get_message. userName is  "<< userName << "index is " << index << endl;
    for(int i = 0; i < users.size(); i++)
    {
        //cout << users[i].name  << " " << userName << endl;
        if(users[i].name == userName)
        {
            //cout << "we in!" << endl;
            index--;//because client is a ^$#(@ ----------------------------------------------------------
            if(0 <= index  &&  index < users[i].messages.size())
            {
                ostringstream oss;
                //cout << "found " << userName << " and a message at index " << index << endl;
                //cout << endl;
                //cout << "message " << users[i].messages[index].subject << " " << users[i].messages[index].length 
                //        << endl << users[i].messages[index].value << endl;
                oss << "message " << users[i].messages[index].subject << " " << users[i].messages[index].length 
                        << endl << users[i].messages[index].value << endl;        
                //send to client and stuff-------------------------------------------------------------------------iss?----
                send_response(client, oss.str());//--------------------------------------------------
                return;
            }
            else
            {   
                send_response(client, "error THIS MESSAGE INDEX DOESN'T EXIST/BAD INPUT!\n");//------------------
                return;
            }
        }
    }
    send_response(client, "error THIS USER DOESN'T EXIST/BAD INPUT!\n");//------------------
    return;
}

bool
Server::send_response(int client, string response) {
    //cout << "in server::send_response. client is "<< client << " response is " << response << endl;
    // prepare to send response
    const char *ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(client, ptr, nleft, 0)) < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                continue;
            } else {
                // an error occurred, so break out
                perror("write");
                return false;
            }
        } else if (nwritten == 0) {
            // the socket is closed
            return false;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return true;
}


void
Server::print_message_object(Message &message) {
     cout << endl;
     cout << "subject: " << message.subject << endl;
     cout << "length:" << message.length << endl;
     cout << "value: " << message.value << endl;
     //cout << "needed ";
     //if (message.needed) cout << "true";
     //else cout << "false";
     //cout << endl;

     cout << endl;
 }
