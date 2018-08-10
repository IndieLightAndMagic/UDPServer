

#include "udpserver.h"

#include <iostream>


int main() {


    Services::UDPServer u("8887");
    u.RunService();
    std::cout << "Hi!";

    return 0;
}