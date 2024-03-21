#pragma once 
#include "utils/thread_utils.h"
#include "utils/lock_free_queue.h" 
#include "utils/macros.h"
#include "order_server/client_response.h"
#include "order_server/client_request.h"
#include "market_data/market_update.h"
#include "me_order_book.h"

namespace Exchange {

    class MatchingEngine final {
        public: 
            MatchingEngine(
                ClientRequestLFQueue* client_requests, 
                ClientRequestLFQueue* client_responses, 
                MEMartketUpdateLFQueue* market_updates
            ); 
            ~MatchingEngine(); 
            auto start() -> void; // start ME loop execution 
            auto stop() -> void; // stop ME loop execution 

            // Deleted default, copy & move constructors and assignment-operators
            MatchingEngine() = delete; 
            MatchingEngine(const MatchinEngine&) = delete; 
            MatchingEngine(const MatchingEngine&&) = delete; 
            MatchingEngine &operator=(const MatchingEngine&) = delete; 
            MatchingEngine &operator=(const MatchingEngine&&) = delete; 

        private: 
            OrderBookHashMap ticker_order_book_; 
            ClientRequestLFQueue* incoming_requests_ = nullptr; 
            ClientResponseLFQueue* outgoing_ogw_responses_ = nullptr; 
            MEMarketUpdateLFQueue* outgoing_md_updates_ = nullptr; 
            volatile bool run_ = false; 
            std::string time_str_; 
            Logger logger_; 
    }; 

}