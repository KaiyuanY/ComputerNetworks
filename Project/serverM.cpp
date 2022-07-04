#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <pthread.h>
#include <sstream>

#define BUFFER_SIZE 1024
//int client, monitor, serverA, serverB, serverC;
int addr_size;

void *handle_client_request(void *p_client_socket)
{
    int client_socket = *((int*)p_client_socket);
    delete (int*)p_client_socket;

    sockaddr_in client_addr;
    char buffer[BUFFER_SIZE];

    while(true)
    {
        std::cout << "waiting for connection..." << std::endl;

        int size = sizeof(sockaddr_in);
        int server = accept(client_socket, (sockaddr *)&client_addr, (socklen_t*)&size);

        recv(server, buffer, BUFFER_SIZE, 0);

        std::cout << buffer << std::endl;
        
        std::string data (buffer);
        std::stringstream ss(data);
        std::string word;
        
        ss >> word;
        if(word == "B")//check balance
        {
            std::string name;
            ss >> name;
            //pull the balance from back servers.
            
            std::cout << "check balance:" << name << std::endl;
            std::string response = "The balance of " + name + " is XXX.";
            strcpy(buffer, response.c_str());
            send(server, buffer, BUFFER_SIZE, 0);
            close(server);
        }
        else//transfer
        {
            std::string from, to, amount;
            ss >> from;
            ss >> to;
            ss >> amount;
            //do something with the transfer

            std::string response = "transfered " + amount + " from " + from + " to " + to;
            strcpy(buffer, response.c_str());
            send(server, buffer, BUFFER_SIZE, 0);
            close(server);
        }

    }
}


int setup_socket(int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        std::cout << "establishing socket failed." << std::endl;
        return -1;
    }

    sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if ((bind(sock, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) 
    {
        std::cout << "bind error." << std::endl;
        return -1;
    }

    if(listen(sock, 10) < 0)
    {
        std::cout << "listen error." << std::endl;
        return -1;
    }

    return sock;
}

int main()
{
    int client, monitor, serverA, serverB, serverC;
    int port_client_local = 25080;
    int port_monitor_local = 26080;
    int port_udp_out = 24080;
    int port_serverA = 21080, port_serverB = 22080, port_serverC = 23080;

    addr_size = sizeof(sockaddr_in);

    client = setup_socket(port_client_local);
    if(client < 0)
    {
        std::cout << "client socket failed." << std::endl;
        return -1;
    }
    monitor = setup_socket(port_monitor_local);
    if(monitor < 0)
    {
        std::cout << "monitor socket failed." << std::endl;
        return -2;
    }
    std::cout << "The main server is up and running." << std::endl;

    //handle client connection in a different thread
    pthread_t t_client;
    int *p_client = new int;
    *p_client = client;
    pthread_create(&t_client, NULL, handle_client_request, p_client);

    //handle monitor connection
    while(true)
    {
        sleep(5);
    }

    return 0;
}
