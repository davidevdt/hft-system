#include "udp_socket.h"

namespace Common {
    auto UDPSocket::destroy() noexcept -> void {
        close(fd_); 
        fd_ = -1;
    }

    auto UDPSocket::bind(const std::string& ip, const std::string& iface, int port) -> int {
        destroy(); 
        fd_ = createSocket(logger_, ip, iface, port, true, false, false, 0, true); 
        inInAddr.sin_addr.s_addr = INADDR_ANY; 
        inInAddr.sin_port = htons(port); 
        inInAddr.sin_family = AF_INET; 
        return fd_; 
    }

    auto UDPSocket::send(const void* data, size_t len) -> void {
        if (len > 0) {
            memcpy(send_buffer_ + next_send_valid_index_, data, len); 
            next_send_valid_index_ += len; 
        }
    }

    auto UDPSocket::sendAndRecv() noexcept -> bool {
        // Prepare IP address (IPv4 protocol)
        struct sockaddr_in srcAddr; 
        socklen_t addrlen = sizeof(srcAddr);
        memset(&srcAddr, 0, sizeof(srcAddr));

        // Receive data
        ssize_t n_rcv = recvfrom(fd_, rcv_buffer_ + next_rcv_valid_index_, UDPBufferSize - next_rcv_valid_index_, MSG_DONTWAIT, (struct sockaddr*)&srcAddr, &addrlen); 
        std::cout << "n_rcev = " << n_rcv << " fd_ = " << fd_ << std::endl; 
        if (n_rcv > 0) {
            next_rcv_valid_index_ += n_rcv; 
            const auto user_time = getCurrentNanos(); 
            logger_.log("%:% %() % read socket:% len:% utime:%\n", 
                __FILE__,__LINE__,__FUNCTION__,
                Common::getCurrentTimeStr(&time_str_), fd_, next_rcv_valid_index_, user_time); 
            recv_callback_(this, user_time);

            // Handle buffer overflow
            // if (next_rcv_valid_index_ >= UDPBufferSize) {
            //     next_rcv_valid_index_ = 0;
            // }
        }

        // Send data 
        // ssize_t n_send = std::min(UDPBufferSize, next_send_valid_index_); 
        // while (n_send > 0) {
        //     auto n_send_this_msg = std::min(static_cast<ssize_t>(next_send_valid_index_), n_send);
        //     const int flags = MSG_DONTWAIT | MSG_NOSIGNAL | (n_send_this_msg < n_send ? MSG_MORE : 0); // NONBLOCKING + NO SIGPIPE TERMINATION SIGNAL + MORE DATA TO BE SENT
        //     auto n = sendto(fd_, send_buffer_, n_send_this_msg, flags, (struct sockaddr*)&srcAddr, addrlen); // sends up to n_send_this_msg from send_buffer_ over the socket fd_;
        //     if (UNLIKELY( n < 0)) {
        //         if (!wouldBlock())
        //             send_disconnected_ = true; 
        //         break; 
        //     }
        
        //     logger_.log("%: % %() % send socket: % len:%\n", 
        //         __FILE__,__LINE__,__FUNCTION__,Common::getCurrentTimeStr(&time_str_), fd_, n);
        //     n_send -= n; // decrements n_send by number of bytes actually sent in this iteration
        //     ASSERT(n == n_send_this_msg, "Don't support partial send lengths yet."); 
        // }

        ssize_t n_send = std::min(UDPBufferSize, next_send_valid_index_);
        std::cout << "n send: " << n_send << ", fd_: " <<  fd_ << std::endl;  
        if (n_send > 0) {
            auto n = sendto(fd_, send_buffer_, n_send, MSG_DONTWAIT,
                            (struct sockaddr*)&inInAddr, sizeof(inInAddr));
            std::cout << "~~~ N = " << n << std::endl; 
            if (UNLIKELY(n < 0)) {
                if (!wouldBlock())
                    send_disconnected_ = true;
                // Handle send error if needed
            }
        }
        next_send_valid_index_ = 0;
        return (n_rcv > 0);
    }
}