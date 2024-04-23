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
struct sockaddr_in serverS_UDP_addr, serverD_UDP_addr, serverU_UDP_addr;
std::unordered_map<std::string, std::string> credentials;
// clients map
struct ClientInfo
{
    int sockfd;
    bool isAuthenticated;
    std::string role;
};
std::unordered_map<int, ClientInfo> clients;
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

void attemptLogin(int sockfd, const std::string &received, ClientInfo &clientInfo)
{
    // char buffer[1024] = {0};
    // read(sock, buffer, 1023);
    // std::string received(buffer);
    size_t delimiter = received.find(':');
    std::string username = received.substr(0, delimiter);
    std::string password = received.substr(delimiter + 1);
    std::cout << received << std::endl;
    std::string response;
    if (password.empty())
    {
        clientInfo.isAuthenticated = true;
        clientInfo.role = "guest";
        response = "Welcome guest " + username + "!";
    }
    else if (credentials[username] == password)
    {
        clientInfo.isAuthenticated = true;
        clientInfo.role = "member";
        response = "Welcome member " + username + "!";
    }
    else
    {
        std::cout << credentials[username] << std::endl;
        std::cout << password << std::endl;

        response = "Invalid login, please try again";
    }
    send(sockfd, response.c_str(), response.length(), 0);
    // close(sock);
}
void sendUdpQuery(const std::string &message, struct sockaddr_in serverAddr)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        std::cerr << "Socket creation failed\n";
        return;
    }

    // struct sockaddr_in serverAddr;
    //  if ()
    //  memset(&serverAddr, 0, sizeof(serverAddr));
    //  serverAddr.sin_family = AF_INET;
    //  serverAddr.sin_port = htons(serverPort);
    //  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Send to backend serverfailed\n";
    }

    char buffer[1024] = {0};
    struct sockaddr_in from;
    socklen_t fromLen = sizeof(from);
    if (recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&from, &fromLen) < 0)
    {
        std::cerr << "Receive failed\n";
    }
    else
    {
        std::cout << "Received from server: " << buffer << std::endl;
    }
    close(sock);
}

void processQuery(int sockfd, const std::string &received, ClientInfo &clientInfo)
{
    // char buffer[1024] = {0};
    // read(sock, buffer, 1023);
    // std::string received(buffer);
    std::string response;
    size_t delimiter = received.find(',');
    std::string querytype = received.substr(0, delimiter);
    std::string roomcode = received.substr(delimiter + 1);
    response = "Query Type: " + querytype + ", Room Code: " + roomcode + ", Client Role: " + clientInfo.role;
    std::cout << response << std::endl;
    send(sockfd, response.c_str(), response.length(), 0);
    // Determine target server based on room code
    if (roomcode[0] == 'S')
    {
        sendUdpQuery(querytype + "," + roomcode, serverS_UDP_addr);
    }
    else if (roomcode[0] == 'D')
    {
        sendUdpQuery(querytype + "," + roomcode, serverD_UDP_addr);
    }
    else
    {
        std::string error = "No valid server found for room code.";
        send(sockfd, error.c_str(), error.length(), 0);
    }

    // if (targetPort > 0)
    // {
    //     sendUdpQuery(querytype + "," + roomCode, ser);
    // }
}
void handleClient(int sockfd)
{
    // while (true)
    // {
    char buffer[1024] = {0};
    int read_size = read(sockfd, buffer, sizeof(buffer) - 1);
    if (read_size <= 0)
        return; // Break on error or disconnect

    std::string received(buffer);
    if (clients[sockfd].isAuthenticated)
    {
        processQuery(sockfd, received, clients[sockfd]);
    }
    else
    {
        attemptLogin(sockfd, received, clients[sockfd]);
    }
    // }
    // close(sockfd);
    // clients.erase(sockfd);
}
int initial_UDP_socket(struct sockaddr_in addr, int portno)
{
    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
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
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        error("ERROR on binding");
    return sockfd;
}
void initial_UDP_connection(struct sockaddr_in *addr, int portno)
{
    memset(addr, 0, sizeof(addr));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr("127.0.0.1");
    addr->sin_port = htons(portno);
}

