
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

//LIBRARY HEADERS
#include "udpserver.h"


Services::UDPServer::UDPServer(const char* portString, const char* ipOrInterfaceNameString) {

    struct addrinfo* pAddrInfoList;

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == -1) return;

    /* Non blocking socket */
    auto flags  = fcntl(m_socket, F_GETFL);
    fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);

    /* Reuse local addresses */
    int reuseddr = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuseddr, sizeof(int));

    auto interfaces         = Services::NetworkInterface::GetInterfaces();
    auto ifaceNameString    = ipOrInterfaceNameString ?  std::string{ ipOrInterfaceNameString } : std::string{};
    auto& ipString          = ifaceNameString;

    for (auto& interface : interfaces)
    {
        if (interface.familyString != "IPV4") continue;
        bool paramHit = interface.interfaceName == ifaceNameString || interface.ip == ipString;
        if (ipOrInterfaceNameString != nullptr && !paramHit) continue;
        else if ((ipOrInterfaceNameString == nullptr  ) || (ipOrInterfaceNameString != nullptr && (paramHit))) {

            //Set port
            auto pSockAddr = reinterpret_cast<sockaddr_in*>(&interface.sckadd);
            auto port = atol(portString);
            pSockAddr->sin_port = htons(port);


            if (bind(m_socket, &interface.sckadd, interface.scklen) == 0){

                auto tempPort       = pSockAddr->sin_port;
                long portByteSwap   = (tempPort & 0x00ff) << 8;
                portByteSwap       += (tempPort & 0xff00) >> 8;
                //Binded ok.
                std::cout << "UDP Socket binded and listening in " << interface.ip <<" : " << portByteSwap << std::endl;
                m_valid = true;
                return;

            }
            continue;
        }
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
        socklen_t           srcAddrLength;

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


const std::vector<Services::NetworkInterface>& Services::NetworkInterface::GetInterfaces() {

    static std::vector<Services::NetworkInterface> interfaces;

    static std::once_flag flag;

    std::call_once(flag, [&]() {

        ifaddrs *ptrIfaddr;
        ifaddrs *ifa;
        int n, family, s;

        if (getifaddrs(&ptrIfaddr) == -1) {
            perror("getifaddrs");
            exit(EXIT_FAILURE);
        }

        /* Walk through linked list, maintaining head pointer so we
           can free list later */

        for (ifa = ptrIfaddr; ifa != NULL; ifa = ifa->ifa_next) {

            if (ifa->ifa_addr == NULL)
                continue;

            interfaces.push_back(NetworkInterface());
            auto interfacesPtr  = interfaces.data();
            auto lastIndex      = interfaces.size() - 1;
            auto host           = std::array<char, NI_MAXHOST>{};
            auto hostPtr        = host.data();

            interfacesPtr               = &interfacesPtr[lastIndex];
            interfacesPtr->interfaceName         = std::string{ifa->ifa_name};
            interfacesPtr->family       = ifa->ifa_addr->sa_family;
            interfacesPtr->scklen       = (interfacesPtr->family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
            interfacesPtr->familyString = (interfacesPtr->family == AF_INET) ? std::string{"IPV4"} : (interfacesPtr->family == AF_INET6) ? std::string{"IPV6"} : std::string{"???"};
            getnameinfo(ifa->ifa_addr, interfacesPtr->scklen, hostPtr, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
            interfacesPtr->ip     = std::string{hostPtr};
            std::memcpy(&interfacesPtr->sckadd, ifa->ifa_addr, sizeof(sockaddr));
        }
        freeifaddrs(ptrIfaddr);
    });

    return interfaces;
}
void Services::NetworkInterface::DisplayInterfaces() {

    auto interfaces = Services::NetworkInterface::GetInterfaces();
    for (auto& interface : interfaces){

        std::cout << "[ " << interface.interfaceName << " ]\t" << interface.familyString << " : " << interface.ip << std::endl;

    }

}


