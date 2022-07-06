#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <cstring>

#define BUFFER_SIZE 1024
#define SERVER_PORT 26080

int main(int argc, char* argv[])
{
    srand(time(0));

    int client;
    char buffer[BUFFER_SIZE];
    std::string server_ip = "127.0.0.1";

    sockaddr_in server_addr;
    client = socket(AF_INET, SOCK_STREAM, 0);
    if(client < 0)
    {
        std::cout << "failed to create socket." << std::endl;
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    if (connect(client,(sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cout << "failed to connect to the server on port " << SERVER_PORT << std::endl;
        return -2;
    }

    //the connection is succesful, lets send data.
    std::cout << "The monitor is up and running." << std::endl;
    if(argc != 2)
    {
        std::cout << "wrong number of arguents." << std::endl;
        return -3;
    }

    std::string arg = argv[1];
    if(arg != "TXLIST")
    {
        std::cout << "unrecognized command line argiment." << std::endl;
        return -4;
    }

    strcpy(buffer, arg.c_str());
    send(client, buffer, BUFFER_SIZE, 0);
    std::cout << "Monitor sent a sorted list request to the main server." << std::endl;

    recv(client, buffer, BUFFER_SIZE, 0);
    std::string response(buffer);
    if(response == "SUCCESS")
    {
        std::cout << "Successfully received a sorted list of transactions from the main server." << std::endl;
        return 1;
    }
    
    return 0;
}


