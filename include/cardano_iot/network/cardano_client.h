#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <cstdint>

namespace cardano_iot
{
    namespace network
    {
        enum class ConnectionStatus
        {
            DISCONNECTED,
            CONNECTING,
            CONNECTED,
            SYNCHRONIZING,
            SYNCED,
            ERROR
        };

        enum class Network
        {
            MAINNET,
            TESTNET,
            PREPROD,
            PREVIEW,
            LOCAL
        };

        struct ChainTip
        {
            std::string hash;
            uint64_t slot;
            uint64_t height;
            uint64_t epoch;
            double sync_progress;
        };

        struct NodeInfo
        {
            std::string version;
            std::string network;
            uint64_t slot;
            uint64_t epoch;
            double sync_progress;
            std::vector<std::string> peers;
            ConnectionStatus status;
        };

        struct UTXOInfo
        {
            std::string tx_hash;
            uint32_t output_index;
            std::string address;
            uint64_t amount;
            std::map<std::string, uint64_t> assets;
            std::string datum_hash;
        };

        struct SubmissionResult
        {
            bool success;
            std::string tx_hash;
            std::string error_message;
            uint64_t submission_timestamp;
        };

        class CardanoClient
        {
        public:
            CardanoClient();
            ~CardanoClient();

            bool initialize(const std::string& node_socket_path = "", Network network = Network::TESTNET);
            void shutdown();

            // Connection management
            bool connect();
            void disconnect();
            ConnectionStatus get_connection_status() const;
            bool is_connected() const;

            // Node information
            NodeInfo get_node_info() const;
            ChainTip get_chain_tip() const;

            // UTXO queries
            std::vector<UTXOInfo> query_utxos(const std::string& address) const;
            uint64_t get_address_balance(const std::string& address) const;

            // Transaction operations
            SubmissionResult submit_transaction(const std::string& cbor_hex) const;
            bool is_transaction_confirmed(const std::string& tx_hash) const;

            // Utility functions
            bool validate_address(const std::string& address) const;
            uint64_t slot_to_timestamp(uint64_t slot) const;

            struct ClientStats
            {
                uint64_t total_queries;
                uint64_t successful_queries;
                uint64_t failed_queries;
                uint64_t transactions_submitted;
                uint64_t connection_attempts;
                double avg_query_time_ms;
            };

            ClientStats get_statistics() const;
            void reset_statistics();

        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl_;
        };

    } // namespace network
} // namespace cardano_iot