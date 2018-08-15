//
// Created by Julian Andres Guarin Reyes on 8/9/18.
//

#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <string>
#include <vector>

#include <netdb.h>
#include <ifaddrs.h>



namespace Services {
    class UDPServer;
    struct NetworkInterface {

        static const std::vector<NetworkInterface>& GetInterfaces();
        static void DisplayInterfaces();
        int         family;
        std::string familyString;
        std::string interfaceName;
        std::string ip;
        sockaddr    sckadd;
        socklen_t   scklen;

        friend class Services::UDPServer;
    private:
        NetworkInterface() = default;

    };
    class UDPServer {

        struct addrinfo bindingAddress;
        int m_socket{-1};
        bool m_valid{false};

    public:
        UDPServer() = default;
        UDPServer(const char* port, const char* ipOrInterfaceName);
        void RunService();
        void StopService();

    };
}

#endif //UDPSERVER_H
