#pragma once 
#include <functional>
#include "socket_utils.h"
#include "logging.h"

namespace Common {
    constexpr size_t UDPBufferSize = 64 * 1024 * 1024; 
    
    struct UDPSocket {

        auto defaultRecvCallback(UDPSocket* socket, Nanos rx_time) noexcept {
            logger_.log("%:% %() % UDPSocket::defaultRecvCallback() socket: % len:% rx:%\n", 
                __FILE__,__LINE__,__FUNCTION__,Common::getCurrentTimeStr(&time_str_), 
                socket->fd_, socket->next_rcv_valid_index_, rx_time); 
        }

        explicit UDPSocket(Logger& logger)
            : logger_(logger) {
            send_buffer_ = new char[UDPBufferSize]; 
            rcv_buffer_ = new char[UDPBufferSize]; 
            recv_callback_ = [this] (auto socket, auto rx_time) {
                defaultRecvCallback(socket, rx_time);
            };
        }

        ~UDPSocket() {
            destroy(); 
            delete[] send_buffer_; send_buffer_ = nullptr; 
            delete[] rcv_buffer_; rcv_buffer_ = nullptr; 
        }

        UDPSocket() = delete; 
        UDPSocket(const UDPSocket&) = delete; 
        UDPSocket(const UDPSocket&&) = delete; 
        UDPSocket& operator=(const UDPSocket&) = delete; 
        UDPSocket& operator=(const UDPSocket&&) = delete; 

        auto destroy() noexcept -> void; 
        auto bind(const std::string&, const std::string&, int) -> int;
        auto send(const void* data, size_t len) -> void; 
        auto sendAndRecv() noexcept -> bool; 

        int fd_ = -1; 
        char* send_buffer_ = nullptr; 
        size_t next_send_valid_index_ = 0; 
        char* rcv_buffer_ = nullptr; 
        size_t next_rcv_valid_index_ = 0; 
        bool send_disconnected_ = false; 
        bool recv_disconnected_ = false; 
        struct sockaddr_in inInAddr; 
        std::function<void(UDPSocket* s, Nanos rx_time)> recv_callback_;  
        std::string time_str_; 
        Logger& logger_; 
    }; 


}