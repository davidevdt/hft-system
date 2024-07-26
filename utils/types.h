#pragma once 
#include <cstdint> 
#include <limits> 
#include "utils/macros.h"

namespace Common {

    // The following limits and constraints are arbitrary and can be modified based on the capacity of the system 
    // on which the trading ecosystem is run 
    constexpr size_t LOG_QUEUE_SIZE = 8 * 1024 * 1024; // size of the lock-free queue used by the logger (max characters number)
    constexpr size_t ME_MAX_TICKERS = 8; // number of trading instruments the exchange supports 

    constexpr size_t ME_MAX_CLIENT_UPDATES = 256 * 1024; // max number unprocessed order requests from all clients
    constexpr size_t ME_MAX_MARKET_UPDATES = 256 * 1024; // max number of market updates generated by the matching engine not yet published 

    constexpr size_t ME_MAX_NUM_CLIENTS = 256; // max number of participan    constexpr size_t ME_MAX_TICKERS = 8;

    constexpr size_t ME_MAX_CLIENT_UPDATES = 256 * 1024;
    constexpr size_t ME_MAX_MARKET_UPDATES = 256 * 1024;

    constexpr size_t ME_MAX_NUM_CLIENTS = 256;
    constexpr size_t ME_MAX_ORDER_IDS = 1024 * 1024;
    constexpr size_t ME_MAX_PRICE_LEVELS = 256;ts in the trading ecosystem 
    constexpr size_t ME_MAX_ORDER_IDS = 1024 * 1024; // max number of orders possible for a single trading instrument 
    constexpr size_t ME_MAX_PRICE_LEVELS = 256; // max depth of price levels for the limit order book that the matching engine maintains

    typedef uint64_t OrderId; 
    constexpr auto OrderId_INVALID = std::numeric_limits<OrderId>::max(); 
    inline auto orderIdToString(OrderId order_id) -> std::string {
        if (UNLIKELY(order_id == OrderId_INVALID)) return "INVALID"; 
        return std::to_string(order_id); 
    }

    typedef uint32_t TickerId; 
    constexpr auto TickerId_INVALID = std::numeric_limits<TickerId>::max(); 
    inline auto tickerIdToString(TickerId ticker_id) -> std::string {
        if (UNLIKELY(ticker_id == TickerId_INVALID)) return "INVALID"; 
        return std::to_string(ticker_id); 
    }

    typedef uint64_t ClientId; 
    constexpr auto ClientId_INVALID = std::numeric_limits<ClientId>::max(); 
    inline auto clientIdToString(ClientId client_id) -> std::string {
        if (UNLIKELY(client_id == ClientId_INVALID)) return "INVALID"; 
        return std::to_string(client_id); 
    }

    typedef int64_t Price; 
    constexpr auto Price_INVALID = std::numeric_limits<Price>::max(); 
    inline auto priceToString(Price price) -> std::string {
        if (UNLIKELY(price == Price_INVALID)) return "INVALID"; 
        return std::to_string(price);  
    }

    typedef uint32_t Qty; 
    constexpr auto Qty_INVALID = std::numeric_limits<Qty>::max(); 
    inline auto qtyToString(Qty qty) -> std::string {
        if (UNLIKELY(qty == Qty_INVALID)) return "INVALID"; 
        return std::to_string(qty); 
    }

    // Priority type: a position in the queue 
    typedef uint64_t Priority; 
    constexpr auto Priority_INVALID = std::numeric_limits<Priority>::max(); 
    inline auto priorityToString(Priority priority) -> std::string {
        if (UNLIKELY(priority == Priority_INVALID)) return "INVALID"; 
        return std::to_string(priority); 
    }

    enum class Side : int8_t {
        INVALID = 0, 
        BUY = 1, 
        SELL = -1 
    }; 

    inline auto sideToString(Side side) -> std::string {
        switch (side) {
            case Side::BUY:
                return "BUY"; 
            case Side::SELL:
                return "SELL"; 
            case Side::INVALID: 
                return "INVALID"; 
        }
        return "UNKNOWN"; 
    }

    inline constexpr auto sideToIndex(Side side) noexcept {
        return static_cast<size_t>(side) + 1; 
    }
    
    inline constexpr auto sideToValue(Side side) noexcept {
        return static_cast<size_t>(side);
    }

    /// Type of trading algorithm.
    enum class AlgoType : int8_t {
        INVALID = 0,
        RANDOM = 1,
        MAKER = 2,
        TAKER = 3,
        MAX = 4
    };

    inline auto algoTypeToString(AlgoType type) -> std::string {
        switch (type) {
        case AlgoType::RANDOM:
            return "RANDOM";
        case AlgoType::MAKER:
            return "MAKER";
        case AlgoType::TAKER:
            return "TAKER";
        case AlgoType::INVALID:
            return "INVALID";
        case AlgoType::MAX:
            return "MAX";
        }

        return "UNKNOWN";
    }

    inline auto stringToAlgoType(const std::string &str) -> AlgoType {
        for (auto i = static_cast<int>(AlgoType::INVALID); i <= static_cast<int>(AlgoType::MAX); ++i) {
            const auto algo_type = static_cast<AlgoType>(i);
            if (algoTypeToString(algo_type) == str)
                return algo_type;
        }
        return AlgoType::INVALID;
    }
}