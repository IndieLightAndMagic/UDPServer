
#include <sys/types.h>

//C
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

//C++
#include <set>
#include <array>
#include <mutex>
#include <thread>
#include <iostream>
//POSIX
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <arpa/inet.h>

//LIBRARY HEADERS
#include "udpserver.h"

constexpr int maxBufferSize = 1 << 13;

std::tuple <std::shared_ptr<unsigned char>, unsigned char*> CreateBuffer(int size){

    auto sp = std::shared_ptr<unsigned char>(
        new unsigned char[maxBufferSize],
        [](unsigned char* ptr){
            delete []ptr;
        }
        );
    auto ptr = sp.get();
    for (auto index = 0; index < size; ++index){
        ptr[index] = 0x0;
    }
    return std::make_tuple(sp, ptr);
}

std::tuple<std::shared_ptr<sockaddr_in>, sockaddr_in*, sockaddr*, int> CreateIpSockAddr(){

    auto    pSock       = std::make_shared<sockaddr_in>();
    auto    pSockRawIn  = pSock.get();
    auto    pSockRaw    = reinterpret_cast<sockaddr*>(pSockRawIn);
    int     slen        = sizeof(sockaddr);

    std::memset(pSockRawIn, 0, sizeof(sockaddr_in));
    return  std::make_tuple(pSock, pSockRawIn, pSockRaw, slen);
}

std::tuple<std::shared_ptr<unsigned char>, unsigned char *, std::shared_ptr<sockaddr_in>, sockaddr_in*, sockaddr*, int> CreateBufferAndIpSockAddr(){

    auto [buffer, bufferRaw]                        = CreateBuffer(maxBufferSize);
    auto [pSock, pSockRawIn, pSockRaw, pSockLen]    = CreateIpSockAddr();

    return std::make_tuple(buffer, bufferRaw, pSock, pSockRawIn, pSockRaw, pSockLen);

}

void Services::UDPSocket::SetSocketBlocking(bool blocking){

    //Get descriptor status flags
    auto flags  =   fcntl(m_socket, F_GETFL);

    //Clear blocking bit
    flags       &=  ~O_NONBLOCK;

    //Select if set the blocking bit or not
    flags       |=  blocking ? 0 : O_NONBLOCK;

    //Set the flag 
    fcntl(m_socket, F_SETFL, flags); 

}

bool Services::UDPSocket::IsSocketBlocking(){

    //Flags.
    auto flags  =   fcntl(m_socket, F_GETFL);
    auto isNonBlocking = flags & O_NONBLOCK ? true : false;
    return !isNonBlocking;

}

void Services::UDPSocket::StartSocket(){

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == -1) return;

    /* Non blocking socket */
    SetSocketBlocking(false);

}


Services::UDPSocket::UDPSocket()
{
    StartSocket();
    m_valid = true;
}

Services::UDPSocket::UDPSocket(const char *portString, const NetworkInterface *pNetworkInterface) {

    StartSocket();
    
    struct addrinfo* pAddrInfoList;

    
    /* Reuse local addresses */
    int reuseddr = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuseddr, sizeof(int));

    //Use a copy of the address of the interface for this particular binding.
    
    auto [pSockAdd, pSockAddRawIn, pSockAddRaw, pSockAddrtobindLength] = CreateIpSockAddr();
    std::memcpy(pSockAddRawIn, &pNetworkInterface->sckadd, pSockAddrtobindLength);
    pSockAddRawIn->sin_port  = htons(atol(portString));


    if (bind(m_socket, pSockAddRaw, pSockAddrtobindLength) == 0) {

        auto tempPort = pSockAddRawIn->sin_port;
        long portByteSwap = (tempPort & 0x00ff) << 8;
        portByteSwap += (tempPort & 0xff00) >> 8;

        //Binded ok.
        std::cout << "UDP Socket binded and listening in " << pNetworkInterface->ip << " : " << portByteSwap << std::endl;
        m_valid = true;
        return;

    }
    else {

        std::cout << "This socket is faulty..... check...\n";
    }

    close(m_socket);

}

