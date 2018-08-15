


#include "udpclient.h"

#include<cstdio> //printf
#include<cstring> //memset
#include<cstdlib> //exit(0);

#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include <fcntl.h>
#include <array>



Services::UDPClient::UDPClient() {

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket != -1){
		m_valid = true;
	}
	/* Non blocking socket */
	auto flags  = fcntl(m_socket, F_GETFL);
    fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);
	
}

std::tuple<bool, long> Services::UDPClient::Send(std::string ipString, std::string portString, unsigned char* pBytes, std::size_t nBytes){

	if (m_valid == false) return std::make_tuple(false, -1);

	struct sockaddr_in si_other;
	auto   pSockAddr = reinterpret_cast<struct sockaddr*>(&si_other);
	
	std::memset(&si_other, 0, nBytes);
	const auto pPort = portString.data();
	auto portx = std::atol(pPort);
	si_other.sin_family	= AF_INET;
	si_other.sin_port	= htons(portx);

	const auto pIp = ipString.data();
	if (inet_aton(pIp , &si_other.sin_addr) == 0)
    {
        return std::make_tuple(false, -1);
    }

    auto nSentBytes = sendto(m_socket, pBytes, nBytes, 0, pSockAddr, sizeof(si_other));
    return std::make_tuple(true, nSentBytes);

}

std::tuple<bool, std::string, std::string, std::vector<unsigned char>> Services::UDPClient::Recv()
{

	auto recvTuple = std::make_tuple(false, std::string{}, std::string{}, std::vector<unsigned char>{});
	if (m_valid == false) return recvTuple;

	struct 	sockaddr_in si_other;
	auto	pSockAddr = reinterpret_cast<struct sockaddr*>(&si_other);
	int 	slen;
	auto	pSocklen = reinterpret_cast<socklen_t *>(&slen);
	
	std::array<unsigned char, 2048> buffer;	
	buffer.fill(0);

	//Read
	auto nBytes = recvfrom(m_socket, buffer.data(), 2048, 0, pSockAddr, pSocklen); 
	
	//Error
	if ( nBytes == -1) return recvTuple;
	
	//No Error but nothing returned.
	std::get<0>(recvTuple) = true;
	if ( nBytes == 0) return recvTuple;

	//Ok something arrived
	//ip
	std::get<1>(recvTuple) = std::string{inet_ntoa(si_other.sin_addr)};
	//port

	std::get<2>(recvTuple) = std::to_string(ntohs(si_other.sin_port));
	//data	
	auto returnBuffer = std::get<3>(recvTuple);
	returnBuffer.insert(returnBuffer.begin(), buffer.begin(), buffer.end());

	return recvTuple;

}






























