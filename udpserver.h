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
        /**
         * @brief      Constructs the service, binding it to a port and a network interface.
         *
         * @param[in]  port               The port, a const char string.
         * @param[in]  ipOrInterfaceNameString  The ip or interface network name to bind the service. A const char string. If the value of this para is nullptr then an interface binded to 127.0.0.1 (most of the time) will be chosen.
         */
        UDPServer(const char* port, const char* ipOrInterfaceNameString = nullptr);
        /**
         * @brief      Runs UDP Server. This function blocks until StopService is Called. 
         */
        void RunService();

        /**
         * @brief      Stops the UDPService. This is called from a different thread than the one running UDPServer::StartService. 
         */
        void StopService();

    };
}

#endif //UDPSERVER_H
