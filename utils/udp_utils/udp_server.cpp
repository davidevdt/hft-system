#include "udp_server.h"

namespace Common {
    auto UDPServer::destroy() {
        close(efd_); 
        efd_ = -1; 
    }

    // Add the provided file descriptor socket to the efd_ member variable
    // EPOLLET: edge-triggered epoll option, it is up to the app developer to read the data when they want
    // instead of being constantly notified 
    // EPOLLIN: notification once data is available to be read 
    auto UDPServer::epoll_add(UDPSocket* socket) -> bool {
        efd_ = epoll_create(1); 
        ASSERT(efd_ >= 0, "epoll_create() failed error: " + std::string(std::strerror(errno))); 
        epoll_event ev{}; 
        ev.events = EPOLLET | EPOLLIN; 
        ev.data.ptr = reinterpret_cast<void *>(socket); 
        return (epoll_ctl(efd_, EPOLL_CTL_ADD, socket->fd_, &ev) != -1); 
    }

    // Creates a new epoll instance ready to listen for sending/receiving sockets 
    // auto UDPServer::bind(const std::string& iface, int port) -> void {
    //     destroy(); 
    //     efd_ = epoll_create(1); 
    //     ASSERT(efd_ >= 0, "epoll_create() failed error: " + std::string(std::strerror(errno))); 
    //     ASSERT(socket_.bind("", iface, port) >= 0, 
    //         "Listener socket failed to connect. iface:" + iface + " port: " + std::to_string(port) +
    //         " error: " + std::string(std::strerror(errno))); 
    //     ASSERT(epoll_add(&socket_), 
    //         "epoll_ctl() failed. error: " + std::string(std::strerror(errno))); 
    // }

    // Removes the UDPSocket from the list of sockets being monitored
    // auto UDPServer::epoll_del(UDPSocket* socket) {
    //     return (epoll_ctl(efd_, EPOLL_CTL_DEL, socket->fd_, nullptr) != -1); 
    // }

    // // Removes the UDPSocket from the list of sockets 
    // auto UDPServer::del(UDPSocket* socket) {
    //     epoll_del(socket); 
    //     sockets_.erase(std::remove(sockets_.begin(), sockets_.end(), socket), sockets_.end()); 
    //     receive_sockets_.erase(std::remove(receive_sockets_.begin(), 
    //         receive_sockets_.end(), socket), receive_sockets_.end()); 
    //     send_sockets_.erase(std::remove(send_sockets_.begin(), 
    //         send_sockets_.end(), socket), send_sockets_.end());
    // }

    // Add socket to send list 
    // void UDPServer::addSendSocket(UDPSocket* socket) {
    //     send_sockets_.push_back(socket);
    // }

    // epoll_wait to wait for new events 
    auto UDPServer::poll() noexcept -> void {
        const int max_events = 1 + receive_sockets_.size(); 
        const int n = epoll_wait(efd_, events_, max_events, 0); // 0 = timeout in ms; here epoll_wait will not block 
        std::cout << "SERVER: EFD = " << efd_ << std::endl; 
        for (int i = 0; i < n; ++i) {
            // Cast epoll_events into sockets 
            epoll_event& event = events_[i]; 
            auto socket = reinterpret_cast<UDPSocket*>(event.data.ptr); 
            std::cout << "~ SERVER ~ Socket: " << socket->fd_ << std::endl; 
            // Check for receiver sockets
            if (event.events & EPOLLIN) {
                // Receiver sockets
                logger_.log("%:% %() % EPOLLIN socket:%\n", __FILE__,__LINE__,__FUNCTION__, 
                    Common::getCurrentTimeStr(&time_str_), socket->fd_); 
                if (std::find(receive_sockets_.begin(), receive_sockets_.end(), socket) == receive_sockets_.end())
                    receive_sockets_.push_back(socket);  
            }

            // Sender sockets
            if (event.events & EPOLLOUT) {
                logger_.log("%:% %() % EPOLLOUT socket:%\n", __FILE__, __LINE__, __FUNCTION__,
                            Common::getCurrentTimeStr(&time_str_), socket->fd_);
                if (std::find(send_sockets_.begin(), send_sockets_.end(), socket) == send_sockets_.end())
                    send_sockets_.push_back(socket);
            }

            // Nor for checking connection errors 

        }
    }

    // Sends and receives data
    auto UDPServer::sendAndRecv() noexcept -> void {
        auto recv = false; 
        for (auto socket: receive_sockets_) {
            if (socket->sendAndRecv()) recv = true; 
        }
        if (recv) {
            // receive_sockets_.clear(); // we don't need to keep the connection   
            recv_finished_callback_(); 
        }

        for (auto socket: send_sockets_) {
            socket->sendAndRecv(); 
        }
        // send_sockets_.clear(); // we don't need to keep the connection 
    }

}
