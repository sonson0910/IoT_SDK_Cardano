#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <cstdint>

namespace cardano_iot
{
    namespace data
    {
        enum class DataType
        {
            SENSOR_READING,
            ACTUATOR_COMMAND,
            SYSTEM_LOG,
            USER_INPUT,
            COMPUTED_RESULT
        };

        enum class ProvenanceEventType
        {
            CREATED,
            MODIFIED,
            ACCESSED,
            TRANSMITTED,
            STORED,
            DELETED
        };

        struct ProvenanceEvent
        {
            std::string event_id;
            std::string data_id;
            ProvenanceEventType event_type;
            std::string actor_id;
            uint64_t timestamp;
            std::map<std::string, std::string> properties;
            std::string transaction_hash;
        };

        struct DataAsset
        {
            std::string asset_id;
            DataType type;
            std::string name;
            std::vector<uint8_t> data_hash;
            uint64_t size_bytes;
            std::map<std::string, std::string> metadata;
            uint64_t created_timestamp;
            std::string creator_id;
        };

        using ProvenanceCallback = std::function<void(const ProvenanceEvent&)>;

        class DataProvenance
        {
        public:
            DataProvenance();
            ~DataProvenance();

            bool initialize();
            void shutdown();

            // Data asset management
            std::string register_data_asset(const DataAsset& asset);
            std::shared_ptr<DataAsset> get_data_asset(const std::string& asset_id) const;
            std::vector<DataAsset> list_data_assets() const;

            // Provenance tracking
            std::string record_event(const ProvenanceEvent& event);
            std::string record_data_creation(const std::string& data_id, const std::string& creator_id);
            std::string record_data_access(const std::string& data_id, const std::string& accessor_id);

            // Provenance queries
            std::vector<ProvenanceEvent> get_provenance_history(const std::string& data_id) const;
            std::vector<ProvenanceEvent> query_events_by_actor(const std::string& actor_id) const;

            // Data integrity
            std::vector<uint8_t> compute_data_hash(const std::vector<uint8_t>& data) const;
            bool verify_data_integrity(const std::string& asset_id, const std::vector<uint8_t>& current_data) const;

            // Blockchain integration
            bool submit_to_blockchain(const std::string& event_id);
            std::string get_blockchain_transaction(const std::string& event_id) const;

            // Statistics
            struct ProvenanceStats
            {
                uint64_t total_assets;
                uint64_t total_events;
                uint64_t blockchain_submissions;
                std::map<DataType, uint64_t> assets_by_type;
                std::map<ProvenanceEventType, uint64_t> events_by_type;
            };

            ProvenanceStats get_statistics() const;
            void reset_statistics();

        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl_;
        };

    } // namespace data
} // namespace cardano_iot