

#include "udpserver.h"

#include <thread>
#include <iostream>

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

    Services::UDPServer u(argv[3], &interfaceMap[argv[2]]);
    std::thread t_service{[&](){

        u.RunService();

    }};
    std::this_thread::sleep_for(std::chrono::seconds(600));
    u.StopService();

    t_service.join();


    return 0;
}