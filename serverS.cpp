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

// Load room data from file
std::unordered_map<std::string, int> load_data(const std::string &filename)
{
    std::unordered_map<std::string, int> roomAvailability;
    std::ifstream infile(filename);

    if (!infile)
    {
        error("ERROR opening file");
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
    int serverS_portno = 41203;
    int sockS_fd = initial_UDP_socket(serverS_addr, serverS_portno);
    int serverM_portno = 44203;
    memset(&serverM_addr, 0, sizeof(serverM_addr));
    serverM_addr.sin_family = AF_INET;
    serverM_addr.sin_port = htons(serverM_portno);
    if (inet_pton(AF_INET, server_ip.c_str(), &serverM_addr.sin_addr) <= 0)
        error("ERROR on inet_pton");
    // Load room data from file
    std::unordered_map<std::string, int> roomAvailability = load_data("single.txt");

    // Send room statuses to the main server
    for (const auto &[roomCode, count] : roomAvailability)
    {
        std::string message = roomCode + "," + std::to_string(count);
        if (sendto(sockS_fd, message.c_str(), message.length(), 0, (struct sockaddr *)&serverM_addr, sizeof(serverM_addr)) < 0)
            error("ERROR in sendto");
    }

    std::cout << "ServerS is running. Sent room statuses to Main Server." << std::endl;

    close(sockS_fd);
    return 0;
}