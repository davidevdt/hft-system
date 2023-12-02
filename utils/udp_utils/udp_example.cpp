#include "time_utils.h"
#include "logging.h"
#include "udp_server.h"

int main(int, char**) {
    using namespace Common; 

    // Prepare logger and server/client callback functions
    std::string time_str_; 
    Logger logger_("udp_example.log"); 
    auto udpServerRecvCallback = [&](UDPSocket* socket, Nanos rx_time) noexcept {
        logger_.log("UDPServer::defaultRecvCallback() socket:% len:% rx:%\n", 
            socket->fd_, socket->next_rcv_valid_index_, rx_time); 
        const std::string reply = "UDPServer received msg: " + \
            std::string(socket->rcv_buffer_, socket->next_rcv_valid_index_); 
        socket->next_rcv_valid_index_ = 0; 
        socket->send(reply.data(), reply.length()); 
    }; 
    auto udpServerRecvFinishedCallback = [&]() noexcept {
        logger_.log("UDPServer::defaultRecvFinishedCallback()\n");         
    };
    auto udpClientRecvCallback = [&](UDPSocket* socket, Nanos rx_time) noexcept {
        const std::string recv_msg = std::string(socket->rcv_buffer_, socket->next_rcv_valid_index_); 
        socket->next_rcv_valid_index_ = 0; 
        logger_.log("UDPServer::defaultRecvCallback() socket:% len:% rx:% msg:%\n", 
            socket->fd_, socket->next_rcv_valid_index_, rx_time, recv_msg); 
    }; 

    // Create, initialize, and connect server and clients 
    const std::string iface = "lo"; 
    const std::string ip = "127.0.0.1"; 
    const int port = 12345; 
    logger_.log("Creating UDPServer of iface:% port:%\n", iface, port); 
    UDPServer server(logger_); 
    server.recv_callback_ = udpServerRecvCallback; 
    server.recv_finished_callback_ = udpServerRecvFinishedCallback; 
    UDPSocket server_socket(logger_); 
    server_socket.bind(ip, iface, port); 
    ASSERT(server.epoll_add(&server_socket), "Couldn't bind socket to server."); 
    // server.bind(iface, port); 
    std::vector<UDPSocket*> clients(5); 
    for (size_t i = 0; i < clients.size(); ++i) {
        clients[i] = new UDPSocket(logger_); 
        clients[i]->recv_callback_ = udpClientRecvCallback; 
        logger_.log("Connecting UDPClient-[%] on ip:% iface:% port:%\n", 
            i, ip, iface, port);
        clients[i]->bind(ip, iface, port); 
        server.poll();  
        // server.addSendSocket(clients[i]); 
    }

    // Clients send data and call appropriate polling and send/receive method 
    using namespace std::literals::chrono_literals; 
    for (auto itr = 0; itr < 5; ++itr) {
        for (size_t i = 0; i < clients.size(); ++i) {
            const std::string client_msg = "CLIENT[" + std::to_string(i) + "] : Sending " +
            std::to_string(itr * 100 + i); 
            logger_.log("Sending UDPClient-[%] %\n", i, client_msg); 
            clients[i]->send(client_msg.data(), client_msg.length()); 
            clients[i]->sendAndRecv(); 
            std::this_thread::sleep_for(500ms); 
            server.poll(); 
            server.sendAndRecv(); 
            // server.addSendSocket(clients[i]); 
        }
    }

    for (auto itr = 0; itr < 5; itr++) {
        for (auto& client: clients) {
            client->sendAndRecv(); 
            server.poll(); 
            server.sendAndRecv(); 
            std::this_thread::sleep_for(500ms); 
        }
    } 

    return 0;
}