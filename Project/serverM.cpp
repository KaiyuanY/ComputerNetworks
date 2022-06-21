#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>

int main()
{
    //create socket
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if(listening < 0)
    {
        std::cout << "serverM socket failed." << std::endl;
        return -1;
    }

    //bind the socket to a ip/port
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(25000 + 080);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

    if(bind(listening, AF_INET, (socketaddr*)&hint, sizeof(hint)) == -1)
    {
        std::cout << "binding failed serverM." << std::endl;
        return -2;
    }

    //mark the socket for listening in
    if(listen(listening, SOMAXCONN) == -1)
    {
        std::cout << "listen failed serverM." << std::endl;
        return -3;
    }

    //accept a call
    sockaddr_in client;
    socklen_t = clientSize;
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    int clientSocket = accept(listening, (socketaddr*)&client, &clientSize);

    if(clientSocket == -1)
    {
        std::cout << "client connection failed serverM." << std::endl;
        return -4;
    }

    close(listening);

    memset(host, 0, NI_MAXHOST);
    memset(service, 0, NI_MAXSERV);

    int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0);
    if(result)
    {
        std::cout << host << " connected on " << service << std::endl;
    }
    else
    {
        std::cout << "" << std::endl;
    }

    return 0;
}
