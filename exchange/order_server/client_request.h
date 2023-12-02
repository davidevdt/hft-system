#pragma once 
#include <sstream> 
#include "utils/types.h"
#include "utils/lock_free_queue.h"
using namespace Common; 

namespace Exchange {
#pragma pack(push,1) // avoid extra padding - 1 byte alignment = no padding between members - to save memory with binary structures sent/received over network

    enum class ClientRequestType : uint8_t {
        INVALID = 0, NEW = 1, CANCEL = 2
    }; 

    inline std::string clientRequestTypeToString(ClientRequestType type) {
        switch (type) {
            case ClientRequestType::NEW: return "NEW"; 
            case ClientRequestType::CANCEL: return "CANCEL"; 
            case ClientRequestType::INVALID: return "INVALID"; 
        }
        return "UNKNOWN"; 
    }

    // MEClientRequest contains information for a single order request from the trading participant to the exchange 
    // This is internal engine structure, not necessarily the format from the client's side 
    struct MEClientRequest {
        ClientRequestType type_ = ClientRequestType::INVALID; 
        ClientId client_id_ = ClientId_INVALID; 
        TickerId ticker_id = TickerId_INVALID; 
        OrderId order_id = OrderId_INVALID; 
        Side side_ = Side::INVALID; 
        Price price_ = Price_INVALID; 
        Qty qty_ = Qty_INVALID; 
        auto toString() const {
            std::stringstream ss; 
            ss << "MEClientRequest" 
               << " ["
               << " client:" << clientIdToString(client_id_)
               << " ticker:" << tickerIdToString(ticker_id)
               << " oid:" << orderIdToString(order_id) 
               << " side:" << sideToString(side_) 
               << " qty:" << qtyToString(qty_) 
               << " price:" << priceToString(price_)
               << "]"; 
            return ss.str();  
        }
    }; 
#pragma pack(pop) // restores the alignment setting to the default (not tightly packed) -> we only want to pack the structures sent over the network, and not others 
    
    typedef LFQueue<MEClientRequest> ClientRequestLFQueue; 
}