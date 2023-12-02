#pragma once 
#include <iostream> 
#include <string> 
#include <unordered_set> 
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h> 
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include "macros.h"
#include "logging.h"

namespace Common {
    // Maximum number of pending connections that can be queued up
    constexpr int MaxTCPServerBacklog = 1024; 

    // Gets interface information, for example: 
    // * use "lo" as input for loopback interface (127.0.0.1 interface)
    // * use "wlp4s0" for Wi-fi interface IP address
    inline auto getIfaceIP(const std::string& iface) -> std::string {
        char buf[NI_MAXHOST] = {'\0'}; 
        ifaddrs *ifaddr = nullptr; 
        if (getifaddrs(&ifaddr) != -1) {
            for (ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
                if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && iface == ifa->ifa_name) {
                    getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), buf, sizeof(buf), 
                        NULL, 0, NI_NUMERICHOST);
                        break; 
                }
            }
            freeifaddrs(ifaddr);
        }
        return buf; 
    }

    // Returns true if the socket file descriptor is already non-blocking
    // or the method is able to make it non-blocking
    inline auto setNonBlocking(int fd) -> bool {
        const auto flags = fcntl(fd, F_GETFL, 0); // ftntl is used to set flags among other things
        if (flags == -1) {  // error occured when retrieving file status flags 
            return false; 
        }
        if (flags & O_NONBLOCK) {   // O_NONBLOCK denotes a non-blocking file descriptor
            return true; 
        }
        return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1); 
    }

    // Delays sending some TCP packets (disables Nagle's algorithm)
    inline auto setNoDelay(int fd) -> bool {
        int one = 1; 
        return (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, 
            reinterpret_cast<void *>(&one), sizeof(one)) != -1); 
    }

    // Generate software timestamp to enable software timestamping
    inline auto setSOTimestamp(int fd) -> bool {
        int one = 1; 
        return (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, 
            reinterpret_cast<void *>(&one), sizeof(one)) != -1); 
    }

    // Checks if a socket operation would block or not 
    inline auto wouldBlock() -> bool {
        // EWOULDBLOCK: a non-blocking operation would block 
        // EINPROGRESS: a non-blocking operation is in prograss
        return (errno == EWOULDBLOCK || errno == EINPROGRESS); 
    } 

    // -- Setting TTL is used to prevent packets from circulating indefinitely in the network 
    // in case of routing loops or other network issues -- 
    // Set Time To Live for our socket (= max nr. of hops a packet can take 
    // from sender to receiver) -> for MULTICAST SOCKETS
    inline auto setMcastTTL(int fd, int mcast_ttl) -> bool {
        return (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, 
            reinterpret_cast<void *>(&mcast_ttl), sizeof(mcast_ttl)) != -1); 
    }

    // Set Time To Live for our socket (= max nr. of hops a packet can take 
    // from sender to receiver) -> for NON-MULTICAST SOCKETS
    inline auto setTTL(int fd, int ttl) -> bool {
        return (setsockopt(fd, IPPROTO_IP, IP_TTL, 
            reinterpret_cast<void *>(&ttl), sizeof(ttl)) != -1); 
    }

    // Add / Join membership / subscription to the multicast stream specified and on the interface specified
    inline auto join(int fd, const std::string& ip) -> bool {
        const ip_mreq mreq{{inet_addr(ip.c_str())}, {htonl(INADDR_ANY)}}; 
        return (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != -1);
    } 
    
    inline auto createSocket(Logger& logger, const std::string& t_ip, const std::string& iface, 
        int port, bool is_udp, bool is_blocking, bool is_listening, int ttl, bool needs_so_timestamp) -> int {
            
        std::string time_str; 
        const auto ip = t_ip.empty() ? getIfaceIP(iface) : t_ip; 
        logger.log("%:% %() % ip:% iface:% port:% is_udp:% is_blocking: % is_listening: % ttl:% SO_time:%\n", 
            __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str), ip, iface, 
            port, is_udp, is_blocking, is_listening, ttl, needs_so_timestamp); 
        
        addrinfo hints{}; 
        hints.ai_family = AF_INET; 
        hints.ai_socktype = is_udp ? SOCK_DGRAM : SOCK_STREAM; 
        hints.ai_protocol = is_udp ? IPPROTO_UDP : IPPROTO_TCP; 
        hints.ai_flags = is_listening ? AI_PASSIVE : 0; 
        if (std::isdigit(ip.c_str()[0])) hints.ai_flags |= AI_NUMERICHOST; 
        hints.ai_flags |= AI_NUMERICSERV; 
        addrinfo* result = nullptr; 
        const auto rc = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result); 
        if (rc) {
            logger.log("getaddinfo() failed. error:% errno:%\n", gai_strerror(rc), strerror(errno));
            return -1; 
        }

        // Create the socket 
        int fd = -1, one = 1; 
        for (addrinfo *rp = result; rp; rp = rp->ai_next) { // loop until rp is not null
            fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol); 
            if (fd == -1) {
                logger.log("socket() failed. errno: %\n", strerror(errno));
                return -1; 
            }
        
            // Set it to non-blocking and disable Nagle's algorithm
            if (!is_blocking) {
                if (!setNonBlocking(fd)) {
                    logger.log("setNonBlocking() failed. errno:%\n", strerror(errno)); 
                    return -1; 
                }
                if (!is_udp && !setNoDelay(fd)) {
                    logger.log("setNoDelay() failed. errno:%\n", strerror(errno)); 
                    return -1; 
                }
            }

            // Connect to socket to target address if it is not a listening socket
            if (!is_listening && connect(fd, rp->ai_addr, rp->ai_addrlen) == 1 && !wouldBlock()) {
                logger.log("connect() failed. errno:%\n", strerror(errno)); 
                return -1; 
            }

            // If the socket is a listening socket, bind the socket to the specific address
            // the clients wants to connect to and call "listen":

            //allows the reuse of a local address for binding, 
            //even if the socket is in a TIME_WAIT state (waiting for connections to fully close)
            if (is_listening && setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, 
                reinterpret_cast<const char *>(&one), sizeof(one)) == -1) {
                    logger.log("setsockopt() SO_REUSEADDR failed. errno:%\n", strerror(errno)); 
                    return -1; 
                }

            // binds the socket to a specific address
            if (is_listening && bind(fd, rp->ai_addr, rp->ai_addrlen) == -1) {
                logger.log("bind() failed. errno:%\n", strerror(errno)); 
                return -1; 
            }

            // puts the socket in a state where it's listening for incoming connections
            if (!is_udp && is_listening && listen(fd, MaxTCPServerBacklog) == -1) {
                logger.log("listen() failed. errno:%\n", strerror(errno)); 
                return -1; 
            }

            // Set the TTL value for the socket just created and return the socket
            if (is_udp && ttl) {
                const bool is_multicast = atoi(ip.c_str()) & 0xe0; 
                if (is_multicast && !setMcastTTL(fd, ttl)) {
                    logger.log("setMcastTTL() failed. errno:%\n", strerror(errno));
                    return -1; 
                }
                if (!is_multicast && !setTTL(fd, ttl)) {
                    logger.log("setTTL() failed. errno:%\n", strerror(errno));
                    return -1; 
                }
            }

            if (needs_so_timestamp && !setSOTimestamp(fd)) {
                logger.log("setSOTimestamp() failed. errno:%\n", strerror(errno));
                return -1;                 
            }
        }

        if (result) {
            freeaddrinfo(result);
        }
        return fd;
    }
}