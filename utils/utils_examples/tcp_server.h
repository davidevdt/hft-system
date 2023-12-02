#pragma once 
#include "tcp_socket.h"

namespace Common {

    struct TCPServer {
        public: 
        int efd_ = 1; 
        TCPSocket listener_socket_; 
        epoll_event events_[1024]; 
        std::vector<TCPSocket*> sockets_, receive_sockets_, send_sockets_, disconnected_sockets_; 
        std::function<void(TCPSocket* s, Nanos rx_time)> recv_callback_; 
        std::function<void()> recv_finished_callback_; // to be called when all sockets have been notified
        std::string time_str_; 
        Logger& logger_; 

        auto defaultRecvCallback(TCPSocket* socket, Nanos rx_time) noexcept {
            logger_.log("%:% %() TCPServer::defaultRecvCallback() socket:% len:% rx:%\n", 
            __FILE__,__LINE__,__FUNCTION__,Common::getCurrentTimeStr(&time_str_), 
            socket->fd_, socket->next_rcv_valid_index_, rx_time); 
        };

        auto defalutRecvFinishedCallback() noexcept {
            logger_.log("%:% %() TCPServer::defalutRecvFinishedCallback()\n", 
            __FILE__,__LINE__,__FUNCTION__,Common::getCurrentTimeStr(&time_str_)); 
        };

        explicit TCPServer(Logger& logger) : listener_socket_(logger), logger_(logger) {
            recv_callback_ = [this](auto socket, auto rx_time) {
                defaultRecvCallback(socket, rx_time); 
            };
            recv_finished_callback_ = [this]() {
                defalutRecvFinishedCallback(); 
            };
        }

        TCPServer() = delete; 
        TCPServer(const TCPServer&) = delete; 
        TCPServer(const TCPServer&&) = delete; 
        TCPServer& operator=(const TCPServer&) = delete; 
        TCPServer& operator=(const TCPServer&&) = delete; 

        auto destroy();
        auto listen(const std::string& iface, int port) -> void; 
        auto epoll_add(TCPSocket* socket); 
        auto epoll_del(TCPSocket* socket); 
        auto del(TCPSocket* socket); 
        auto poll() noexcept -> void; 
        auto sendAndRecv() noexcept -> void; 
    }; 
}