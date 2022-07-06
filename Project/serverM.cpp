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
#include <algorithm>
#include <fstream>
#include <cstring>

#include <sys/wait.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>

#define BUFFER_SIZE 1024
#define DEBUG false
#define PORT_UDP_M 24080
#define PORT_SERVER_A 21080
#define PORT_SERVER_B 22080
#define PORT_SERVER_C 23080
#define PORT_CLIENT_LOCAL 25080
#define PORT_MONITOR_LOCAL 26080

int client, monitor;
sockaddr_in senderA_addr, senderB_addr, senderC_addr, receiver_addr;
int serverA_send_fd;
int serverB_send_fd;
int serverC_send_fd;
int recv_fd;
int addr_size;

struct Transaction
{
    int id;
    std::string from;
    std::string to;
    int amount;
};


std::string encrypt(std::string line)
{
    //unsigned pos = line.find(' ');
    for(unsigned i = 0; i < line.size(); i++)
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
    //unsigned pos = line.find(' ');
    for(unsigned i = 0; i < line.size(); i++)
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
                line[i] = 'z' + change + 1;
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
                line[i] = 'Z' + change + 1;
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
                line[i] = '9' + change + 1;
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

int pull_from_back_server(int &send_fd, sockaddr_in &sender_addr, int &recv_fd, std::string name, std::vector<Transaction*> &records)
{
    int max_serial = 0;
    socklen_t addr_len = 0;
    sockaddr_in their_addr;
    char udp_buffer[BUFFER_SIZE];
    std::string encrypted_name;
    if(name != "TXLIST")
    {
        encrypted_name = encrypt(name);
    }
    else
    {
        encrypted_name = name;
    }
    strcpy(udp_buffer, encrypted_name.c_str());

    
    int len = sendto(send_fd, udp_buffer, BUFFER_SIZE, 0,
                    (sockaddr *)&sender_addr, sizeof(sender_addr));
    
    
    while(true)
    {
        int bytes = recvfrom(recv_fd, udp_buffer, BUFFER_SIZE, 0, (sockaddr*)&their_addr, &addr_len);
        std::string record(udp_buffer);
        if(record[0] != '#')
        {
            if(DEBUG)
            {
                std::cout << "encrypted record = " << record << std::endl;
            }
            //store the record
            std::stringstream ss (record);
            Transaction* t = new Transaction;
            ss >> t->id;
            std::string from, to, amount;
            ss >> from;
            ss >> to;
            ss >> amount;

            t->from = decrypt(from);
            t->to = decrypt(to);
            amount = decrypt(amount);
            t->amount = std::stoi(amount);
            records.push_back(t);

            if(DEBUG)
                std::cout << "id=" << t->id << ",from=" << t->from << ",to=" << t->to << ",amount=" << t->amount << std::endl;
        }
        else //last entry, which only contains the max serial number of the block
        {
            std::stringstream ss;
            ss << record;
            std::string trash;
            ss >> trash;
            ss << record;
            int temp_max;
            ss >> temp_max;
            
            if(temp_max > max_serial)
            {
                max_serial = temp_max;
            }
            if(DEBUG)
                std::cout << "max serial = " << max_serial << std::endl;
            break;
        }
    }
    return max_serial;
}

int calculate_balance(std::vector<Transaction*> records_A, std::vector<Transaction*> records_B, std::vector<Transaction*> records_C,
                        std::string name)
{
    int total = 1000;
    for(auto trans : records_A)
    {
        if(trans->from == name)
        {
            total -= trans->amount;
        }
        else
        {
            total += trans->amount;
        }
        delete trans;
    }
    for(auto trans : records_B)
    {
        if(trans->from == name)
        {
            total -= trans->amount;
        }
        else
        {
            total += trans->amount;
        }
        delete trans;
    }
    for(auto trans : records_C)
    {
        if(trans->from == name)
        {
            total -= trans->amount;
        }
        else
        {
            total += trans->amount;
        }
        delete trans;
    }
    return total;
}

void *handle_client_request(void *p_client_socket)
{

    int client_socket = *((int*)p_client_socket);
    delete (int*)p_client_socket;

    sockaddr_in client_addr;
    char buffer[BUFFER_SIZE];

    socklen_t addr_len = 0;
    sockaddr_in their_addr;

    while(true)
    {
                
        int size = sizeof(sockaddr_in);
        int server = accept(client_socket, (sockaddr *)&client_addr, (socklen_t*)&size);

        recv(server, buffer, BUFFER_SIZE, 0);
        
        std::string data (buffer);
        std::stringstream ss(data);
        std::string word;
        
        ss >> word;
        if(word == "B")//check balance
        {
            std::vector<Transaction*> records_A;
            std::vector<Transaction*> records_B;
            std::vector<Transaction*> records_C;
            int max_serial = 0;
            std::string name;
            ss >> name;
            //pull the balance from back servers.
            
            std::cout << "The main server received input=" << name <<
                " from the client using TCP over port " << PORT_CLIENT_LOCAL << std::endl;
            


            //int pull_from_back_server(int &send_fd, sockaddr_in &sender_addr, int &recv_fd, std::string name, std::vector<Transaction*> &records)
            std::cout << "The main server sent a request to server A" << std::endl;
            int max_serial_A = pull_from_back_server(serverA_send_fd, senderA_addr, recv_fd, name, records_A);
            std::cout << "The main server received transactions from Server A using UDP over port " << PORT_UDP_M << "." << std::endl;

            std::cout << "The main server sent a request to server B" << std::endl;
            int max_serial_B = pull_from_back_server(serverB_send_fd, senderB_addr, recv_fd, name, records_B);
            std::cout << "The main server received transactions from Server B using UDP over port " << PORT_UDP_M << "." << std::endl;

            std::cout << "The main server sent a request to server C" << std::endl;
            int max_serial_C = pull_from_back_server(serverC_send_fd, senderC_addr, recv_fd, name, records_C);
            std::cout << "The main server received transactions from Server C using UDP over port " << PORT_UDP_M << "." << std::endl;

            // int max_serial = max(max_serial_A, max(max_serial_B, max_serial_C));

            //now we have requested data from all 3 blocks, lets do calculations.
            if(records_A.size() == 0 && records_B.size() == 0 && records_C.size() == 0)
            {
                std::string response = "Unable to proceed with the transaction as " + name + " is not part of the network.";
                strcpy(buffer, response.c_str());
                send(server, buffer, BUFFER_SIZE, 0);
                close(server);
                continue;
            }

            //calculations
            int balance = calculate_balance(records_A, records_B, records_C, name);
            
            //respond to the client after all the calculations.
            std::string response = "The current balance of " + name + " is : " + std::to_string(balance) + " txcoins.";
            strcpy(buffer, response.c_str());
            send(server, buffer, BUFFER_SIZE, 0);

            std::cout << "The main server sent the current balance to the client." << std::endl;
            close(server);
        }
        else//transfer
        {
            //transfer info from client
            std::string from, to, amount;
            ss >> from;
            ss >> to;
            ss >> amount;
            std::cout << "The main server received from "+ from +" to transfer "+ amount 
                        + " coins to "+ to +" using TCP over port " + std::to_string(PORT_CLIENT_LOCAL) + "." << std::endl;
            //do something with the transfer
            std::vector<Transaction*> records_A_from, records_A_to;
            std::vector<Transaction*> records_B_from, records_B_to;
            std::vector<Transaction*> records_C_from, records_C_to;

            std::cout << "The main server sent a request to server A" << std::endl;
            int max_serial_A = pull_from_back_server(serverA_send_fd, senderA_addr, recv_fd, from, records_A_from);
            pull_from_back_server(serverA_send_fd, senderA_addr, recv_fd, to, records_A_to);
            std::cout << "The main server received transactions from Server A using UDP over port " << PORT_UDP_M << "." << std::endl;

            std::cout << "The main server sent a request to server B" << std::endl;
            int max_serial_B = pull_from_back_server(serverB_send_fd, senderB_addr, recv_fd, from, records_B_from);
            pull_from_back_server(serverB_send_fd, senderB_addr, recv_fd, to, records_B_to);
            std::cout << "The main server received transactions from Server B using UDP over port " << PORT_UDP_M << "." << std::endl;

            std::cout << "The main server sent a request to server C" << std::endl;
            int max_serial_C = pull_from_back_server(serverC_send_fd, senderC_addr, recv_fd, from, records_C_from);
            pull_from_back_server(serverC_send_fd, senderC_addr, recv_fd, to, records_C_to);
            std::cout << "The main server received transactions from Server C using UDP over port " << PORT_UDP_M << "." << std::endl;

            if(DEBUG)
            {
                std::cout << "******records_A_from's size = " << records_A_from.size() << std::endl;
                std::cout << "******records_A_to's size = " << records_A_to.size() << std::endl;
                std::cout << "******records_B_from's size = " << records_B_from.size() << std::endl;
                std::cout << "******records_B_to's size = " << records_A_to.size() << std::endl;
                std::cout << "******records_C_from's size = " << records_C_from.size() << std::endl;
                std::cout << "******records_C_to's size = " << records_C_to.size() << std::endl;
            }
            //check if both are part of the network
            if(records_A_from.size() == 0 && records_B_from.size() == 0 && records_C_from.size() == 0
                && records_A_to.size() == 0 && records_B_to.size() == 0 && records_C_to.size() == 0)
            {
                std::string response = "Unable to proceed with the transaction as " + from + " and " + to + " are not part of the network.";
                strcpy(buffer, response.c_str());
                send(server, buffer, BUFFER_SIZE, 0);
                close(server);
                std::cout << "The main server sent the result of the transaction to the client." << std::endl;
                continue;
            }
            else if(records_A_from.size() == 0 && records_B_from.size() == 0 && records_C_from.size() == 0)
            {
                std::string response = "Unable to proceed with the transaction as " + from + " is not part of the network.";
                strcpy(buffer, response.c_str());
                send(server, buffer, BUFFER_SIZE, 0);
                close(server);
                std::cout << "The main server sent the result of the transaction to the client." << std::endl;
                continue;
            }
            else if(records_A_to.size() == 0 && records_B_to.size() == 0 && records_C_to.size() == 0)
            {
                std::string response = "Unable to proceed with the transaction as " + to + " is not part of the network.";
                strcpy(buffer, response.c_str());
                send(server, buffer, BUFFER_SIZE, 0);
                close(server);
                std::cout << "The main server sent the result of the transaction to the client." << std::endl;
                continue;
            }

            //both people exist in the network, now check sender's balance 
            int balance = calculate_balance(records_A_from, records_B_from, records_C_from, from);
            int remainder = balance - std::stoi(amount);
            if(remainder < 0)//insufficient balance
            {
                std::string response = from + " was unable to transfer " + amount 
                                        + " txcoins to " + to + " because of insufficient balance.\nThe current balance of " 
                                        + from + " is " + std::to_string(balance); 
                strcpy(buffer, response.c_str());
                send(server, buffer, BUFFER_SIZE, 0);
                std::cout << "The main server sent the result of the transaction to the client." << std::endl;
                close(server);
                continue;
            }

            //if everything is fine, continue to transfer.
            int max_serial = std::max(max_serial_A, std::max(max_serial_B, max_serial_C));
            std::string new_record;
            new_record = std::to_string(max_serial + 1) + " " + encrypt(from) + " " + encrypt(to) + " " + encrypt(amount);
            
            char udp_buffer[BUFFER_SIZE];
            strcpy(udp_buffer, new_record.c_str());
            int rand_server = 1 + (rand() % 3);
            if(rand_server == 1)//server A
            {
                int len = sendto(serverA_send_fd, udp_buffer, BUFFER_SIZE, 0,
                    (sockaddr *)&senderA_addr, sizeof(senderA_addr));
            }
            else if(rand_server == 2)//server B
            {
                int len = sendto(serverB_send_fd, udp_buffer, BUFFER_SIZE, 0,
                    (sockaddr *)&senderB_addr, sizeof(senderB_addr));
            }
            else //server C
            {
                int len = sendto(serverC_send_fd, udp_buffer, BUFFER_SIZE, 0,
                    (sockaddr *)&senderC_addr, sizeof(senderC_addr));
            }

            if(DEBUG)
                std::cout << "new record was sent to server " << rand_server << std::endl;

            std::string response = from + " successfully transferred " + amount + " txcoins to " + to 
                                    + ".\nThe current balance of " + from + " is: " + std::to_string(remainder) + " txcoins.";
            strcpy(buffer, response.c_str());
            send(server, buffer, BUFFER_SIZE, 0);
            std::cout << "The main server sent the result of the transaction to the client." << std::endl;
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

    client = setup_socket(PORT_CLIENT_LOCAL);
    if(client < 0)
    {
        std::cout << "client socket failed." << std::endl;
        return -1;
    }
    monitor = setup_socket(PORT_MONITOR_LOCAL);
    if(monitor < 0)
    {
        std::cout << "monitor socket failed." << std::endl;
        return -2;
    }
    std::cout << "The main server is up and running." << std::endl;

    //udp connection setup
    serverA_send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    serverB_send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    serverC_send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    recv_fd = socket(AF_INET, SOCK_DGRAM, 0);

    senderA_addr.sin_family = AF_INET;
    senderA_addr.sin_port = htons(PORT_SERVER_A);
    senderA_addr.sin_addr.s_addr = INADDR_ANY;
    senderB_addr.sin_family = AF_INET;
    senderB_addr.sin_port = htons(PORT_SERVER_B);
    senderB_addr.sin_addr.s_addr = INADDR_ANY;
    senderC_addr.sin_family = AF_INET;
    senderC_addr.sin_port = htons(PORT_SERVER_C);
    senderC_addr.sin_addr.s_addr = INADDR_ANY;

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(PORT_UDP_M);
    receiver_addr.sin_addr.s_addr = INADDR_ANY;

    bind(recv_fd, (sockaddr*)&receiver_addr, sizeof(receiver_addr));

    //handle client connection in a different thread
    pthread_t t_client;
    int *p_client = new int;
    *p_client = client;
    pthread_create(&t_client, NULL, handle_client_request, p_client);

    //handle monitor connection
    while(true)
    {
        char buffer[BUFFER_SIZE];
        std::vector<Transaction*> records_A;
        std::vector<Transaction*> records_B;
        std::vector<Transaction*> records_C;
        std::vector<Transaction*> records_combined;
        int max_serial = 0;
        int size = sizeof(sockaddr_in);
        sockaddr_in monitor_addr;

        //get monitor connection
        int server = accept(monitor, (sockaddr *)&monitor_addr, (socklen_t*)&size);

        recv(server, buffer, BUFFER_SIZE, 0);
        std::string data (buffer);

        if(data != "TXLIST")
        {
            std::cout << "unrecognized monitor input." << std::endl;
        }
        std::cout << "The main server received a sorted list request from the monitor using TCP over port " 
                    << PORT_MONITOR_LOCAL << "." << std::endl;

        std::cout << "The main server sent a request to server A" << std::endl;
        int max_serial_A = pull_from_back_server(serverA_send_fd, senderA_addr, recv_fd, data, records_A);
        std::cout << "The main server received transactions from Server A using UDP over port " << PORT_UDP_M << "." << std::endl;

        std::cout << "The main server sent a request to server B" << std::endl;
        int max_serial_B = pull_from_back_server(serverB_send_fd, senderB_addr, recv_fd, data, records_B);
        std::cout << "The main server received transactions from Server B using UDP over port " << PORT_UDP_M << "." << std::endl;

        std::cout << "The main server sent a request to server C" << std::endl;
        int max_serial_C = pull_from_back_server(serverC_send_fd, senderC_addr, recv_fd, data, records_C);
        std::cout << "The main server received transactions from Server C using UDP over port " << PORT_UDP_M << "." << std::endl;

        if(DEBUG)
            std::cout << "Transactions vector size = " << records_combined.size() << std::endl;
        //combine results from 3 servers
        for(auto trans : records_A)
        {
            records_combined.push_back(trans);
        }
        for(auto trans : records_B)
        {
            records_combined.push_back(trans);
        }
        for(auto trans : records_C)
        {
            records_combined.push_back(trans);
        }
        //sort the list and print it to file
        std::sort(records_combined.begin(), records_combined.end(), [](const Transaction* lhs, const Transaction* rhs) {
            return lhs->id < rhs->id;
        });
        std::ofstream txlist_file("txchain.txt");
        for(auto trans : records_combined)
        {
            txlist_file << trans->id << " " << trans->from << " " << trans->to << " " << trans->amount << std::endl;
            delete trans;
        }
        txlist_file.close();

        std::string response = "SUCCESS";
        strcpy(buffer, response.c_str());
        send(server, buffer, BUFFER_SIZE, 0);

        std::cout << "The main server sent the result of the transaction to the client." << std::endl;
    }

    return 0;
}
