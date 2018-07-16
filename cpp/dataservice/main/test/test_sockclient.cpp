/**
 * Test socket client connecting to the server.
 */

#include <string>
#include <iostream>
#include "../src/socketservice.h"


int main(int argc, char* argv[])
{
    using namespace goiot;
    SocketService s;
    int client_sockfd = s.Connect(8080, "localhost");
    if (client_sockfd == -1)
    {
        std::cout << "Remote connection failed. Exit..." << std::endl;
        return 0;
    }
    char ch = 'a';
    for (int i = 0; i < 10; i++)
    {
        s.Write(client_sockfd, &ch, 1);
        std::cout << "Write char '" << ch << "' ." << std::endl;
        s.Read(client_sockfd, &ch, 1);
        std::cout << "Read char '" << ch << "' ." << std::endl;
    }

    std::cout << "Close connection." << std::endl;
    s.Close(client_sockfd);
    return 0;
}