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

            auto processClientRequest(const MEClientRequest* client_request) noexcept {
                auto order_book = ticker_order_book_[client_request->ticker_id_]; 
                switch (client_request->type_) {
                    case ClientRequestType::NEW: {
                        START_MEASURE(Exchange_MEOrderBook_add);
                        order_book->add(
                            client_request->client_id_, 
                            client_request->order_id_, 
                            client_request->ticker_id_, 
                            client_request->side_, 
                            client_request->price_, 
                            client_request->qty_
                        )
                        END_MEASURE(Exchange_MEOrderBook_add, logger_);
                    }
                    break; 

                    case ClientRequestType::CANCEL: {
                        START_MEASURE(Exchange_MEOrderBook_cancel);
                        order_book->cancel(
                            client_request->client_id_, 
                            client_request->order_id_, 
                            client_request->ticker_id_
                        ); 
                        END_MEASURE(Exchange_MEOrderBook_cancel, logger_);
                    }
                    break; 

                    default: {
                        FATAL("Received invalid client-request-type: " + clientRequestTypeToString(client_request->type_));
                    }
                    break; 
                }
            }

            auto sendClientResponse(const MEClientResponse* client_response) noexcept {
                logger_.log("%:% %() % Sending %\n", 
                __FILE__, __LINE__, __FUNCTION__, 
                Common::getCurrentTimeStr(&time_str_), client_response->toString());
                auto next_write =  outgoing_ogw_responses_->getNextToWriteTo(); 
                *next_write = std::move(*client_response);
                outgoing_ogw_responses_->updateWriteIndex(); 
                TTT_MEASURE(T4t_MatchingEngine_LFQueue_write, logger_);
            }

            auto sendMarketUpdate(const MEMarketUpdate* market_update) noexcept {
                logger_.log("%:% %() % Sending %\n", __FILE__, __LINE__, __FUNCTION__, 
                Common::getCurrentTimeStr(&time_str_), market_update->toString()); 
                auto next_write = outgoing_md_updates_->getNextToWriteTo(); 
                *next_write = *market_update; 
                outgoing_md_updates_->updateWriteIndex(); 
                TTT_MEASURE(T4_MatchingEngine_LFQueue_write, logger_);
            }

            auto run() noexcept {
                logger_.log("%:% %() %\n", __FILE__,__LINE__,__FUNCTION__,
                            Common::getCurrentTimeStr(&time_str_)); 
                
                while (run_) {
                    const auto me_client_request = incoming_requests_->getNextToRead(); 
                    if (LIKELY(me_client_request)) {
                        TTT_MEASURE(T3_MatchingEngine_LFQueue_read, logger_);
                        logger_.log("%:% %() % Processing %\n", 
                        __FILE__, __LINE__, __FUNCTION__, 
                        Common::getCurrentTimeStr(&time_str_), 
                        me_client_request->toString()); 
                        START_MEASURE(Exchange_MatchingEngine_processClientRequest);
                        processClientRequest(me_client_request); 
                        END_MEASURE(Exchange_MatchingEngine_processClientRequest, logger_);
                        incoming_requests_->updateReadIndex(); 
                    }
                }
            }

            // Deleted default, copy & move constructors and assignment-operators
            MatchingEngine() = delete; 
            MatchingEngine(const MatchinEngine&) = delete; 
            MatchingEngine(const MatchingEngine&&) = delete; 
            MatchingEngine &operator=(const MatchingEngine&) = delete; 
            MatchingEngine &operator=(const MatchingEngine&&) = delete; 

        private: 
            OrderBookHashMap ticker_order_book_; 
            ClientRequestLFQueue* incoming_requests_ = nullptr; 
            ClientResponseLFQueue* outgoing_ogw_responses_ = nullptr; // ogw: order gateway 
            MEMarketUpdateLFQueue* outgoing_md_updates_ = nullptr; 
            volatile bool run_ = false; 
            std::string time_str_; 
            Logger logger_; 
    }; 

}