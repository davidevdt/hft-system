#pragma once 
#include <sstream> 
#include "utils/types.h"
#include "utils/lock_free_queue.h"

using namespace Common; 

namespace Exchange {
#prama pack(push, 1)
    enum class ClientResponsType: uint8_t {
        INVALID = 0, 
        ACCEPTED = 1, 
        CANCELED = 2, 
        FILLED = 3, 
        CANCEL_REJECTED = 4 // cancel request is rejected by the matching engine 
    }; 
    inline std::string clientResponsTypeToString(ClientResponsType type) {
        switch (type) {
            case ClientResponsType::ACCEPTED: return "ACCEPTED"; 
            case ClientResponsType::CANCELED: return "CANCELED"; 
            case ClientResponsType::FILLED: return "FILLED"; 
            case ClientResponsType::CANCEL_REJECTED: return "CANCEL_REJECTED"; 
            case ClientResponsType::INVALID: return "INVALID"; 
        }
        return "UNKNOWN"; 
    }

    // Sends the order response to the client 
    struct MEClientResponse {
        ClientResponsType type_ = ClientResponsType::INVALID; 
        ClientId client_id_ = ClientId_INVALID; 
        TickerId ticker_id_ = TickerId_INVALID; 
        OrderId client_order_id_ = OrderId_INVALID; // order id, unique to each client (a same order id numer can be used by different clients)
        OrderId market_order_id_ = OrderId_INVALID;  // ordrt id, unique to the whole market (unique order id across all market participants)
        Side side_ = Side::INVALID; 
        Price price_ = Price_INVALID; 
        Qty exec_qty_ = Qty_INVALID; // not cumulative: partially executed orders (for multiple times) have a different MEClientResponse message for each individual execution, not across all of them 
        Qty leaves_qty_ = Qty_INVALID; // how much of the original order's quantity is still live in the matchin engine's order book 
        auto toString() const {
            std::stringstream ss; 
            ss << "MEClientResponse"
               << " ["
               << "type:" << clientResponsTypeToString(type_) 
               << " client:" << clientIdToString(client_id_) 
               << " ticker:" << tickerIdToString(ticker_id_) 
               << " coid:" << orderIdToString(client_order_id_) 
               << " moid:" << orderIdToString(market_order_id_) 
               << " side:" << sideToString(side_) 
               << " exec_qty:" << qtyToString(exec_qty_) 
               << " leaves_qty:" << qtyToString(leaves_qty_) 
               << " price:" << priceToString(price_) 
               << "]";
            return ss.str();  
        }
    };
#pragma(pop)

    typedef LFQueue<MEClientResponse> ClientReponseLFQueue; 
}