int main()
{
    credentials = loadCredentials("member.txt");
    std::string server_ip = "127.0.0.1";
    int serverM_UDP_portno = 44203;
    int serverM_TCP_portno = 45203;
    int sockM_UDP_fd = initial_UDP_socket(serverM_UDP_addr, serverM_UDP_portno);
    int sockM_TCP_fd = initial_TCP_socket(serverM_TCP_addr, serverM_TCP_portno);
    initial_UDP_connection(&serverS_UDP_addr, 41203);
    initial_UDP_connection(&serverD_UDP_addr, 42203);
    initial_UDP_connection(&serverU_UDP_addr, 43203);
    // fd_set readfds;
    //  int max_fd;
    // int newsockfd;
    // socklen_t subserver_len;
    // subserver_len = sizeof(subserver_addr);
    // char buffer[256];
    std::unordered_map<std::string, int> roomStatuses; // Stores room statuses from backend servers

    std::cout << "ServerM is running. Waiting for room statuses..." << std::endl;
    listen(sockM_TCP_fd, 5);

    fd_set readfds;
    int max_sd;
    while (true)
    {
        FD_ZERO(&readfds);
        FD_SET(sockM_UDP_fd, &readfds);
        FD_SET(sockM_TCP_fd, &readfds);
        max_sd = std::max(sockM_UDP_fd, sockM_TCP_fd);
        for (auto &pair : clients)
        {
            FD_SET(pair.first, &readfds);
            max_sd = std::max(max_sd, pair.first);
        }
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            printf("Select error");
        }

        if (FD_ISSET(sockM_UDP_fd, &readfds))
        {
            handleUDP(sockM_UDP_fd);
        }

        // if (FD_ISSET(sockM_TCP_fd, &readfds))
        // {
        //     newsockfd = accept(sockM_TCP_fd, (struct sockaddr *)&subserver_addr, &subserver_len);
        //     if (newsockfd < 0)
        //         error("ERROR on accept");
        //     handleTCP(newsockfd);
        // }
        if (FD_ISSET(sockM_TCP_fd, &readfds))
        {
            struct sockaddr_in cli_addr;
            socklen_t clilen = sizeof(cli_addr);
            int newsockfd = accept(sockM_TCP_fd, (struct sockaddr *)&cli_addr, &clilen);
            if (newsockfd < 0)
            {
                error("ERROR on accept");
            }
            else
            {
                clients[newsockfd] = {newsockfd, false, ""}; // Initialize a new client session
                handleClient(newsockfd);                     // Manage client connection in a function
            }
        }
        for (auto it = clients.begin(); it != clients.end();)
        {
            if (FD_ISSET(it->first, &readfds))
            {
                handleClient(it->first); // Continue handling existing client session
                // if (/* condition to erase client */)
                // {
                //     close(it->first);
                //     it = clients.erase(it); // Safely erase client and advance iterator
                // }
                // else
                // {
                //     ++it;
                // }
                ++it;
            }
            else
            {
                ++it;
            }
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

// // Assuming the client sends queries like "check:room123" or "reserve:room123"
// size_t delimiter = received.find(':');
// if (delimiter == std::string::npos)
// {
//     const std::string error_msg = "Invalid query format. Use action:roomcode.";
//     send(sockfd, error_msg.c_str(), error_msg.length(), 0);
//     return;
// }

// std::string action = received.substr(0, delimiter);
// std::string roomCode = received.substr(delimiter + 1);

// if (action == "check")
// {
//     // Handle room check
//     std::string response = "Availability for " + roomCode + ": ";
//     response += "10 rooms available"; // Placeholder response
//     send(sockfd, response.c_str(), response.length(), 0);
// }
// else if (action == "reserve")
// {
//     if (clientInfo.role == "member")
//     {
//         // Handle reservation
//         std::string response = "Reservation for " + roomCode + " successful.";
//         send(sockfd, response.c_str(), response.length(), 0);
//     }
//     else
//     {
//         std::string error_msg = "Guests cannot make reservations.";
//         send(sockfd, error_msg.c_str(), error_msg.length(), 0);
//     }
// }
// else
// {
//     const std::string error_msg = "Unknown action. Available actions: check, reserve.";
//     send(sockfd, error_msg.c_str(), error_msg.length(), 0);
// }