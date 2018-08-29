//
// Created by Julian Andres Guarin Reyes on 8/9/18.
//

#ifndef UDPSERVER_H
#define UDPSERVER_H



#include <queue>
#include <string>
#include <vector>
#include <netdb.h>


#include <ifaddrs.h>
#include <arpa/inet.h>

#include <networkinterfacesutil.h>
#include <signalslot.h>

namespace Services {
    class UDPServer {


        struct addrinfo bindingAddress;
        int m_socket{-1};
        bool m_valid{false};

    public:
        //an array of bytes to hold sockaddr_in structure.
        using sockadd_data_array    = std::array<unsigned char, sizeof(sockaddr_in)>;

        //an udp datagram: the incoming info and the
        using datagram_tuple        = std::tuple<long, sockaddr_in*, unsigned char*>;


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

        /*!
         * A signal to notify data has arrived and is ready to be read
         */
        GTech::Signal<datagram_tuple> dataIsReady;

    private:

        //incoming data Q.
        std::queue<datagram_tuple> rxBuffer;

    };
}

#endif //UDPSERVER_H
