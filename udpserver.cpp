
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <array>


namespace Services {

    class UDPServer {

        struct addrinfo hints;
        int m_socket;
        bool m_valid{true};

        void Run();
    public:
        UDPServer(const char* port);
        void StopService();
    };
}

Services::UDPServer::UDPServer(const char* port) {

    
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

        m_socket = socket(pAddrInfo->ai_family, pAddrInfo->ai_socktype, pAddrInfo->ai_protocol);
        if (m_socket == -1) continue;
        if (bind(m_socket, pAddrInfo->ai_addr, pAddrInfo->ai_addrlen) == 0) break;
        close(m_socket);

    }
    if (pAddrInfo == nullptr) {
        m_valid = false;
        return;
    }

    freeaddrinfo(pAddrInfoList);


}

void Services::UDPServer::StopService()
{
    m_valid = false;
}

void Services::Run(){

    recvfrom()
}


