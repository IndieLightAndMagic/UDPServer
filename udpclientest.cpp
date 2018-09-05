
/*
    Simple udp client
*/
#include <cstdio>
#include <cstring>
#include <iostream>
#include "udpserver.h"

#define BUFLEN 2048 //Max length of buffer

using namespace Services;
void die(const char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char ** argv)
{
    Services::UDPSocket x;
    x.SetSocketBlocking(true);
    char buf[BUFLEN];
    
    auto messageSP  = std::shared_ptr<unsigned char>(
            new unsigned char[BUFLEN],
            [](unsigned char* ptr){
                delete []ptr;
                std::cout << "message buffer out!!!!\n";
            }
    );
    auto messageRaw = reinterpret_cast<char*>(messageSP.get());
    std::memset(messageRaw, 0, BUFLEN);

    if (argc < 3) {
        std::cout << "Usage: idpclientest[ip] [port]\n\n";
        std::cout << "\tip: The ip address of the remote service.\n";
        std::cout << "\tport : The number of the UDP port to use.\n\n";
        std::cout << "\tExample : udpclient test 192.168.0.15 8888\n";
        return 0;
    }


    while(1)
    {
        std::memset(messageRaw, 0, BUFLEN);
        printf("Enter messageRaw : ");

        std::fgets(messageRaw, BUFLEN, stdin);
        Services::UDPSocket::datagram_tuple dataTuple;

        //send the messageRaw
        {
            auto datagramTuple = Services::UDPSocket::CreateDatagram(std::string{argv[1]}, std::string{argv[2]}, reinterpret_cast<unsigned char*>(messageRaw), std::strlen(messageRaw));
            x.SendDatagram(datagramTuple);
        }

        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        std::memset(buf, 0, BUFLEN);

        //try to receive some data, this is a blocking call
        auto[bValid, errorCondition, datagramTuple] = x.RecvDatagram();
        if (bValid){
            auto [nData, pSockAddrIn, pData] = datagramTuple;
            std::cout << pData.get() << "\n";
        } else {
            break;
        }
        //std::puts(reinterpret_cast<const char*>(incomingMessage.data()));
    }

    return 0;
}