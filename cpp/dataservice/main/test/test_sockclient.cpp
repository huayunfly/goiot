/**
 * Test socket client connecting to the server.
 */

#include <string>
#include <iostream>
#include "../src/socketservice.h"
#include "../src/aenet.h"

int main(int argc, char *argv[])
{
    using namespace goiot;

    /* Socketservice */
    SocketService s;
    int client_sockfd;
    client_sockfd = s.Connect(8000, "localhost");
    if (client_sockfd == -1)
    {
        std::cout << "localhost connection failed..." << std::endl;
    }

    if (client_sockfd > 0)
    {
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
    }

    /* AENet Wrapper */
    char err[AENet::ANET_ERR_LEN];
    const int BUFF_SIZE = 512;
    client_sockfd = AENet::anetTcpConnect(err, "localhost", 8080);
    if (client_sockfd == AENet::ANET_ERR)
    {
        std::cout << "Error-> " << std::string(err) << std::endl;
    }
    if (client_sockfd > 0)
    {
        char buff[BUFF_SIZE];
        AENet::anetWrite(client_sockfd, "Hello", 5);
        AENet::anetRead(client_sockfd, buff, 2);
        std::cout << "Client reads: " << std::string(buff) << std::endl;
        AENet::anetClose(client_sockfd);
    }

    return 0;
}