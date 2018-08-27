//
// Created by Julian Andres Guarin Reyes on 8/9/18.
//

#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <string>
#include <vector>

#include <netdb.h>
#include <ifaddrs.h>
#include <networkinterfacesutil.h>


namespace Services {
    class UDPServer {

        struct addrinfo bindingAddress;
        int m_socket{-1};
        bool m_valid{false};

    public:
        UDPServer() = default;
        /**
         * @brief      Constructs the service, binding it to a port and a network interface.
         *
         * @param[in]  port The port, a const char string.
         * @param[in]  pNetworkInterface   The interface network to bind the service.
         */
        UDPServer(const char *port, const NetworkInterface *pNetworkInterface);
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
