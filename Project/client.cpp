#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[])
{
    srand(time(0));
    int port = 20000 + rand() %100;
    int server_port = 25080;

    int client;
    char buffer[BUFFER_SIZE];
    std::string server_ip = "127.0.0.1";

    sockaddr_in server_addr;
    client = socket(AF_INET, SOCK_STREAM, 0);
    if(client < 0)
    {
        std::cout << "failed to create socket." << std::endl;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    if (connect(client,(sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cout << "failed to connect to the server on port " << server_port << std::endl;
    }

    //the connection is succesful, lets send data.
    std::cout << "The client is up and running." << std::endl;
    if (argc == 2) {//check wallet
        std::string name = argv[1];
        std::cout << name + " sent a balance enquiry request to the main server." << std::endl;

        std::string data = "B " + name; // B for balance
        strcpy(buffer, data.c_str());
        //std::cout << "buffer = [ " << buffer << " ]" << std::endl; 
        send(client, buffer, BUFFER_SIZE, 0);
        recv(client, buffer, BUFFER_SIZE, 0);
        std::string data_recv (buffer);
        std::cout << data_recv << std::endl;

        close(client);
        return 1;
    }
    else if (argc == 4) {//transfer
        std::string from = argv[1];
        std::string to = argv[2];
        std::string amount = argv[3];
        std::cout << from + " has requested to transfer " + amount + " txcoins to " + to + "." << std::endl;

        std::string msg = "T " + from + " " + to + " " + amount;
        strcpy(buffer, msg.c_str());
        send(client, buffer, BUFFER_SIZE, 0);
        recv(client, buffer, BUFFER_SIZE, 0);
        std::string msg_recv (buffer);
        
    }
    else {
        //unrecognized argc
        std::cout << "Wrong number of arguents, please check your inputs." << std::endl;
    }
    return 0;
}


