#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include<tuple>
#include<string>
#include<vector>


namespace Services {


    class UDPClient {

        int m_socket{-1};
        bool m_valid{false};

    public:
        UDPClient();
        /**
         * @brief      Send some info through UDP socket.
         *
         * @param[in]  ipString      ip of the receiver
         * @param[in]  portString    port of communication
         * @param      pBytes  Pointer to memory area with data
         * @param[in]  nBytes  Data size in bytes
         *
         * @return     A tuple describing data sending operation. If Send was successful (std::get<0>) is true and how many byte were sent (std::get<1>()) is also informed.
         */
        std::tuple<bool, long> Send(std::string ipString, std::string portString, unsigned char* pBytes, std::size_t nBytes);
        /**
         * @brief      Receive info from a transmitter.
         *
         * @return     A tuple describing reception operation.
         * On success std::get<0>() is false, and the other tuple values must be ignored by the user.
         * On success std::get<0> is true and
         * incoming data transmitter's Ip and Port are returned in
         * std::strings in std::get<1>() and std::get<2>() respectively.
         * Payload comes in std::get<3>().
         */

        std::tuple<bool, std::string, std::string, std::vector<unsigned char>> Recv();
    };
}





#endif //UDPCLIENT_H

