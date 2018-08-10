
#include <sys/types.h>

//C
#include <cstdio>
#include <cstdlib>
#include <cstring>

//C++
#include <array>
#include <thread>
#include <iostream>

//POSIX
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <set>


//LIBRARY HEADERS
#include "udpserver.h"


Services::UDPServer::UDPServer(const char* port) {


    static std::set<int> activeSockets{};

    std::memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_DGRAM;
    hints.ai_flags      = AI_PASSIVE;
    hints.ai_protocol   = 0;
    hints.ai_canonname  = nullptr;
    hints.ai_addr       = nullptr;
    hints.ai_next       = nullptr;

    struct addrinfo* pAddrInfoList;

    if(getaddrinfo(nullptr, port, &hints, &pAddrInfoList)){
        m_valid = false;
        return;
    }

    auto pAddrInfo = pAddrInfoList;
    for(; pAddrInfo != nullptr; pAddrInfo = pAddrInfo->ai_next){

        /* Create Socket */
        m_socket = socket(pAddrInfo->ai_family, pAddrInfo->ai_socktype, pAddrInfo->ai_protocol);
        if (m_socket == -1) continue;

        /* Non blocking socket */
        auto flags  = fcntl(m_socket, F_GETFL);
        fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);

        /* Reuse local addresses */
        int reuseddr = 1;
        setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuseddr, sizeof(int));

        /* Bind socket to pAddrInfo */
        if (bind(m_socket, pAddrInfo->ai_addr, pAddrInfo->ai_addrlen) == 0){

            //Add to active sockets
            activeSockets.insert(m_socket);

            //Message
            std::cout << "UDP Socket binded and listening in port: " << reinterpret_cast<sockaddr_in*>(pAddrInfo->ai_addr)->sin_port << std::endl;
            break;
        }

        //Message
        std::cout << "BIND UDP Socket FAILED!!!!" << std::endl;
        close(m_socket);

    }
    if (pAddrInfo == nullptr) {
        m_valid = false;
        return;
    }

    freeaddrinfo(pAddrInfoList);


}

void Services::UDPServer::StopService() {

    m_valid = false;

}

void Services::UDPServer::RunService() {

    m_valid = true;
    std::once_flag flag, flagread;

    while(m_valid)
    {
        std::call_once(flag, [&](){
            std::cout << "UDP Service running. " << std::endl;
        });
        static unsigned long const maxBufferSize = 1 << 13;
        std::array<unsigned char, maxBufferSize> buffer;
        buffer.fill(0);

        auto bufferRawData  = buffer.data();
        auto bufferSize     = buffer.size();
        struct sockaddr_in  srcAddr;
        socklen_t           srcAddrLength;

        auto nData = recvfrom(m_socket, bufferRawData, bufferSize, 0, reinterpret_cast<struct sockaddr*>(&srcAddr), &srcAddrLength);
        std::call_once(flagread, [&](){
            std::cout << "Recvfrom result = " << nData << std::endl;
        });
        if (nData > 0) {

            /* Check Info on the peer */
            std::string srcAddrString{inet_ntoa(srcAddr.sin_addr)};
            auto        srcPort{ntohs(srcAddr.sin_port)};
            std::cout << "\t" << srcAddrString << " : \t" << srcPort << ":\n";

            /* Data */
            for (long index = 0; index < nData; ++index){

                std::cout << bufferRawData[index];

                if (index + 1 == nData) std::cout << std::endl;

            }



        } else if (nData == 0) {
            /* Peer Shut Down */
        } else {
            /* Error */
        }
    }

    close(m_socket);

}

void Services::UDPServer::StopAllServices() {


}


