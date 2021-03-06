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
         * @return     The number of bytes sent.
         */
        long SendDatagram(const datagram_tuple& datagramTuple);

        /**
         * @brief      Recieve a Datagram. This function doesn't block.
         *
         * @return     A tuple with 3 fields: a bool to check if a datagram was received (true) or not(false). An error_condition which should be ignored if message is valid. A datagram with the received datagram if message is valid.
         */
        std::tuple<bool, std::error_condition, datagram_tuple> RecvDatagram();


        /**
         * @brief Receive a Datagram. This function should block.
         * @param func  A func to call with the received datagram if a valid datagram is received.
         * @return true if a valid datagram was received.
         */
        bool RecvDatagramAndCallFunction(void(*func)(datagram_tuple)){

            auto resultTuple    = RecvDatagram();
            auto validDatagram  = std::get<0>(resultTuple);

            if (validDatagram){
                func(std::get<2>(resultTuple));
            }
            return validDatagram;

        }

        /**
         *
         * @tparam T A class Type.
         * @param pInst An pointer of instance T.
         * @param func A public method of class T, which will be called with parameter of datagram_tuple type.
         * @return true if a valid datagram was received, otherwise false is returned.
         * 
         */
        template <typename T>
        bool RecvDatagramAndCallMethod(T* pInst, void (T::*func)(datagram_tuple)){

            auto resultTuple    = RecvDatagram();
            auto validaDatagram = std::get<0>(resultTuple);
            if (validaDatagram){
                pInst->func(std::get<2>(resultTuple));
            }
            return validaDatagram;

        }
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
         * @brief      Sets the socket blocking. Non service should be running on this socket when setting the blockin / non - blocking condition.
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

