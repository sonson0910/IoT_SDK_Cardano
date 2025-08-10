/**
 * @file cardano_iot.h
 * @brief Main header file for Cardano IoT SDK
 *
 * This SDK provides comprehensive IoT integration capabilities for the Cardano blockchain ecosystem.
 * It enables secure device management, data provenance, smart contract integration, and
 * energy-efficient operations for IoT devices.
 *
 * Key Features:
 * - Device Identity & Authentication Management
 * - Cryptographic Security & Data Encryption
 * - Smart Contract Integration & Automation
 * - Microtransaction Support
 * - Energy-Efficient Operations
 * - Data Provenance & Integrity
 * - Scalable P2P Networking
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 * @date 2024
 */

#ifndef CARDANO_IOT_H
#define CARDANO_IOT_H

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <map>
#include <cstdint>

// Core modules
#include "core/device_manager.h"
#include "core/crypto_manager.h"
#include "core/transaction_manager.h"
#include "core/smart_contract_interface.h"

// Utility modules
#include "utils/logger.h"
#include "utils/config.h"

// Energy management
#include "energy/power_manager.h"

// Network modules
#include "network/cardano_client.h"
#include "network/p2p_network.h"

// Security modules
#include "security/authentication.h"
#include "security/encryption.h"

// Data modules
#include "data/data_provenance.h"

namespace cardano_iot
{

    /**
     * @brief Main SDK initialization and management class
     *
     * This class serves as the entry point for the Cardano IoT SDK, providing
     * a unified interface to all SDK functionalities.
     */
    class CardanoIoTSDK
    {
    public:
        /**
         * @brief SDK initialization configuration
         */
        struct Config
        {
            std::string network_type = "mainnet"; // mainnet, testnet, preprod
            std::string node_socket_path;
            std::string node_config_path;
            bool enable_logging = true;
            bool enable_encryption = true;
            bool enable_power_management = true;
            uint32_t max_devices = 1000;
            uint32_t transaction_timeout_ms = 30000;
        };

        /**
         * @brief Device registration information
         */
        struct DeviceInfo
        {
            std::string device_id;
            std::string device_type;
            std::string manufacturer;
            std::string firmware_version;
            std::vector<std::string> capabilities;
            std::string public_key;
            bool low_power_mode = false;
        };

        /**
         * @brief IoT data structure for blockchain storage
         */
        struct IoTData
        {
            std::string device_id;
            std::string data_type;
            std::string payload;
            uint64_t timestamp;
            std::string signature;
            std::string hash;
            std::map<std::string, std::string> metadata;
        };

        /**
         * @brief Constructor
         * @param config SDK configuration
         */
        explicit CardanoIoTSDK(const Config &config);

        /**
         * @brief Destructor
         */
        ~CardanoIoTSDK();

        /**
         * @brief Initialize the SDK
         * @return true if initialization successful, false otherwise
         */
        bool initialize();

        /**
         * @brief Shutdown the SDK
         */
        void shutdown();

        /**
         * @brief Check if SDK is initialized and ready
         * @return true if ready, false otherwise
         */
        bool is_ready() const;

        // Device Management
        /**
         * @brief Register a new IoT device
         * @param device_info Device information
         * @return true if registration successful
         */
        bool register_device(const DeviceInfo &device_info);

        /**
         * @brief Authenticate a device
         * @param device_id Device identifier
         * @param challenge Authentication challenge
         * @param signature Device signature
         * @return true if authentication successful
         */
        bool authenticate_device(const std::string &device_id,
                                 const std::string &challenge,
                                 const std::string &signature);

        /**
         * @brief Get list of registered devices
         * @return Vector of device IDs
         */
        std::vector<std::string> get_registered_devices() const;

        // Data Management
        /**
         * @brief Submit IoT data to the blockchain
         * @param data IoT data to submit
         * @return Transaction ID if successful, empty string otherwise
         */
        std::string submit_data(const IoTData &data);

        /**
         * @brief Query IoT data from the blockchain
         * @param device_id Device identifier
         * @param start_time Start timestamp
         * @param end_time End timestamp
         * @return Vector of IoT data entries
         */
        std::vector<IoTData> query_data(const std::string &device_id,
                                        uint64_t start_time = 0,
                                        uint64_t end_time = 0) const;

        /**
         * @brief Verify data integrity
         * @param data IoT data to verify
         * @return true if data is valid and hasn't been tampered with
         */
        bool verify_data_integrity(const IoTData &data) const;

        // Smart Contract Integration
        /**
         * @brief Deploy a smart contract for IoT automation
         * @param contract_code Contract source code
         * @param parameters Initial parameters
         * @return Contract address if successful
         */
        std::string deploy_contract(const std::string &contract_code,
                                    const std::map<std::string, std::string> &parameters);

        /**
         * @brief Execute a smart contract function
         * @param contract_address Contract address
         * @param function_name Function to call
         * @param parameters Function parameters
         * @return Execution result
         */
        std::string execute_contract(const std::string &contract_address,
                                     const std::string &function_name,
                                     const std::map<std::string, std::string> &parameters);

        // Transaction Management
        /**
         * @brief Send ADA to a device for operational costs
         * @param device_id Target device
         * @param amount Amount in lovelace
         * @return Transaction ID if successful
         */
        std::string send_ada(const std::string &device_id, uint64_t amount);

        /**
         * @brief Get device ADA balance
         * @param device_id Device identifier
         * @return Balance in lovelace
         */
        uint64_t get_device_balance(const std::string &device_id) const;

        // Energy Management
        /**
         * @brief Set device power mode
         * @param device_id Device identifier
         * @param low_power Enable low power mode
         */
        void set_power_mode(const std::string &device_id, bool low_power);

        /**
         * @brief Get device power statistics
         * @param device_id Device identifier
         * @return Power consumption data
         */
        std::map<std::string, double> get_power_stats(const std::string &device_id) const;

        // Event Handling
        /**
         * @brief Event callback types
         */
        using DeviceEventCallback = std::function<void(const std::string &, const std::string &)>;
        using DataEventCallback = std::function<void(const IoTData &)>;
        using TransactionEventCallback = std::function<void(const std::string &, bool)>;

        /**
         * @brief Set device event callback
         * @param callback Function to call on device events
         */
        void set_device_event_callback(DeviceEventCallback callback);

        /**
         * @brief Set data event callback
         * @param callback Function to call on new data
         */
        void set_data_event_callback(DataEventCallback callback);

        /**
         * @brief Set transaction event callback
         * @param callback Function to call on transaction events
         */
        void set_transaction_event_callback(TransactionEventCallback callback);

        // Utility Functions
        /**
         * @brief Get SDK version
         * @return Version string
         */
        static std::string get_version();

        /**
         * @brief Get network status
         * @return Network information
         */
        std::map<std::string, std::string> get_network_status() const;

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl_;
    };

    /**
     * @brief Global SDK instance accessor (singleton pattern)
     * @return Reference to the global SDK instance
     */
    CardanoIoTSDK &get_sdk_instance();

    /**
     * @brief Initialize global SDK instance
     * @param config SDK configuration
     * @return true if initialization successful
     */
    bool initialize_sdk(const CardanoIoTSDK::Config &config);

    /**
     * @brief Shutdown global SDK instance
     */
    void shutdown_sdk();

} // namespace cardano_iot

#endif // CARDANO_IOT_H