void Services::UDPSocket::StopService() {

    m_valid = false;

}

void Services::UDPSocket::RunService() {

    m_valid = true;
    auto isABlockingSocket      = IsSocketBlocking();
    auto isNOTABlockingSocket   = !isABlockingSocket;


    std::once_flag flag, flagread;
    while(m_valid)
    {
        std::call_once(flag, [&](){
            std::cout << "UDP Service running. " << std::endl;
        });
        static unsigned long const maxBufferSize    = 1 << 13;


        auto [buffer, bufferRaw, pSock, pSockRawIn, pSockRaw, pSrcAddrLen] = CreateBufferAndIpSockAddr();

        /* receive in an unblocking fashion. */
        auto nRawDataSize   = recvfrom(m_socket, bufferRaw, maxBufferSize, 0, reinterpret_cast<sockaddr*>(pSock.get()), reinterpret_cast<socklen_t *>(&pSrcAddrLen));
        auto errorNumber    = errno;
        
        auto errorOcurred   = (nRawDataSize < 1) && (EWOULDBLOCK == errorNumber && isABlockingSocket); 

        std::call_once(flagread, [&](){
            std::cout << "Recvfrom result = " << nRawDataSize << std::endl;
            if ( !errorOcurred )
            {
                std::cout << "Yep, EAGAIN -- This seems to be a non blocking socket." << std::endl;
            }
            else 
            {
                std::cout << "Nop, A bad ugly bug is going on -- . Emitting a sitting duck socket error." << std::endl;
            }
        });
        if (nRawDataSize > 0) {

            /* Check Info on the peer */
            auto datagram = datagram_tuple{nRawDataSize, pSock, buffer};
            datagramReceived.emit(datagram);
            
        } else if (errorOcurred) {

            EmitError(*this, errorNumber);

        } else {

            /* We should emit the non blocking socket, didn't received data.  */
        }

    }

    close(m_socket);
    std::cout << "Socket is closed....\n";
}

long Services::UDPSocket::SendDatagram(const Services::UDPSocket::datagram_tuple &datagramTuple) {

    auto& [nRawDataSize, pSrcAddrIn , pBufferData] = datagramTuple;
    auto pSrcAddr                                   = reinterpret_cast<sockaddr*>(pSrcAddrIn.get());
    auto pBufferRawData                             = pBufferData.get();
    long nBytesSentAccumulator                      = 0;

    while (nBytesSentAccumulator < nRawDataSize){

        auto nBytesSentResult   = sendto(m_socket, &pBufferRawData[nBytesSentAccumulator], nRawDataSize, 0, pSrcAddr, sizeof(*pSrcAddr));
        auto errorNumber        = errno;
        
        if (nBytesSentResult < 0 ){
        
            Services::UDPSocket::EmitError(*this, errorNumber);
        
        } else if (nBytesSentResult){
        
            Services::UDPSocket::datagramSent.emit(nBytesSentAccumulator, nBytesSentResult, datagramTuple);
            nBytesSentAccumulator += nBytesSentResult;
        
        } else {
        
            Services::UDPSocket::EmitError(*this, 0);
        
        }
    }
    return nBytesSentAccumulator;
}

std::tuple<bool, std::error_condition, Services::UDPSocket::datagram_tuple> InvalidDatagram(){

    bool valid = false;
    auto errorcondition = std::error_condition{};
    auto datagram   = std::make_tuple(0, nullptr, nullptr);

    return std::make_tuple(valid, errorcondition, datagram);

};

