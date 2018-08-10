//
// Created by Julian Andres Guarin Reyes on 8/9/18.
//

#ifndef UDPSERVER_UDPSERVER_H
#define UDPSERVER_UDPSERVER_H


#include <netdb.h>

namespace Services {

    class UDPServer {

        struct addrinfo hints;
        int m_socket{-1};
        bool m_valid{false};

    public:
        UDPServer() = default;
        UDPServer(const char* port);
        void RunService();
        void StopService();
        static void StopAllServices();
    };
}

#endif //UDPSERVER_UDPSERVER_H
