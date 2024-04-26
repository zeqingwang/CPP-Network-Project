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
#include <cctype>
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
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(client_addr);
    char buffer[1024] = {0};
    std::string role;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    // Beej code
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(45203);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // end
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error("ERROR connecting");
    if (getsockname(sockfd, (struct sockaddr *)&client_addr, &len) == -1)
    {
        error(" failed");
    }
    std::cout << "Client is up and running.";
    std::string username, password;
login:
    std::cout << "Please enter the username: ";
    std::cin >> username;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Please enter the password: ";
    std::getline(std::cin, password);

    if (!password.empty())
    {
        std::cout << username + " sent an authentication request to the main server." << std::endl;
    }
    else
    {
        std::cout << username + " sent a guest request to the main server using TCP over port " << ntohs(client_addr.sin_port) << ".\n";
    }
    std::string raw_username = username;
    username = encrypt(username);
    password = encrypt(password);
    // }

    std::string message = username + ":" + password;
    send(sockfd, message.c_str(), message.length(), 0);

    int n = read(sockfd, buffer, 1023);
    if (n < 0)
        error("ERROR reading from socket");

    std::string strBuffer(buffer);

    if (strBuffer == "member")
    {
        std::cout << "Welcome member " + raw_username + "!" << std::endl;
        role = "member";
    }
    else if (strBuffer == "guest")
    {
        std::cout << "Welcome guest " + raw_username + "!" << std::endl;
        role = "guest";
    }
    else if (strBuffer == "password")
    {
        std::cout << "Failed login: Password does not match." << std::endl;
        goto login;
    }
    else if (strBuffer == "username")
    {
        std::cout << "Failed login: Username does not exist." << std::endl;
        goto login;
    }
    else
    {
        // std::cout << "Buffer content is not 'member'." << std::endl;
    }
    while (true)
    {
        std::string roomcode, querytype;
        std::cout << "Please enter the room code: ";
        std::cin >> roomcode;
        std::cout << "Would you like to search for the availability or make a reservation? (Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation ): ";
        std::cin >> querytype;

        if (querytype == "Availability")
        {
            querytype = "availability";
        }
        else if (querytype == "Reservation")
        {
            querytype = "reservation";
        }
        else
        {
            std::cout << "Operation command error, (Enter “Availability” or Enter “Reservation”)." << std::endl;
            std::cout << '\n';
            std::cout << "-----Start a new request-----" << std::endl;
            continue;
        }
        std::string message = querytype + "," + roomcode;
        send(sockfd, message.c_str(), message.length(), 0);
        std::cout << raw_username + " sent an " + querytype + " request to the main server." << std::endl;
        int n = read(sockfd, buffer, 1023);
        if (n < 0)
            error("ERROR reading from socket");
        std::string response(buffer, n);
        // std::string response(buffer);
        // std::cout << "XXX" + response << std::endl;
        auto pos = response.find(',');
        std::string temp = response.substr(pos + 1);
        pos = temp.find(',');

        std::string queryresult = temp.substr(pos + 1);
        // std::cout << "XXX" + queryresult << std::endl;

        if (querytype == "availability")
        {
            if (queryresult == "yes")
            {
                std::cout << "The client received the response from the main server using TCP over port " << ntohs(client_addr.sin_port) << ".\n";
                std::cout << "The requested room is available." << std::endl;
                std::cout << '\n';
                std::cout << "-----Start a new request-----" << std::endl;
            }
            else if (queryresult == "no")
            {
                std::cout << "The client received the response from the main server using TCP over port " << ntohs(client_addr.sin_port) << ".\n";
                std::cout << "The requested room is not available." << std::endl;
                std::cout << '\n';
                std::cout << "-----Start a new request-----" << std::endl;
            }
            else if (queryresult == "NA")
            {
                std::cout << "The client received the response from the main server using TCP over port " << ntohs(client_addr.sin_port) << ".\n";
                std::cout << "Not able to find the room layout." << std::endl;
                std::cout << '\n';
                std::cout << "-----Start a new request-----" << std::endl;
            }
        }
        else if (querytype == "reservation")
        {
            // std::cout << "XXXX:" + role << std::endl;
            if (role == "member")
            {
                if (queryresult == "yes")
                {
                    std::cout << "The client received the response from the main server using TCP over port " << ntohs(client_addr.sin_port) << ".\n";
                    std::cout << "Congratulation! The reservation for Room " + roomcode << std::endl;
                    std::cout << '\n';
                    std::cout << "-----Start a new request-----" << std::endl;
                }
                else if (queryresult == "no")
                {
                    std::cout << "The client received the response from the main server using TCP over port " << ntohs(client_addr.sin_port) << ".\n";
                    std::cout << "Sorry! The requested room is not available." << std::endl;
                    std::cout << '\n';
                    std::cout << "-----Start a new request-----" << std::endl;
                }
                else if (queryresult == "NA")
                {
                    std::cout << "The client received the response from the main server using TCP over port " << ntohs(client_addr.sin_port) << ".\n";
                    std::cout << "Oops! Not able to find the room." << std::endl;
                    std::cout << '\n';
                    std::cout << "-----Start a new request-----" << std::endl;
                }
            }
            else if (role == "guest")
            {
                std::cout << "Permission denied: Guest cannot make a reservation." << std::endl;
                std::cout << '\n';
                std::cout << "-----Start a new request-----" << std::endl;
            }
        }
        else
        {
        }
    }
    close(sockfd);
    return 0;
}