//
// Created by Julian Andres Guarin Reyes on 8/9/18.
//

#ifndef __NETWORKINTERFACES_UTIL__
#define __NETWORKINTERFACES_UTIL__

#include <map>
#include <set>
#include <string>
#include <vector>

#include <netdb.h>
#include <ifaddrs.h>


namespace Services {
    struct NetworkInterface {
        /*!
         * Key with ip family type (IPV4, IPV6, xxx), which maps to a Service::NetworkInterface type.
         */
        using NetworkInterfaceMap = std::map<std::string, Services::NetworkInterface>;
        /*!
         * Key with interface name (bridge100, en0 lo0, etc) which maps to InterfaceName to IpFamilyNetworkInterface_Map.
         */
        /*!
         * @brief Get a list of network interfaces using a specific Ip Family Protocol.
         * @param familyName : Transport Protocol family name for example "IPV4", "IPV6"
         * @return
         */
        static const std::map<std::string, NetworkInterfaceMap> GetNetworkInterfacesMap();
        /*!
         * @brief Display in stdout the system network interfaces.
         * @param familyName : Transport Protocol family name filter. If a std::string empty is passed this will be ignored and all the interfaces will be printed.
         */
        static void DisplayInterfaces();

        static std::tuple<bool, NetworkInterface> IsAValidIp(std::string ip);

        int         family;
        std::string familyString;
        std::string interfaceName;
        std::string ip;
        socklen_t   scklen;
        sockaddr    sckadd;
        sockaddr    netmsk;
        sockaddr    dstadr;
        ifaddrs     m_ifa;
        ifaddrs*    m_pIfa{&m_ifa};


    public:
        NetworkInterface() = default;
        NetworkInterface(ifaddrs *ifa);

    };

}

#endif //__NETWORKINTERFACES_UTIL__
