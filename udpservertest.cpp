

#include "udpserver.h"

#include <thread>
#include <iostream>

class MyUDPServer : public Services::UDPSocket {

public:
    MyUDPServer(const char* port, Services::NetworkInterface* pNetworkInterface):UDPSocket(port, pNetworkInterface){

    }
    void onDataIsReady(Services::UDPSocket::datagram_tuple datagramTuple)
    {
        std::cout << "Data Arrived!!!!!\n";

        auto [ nData, pSrcAddrIn , pBufferRawData] = datagramTuple;
        std::string srcAddrString{inet_ntoa(pSrcAddrIn->sin_addr)};
        auto        srcPort{ntohs(pSrcAddrIn->sin_port)};
        /* Display Sender */
        std::cout << "\t" << srcAddrString << " : \t" << srcPort << "\n";
        /* Display Data */
        for (long index = 0; index < nData; ++index){

            std::cout << pBufferRawData[index];
            if (index + 1 == nData) std::cout << std::endl;

        }
        std::cout << "\n";

    }
    void onDataIsReadyPing(Services::UDPSocket::datagram_tuple datagramTuple){
        std::cout << "Sending.....\n";
        SendDatagram(datagramTuple);
    }

};

int main(int argc, char ** argv) {

    if (argc < 4) {
        std::cout << "Usage: updservertest [interfacename] [ipfamily] [port]\n\n";
        std::cout << "\tinterfacename : The name of the interface to bind the service to. eg. lo0, bridge100, en0 etc.\n";
        std::cout << "\tipfamily : The name of the ip protocol family to use IPV4 / IPV6.\n\n";
        std::cout << "\tport : The number of the UDP port to use.\n\n";
        std::cout << "\tExample : udpservertest en0 IPV4\n";
        return 0;
    }
    auto interfacesMap  = Services::NetworkInterface::GetNetworkInterfacesMap();
    auto interfaceFound = interfacesMap.find(argv[1]) != interfacesMap.end();

    if (!interfaceFound){
        std::cout << "\n\tInterface " << argv[1] << " was not found in the interfacesMap key set.\n";
        return 0;
    }

    auto interfaceMap   = interfacesMap[argv[1]];
    auto ipFamilyFound  = interfaceMap.find(argv[2]) != interfaceMap.end();

    if (!ipFamilyFound){
        std::cout << "\n\tIp Family" << argv[2] << " was not found for the selected interface.\n";
        return 0;
    }

    MyUDPServer u(argv[3], &interfaceMap[argv[2]]);
    u.datagramReceived.connect_member(&u, &MyUDPServer::onDataIsReady);
    u.datagramReceived.connect_member(&u, &MyUDPServer::onDataIsReadyPing);
    std::thread t_service{[&](){

        u.RunService();

    }};
    std::this_thread::sleep_for(std::chrono::seconds(600));
    u.StopService();

    t_service.join();


    return 0;
}