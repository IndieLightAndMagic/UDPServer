

#include "udpserver.h"

#include <thread>
#include <iostream>

int main() {


    Services::NetworkInterface::DisplayInterfaces();
    auto interfaceMap = Services::NetworkInterface::GetNetworkInterfacesMap();
    Services::UDPServer u("8888", &interfaceMap["bridge100"]["IPV4"]);
    std::thread t_service{[&](){

        u.RunService();

    }};
    std::this_thread::sleep_for(std::chrono::seconds(600));
    u.StopService();

    t_service.join();


    return 0;
}