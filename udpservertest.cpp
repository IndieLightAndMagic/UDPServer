

#include "udpserver.h"

#include <thread>
#include <iostream>

int main() {

    Services::NetworkInterface::DisplayInterfaces();
    Services::UDPServer u("8888", nullptr);
    std::thread t_service{[&](){

        u.RunService();

    }};
    std::this_thread::sleep_for(std::chrono::seconds(600));
    u.StopService();

    t_service.join();


    return 0;
}