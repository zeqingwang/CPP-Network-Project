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
#include <sstream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <fstream>
#include <signal.h>
struct sockaddr_in serverM_UDP_addr, serverM_TCP_addr, subserver_addr;
std::unordered_map<std::string, std::string> credentials;
struct ClientInfo
{
    int sockfd;
    bool isAuthenticated;
    std::string role;
};
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

std::unordered_map<std::string, std::string> loadCredentials(const std::string &filename)
{
    std::unordered_map<std::string, std::string> credentials;
    std::ifstream infile(filename);

    if (!infile)
    {
        error("ERROR opening file");
    }

    std::string line;
    while (std::getline(infile, line))
    {
        auto delimiter_pos = line.find(',');
        std::string username = line.substr(0, delimiter_pos);
        std::string password = line.substr(delimiter_pos + 2);
        // int count = std::stoi(line.substr(delimiter_pos + 1));
        credentials[username] = password;
    }

    return credentials;
}
void handleUDP(int sockM_UDP_fd)
{

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    char buffer[1024] = {0};
    int n = recvfrom(sockM_UDP_fd, buffer, 1023, 0, (struct sockaddr *)&cli_addr, &clilen);
    if (n < 0)
    {
        error("ERROR in recvfrom");
    }

    std::cout << "Received UDP data: " << buffer << std::endl;

    // Parse the received UDP data
    std::string data(buffer, n); // Convert char array to string
    std::istringstream dataStream(data);
    std::string entry;

    // Process each room status entry separated by ';'
    while (std::getline(dataStream, entry, ';'))
    {
        size_t delimiter_pos = entry.find(',');
        if (delimiter_pos != std::string::npos)
        {
            std::string roomCode = entry.substr(0, delimiter_pos);
            int count = std::stoi(entry.substr(delimiter_pos + 1));
            std::cout << "Room Code: " << roomCode << ", Availability: " << count << std::endl;
        }
    }
}

void handleTCP(int sock)
{
    char buffer[1024] = {0};
    read(sock, buffer, 1023);
    std::string received(buffer);
    size_t delimiter = received.find(':');
    std::string username = received.substr(0, delimiter);
    std::string password = received.substr(delimiter + 1);
    std::cout << received << std::endl;
    std::string response;
    if (password.empty())
    {
        // client.role = "guest";
        response = "Welcome guest " + username + "!";
    }
    else if (credentials[username] == password)
    {
        // client.isAuthenticated = true;
        //  client.role = "member";
        response = "Welcome member " + username + "!";
    }
    else
    {
        std::cout << credentials[username] << std::endl;
        std::cout << password << std::endl;

        response = "Invalid login, please try again";
    }
    send(sock, response.c_str(), response.length(), 0);
    close(sock);
    //}
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
    credentials = loadCredentials("member.txt");
    std::string server_ip = "127.0.0.1";
    int serverM_UDP_portno = 44203;
    int serverM_TCP_portno = 45203;
    int sockM_UDP_fd = initial_UDP_socket(serverM_UDP_addr, serverM_UDP_portno);
    int sockM_TCP_fd = initial_TCP_socket(serverM_TCP_addr, serverM_TCP_portno);
    // fd_set readfds;
    //  int max_fd;
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
    }

    close(sockM_UDP_fd);
    close(sockM_TCP_fd);
    return 0;
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