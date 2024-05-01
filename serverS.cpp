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

int serverSfd;
struct sockaddr_in serverS_addr, serverM_addr;
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

std::unordered_map<std::string, int> load_data(const std::string &filename)
{
    std::unordered_map<std::string, int> roomAvailability;
    std::ifstream infile(filename);

    if (!infile)
    {
        error("error opening file");
    }

    std::string line;
    while (std::getline(infile, line))
    {
        auto delimiter_pos = line.find(',');
        std::string roomCode = line.substr(0, delimiter_pos);
        int count = std::stoi(line.substr(delimiter_pos + 1));
        roomAvailability[roomCode] = count;
    }

    return roomAvailability;
}
int initial_UDP_socket(struct sockaddr_in addr, int portno)
{
    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("error opening socket");
    // from Beej's guide
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        error("error on binding");
    // end
    return sockfd;
}
void respondToQuery(int sockfd, struct sockaddr_in &client_addr, std::unordered_map<std::string, int> &roomAvailability)
{
    char buffer[1024];
    socklen_t client_addr_len = sizeof(client_addr);
    int n = recvfrom(sockfd, buffer, 1023, 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if (n < 0)
        error("error in recvfrom");

    buffer[n] = '\0';
    std::string received(buffer);

    auto pos = received.find(',');
    std::string queryType = received.substr(0, pos);
    std::string roomCode = received.substr(pos + 1);
    std::cout << "The Server S received an " + queryType + " request from the main server." << std::endl;
    std::string response;
    std::string output;
    std::string conclusion = "The Server S finished sending the response to the main server.";
    std::string temp = queryType + "," + roomCode;
    if (queryType == "reservation")
    {
        if (roomAvailability.count(roomCode) > 0)
        {
            if (roomAvailability[roomCode] > 0)
            {
                roomAvailability[roomCode]--;
                output = "Successful reservation. The count of Room " + roomCode + " is now " + std::to_string(roomAvailability[roomCode]) + ".";
                conclusion = "The Server S finished sending the response and the updated room status to the main server.";
                response = temp + ",yes";
            }
            else
            {
                output = "Cannot make a reservation. Room " + roomCode + " is not available.";
                response = temp + ",no";
            }
        }
        else
        {
            output = "Cannot make a reservation. Not able to find the room layout.";
            response = temp + ",NA";
        }
    }
    else
    {
        if (roomAvailability.count(roomCode) > 0)
        {
            if (roomAvailability[roomCode] > 0)
            {

                output = "Room " + roomCode + " is available.";
                response = temp + ",yes";
            }
            else
            {
                output = "Room " + roomCode + " is not available.";
                response = temp + ",no";
            }
        }
        else
        {
            output = "Not able to find the room layout.";
            response = temp + ",NA";
        }
    }

    std::cout << output << std::endl;
    sendto(sockfd, response.c_str(), response.length(), 0, (struct sockaddr *)&client_addr, client_addr_len);
    std::cout << conclusion << std::endl;
}

int main()
{
    std::string server_ip = "127.0.0.1";
    int serverS_portno = 41203;
    std::string server_name = "s";
    int sockS_fd = initial_UDP_socket(serverS_addr, serverS_portno);
    int serverM_portno = 44203;
    std::cout << "The Server S is up and running using UDP on port 41203." << std::endl;

    // from Beej's guide
    memset(&serverM_addr, 0, sizeof(serverM_addr));
    serverM_addr.sin_family = AF_INET;
    serverM_addr.sin_port = htons(serverM_portno);
    if (inet_pton(AF_INET, server_ip.c_str(), &serverM_addr.sin_addr) <= 0)
        error("error on inet_pton");
    // end
    std::unordered_map<std::string, int> roomAvailability = load_data("single.txt");
    std::string message;
    for (const auto &[roomCode, count] : roomAvailability)
    {
        if (!message.empty())
        {
            message += ";";
        }
        message += roomCode + "," + std::to_string(count);
    }

    if (!message.empty())
    {
        if (sendto(sockS_fd, message.c_str(), message.length(), 0,
                   (struct sockaddr *)&serverM_addr, sizeof(serverM_addr)) < 0)
        {
            error("error in sendto");
        }

        std::cout << "The Server S has sent the room status to the main server." << std::endl;
    }
    else
    {
        std::cout << "No room statuses to send." << std::endl;
    }

    while (true)
    {
        respondToQuery(sockS_fd, serverM_addr, roomAvailability);
    }
    close(sockS_fd);
    return 0;
}