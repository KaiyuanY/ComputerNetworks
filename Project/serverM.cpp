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
#include <vector>
#include <unordered_map>

#define BUFFER_SIZE 1024
#define DEBUG true
int client, monitor;
int port_client_local = 25080;
int port_monitor_local = 26080;
int port_udp_out = 24080;
int port_serverA = 21080, port_serverB = 22080, port_serverC = 23080;
int addr_size;


std::string encrypt(std::string line)
{
    unsigned_int pos = line.find(' ');
    for(unsigned_int i = pos+1; i < line.size(); i++)
    {
        if(line[i] == ' ')
        {
            continue;
        }
        if(line[i] >= 'a' && line[i] <= 'z')
        {
            line[i] = 'a' + ((line[i] - 'a' + 3)%26);
        }
        else if(line[i] >= 'A' && line[i] <= 'Z')
        {
            line[i] = 'A' + ((line[i] - 'A' + 3)%26);
        }
        else if(line[i] >= '0' && line[i] <= '9')
        {
            line[i] = '0' + ((line[i] - '0' + 3)%10);
        }
        else
        {
            //should not happen.
        }
    }
    return line;
}

std::string decrypt(std::string line)
{
    unsigned_int pos = line.find(' ');
    for(unsigned_int i = pos+1; i < line.size(); i++)
    {
        if(line[i] == ' ')
        {
            continue;
        }

        if(line[i] >= 'a' && line[i] <= 'z')
        {
            int change = line[i] - 'a' - 3;
            if(change < 0)
            {
                line[i] = 'z' + change;
            }
            else
            {
                line[i] = 'a' + change;
            }
        }
        else if(line[i] >= 'A' && line[i] <= 'Z')
        {
            int change = line[i] - 'A' - 3;
            if(change < 0)
            {
                line[i] = 'Z' + change;
            }
            else
            {
                line[i] = 'A' + change;
            }
        }
        else if(line[i] >= '0' && line[i] <= '9')
        {
            int change = line[i] - '0' - 3;
            if(change < 0)
            {
                line[i] = '9' + change;
            }
            else
            {
                line[i] = '0' + change;
            }
        }
        else
        {
            //should not happen.
        }
    }
    return line;
}

void *handle_client_request(void *p_client_socket)
{
    int client_socket = *((int*)p_client_socket);
    delete (int*)p_client_socket;

    sockaddr_in client_addr;
    char buffer[BUFFER_SIZE];

    int serverA_send_fd;
    int serverB_send_fd;
    int serverC_send_fd;
    int recv_fd;
    sockaddr_in senderA_addr, senderB_addr, senderC_addr, receiver_addr;

    int serverA_send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int serverB_send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int serverC_send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int recv_fd = socket(AF_INET, SOCK_DGRAM, 0);

    senderA_addr.sin_family = AF_INET;
    senderA_addr.sin_port = htons(port_serverA);
    serverA_addr.sin_addr.s_addr = INADDR_ANY;
    senderB_addr.sin_family = AF_INET;
    senderB_addr.sin_port = htons(port_serverB);
    serverB_addr.sin_addr.s_addr = INADDR_ANY;
    senderC_addr.sin_family = AF_INET;
    senderC_addr.sin_port = htons(port_serverC);
    serverC_addr.sin_addr.s_addr = INADDR_ANY;

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(port_udp_out);
    receiver_addr.sin_addr.s_addr = INADDR_ANY;

    bind(recv_fd, (sockaddr*)&receiver_addr, sizeof(receiver_addr));

    socklen_t len = 0;
    sockaddr_in their_addr;

    while(true)
    {
        // std::cout << "waiting for connection..." << std::endl;

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
            
            std::cout << "The main server received input=" << name <<
                " from the client using TCP over port " << port_client_local << std::endl;


            name = encrypt(name);
            if(DEBUG)
            {
                std::cout << "encrypted name = " << name << std::endl;
            }

            //server A
            char udp_buffer[BUFFER_SIZE];
            strcpy(udp_buffer, name.c_str());
            int len = sendto(serverA_send_fd, udp_buffer, BUFFER_SIZE, 
                            (sockaddr *)&senderA_addr, sizeof(senderA_addr));
            
            while(true)
            {
                int bytes = recvfrom(recv_fd, udp_buffer, BUFFER_SIZE, 0, (sockaddr*)&their_addr, &addr_len);
                std::string record(udp_buffer);
                if(record[0] != '#')
                {
                    record = decrypt(record);
                    if(DEBUG)
                    {
                        std::cout << "decrypted record = " << record << std::endl;
                    }
                }
                else
                {
                    break;
                }
            }



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
    srand(time(0));
    

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
