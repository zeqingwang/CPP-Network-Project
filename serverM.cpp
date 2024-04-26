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

struct ClientInfo
{
    int sockfd;
    std::string username;
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

    std::string data(buffer, n);
    std::istringstream dataStream(data);
    std::string entry;
    std::string server_name;

    while (std::getline(dataStream, entry, ';'))
    {

        size_t delimiter_pos = entry.find(',');
        if (delimiter_pos != std::string::npos)
        {
            std::string roomCode = entry.substr(0, delimiter_pos);
            server_name = roomCode.substr(0, 1);
            // int count = std::stoi(entry.substr(delimiter_pos + 1));
        }
    }
    std::cout << "The main server has received the room status from Server " + server_name + " using UDP over port 44203." << std::endl;
}

void attemptLogin(int sockfd, const std::string &received, ClientInfo &clientInfo)
{

    size_t delimiter = received.find(':');
    std::string username = received.substr(0, delimiter);
    std::string password = received.substr(delimiter + 1);

    std::string response;
    std::string output;
    std::string conclusion = "The main server sent the authentication result to the client.";
    if (password.empty())
    {

        output = "The main server received the guest request for " + username + " using TCP over port 45203. The main server accepts " + username + " as a guest.";
        conclusion = "The main server sent the guest response to the client.";
        clientInfo.isAuthenticated = true;
        clientInfo.role = "guest";
        clientInfo.username = username;
        response = "guest";
    }
    else if (credentials.count(username) <= 0)
    {
        output = "The main server received the authentication for " + username + " using TCP over port 45203.";
        response = "username";
    }
    else if (credentials[username] == password)
    {
        output = "The main server received the authentication for " + username + " using TCP over port 45203.";
        clientInfo.isAuthenticated = true;
        clientInfo.role = "member";
        clientInfo.username = username;
        response = "member";
    }
    else if (credentials[username] != password)
    {
        output = "The main server received the authentication for " + username + " using TCP over port 45203.";

        response = "password";
    }
    std::cout << output << std::endl;
    send(sockfd, response.c_str(), response.length(), 0);
    std::cout << conclusion << std::endl;
    // close(sock);
}
void sendUdpQuery(int sockfd, const std::string &message, struct sockaddr_in serverAddr, std::string server_name)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        std::cerr << "Socket creation failed\n";
        return;
    }

    if (sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Send to backend server failed\n";
    }

    std::cout << "The main server sent a request to Server " + server_name + "." << std::endl;
    char buffer[1024] = {0};
    struct sockaddr_in from;
    socklen_t fromLen = sizeof(from);
    if (recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&from, &fromLen) < 0)
    {
        std::cerr << "Receive failed\n";
    }
    else
    {
        std::string received(buffer);

        auto pos = received.find(',');
        std::string queryType = received.substr(0, pos);
        std::string temp = received.substr(pos + 1);
        pos = temp.find(',');
        std::string roomCode = temp.substr(0, pos);
        std::string queryResult = temp.substr(pos + 1);

        if (queryType == "availability")
        {
            std::cout << "The main server received the response from Server " + server_name + " using UDP over port 44203." << std::endl;
        }
        else
        {
            if (queryResult == "yes")
            {
                std::cout << "The main server received the response and the updated room status from Server " + server_name + " using UDP over port 44203." << std::endl;
                std::cout << "The room status of Room " + roomCode + " has been updated." << std::endl;
            }
            else
            {
                std::cout << "The main server received the response from Server " + server_name + " using UDP over port 44203." << std::endl;
            }
        }
        send(sockfd, received.c_str(), received.length(), 0);
        std::cout << "The main server sent the " + queryType + " information to the client." << std::endl;
    }

    close(sock);
}

void processQuery(int sockfd, const std::string &received, ClientInfo &clientInfo)
{

    std::string response;
    size_t delimiter = received.find(',');
    std::string querytype = received.substr(0, delimiter);
    std::string roomcode = received.substr(delimiter + 1);

    std::cout << "The main server has received the " + querytype + " request on Room " + roomcode + " from " + clientInfo.username + " using TCP over port 45203." << std::endl;

    if (clientInfo.role == "guest" && querytype == "reservation")
    {

        std::cout << clientInfo.username + " cannot make a reservation." << std::endl;
        response = querytype + "," + roomcode + ",dennied";
        send(sockfd, response.c_str(), response.length(), 0);
        std::cout << "The main server sent the error message to the client." << std::endl;
    }
    else
    {
        if (roomcode[0] == 'S')
        {
            sendUdpQuery(sockfd, querytype + "," + roomcode, serverS_UDP_addr, roomcode.substr(0, 1));
        }
        else if (roomcode[0] == 'D')
        {
            sendUdpQuery(sockfd, querytype + "," + roomcode, serverD_UDP_addr, roomcode.substr(0, 1));
        }
        else if (roomcode[0] == 'U')
        {
            sendUdpQuery(sockfd, querytype + "," + roomcode, serverU_UDP_addr, roomcode.substr(0, 1));
        }
        else
        {
            std::string error = querytype + "," + roomcode + ",NA";
            send(sockfd, error.c_str(), error.length(), 0);
        }
    }
}
void handleClient(int sockfd)
{

    char buffer[1024] = {0};
    int read_size = read(sockfd, buffer, sizeof(buffer) - 1);
    if (read_size <= 0)
        return;

    std::string received(buffer);
    if (clients[sockfd].isAuthenticated)
    {
        processQuery(sockfd, received, clients[sockfd]);
    }
    else
    {
        attemptLogin(sockfd, received, clients[sockfd]);
    }
}
int initial_UDP_socket(struct sockaddr_in addr, int portno)
{
    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    // Beej code
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(portno);
    // end
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
    // Beej code
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(portno);
    // end
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        error("ERROR on binding");
    return sockfd;
}
void initial_UDP_connection(struct sockaddr_in *addr, int portno)
{
    // Beej code;
    memset(addr, 0, sizeof(*addr));
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

    std::unordered_map<std::string, int> roomStatuses;

    std::cout << "ServerM is running. Waiting for room statuses..." << std::endl;
    listen(sockM_TCP_fd, 5);

    fd_set readfds;
    int max_sd;
    while (true)
    {
        // These FD functions after finish after reviewing the Beej tutorial
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
                clients[newsockfd] = {newsockfd, "", false, ""};
                handleClient(newsockfd);
            }
        }
        for (auto it = clients.begin(); it != clients.end();)
        {
            if (FD_ISSET(it->first, &readfds))
            {
                handleClient(it->first);

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
