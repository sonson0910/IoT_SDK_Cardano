#include "cardano_iot/data/data_provenance.h"
#include "cardano_iot/utils/logger.h"

#include <chrono>
#include <sstream>
#include <random>
#include <algorithm>
#include <mutex>
#include <functional>

namespace cardano_iot
{
    namespace data
    {
        struct DataProvenance::Impl
        {
            bool initialized_ = false;
            mutable std::mutex provenance_mutex_;
            mutable std::mutex stats_mutex_;

            // Storage
            std::map<std::string, std::shared_ptr<DataAsset>> assets_;
            std::map<std::string, std::vector<ProvenanceEvent>> asset_events_;
            std::map<std::string, ProvenanceEvent> all_events_;

            // Callbacks
            ProvenanceCallback global_callback_;
            std::map<std::string, ProvenanceCallback> data_callbacks_;
            std::map<std::string, ProvenanceCallback> actor_callbacks_;

            // Statistics
            ProvenanceStats stats_ = {};

            // Generate unique IDs
            std::string generate_event_id() const {
                auto now = std::chrono::system_clock::now();
                auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
                
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1000, 9999);
                
                std::stringstream ss;
                ss << "prov_" << timestamp << "_" << dis(gen);
                return ss.str();
            }

            std::string generate_asset_id() const {
                auto now = std::chrono::system_clock::now();
                auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
                
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1000, 9999);
                
                std::stringstream ss;
                ss << "asset_" << timestamp << "_" << dis(gen);
                return ss.str();
            }

            // Compute SHA256 hash (simplified)
            std::vector<uint8_t> compute_sha256(const std::vector<uint8_t>& data) const {
                std::hash<std::string> hasher;
                std::string data_str(data.begin(), data.end());
                auto hash_value = hasher(data_str);
                
                std::vector<uint8_t> hash(32);
                for (size_t i = 0; i < 32; ++i) {
                    hash[i] = static_cast<uint8_t>((hash_value >> (8 * (i % 8))) & 0xFF);
                }
                
                return hash;
            }

