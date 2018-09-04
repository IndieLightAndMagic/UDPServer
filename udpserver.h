//
// Created by Julian Andres Guarin Reyes on 8/9/18.
//
#ifndef __UDPSERVICE_H__
#define __UDPSERVICE_H__


//C++
#include <queue>
#include <memory>
#include <string>
#include <vector>
#include <system_error>

//POSIX
#include <netdb.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

//MINE
#include <networkinterfacesutil.h>
#include <signalslot.h>

namespace Services {
    
    class UDPSocket {

        /*!
        True when the descriptor is a valid one.
        */
        bool m_valid{false};
        /**
         * @brief      Starts a datagram socket.
         */
        void StartSocket();

    protected:
        int m_socket{-1};


    public:
        //an array of bytes to hold sockaddr_in structure.
        using sockadd_data_array    = std::array<unsigned char, sizeof(sockaddr_in)>;

        /*!
         * an udp datagram: Size of Incoming/Outcoming Data, Pointer to a struct with the Address of the Incoming/Outcoming Sender/Receiver, Address to the beginning of the data.
         */
        using datagram_tuple        = std::tuple<long, std::shared_ptr<struct sockaddr_in>, std::shared_ptr<unsigned char>>;

        /**
         * @brief      Constructor for a non listening socket (normally used for clients)
         */
        UDPSocket();

        /**
         * @brief      Constructs the service, binding it to a port and a network interface.
         *
         * @param[in]  port The port, a const char string.
         * @param[in]  pNetworkInterface   The interface network to bind the service.
         */
        UDPSocket(const char *port, const NetworkInterface *pNetworkInterface);

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
        GTech::Signal<datagram_tuple> datagramReceived;

        /*!
         * A signal to notify data was sent. <SentChunkOffset, SentChunkSize, Whole Datagram>
         */
        GTech::Signal<long , long, const datagram_tuple&> datagramSent;

        
        static void EmitError(const UDPSocket& u, int errorNumber);
        GTech::Signal<std::error_condition, std::string>    datagramError;
        GTech::Signal<std::string>                          datagramUnknownError;


        /*!
         * @brief Send a datagram tuple.
         * @param datagramTuple is the tuple containing a long type byte size of the datagram, a pointer to sockaddr_in structure which contains info regarding the dest address port and a pointer to the byte vector itself with the data to be transmitted.
         */
        long SendDatagram(const datagram_tuple& datagramTuple);

        /**
         * @brief      Recieve a Datagram. This function doesn't block. 
         *
         * @return     A tuple with 3 fields: a bool to check if a datagram was received (true) or not(false). An error_condition which should be ignored if message is valid. A datagram with the received datagram if message is valid. 
         */
        std::tuple<bool, std::error_condition, datagram_tuple> RecvDatagram();

        /**
         * @brief      Create a Datagram with message and destination ip:port address. 
         *
         * @param[in]  ip    { parameter_description }
         * @param[in]  port  The port
         *
         * @return     { description_of_the_return_value }
         */
        static datagram_tuple CreateDatagram(std::string ip, std::string port, unsigned char* pDataBuffer, long sz);

        /**
         * @brief      Sets the socket blocking.
         *
         * @param[in]  blocking  The blocking
         */
        void SetSocketBlocking(bool blocking);

        /**
         * @brief      Determines if socket blocking.
         *
         * @return     True if socket blocking, False otherwise.
         */
        bool IsSocketBlocking();
        
        ~UDPSocket();
    };
}

#endif //__UDPSERVICE_H__

