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
        enum class PeerStatus
        {
            UNKNOWN,
            CONNECTING,
            CONNECTED,
            AUTHENTICATED,
            DISCONNECTED,
            BANNED
        };

        enum class MessageType
        {
            HANDSHAKE,
            PING,
            PONG,
            DATA_SYNC,
            DEVICE_DISCOVERY,
            CAPABILITY_EXCHANGE,
            MESH_UPDATE,
            ENCRYPTED_DATA,
            BROADCAST
        };

        struct PeerInfo
        {
            std::string peer_id;
            std::string endpoint;
            std::string public_key;
            PeerStatus status;
            uint64_t last_seen;
            uint64_t bytes_sent;
            uint64_t bytes_received;
            double latency_ms;
            std::vector<std::string> capabilities;
        };

        struct NetworkMessage
        {
            std::string message_id;
            MessageType type;
            std::string sender_id;
            std::string recipient_id;
            std::vector<uint8_t> payload;
            uint64_t timestamp;
            bool encrypted;
            std::string signature;
        };

        struct MeshTopology
        {
            std::map<std::string, PeerInfo> peers;
            std::map<std::string, std::vector<std::string>> connections;
            uint32_t total_peers;
            uint32_t connected_peers;
            double network_diameter;
        };

        using MessageCallback = std::function<void(const NetworkMessage&)>;
        using PeerCallback = std::function<void(const PeerInfo&, bool connected)>;

        class P2PNetwork
        {
        public:
            P2PNetwork();
            ~P2PNetwork();

            bool initialize(const std::string& listen_address, uint16_t port);
            void shutdown();

            // Connection management
            bool start_listening();
            void stop_listening();
            bool connect_to_peer(const std::string& endpoint);
            bool disconnect_from_peer(const std::string& peer_id);
            bool is_listening() const;

            // Peer management
            std::vector<PeerInfo> get_connected_peers() const;
            PeerInfo get_peer_info(const std::string& peer_id) const;
            bool add_trusted_peer(const std::string& peer_id, const std::string& public_key);
            bool remove_peer(const std::string& peer_id);
            bool ban_peer(const std::string& peer_id, uint32_t duration_seconds = 3600);

            // Messaging
            bool send_message(const std::string& peer_id, const NetworkMessage& message);
            bool broadcast_message(const NetworkMessage& message, const std::vector<std::string>& exclude_peers = {});
            bool send_encrypted_message(const std::string& peer_id, const std::vector<uint8_t>& data);

            // Message handling
            void set_message_handler(MessageType type, MessageCallback callback);
            void set_peer_event_handler(PeerCallback callback);

            // Device discovery
            bool enable_device_discovery(bool enable = true);
            std::vector<std::string> discover_nearby_devices();
            bool announce_device_presence();

            // Mesh networking
            MeshTopology get_mesh_topology() const;
            bool enable_mesh_routing(bool enable = true);
            std::vector<std::string> find_route_to_peer(const std::string& target_peer_id) const;
            bool forward_message(const NetworkMessage& message, const std::string& next_hop);

            // Network synchronization
            bool sync_with_peers();
            bool request_peer_list_from(const std::string& peer_id);
            bool share_peer_list_with(const std::string& peer_id);

            // Security
            bool authenticate_peer(const std::string& peer_id, const std::string& challenge);
            bool verify_peer_signature(const std::string& peer_id, const std::string& signature, const std::vector<uint8_t>& data);
            std::string generate_challenge() const;

            // Quality of Service
            struct QoSMetrics
            {
                double packet_loss_rate;
                double average_latency_ms;
                double throughput_kbps;
                uint32_t active_connections;
                uint32_t message_queue_size;
            };

            QoSMetrics get_qos_metrics() const;
            bool set_bandwidth_limit(uint32_t kbps);
            bool set_message_priority(const std::string& message_id, uint8_t priority);

            // Configuration
            struct P2PConfig
            {
                std::string listen_address = "0.0.0.0";
                uint16_t listen_port = 3001;
                uint32_t max_connections = 50;
                uint32_t connection_timeout_ms = 30000;
                uint32_t heartbeat_interval_ms = 30000;
                uint32_t message_timeout_ms = 10000;
                bool enable_encryption = true;
                bool enable_discovery = true;
                bool enable_mesh_routing = true;
                uint32_t discovery_interval_ms = 60000;
                std::string network_id = "cardano_iot_testnet";
            };

            void update_config(const P2PConfig& config);
            P2PConfig get_config() const;

            // Statistics
            struct NetworkStats
            {
                uint64_t messages_sent;
                uint64_t messages_received;
                uint64_t bytes_sent;
                uint64_t bytes_received;
                uint64_t connections_established;
                uint64_t connections_lost;
                uint64_t discovery_attempts;
                uint64_t successful_authentications;
                uint64_t failed_authentications;
                double uptime_seconds;
            };

            NetworkStats get_statistics() const;
            void reset_statistics();

        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl_;
        };

    } // namespace network
} // namespace cardano_iot
