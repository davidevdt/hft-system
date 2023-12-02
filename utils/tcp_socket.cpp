#include "tcp_socket.h"

namespace Common {
    auto TCPSocket::destroy() noexcept -> void {
        close(fd_); 
        fd_ = -1;
    }

    auto TCPSocket::connect(const std::string& ip, const std::string& iface, int port, bool is_listening) -> int {
        destroy(); 
        fd_ = createSocket(logger_, ip, iface, port, false, false, is_listening, 0, true); 
        inInAddr.sin_addr.s_addr = INADDR_ANY; 
        inInAddr.sin_port = htons(port); 
        inInAddr.sin_family = AF_INET; 
        return fd_; 
    }

    auto TCPSocket::send(const void* data, size_t len) -> void {
        if (len > 0) {
            memcpy(send_buffer_ + next_send_valid_index_, data, len); 
            next_send_valid_index_ += len; 
        }
    }

    auto TCPSocket::sendAndRecv() noexcept -> bool {
        // Set up control header and message header:
        char ctrl[CMSG_SPACE(sizeof(struct timeval))]; 
        struct cmsghdr* cmsg = (struct cmsghdr*) &ctrl; // control message header
        struct iovec io; // data to be received (or sent)
        io.iov_base = rcv_buffer_ + next_rcv_valid_index_; 
        io.iov_len = TCPBufferSize - next_rcv_valid_index_; 
        msghdr msg; // message header 
        msg.msg_control = ctrl; 
        msg.msg_controllen = sizeof(ctrl); 
        msg.msg_name = &inInAddr; 
        msg.msg_namelen = sizeof(inInAddr); 
        msg.msg_iov = &io; 
        msg.msg_iovlen = 1; 
        const auto n_rcv = recvmsg(fd_, &msg, MSG_DONTWAIT); 
        // Extract the message timestamp provided by the kernel:
        if (n_rcv > 0) {
            next_rcv_valid_index_ += n_rcv; 
            Nanos kernel_time = 0; 
            struct timeval time_kernel; 
            if (cmsg->cmsg_level == SOL_SOCKET && // socket-level 
                cmsg->cmsg_type == SCM_TIMESTAMP && // include a timestamp with the message 
                cmsg->cmsg_len == CMSG_LEN(sizeof(time_kernel))) { // length of control message 
                memcpy(&time_kernel, CMSG_DATA(cmsg), sizeof(time_kernel)); 
                kernel_time = time_kernel.tv_sec * NANOS_TO_SECS + time_kernel.tv_usec * NANOS_TO_MICROS;
            }
            const auto user_time = getCurrentNanos(); 
            logger_.log("%:% %() % read socket:% len:% utime:% ktime:% diff:%\n", 
                __FILE__,__LINE__,__FUNCTION__,
                Common::getCurrentTimeStr(&time_str_), fd_, next_rcv_valid_index_, 
                user_time, kernel_time, (user_time-kernel_time)); 
            recv_callback_(this, kernel_time);

            // Handle buffer overflow
            // if (next_rcv_valid_index_ >= TCPBufferSize) {
            //     next_rcv_valid_index_ = 0;
            // }
        }

        // Send data over a socket in chunks 
        ssize_t n_send = std::min(TCPBufferSize, next_send_valid_index_); // n bytes to send
        while (n_send > 0) {
            auto n_send_this_msg = std::min(static_cast<ssize_t>(next_send_valid_index_), n_send); // n bytes to send in this iteration
            const int flags = MSG_DONTWAIT | MSG_NOSIGNAL | (n_send_this_msg < n_send ? MSG_MORE : 0); // NONBLOCKING + NO SIGPIPE TERMINATION SIGNAL + MORE DATA TO BE SENT
            auto n = ::send(fd_, send_buffer_, n_send_this_msg, flags); // sends up to n_send_this_msg from send_buffer_ over the socket fd_;
            if (UNLIKELY( n < 0)) {
                if (!wouldBlock())
                    send_disconnected_ = true; 
                break; 
            }
            logger_.log("%: % %() % send socket: % len:%\n", 
                __FILE__,__LINE__,__FUNCTION__,Common::getCurrentTimeStr(&time_str_), fd_, n);
            n_send -= n; // decrements n_send by number of bytes actually sent in this iteration
            ASSERT(n == n_send_this_msg, "Don't support partial send lengths yet."); 
        }
        next_send_valid_index_ = 0; 
        return (n_rcv > 0); 
    }
}