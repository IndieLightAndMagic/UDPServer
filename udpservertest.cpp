

#include "udpserver.h"

#include <thread>
#include <iomanip>
#include <iostream>

class MyUDPServer : public Services::UDPSocket {

public:
    GTech::Signal<> errorHappened;
    MyUDPServer(const char* port, Services::NetworkInterface* pNetworkInterface):UDPSocket(port, pNetworkInterface){
        
    }
    
    void onDatagramSent(long chunkOffset, long chunkSize, const datagram_tuple& datagramTuple){

        const char hexdigits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

        std::cout << "0x" << std::hex << chunkOffset << std::dec << " [ " << chunkSize << " ] \n";
        auto [nSize, pSocl, pBuffer] = datagramTuple;
        auto pBufferData = pBuffer.get();

        for ( int index = 0; index < nSize; ++index ){

            auto value = pBufferData[index];

            auto renderAddress = (index % 16 == 0) ? true : false;

            if (renderAddress) std::cout << "\n\t0x" << std::setw(4) << std::setfill('0') << std::hex << index << std::dec << " ";

            decltype(value) lowIndx = value & 0x0f;
            decltype(value) highIdx = (value >> 4) & 0x0f;
            std::string accumulator = std::string{hexdigits[highIdx]} + std::string{hexdigits[lowIndx]};
            std::cout << accumulator << " ";

            auto renderLine = ((index % 0x10 == 0x0f) || (index + 1 == nSize));
            if (renderLine){
                auto width = 3 * (0x0f - index % 0x10) + 1;
                std::cout << std::setw(width) << std::setfill(' ') << "[" ;
                auto lineidx = index - (index % 0x10);
                for (;lineidx < index + 1; ++lineidx) {

                    if ( pBufferData[lineidx] != '\n' && pBufferData[lineidx] != '\t')
                        std::cout << pBufferData[lineidx];
                    else
                        std::cout << " ";

                }
                std::cout << "]" << std::endl;

            }

        }

    }

    void onDataIsReady(Services::UDPSocket::datagram_tuple datagramTuple)
    {
        std::cout << "Data Arrived!!!!!\n";
        static unsigned int maxcount = 0;
        ++maxcount;
        if (maxcount > 1) {
            errorHappened.emit();
            return;
        }
        auto [nData, pSrcAddrIn , pBufferData]  = datagramTuple;
        auto pBufferRawData                     = pBufferData.get();
        auto pRawSrcAddrIn                      = pSrcAddrIn.get();

        std::string srcAddrString{inet_ntoa(pRawSrcAddrIn->sin_addr)};
        auto srcPort{ntohs(pRawSrcAddrIn->sin_port)};
        
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
        std::cout << "Sending: \n";
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
    u.datagramReceived.connect_member(  &u, &MyUDPServer::onDataIsReady);
    u.datagramReceived.connect_member(  &u, &MyUDPServer::onDataIsReadyPing);
    u.datagramSent.connect_member(      &u, &MyUDPServer::onDatagramSent);

    std::thread t_service{[&](){

        u.RunService();
        std::cout << "Service Finished!!!\n";
    
    }};
    std::this_thread::sleep_for(std::chrono::seconds(600));
    std::cout << "Pre Stop\n";
    u.StopService();
    std::cout << "Post Stop\n";
    t_service.join();


    return 0;
}