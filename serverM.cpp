#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <iostream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <signal.h>
struct sockaddr_in serverM_UDP_addr, serverM_TCP_addr, subserver_addr;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
void handleUDP(int sockM_UDP_fd)
{
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    char buffer[1024] = {0};
    int n = recvfrom(sockM_UDP_fd, buffer, 1023, 0, (struct sockaddr *)&cli_addr, &clilen);
    if (n < 0)
        error("ERROR in recvfrom");

    std::cout << "Received UDP data: " << buffer << std::endl;
}

// void handleTCP(int sockM_TCP_fd)
// {
//     char buffer[1024] = {0};
//     int n = read(sockM_TCP_fd, buffer, 1023);
//     if (n < 0)
//         error("ERROR reading from socket");

//     std::string response = "ServerM: Welcome!";
//     send(sockM_TCP_fd, response.c_str(), response.length(), 0);
//     close(sockM_TCP_fd);
// }
void handleTCP(int sock)
{
    char buffer[1024] = {0};
    read(sock, buffer, 1023);
    std::string received(buffer);
    size_t delimiter = received.find(':');
    std::string username = received.substr(0, delimiter);
    std::string password = received.substr(delimiter + 1);

    std::string response;
    if (password.empty())
    {
        response = "Welcome guest " + username + "!";
    }
    else
    {
        // Placeholder for checking encrypted credentials
        response = "Welcome member " + username + "!";
    }

    send(sock, response.c_str(), response.length(), 0);
    close(sock);
}

int initial_UDP_socket(struct sockaddr_in addr, int portno)
{
    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        error("ERROR on binding");

    return sockfd;
}
int initial_TCP_socket(struct sockaddr_in addr, int portno)
{
    int sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        error("ERROR on binding");
    return sockfd;
}

int main()
{

    std::string server_ip = "127.0.0.1";
    int serverM_UDP_portno = 44203;
    int serverM_TCP_portno = 45203;
    int sockM_UDP_fd = initial_UDP_socket(serverM_UDP_addr, serverM_UDP_portno);
    int sockM_TCP_fd = initial_TCP_socket(serverM_TCP_addr, serverM_TCP_portno);
    int newsockfd;
    socklen_t subserver_len;

    // char buffer[256];
    std::unordered_map<std::string, int> roomStatuses; // Stores room statuses from backend servers

    std::cout << "ServerM is running. Waiting for room statuses..." << std::endl;
    listen(sockM_TCP_fd, 5);
    subserver_len = sizeof(subserver_addr);
    fd_set readfds;
    int max_sd;
    while (true)
    {
        FD_ZERO(&readfds);
        FD_SET(sockM_UDP_fd, &readfds);
        FD_SET(sockM_TCP_fd, &readfds);
        max_sd = std::max(sockM_UDP_fd, sockM_TCP_fd);

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            printf("Select error");
        }

        if (FD_ISSET(sockM_UDP_fd, &readfds))
        {
            handleUDP(sockM_UDP_fd);
        }

        if (FD_ISSET(sockM_TCP_fd, &readfds))
        {
            newsockfd = accept(sockM_TCP_fd, (struct sockaddr *)&subserver_addr, &subserver_len);
            if (newsockfd < 0)
                error("ERROR on accept");
            handleTCP(newsockfd);
        }
        // subserver_len = sizeof(subserver_addr);
        // memset(buffer, 0, 256);
        // int n = recvfrom(sockM_fd, buffer, 256, 0, (struct sockaddr *)&serverM_addr, &subserver_len);
        // if (n < 0)
        //     error("ERROR in recvfrom");

        // std::string received(buffer);
        // std::cout << "Received room status from backend: " << received << std::endl;

        // // Process the received room status (example: "S233,6")
        // auto delimiter_pos = received.find(',');
        // std::string roomCode = received.substr(0, delimiter_pos);
        // int count = std::stoi(received.substr(delimiter_pos + 1));

        // roomStatuses[roomCode] = count; // Store in the map
    }

    close(sockM_UDP_fd);
    close(sockM_TCP_fd);
    return 0;
}