            // Process provenance event
            void process_event(const ProvenanceEvent& event) {
                // Store event
                all_events_[event.event_id] = event;
                asset_events_[event.data_id].push_back(event);

                // Update statistics
                {
                    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
                    stats_.total_events++;
                    stats_.events_by_type[event.event_type]++;
                }

                // Notify callbacks
                if (global_callback_) {
                    global_callback_(event);
                }

                auto data_cb = data_callbacks_.find(event.data_id);
                if (data_cb != data_callbacks_.end() && data_cb->second) {
                    data_cb->second(event);
                }

                auto actor_cb = actor_callbacks_.find(event.actor_id);
                if (actor_cb != actor_callbacks_.end() && actor_cb->second) {
                    actor_cb->second(event);
                }
            }
        };

        DataProvenance::DataProvenance() : pimpl_(std::make_unique<Impl>()) {}
        DataProvenance::~DataProvenance() = default;

        bool DataProvenance::initialize()
        {
            std::lock_guard<std::mutex> lock(pimpl_->provenance_mutex_);
            
            if (pimpl_->initialized_) {
                return true;
            }

            pimpl_->initialized_ = true;
            
            utils::Logger::instance().log(utils::LogLevel::INFO, "DataProvenance", 
                                        "Data provenance system initialized");
            return true;
        }

        void DataProvenance::shutdown()
        {
            std::lock_guard<std::mutex> lock(pimpl_->provenance_mutex_);
            
            if (!pimpl_->initialized_) {
                return;
            }

            // Clear all data
            pimpl_->assets_.clear();
            pimpl_->asset_events_.clear();
            pimpl_->all_events_.clear();
            pimpl_->data_callbacks_.clear();
            pimpl_->actor_callbacks_.clear();
            pimpl_->global_callback_ = nullptr;
            pimpl_->initialized_ = false;

            utils::Logger::instance().log(utils::LogLevel::INFO, "DataProvenance", 
                                        "Data provenance system shutdown");
        }

        std::string DataProvenance::register_data_asset(const DataAsset& asset)
        {
            if (!pimpl_->initialized_) {
                return "";
            }

            std::lock_guard<std::mutex> lock(pimpl_->provenance_mutex_);
            
            std::string asset_id = asset.asset_id.empty() ? pimpl_->generate_asset_id() : asset.asset_id;
            
            auto shared_asset = std::make_shared<DataAsset>(asset);
            shared_asset->asset_id = asset_id;
            pimpl_->assets_[asset_id] = shared_asset;

            // Update statistics
            {
                std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                pimpl_->stats_.total_assets++;
                pimpl_->stats_.assets_by_type[asset.type]++;
            }

            // Record creation event
            record_data_creation(asset_id, asset.creator_id);

            utils::Logger::instance().log(utils::LogLevel::INFO, "DataProvenance", 
                                        "Registered data asset: " + asset_id);

            return asset_id;
        }

        std::shared_ptr<DataAsset> DataProvenance::get_data_asset(const std::string& asset_id) const
        {
            if (!pimpl_->initialized_) {
                return nullptr;
            }

            std::lock_guard<std::mutex> lock(pimpl_->provenance_mutex_);
            
            auto it = pimpl_->assets_.find(asset_id);
            if (it != pimpl_->assets_.end()) {
                return it->second;
            }

            return nullptr;
        }

        std::vector<DataAsset> DataProvenance::list_data_assets() const
        {
            if (!pimpl_->initialized_) {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->provenance_mutex_);
            
            std::vector<DataAsset> assets;
            for (const auto& [asset_id, asset_ptr] : pimpl_->assets_) {
                assets.push_back(*asset_ptr);
            }

            return assets;
        }

        std::string DataProvenance::record_event(const ProvenanceEvent& event)
        {
            if (!pimpl_->initialized_) {
                return "";
            }

            std::lock_guard<std::mutex> lock(pimpl_->provenance_mutex_);
            
            ProvenanceEvent event_copy = event;
            if (event_copy.event_id.empty()) {
                event_copy.event_id = pimpl_->generate_event_id();
            }
            
            if (event_copy.timestamp == 0) {
                event_copy.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            }

            pimpl_->process_event(event_copy);

            utils::Logger::instance().log(utils::LogLevel::INFO, "DataProvenance", 
                                        "Recorded provenance event: " + event_copy.event_id + 
                                        " for data: " + event_copy.data_id);

            return event_copy.event_id;
        }

        std::string DataProvenance::record_data_creation(const std::string& data_id, const std::string& creator_id)
        {
            ProvenanceEvent event;
            event.data_id = data_id;
            event.event_type = ProvenanceEventType::CREATED;
            event.actor_id = creator_id;
            event.properties["operation"] = "create";
            event.properties["creator"] = creator_id;

            return record_event(event);
        }

        std::string DataProvenance::record_data_access(const std::string& data_id, const std::string& accessor_id)
        {
            ProvenanceEvent event;
            event.data_id = data_id;
            event.event_type = ProvenanceEventType::ACCESSED;
            event.actor_id = accessor_id;
            event.properties["operation"] = "access";
            event.properties["accessor"] = accessor_id;

            return record_event(event);
        }

        std::vector<ProvenanceEvent> DataProvenance::get_provenance_history(const std::string& data_id) const
        {
            if (!pimpl_->initialized_) {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->provenance_mutex_);
            
            auto it = pimpl_->asset_events_.find(data_id);
            if (it != pimpl_->asset_events_.end()) {
                return it->second;
            }

            return {};
        }

        std::vector<ProvenanceEvent> DataProvenance::query_events_by_actor(const std::string& actor_id) const
        {
            if (!pimpl_->initialized_) {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->provenance_mutex_);
            
            std::vector<ProvenanceEvent> actor_events;
            for (const auto& [event_id, event] : pimpl_->all_events_) {
                if (event.actor_id == actor_id) {
                    actor_events.push_back(event);
                }
            }

            // Sort by timestamp
            std::sort(actor_events.begin(), actor_events.end(),
                     [](const ProvenanceEvent& a, const ProvenanceEvent& b) {
                         return a.timestamp < b.timestamp;
                     });

            return actor_events;
        }

        std::vector<uint8_t> DataProvenance::compute_data_hash(const std::vector<uint8_t>& data) const
        {
            return pimpl_->compute_sha256(data);
        }

        bool DataProvenance::verify_data_integrity(const std::string& asset_id, const std::vector<uint8_t>& current_data) const
        {
            auto asset = get_data_asset(asset_id);
            if (!asset) {
                return false;
            }

            auto computed_hash = compute_data_hash(current_data);
            return computed_hash == asset->data_hash;
        }

        bool DataProvenance::submit_to_blockchain(const std::string& event_id)
        {
            if (!pimpl_->initialized_) {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->provenance_mutex_);
            
            auto it = pimpl_->all_events_.find(event_id);
            if (it != pimpl_->all_events_.end()) {
                // Mock blockchain submission
                std::stringstream ss;
                ss << "tx_" << std::hex << std::random_device{}();
                it->second.transaction_hash = ss.str();

                // Update statistics
                {
                    std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                    pimpl_->stats_.blockchain_submissions++;
                }

                utils::Logger::instance().log(utils::LogLevel::INFO, "DataProvenance", 
                                            "Submitted event to blockchain: " + event_id + 
                                            " (TX: " + it->second.transaction_hash + ")");
                return true;
            }

            return false;
        }

        std::string DataProvenance::get_blockchain_transaction(const std::string& event_id) const
        {
            if (!pimpl_->initialized_) {
                return "";
            }

            std::lock_guard<std::mutex> lock(pimpl_->provenance_mutex_);
            
            auto it = pimpl_->all_events_.find(event_id);
            if (it != pimpl_->all_events_.end()) {
                return it->second.transaction_hash;
            }

            return "";
        }

        DataProvenance::ProvenanceStats DataProvenance::get_statistics() const
        {
            std::lock_guard<std::mutex> lock(pimpl_->stats_mutex_);
            return pimpl_->stats_;
        }

        void DataProvenance::reset_statistics()
        {
            std::lock_guard<std::mutex> lock(pimpl_->stats_mutex_);
            pimpl_->stats_ = {};
            
            utils::Logger::instance().log(utils::LogLevel::INFO, "DataProvenance", 
                                        "Statistics reset");
        }

    } // namespace data
} // namespace cardano_iot
