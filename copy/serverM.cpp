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
#include <fstream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <signal.h>
struct sockaddr_in serverM_UDP_addr, serverM_TCP_addr, subserver_addr;
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
    std::ifstream file(filename);
    std::string line, username, password;

    while (getline(file, line))
    {
        size_t delimiter = line.find(',');
        if (delimiter != std::string::npos)
        {
            username = line.substr(0, delimiter);
            password = line.substr(delimiter + 1);
            credentials[username] = password;
        }
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
        error("ERROR in recvfrom");

    std::cout << "Received UDP data: " << buffer << std::endl;
}

void handleClient(int sockfd, std::unordered_map<std::string, std::string> &credentials)
{
    char buffer[1024] = {0};
    std::cout << "serverM get tcp " << buffer << std::endl;
    if (read(sockfd, buffer, sizeof(buffer) - 1) > 0)
    {
        std::string received(buffer);
        // auto &client = clients[sockfd];

        size_t delimiter = received.find(':');
        std::string username = received.substr(0, delimiter);
        std::string password = received.substr(delimiter + 1);

        // if (client.isAuthenticated)
        // {
        //     // Handle other queries here
        //     std::string response = "Processed your request: " + received;
        //     send(sockfd, response.c_str(), response.length(), 0);
        // }
        // else
        // {
        std::string response;
        // auto credentials = loadCredentials("member_unencrypted.txt");
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
            response = "Invalid login, please try again";
        }
        send(sockfd, response.c_str(), response.length(), 0);
        //}
    }
    else
    {
        close(sockfd);
        // clients.erase(sockfd);
    }
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
    if (listen(sockfd, 5) < 0)
        error("ERROR on listening");
    return sockfd;
}
int main()
{

    std::unordered_map<std::string, std::string> credentials = loadCredentials("member_unencrypted.txt");
    int serverM_TCP_portno = 45203;
    int serverM_UDP_portno = 44203;

    int sockM_TCP_fd = initial_TCP_socket(serverM_TCP_addr, serverM_TCP_portno);
    int sockM_UDP_fd = initial_UDP_socket(serverM_UDP_addr, serverM_UDP_portno);

    // socklen_t subserver_len;

    // char buffer[256];
    std::unordered_map<std::string, int> roomStatuses; // Stores room statuses from backend servers

    std::cout << "ServerM is running. Waiting for room statuses..." << std::endl;
    // listen(sockM_TCP_fd, 5);
    // subserver_len = sizeof(subserver_addr);
    // fd_set readfds;
    // int max_sd;
    // std::unordered_map<int, ClientInfo> clients;

    while (true)
    {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        handleUDP(sockM_UDP_fd);
        int newsockfd = accept(sockM_TCP_fd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");

        pid_t pid = fork();
        if (pid < 0)
        {
            error("ERROR on fork");
        }

        if (pid == 0)
        {                                         // Child process
            close(sockM_TCP_fd);                  // Close the listening socket in the child process
            handleClient(newsockfd, credentials); // Process the client's requests
            exit(0);
        }
        else
        {                     // Parent process
            close(newsockfd); // Parent doesn't need this socket
        }

        // Continuously handle UDP messages in the main loop
        }

    // Close sockets (not typically reached)
    close(sockM_TCP_fd);
    close(sockM_UDP_fd);
    return 0;
}
// int main()
// {
//     std::unordered_map<std::string, std::string> credentials = loadCredentials("member_unencrypted.txt");

//     std::string server_ip = "127.0.0.1";
//     int serverM_UDP_portno = 44203;
//     int serverM_TCP_portno = 45203;
//     int sockM_UDP_fd = initial_UDP_socket(serverM_UDP_addr, serverM_UDP_portno);
//     int sockM_TCP_fd = initial_TCP_socket(serverM_TCP_addr, serverM_TCP_portno);
//     // int newsockfd;
//     socklen_t subserver_len;

//     // char buffer[256];
//     std::unordered_map<std::string, int> roomStatuses; // Stores room statuses from backend servers

//     std::cout << "ServerM is running. Waiting for room statuses..." << std::endl;
//     listen(sockM_TCP_fd, 5);
//     subserver_len = sizeof(subserver_addr);
//     fd_set readfds;
//     int max_sd;
//     std::unordered_map<int, ClientInfo> clients;
//     while (true)
//     {
//         FD_ZERO(&readfds);
//         FD_SET(sockM_UDP_fd, &readfds);
//         FD_SET(sockM_TCP_fd, &readfds);
//         max_sd = std::max(sockM_UDP_fd, sockM_TCP_fd);
//         for (auto &pair : clients)
//         {
//             FD_SET(pair.first, &readfds); // Include each client's socket in the set of file descriptors to be monitored for reading
//             if (pair.first > max_sd)
//                 max_sd = pair.first; // Update the maximum file descriptor number, which is needed for the select() call
//         }
//         int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
//         if ((activity < 0) && (errno != EINTR))
//         {
//             printf("Select error");
//         }
//         if (FD_ISSET(sockM_TCP_fd, &readfds))
//         {
//             printf("111111");
//             int newsockfd = accept(sockM_TCP_fd, NULL, NULL);
//             if (newsockfd < 0)
//                 error("ERROR on accept");
//             clients[newsockfd] = ClientInfo{newsockfd, false, "unknown"};
//         }
//         for (auto it = clients.begin(); it != clients.end();)
//         {
//             if (FD_ISSET(it->first, &readfds))
//             {
//                 handleClient(it->first, clients, credentials);
//                 it = clients.begin(); // Reset iterator since the map may have been modified
//             }
//             else
//             {
//                 ++it;
//             }
//         }

//         if (FD_ISSET(sockM_UDP_fd, &readfds))
//         {
//             handleUDP(sockM_UDP_fd);
//         }
//     }

//     close(sockM_UDP_fd);
//     close(sockM_TCP_fd);
//     return 0;
// }