
#include <sys/types.h>

//C
#include <cstdio>
#include <cstdlib>
#include <cstring>

//C++
#include <set>
#include <array>
#include <mutex>
#include <thread>
#include <iostream>

//POSIX
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <networkinterfacesutil.h>

//LIBRARY HEADERS
#include "udpserver.h"

Services::UDPServer::UDPServer(const char *portString, const NetworkInterface *pNetworkInterface) {

    struct addrinfo* pAddrInfoList;

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == -1) return;

    /* Non blocking socket */
    auto flags  = fcntl(m_socket, F_GETFL);
    fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);

    /* Reuse local addresses */
    int reuseddr = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuseddr, sizeof(int));

    //Use a copy of the address of the interface for this particular binding.
    sockaddr sockaddrtobind;
    auto pSockAddr_in   = reinterpret_cast<sockaddr_in*>(&sockaddrtobind);
    auto pSockAddr      = reinterpret_cast<sockaddr*>(pSockAddr_in);
    std::memcpy(pSockAddr_in, &pNetworkInterface->sckadd, sizeof(sockaddr));
    pSockAddr_in->sin_port = htons(atol(portString));
    auto socklen        = pNetworkInterface->scklen;

    if (bind(m_socket, pSockAddr, socklen) == 0) {

        auto tempPort = pSockAddr_in->sin_port;
        long portByteSwap = (tempPort & 0x00ff) << 8;
        portByteSwap += (tempPort & 0xff00) >> 8;
        //Binded ok.
        std::cout << "UDP Socket binded and listening in " << pNetworkInterface->ip << " : " << portByteSwap << std::endl;
        m_valid = true;
        return;
    }

    close(m_socket);
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
        socklen_t           srcAddrLength{sizeof(sockaddr_in)};

        /* receive in an unblocking fashion. */
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




