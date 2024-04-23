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
#include <string>
#include <cstring>
#include <limits>
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

std::string encrypt(const std::string &data)
{
    std::string encrypted = data;
    for (char &c : encrypted)
    {
        if (isalpha(c))
        {
            char base = isupper(c) ? 'A' : 'a';
            c = (c - base + 3) % 26 + base;
        }
        else if (isdigit(c))
        {
            c = (c - '0' + 3) % 10 + '0';
        }
    }
    return encrypted;
}

int main()
{

    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[1024] = {0};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(45203);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error("ERROR connecting");

    std::string username, password;
    std::cout << "Please enter the username: ";
    std::cin >> username;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Please enter the password (\"Enter\" to skip for guests): ";
    std::getline(std::cin, password);
    std::cout << "password: " + password << std::endl;

    // if (!password.empty())
    // {
    username = encrypt(username);
    password = encrypt(password);
    // }

    std::string message = username + ":" + password;
    send(sockfd, message.c_str(), message.length(), 0);
    std::cout << "client sent to serverM ";
    int n = read(sockfd, buffer, 1023);
    if (n < 0)
        error("ERROR reading from socket");

    std::cout << buffer << std::endl; // Display server's response
    while (true)
    {
        std::string roomcode, querytype;
        std::cout << "Please enter the room code: ";
        std::cin >> roomcode;
        std::cout << "Would you like to search for the availability or make a reservation? (Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation ): ";
        std::cin >> querytype;
        std::string message = querytype + "," + roomcode;
        send(sockfd, message.c_str(), message.length(), 0);
        std::cout << "client sent to serverM ";
        int n = read(sockfd, buffer, 1023);
        if (n < 0)
            error("ERROR reading from socket");

        std::cout << buffer << std::endl;

        // availability or Enter “Reservation” to make a reservation ): Availability
    }
    close(sockfd);
    return 0;
}