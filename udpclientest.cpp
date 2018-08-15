
/*
    Simple udp client
*/

#include "udpclient.h"
#include "udpserver.h"

#define SERVER "127.0.0.1"
#define BUFLEN 2048 //Max length of buffer
#define PORT "8888"   //The port on which to send data

using namespace Services;
void die(const char *s)
{
    perror(s);
    exit(1);
}

int main(void)
{
    UDPClient x;
    char buf[BUFLEN];
    char message[BUFLEN];

    while(1)
    {
        std::memset(message, 0, BUFLEN);
        printf("Enter message : ");
        gets(message);

        //send the message
        {
            auto [ bValid, nBytesSent ] = x.Send(std::string{SERVER}, std::string{PORT}, reinterpret_cast<unsigned char*>(message), strlen(message));
            if (bValid == false)
            {
                die("sendto()");
                break;
            }

        }

        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        std::memset(buf, 0, BUFLEN);

        //try to receive some data, this is a blocking call
        auto [bValid, incomingIp, incomingPort, incomingMessage] = x.Recv();

        if (bValid == false)
        {
            die("recvfrom()");
            break;
        }

        puts(reinterpret_cast<const char*>(incomingMessage.data()));
    }

    return 0;
}