#pragma once 
#include "udp_socket.h"

namespace Common {

    struct UDPServer {
        public: 
        int efd_ = 1; 
        epoll_event events_[1024]; 
        std::vector<UDPSocket*> receive_sockets_, send_sockets_; // no need of keep track of disconnected sockets as in UDP we have a connectionless protocol 
        std::function<void(UDPSocket* s, Nanos rx_time)> recv_callback_; 
        std::function<void()> recv_finished_callback_; // to be called when all sockets have been notified
        std::string time_str_; 
        Logger& logger_; 

        auto defaultRecvCallback(UDPSocket* socket, Nanos rx_time) noexcept {
            logger_.log("%:% %() UDPServer::defaultRecvCallback() socket:% len:% rx:%\n", 
            __FILE__,__LINE__,__FUNCTION__,Common::getCurrentTimeStr(&time_str_), 
            socket->fd_, socket->next_rcv_valid_index_, rx_time); 
        };

        auto defalutRecvFinishedCallback() noexcept {
            logger_.log("%:% %() UDPServer::defalutRecvFinishedCallback()\n", 
            __FILE__,__LINE__,__FUNCTION__,Common::getCurrentTimeStr(&time_str_)); 
        };

        explicit UDPServer(Logger& logger) : logger_(logger) {
            recv_callback_ = [this](auto socket, auto rx_time) {
                defaultRecvCallback(socket, rx_time); 
            };
            recv_finished_callback_ = [this]() {
                defalutRecvFinishedCallback(); 
            };
        }

        ~UDPServer() {
            for (auto socket : send_sockets_)
                delete socket;

            for (auto socket : receive_sockets_)
                delete socket;
        }

        UDPServer() = delete; 
        UDPServer(const UDPServer&) = delete; 
        UDPServer(const UDPServer&&) = delete; 
        UDPServer& operator=(const UDPServer&) = delete; 
        UDPServer& operator=(const UDPServer&&) = delete; 

        auto destroy();
        // auto bind(const std::string& iface, int port) -> void; 
        auto epoll_add(UDPSocket* socket) -> bool; 
        auto epoll_del(UDPSocket* socket); 
        auto del(UDPSocket* socket); 
        auto poll() noexcept -> void; 
        auto sendAndRecv() noexcept -> void; 
        // void addSendSocket(UDPSocket* socket); 
    }; 
}