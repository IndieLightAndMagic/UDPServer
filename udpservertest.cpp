

#include "udpserver.h"

#include <iostream>


int main() {

    Services::UDPServer u("8887");
    std::thread t_service{[&](){

        u.RunService();

    }};
    std::this_thread::sleep_for(std::chrono::seconds(5));
    u.StopService();

    t_service.join();


    return 0;
}