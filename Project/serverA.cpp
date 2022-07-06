#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <sys/wait.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>

#define BUFFER_SIZE 1024
#define BLOCK_FILE "block1.txt"
#define LOCAL_PORT 21080
#define MAIN_SERVER_PORT 24080

int main()
{
    char buffer[BUFFER_SIZE];
    sockaddr_in sender_addr, receiver_addr, their_addr;
    socklen_t addr_len = 0;
    std::cout << "The ServerA is up and running using UDP on port: " << LOCAL_PORT << std::endl;

    //setup both incoming and outgoing udp specs
    int sender_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int receiver_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sender_fd == -1 || receiver_fd == -1)
    {
        std::cout << "socket creation failed." << std::endl;
        return -1;
    }

    sender_addr.sin_family = AF_INET;
    sender_addr.sin_port = htons(MAIN_SERVER_PORT);
    sender_addr.sin_addr.s_addr = INADDR_ANY;

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(LOCAL_PORT);
    receiver_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(receiver_fd, (sockaddr*)&receiver_addr, sizeof(receiver_addr)) < 0)
    {
        std::cout << "binding udp port failed." << std::endl;
        return -2;
    }

    while(true)
    {
        //initial receive
        int bytes = recvfrom(receiver_fd, buffer, BUFFER_SIZE, 0, (sockaddr*)&their_addr, &addr_len);
        if(bytes < 0)
        {
            std::cout << "ServerA: udp receive failed." << std::endl;
        }

        std::cout << "The ServerA received a request from the Main Server." << std::endl;

        std::string query(buffer);

        if(query == "TXLIST")
        {
            //TXLIST: send everything back
            std::ifstream block_file(BLOCK_FILE);
            std::string line;
            char send_buffer[BUFFER_SIZE];
            if(block_file.is_open())
            {
                while(getline(block_file, line))
                {
                    strcpy(send_buffer, line.c_str());
                    int len = sendto(sender_fd, send_buffer, BUFFER_SIZE, 0,
                            (sockaddr *)&sender_addr, sizeof(sender_addr));
                    if(len < 0)
                    {
                        std::cout << "Server: udp send failed." << std::endl;
                    }
                }
                block_file.close();
            }
            std::string last_line = "# 0";
            strcpy(send_buffer, last_line.c_str());
            int len = sendto(sender_fd, send_buffer, BUFFER_SIZE, 0,
                    (sockaddr *)&sender_addr, sizeof(sender_addr));
        }
        else if(query[0] >= '0' && query[0] <= '9')//it is a new transaction, append to block file
        {
            std::ofstream outfile(BLOCK_FILE, std::ios_base::app);
            outfile << query << std::endl;
            outfile.close();
            
        }
        else//send all records of the requested person
        {
            int max_serial = 0;//keep track of the largest serial number in this block
            std::ifstream block_file(BLOCK_FILE);
            std::string line;
            if(block_file.is_open())
            {
                while (getline(block_file, line))
                {
                    std::string line_cpy = line;
                    std::stringstream ss(line_cpy);
                    int serial;
                    ss >> serial;
                    if(serial > max_serial)
                    {
                        max_serial = serial;
                    }
                    std::string name1, name2;
                    ss >> name1;
                    ss >> name2;

                    if(name1 == query || name2 == query)//found a matching record
                    {
                        char send_buffer[BUFFER_SIZE];
                        strcpy(send_buffer, line.c_str());
                        int len = sendto(sender_fd, send_buffer, BUFFER_SIZE, 0,
                            (sockaddr *)&sender_addr, sizeof(sender_addr));
                        if(len < 0)
                        {
                            std::cout << "ServerA: udp send failed." << std::endl;
                        }
                    }
                }
                block_file.close();

                //after sending all records, send the max serial number
                char send_buffer[BUFFER_SIZE];
                std::string s = "# " + std::to_string(max_serial);
                strcpy(send_buffer, s.c_str());
                int len = sendto(sender_fd, send_buffer, BUFFER_SIZE, 0,
                            (sockaddr *)&sender_addr, sizeof(sender_addr));
            }
        }
        //done with the sending. back to the top of the while loop.
        std::cout << "The ServerA finished sending the response to the Main Server." << std::endl;
    }
}