std::tuple<bool, std::error_condition, Services::UDPSocket::datagram_tuple> Services::UDPSocket::RecvDatagram() {


    if (m_valid == false) return InvalidDatagram();

    auto    [pSock, pSockRawIn, pSockRaw, pSockLen] = CreateIpSockAddr();
    auto    [buffer, bufferRaw] = CreateBuffer(maxBufferSize);
    
    //Read
    auto nBytes = recvfrom(m_socket, buffer.get(), maxBufferSize, 0, pSockRaw, reinterpret_cast<socklen_t *>(&pSockLen));
    
    //Error
    if ( nBytes == -1 ) return InvalidDatagram();
    
    //No Error but nothing returned.
    auto valid = true;
    if ( nBytes == 0){
        return std::make_tuple(true, std::error_condition{}, std::make_tuple(0, nullptr, nullptr));
    } 
    return std::make_tuple(true, std::error_condition{}, std::make_tuple(nBytes, pSock, buffer));
    
}

Services::UDPSocket::datagram_tuple Services::UDPSocket::CreateDatagram(std::string ipString, std::string portString , unsigned char* pDataBuffer, long sz){

    auto pSockAddr          = std::make_shared<struct sockaddr_in>();
    sockaddr_in* si_other   = pSockAddr.get();

    std::memset(si_other, 0, sizeof(sockaddr_in));
    auto portx = std::atol(portString.data());
    si_other->sin_family = AF_INET;
    si_other->sin_port   = htons(portx);

    const auto pIp = ipString.data();
    if (inet_aton(pIp , &si_other->sin_addr) == 0)
    {
        return std::make_tuple(0, nullptr, nullptr);
    }
    auto [dataBuffer, dataBufferRaw] = CreateBuffer(sz);
    std::memcpy(dataBufferRaw, pDataBuffer, sz);
    
    return std::make_tuple(sz, pSockAddr, dataBuffer);

}
void Services::UDPSocket::EmitError(const UDPSocket& u, int errorNumber){

    switch (errorNumber)
    {
        case EAFNOSUPPORT:
            u.datagramError.emit(std::make_error_condition(std::errc::address_family_not_supported), std::make_error_condition(std::errc::address_family_not_supported).message());
            break;
        case EADDRINUSE:
            u.datagramError.emit(std::make_error_condition(std::errc::address_in_use), std::make_error_condition(std::errc::address_in_use).message());
            break;
        case EADDRNOTAVAIL:
            u.datagramError.emit(std::make_error_condition(std::errc::address_not_available), std::make_error_condition(std::errc::address_not_available).message());
            break;
        case EISCONN:
            u.datagramError.emit(std::make_error_condition(std::errc::already_connected), std::make_error_condition(std::errc::already_connected).message());
            break;
        case E2BIG:
            u.datagramError.emit(std::make_error_condition(std::errc::argument_list_too_long), std::make_error_condition(std::errc::argument_list_too_long).message());
            break;
        case EDOM:
            u.datagramError.emit(std::make_error_condition(std::errc::argument_out_of_domain), std::make_error_condition(std::errc::argument_out_of_domain).message());
            break;
        case EFAULT:
            u.datagramError.emit(std::make_error_condition(std::errc::bad_address), std::make_error_condition(std::errc::bad_address).message());
            break;
        case EBADF:
            u.datagramError.emit(std::make_error_condition(std::errc::bad_file_descriptor), std::make_error_condition(std::errc::bad_file_descriptor).message());
            break;
        case EBADMSG:
            u.datagramError.emit(std::make_error_condition(std::errc::bad_message), std::make_error_condition(std::errc::bad_message).message());
            break;
        case EPIPE:
            u.datagramError.emit(std::make_error_condition(std::errc::broken_pipe), std::make_error_condition(std::errc::broken_pipe).message());
            break;
        case ECONNABORTED:
            u.datagramError.emit(std::make_error_condition(std::errc::connection_aborted), std::make_error_condition(std::errc::connection_aborted).message());
            break;
        case EALREADY:
            u.datagramError.emit(std::make_error_condition(std::errc::connection_already_in_progress), std::make_error_condition(std::errc::connection_already_in_progress).message());
            break;
        case ECONNREFUSED:
            u.datagramError.emit(std::make_error_condition(std::errc::connection_refused), std::make_error_condition(std::errc::connection_refused).message());
            break;
        case ECONNRESET:
            u.datagramError.emit(std::make_error_condition(std::errc::connection_reset), std::make_error_condition(std::errc::connection_reset).message());
            break;
        case EXDEV:
            u.datagramError.emit(std::make_error_condition(std::errc::cross_device_link), std::make_error_condition(std::errc::cross_device_link).message());
            break;
        case EDESTADDRREQ:
            u.datagramError.emit(std::make_error_condition(std::errc::destination_address_required), std::make_error_condition(std::errc::destination_address_required).message());
            break;
        case EBUSY:
            u.datagramError.emit(std::make_error_condition(std::errc::device_or_resource_busy), std::make_error_condition(std::errc::device_or_resource_busy).message());
            break;
        case ENOTEMPTY:
            u.datagramError.emit(std::make_error_condition(std::errc::directory_not_empty), std::make_error_condition(std::errc::directory_not_empty).message());
            break;
        case ENOEXEC:
            u.datagramError.emit(std::make_error_condition(std::errc::executable_format_error), std::make_error_condition(std::errc::executable_format_error).message());
            break;
        case EEXIST:
            u.datagramError.emit(std::make_error_condition(std::errc::file_exists), std::make_error_condition(std::errc::file_exists).message());
            break;
        case EFBIG:
            u.datagramError.emit(std::make_error_condition(std::errc::file_too_large), std::make_error_condition(std::errc::file_too_large).message());
            break;
        case ENAMETOOLONG:
            u.datagramError.emit(std::make_error_condition(std::errc::filename_too_long), std::make_error_condition(std::errc::filename_too_long).message());
            break;
        case ENOSYS:
            u.datagramError.emit(std::make_error_condition(std::errc::function_not_supported), std::make_error_condition(std::errc::function_not_supported).message());
            break;
        case EHOSTUNREACH:
            u.datagramError.emit(std::make_error_condition(std::errc::host_unreachable), std::make_error_condition(std::errc::host_unreachable).message());
            break;
        case EIDRM:
            u.datagramError.emit(std::make_error_condition(std::errc::identifier_removed), std::make_error_condition(std::errc::identifier_removed).message());
            break;
        case EILSEQ:
            u.datagramError.emit(std::make_error_condition(std::errc::illegal_byte_sequence), std::make_error_condition(std::errc::illegal_byte_sequence).message());
            break;
        case ENOTTY:
            u.datagramError.emit(std::make_error_condition(std::errc::inappropriate_io_control_operation), std::make_error_condition(std::errc::inappropriate_io_control_operation).message());
            break;
        case EINTR:
            u.datagramError.emit(std::make_error_condition(std::errc::interrupted), std::make_error_condition(std::errc::interrupted).message());
            break;
        case EINVAL:
            u.datagramError.emit(std::make_error_condition(std::errc::invalid_argument), std::make_error_condition(std::errc::invalid_argument).message());
            break;
        case ESPIPE:
            u.datagramError.emit(std::make_error_condition(std::errc::invalid_seek), std::make_error_condition(std::errc::invalid_seek).message());
            break;
        case EIO:
            u.datagramError.emit(std::make_error_condition(std::errc::io_error), std::make_error_condition(std::errc::io_error).message());
            break;
        case EISDIR:
            u.datagramError.emit(std::make_error_condition(std::errc::is_a_directory), std::make_error_condition(std::errc::is_a_directory).message());
            break;
        case EMSGSIZE:
            u.datagramError.emit(std::make_error_condition(std::errc::message_size), std::make_error_condition(std::errc::message_size).message());
            break;
        case ENETDOWN:
            u.datagramError.emit(std::make_error_condition(std::errc::network_down), std::make_error_condition(std::errc::network_down).message());
            break;
        case ENETRESET:
            u.datagramError.emit(std::make_error_condition(std::errc::network_reset), std::make_error_condition(std::errc::network_reset).message());
            break;
        case ENETUNREACH:
            u.datagramError.emit(std::make_error_condition(std::errc::network_unreachable), std::make_error_condition(std::errc::network_unreachable).message());
            break;
        case ENOBUFS:
            u.datagramError.emit(std::make_error_condition(std::errc::no_buffer_space), std::make_error_condition(std::errc::no_buffer_space).message());
            break;
        case ECHILD:
            u.datagramError.emit(std::make_error_condition(std::errc::no_child_process), std::make_error_condition(std::errc::no_child_process).message());
            break;
        case ENOLINK:
            u.datagramError.emit(std::make_error_condition(std::errc::no_link), std::make_error_condition(std::errc::no_link).message());
            break;
        case ENOLCK:
            u.datagramError.emit(std::make_error_condition(std::errc::no_lock_available), std::make_error_condition(std::errc::no_lock_available).message());
            break;
        case ENODATA:
            u.datagramError.emit(std::make_error_condition(std::errc::no_message_available), std::make_error_condition(std::errc::no_message_available).message());
            break;
        case ENOMSG:
            u.datagramError.emit(std::make_error_condition(std::errc::no_message), std::make_error_condition(std::errc::no_message).message());
            break;
        case ENOPROTOOPT:
            u.datagramError.emit(std::make_error_condition(std::errc::no_protocol_option), std::make_error_condition(std::errc::no_protocol_option).message());
            break;
        case ENOSPC:
            u.datagramError.emit(std::make_error_condition(std::errc::no_space_on_device), std::make_error_condition(std::errc::no_space_on_device).message());
            break;
        case ENOSR:
            u.datagramError.emit(std::make_error_condition(std::errc::no_stream_resources), std::make_error_condition(std::errc::no_stream_resources).message());
            break;
        case ENXIO:
            u.datagramError.emit(std::make_error_condition(std::errc::no_such_device_or_address), std::make_error_condition(std::errc::no_such_device_or_address).message());
            break;
        case ENODEV:
            u.datagramError.emit(std::make_error_condition(std::errc::no_such_device), std::make_error_condition(std::errc::no_such_device).message());
            break;
        case ENOENT:
            u.datagramError.emit(std::make_error_condition(std::errc::no_such_file_or_directory), std::make_error_condition(std::errc::no_such_file_or_directory).message());
            break;
        case ESRCH:
            u.datagramError.emit(std::make_error_condition(std::errc::no_such_process), std::make_error_condition(std::errc::no_such_process).message());
            break;
        case ENOTDIR:
            u.datagramError.emit(std::make_error_condition(std::errc::not_a_directory), std::make_error_condition(std::errc::not_a_directory).message());
            break;
        case ENOTSOCK:
            u.datagramError.emit(std::make_error_condition(std::errc::not_a_socket), std::make_error_condition(std::errc::not_a_socket).message());
            break;
        case ENOSTR:
            u.datagramError.emit(std::make_error_condition(std::errc::not_a_stream), std::make_error_condition(std::errc::not_a_stream).message());
            break;
        case ENOTCONN:
            u.datagramError.emit(std::make_error_condition(std::errc::not_connected), std::make_error_condition(std::errc::not_connected).message());
            break;
        case ENOMEM:
            u.datagramError.emit(std::make_error_condition(std::errc::not_enough_memory), std::make_error_condition(std::errc::not_enough_memory).message());
            break;
        case ENOTSUP:
            u.datagramError.emit(std::make_error_condition(std::errc::not_supported), std::make_error_condition(std::errc::not_supported).message());
            break;
        case ECANCELED:
            u.datagramError.emit(std::make_error_condition(std::errc::operation_canceled), std::make_error_condition(std::errc::operation_canceled).message());
            break;
        case EINPROGRESS:
            u.datagramError.emit(std::make_error_condition(std::errc::operation_in_progress), std::make_error_condition(std::errc::operation_in_progress).message());
            break;
        case EPERM:
            u.datagramError.emit(std::make_error_condition(std::errc::operation_not_permitted), std::make_error_condition(std::errc::operation_not_permitted).message());
            break;
        case EOPNOTSUPP:
            u.datagramError.emit(std::make_error_condition(std::errc::operation_not_supported), std::make_error_condition(std::errc::operation_not_supported).message());
            break;
        case EAGAIN:
            u.datagramError.emit(std::make_error_condition(std::errc::resource_unavailable_try_again), std::make_error_condition(std::errc::resource_unavailable_try_again).message());
            break;
        case EOWNERDEAD:
            u.datagramError.emit(std::make_error_condition(std::errc::owner_dead), std::make_error_condition(std::errc::owner_dead).message());
            break;
        case EACCES:
            u.datagramError.emit(std::make_error_condition(std::errc::permission_denied), std::make_error_condition(std::errc::permission_denied).message());
            break;
        case EPROTO:
            u.datagramError.emit(std::make_error_condition(std::errc::protocol_error), std::make_error_condition(std::errc::protocol_error).message());
            break;
        case EPROTONOSUPPORT:
            u.datagramError.emit(std::make_error_condition(std::errc::protocol_not_supported), std::make_error_condition(std::errc::protocol_not_supported).message());
            break;
        case EROFS:
            u.datagramError.emit(std::make_error_condition(std::errc::read_only_file_system), std::make_error_condition(std::errc::read_only_file_system).message());
            break;
        case EDEADLK:
            u.datagramError.emit(std::make_error_condition(std::errc::resource_deadlock_would_occur), std::make_error_condition(std::errc::resource_deadlock_would_occur).message());
            break;
        case ERANGE:
            u.datagramError.emit(std::make_error_condition(std::errc::result_out_of_range), std::make_error_condition(std::errc::result_out_of_range).message());
            break;
        case ENOTRECOVERABLE:
            u.datagramError.emit(std::make_error_condition(std::errc::state_not_recoverable), std::make_error_condition(std::errc::state_not_recoverable).message());
            break;
        case ETIME:
            u.datagramError.emit(std::make_error_condition(std::errc::stream_timeout), std::make_error_condition(std::errc::stream_timeout).message());
            break;
        case ETXTBSY:
            u.datagramError.emit(std::make_error_condition(std::errc::text_file_busy), std::make_error_condition(std::errc::text_file_busy).message());
            break;
        case ETIMEDOUT:
            u.datagramError.emit(std::make_error_condition(std::errc::timed_out), std::make_error_condition(std::errc::timed_out).message());
            break;
        case ENFILE:
            u.datagramError.emit(std::make_error_condition(std::errc::too_many_files_open_in_system), std::make_error_condition(std::errc::too_many_files_open_in_system).message());
            break;
        case EMFILE:
            u.datagramError.emit(std::make_error_condition(std::errc::too_many_files_open), std::make_error_condition(std::errc::too_many_files_open).message());
            break;
        case EMLINK:
            u.datagramError.emit(std::make_error_condition(std::errc::too_many_links), std::make_error_condition(std::errc::too_many_links).message());
            break;
        case ELOOP:
            u.datagramError.emit(std::make_error_condition(std::errc::too_many_symbolic_link_levels), std::make_error_condition(std::errc::too_many_symbolic_link_levels).message());
            break;
        case EOVERFLOW:
            u.datagramError.emit(std::make_error_condition(std::errc::value_too_large), std::make_error_condition(std::errc::value_too_large).message());
            break;
        case EPROTOTYPE:
            u.datagramError.emit(std::make_error_condition(std::errc::wrong_protocol_type), std::make_error_condition(std::errc::wrong_protocol_type).message());
            break;
        default:
            u.datagramUnknownError.emit(std::string{"Unknown Error"});
            break;
    }

}

Services::UDPSocket::~UDPSocket() {
    std::cout << "UDPSocket out.\n";
}