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
struct sockaddr_in serverM_addr, subserver_addr;

void error(const char *msg)
{
    perror(msg);
    exit(1);
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
    int serverM_portno = 44203;
    int sockM_fd = initial_UDP_socket(serverM_addr, serverM_portno);

    socklen_t subserver_len;

    char buffer[256];
    std::unordered_map<std::string, int> roomStatuses; // Stores room statuses from backend servers

    std::cout << "ServerM is running. Waiting for room statuses..." << std::endl;

    while (true)
    {

        subserver_len = sizeof(subserver_addr);
        memset(buffer, 0, 256);
        int n = recvfrom(sockM_fd, buffer, 256, 0, (struct sockaddr *)&serverM_addr, &subserver_len);
        if (n < 0)
            error("ERROR in recvfrom");

        std::string received(buffer);
        std::cout << "Received room status from backend: " << received << std::endl;

        // Process the received room status (example: "S233,6")
        auto delimiter_pos = received.find(',');
        std::string roomCode = received.substr(0, delimiter_pos);
        int count = std::stoi(received.substr(delimiter_pos + 1));

        roomStatuses[roomCode] = count; // Store in the map
    }

    close(sockM_fd);
    return 0;
}