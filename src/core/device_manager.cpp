/**
 * @file device_manager.cpp
 * @brief Implementation of Device Manager for Cardano IoT SDK
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#include "cardano_iot/core/device_manager.h"
#include "cardano_iot/utils/logger.h"
#include "cardano_iot/network/network_utils.h"
#include "cardano_iot/security/authentication.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <mutex>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

using json = nlohmann::json;

namespace cardano_iot::core
{

    // PIMPL implementation
    class DeviceManager::Impl
    {
    public:
        std::unordered_map<std::string, std::shared_ptr<Device>> devices_;
        std::unordered_map<std::string, std::string> active_challenges_;
        std::unordered_map<std::string, bool> authenticated_devices_;
        DeviceEventCallback event_callback_;
        mutable std::mutex devices_mutex_;
        mutable std::mutex challenges_mutex_;
        bool initialized_ = false;

        // Statistics
        std::atomic<uint32_t> total_registrations_{0};
        std::atomic<uint32_t> total_authentications_{0};
        std::atomic<uint32_t> failed_authentications_{0};

        void notify_event(const std::string &device_id, DeviceEvent event, const std::string &details = "")
        {
            if (event_callback_)
            {
                event_callback_(device_id, event, details);
            }

            // Log event
            utils::Logger::instance().log(utils::LogLevel::INFO,
                                          "DeviceManager",
                                          "Device event: " + device_id + " -> " + event_to_string(event) +
                                              (details.empty() ? "" : " (" + details + ")"));
        }

        std::string generate_device_address(const std::string &public_key)
        {
            // Simplified address generation using network-aware prefix
            std::hash<std::string> hasher;
            auto hash = hasher(public_key);
            auto net = network::Network::TESTNET; // default; DeviceManager doesn't track network, default to testnet
            std::stringstream ss;
            ss << network_utils::address_prefix(net, false) << std::hex << hash;
            return ss.str().substr(0, 64);
        }
    };

    DeviceManager::DeviceManager() : pimpl_(std::make_unique<Impl>()) {}

    DeviceManager::~DeviceManager() = default;

    bool DeviceManager::initialize(const std::string &config_path)
    {
        std::lock_guard<std::mutex> lock(pimpl_->devices_mutex_);

        if (pimpl_->initialized_)
        {
            return true;
        }

        utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager", "Initializing device manager");

        // Load configuration if provided
        if (!config_path.empty())
        {
            try
            {
                std::ifstream config_file(config_path);
                if (config_file.is_open())
                {
                    json config;
                    config_file >> config;

                    // Process configuration
                    utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager",
                                                  "Loaded configuration from: " + config_path);
                }
            }
            catch (const std::exception &e)
            {
                utils::Logger::instance().log(utils::LogLevel::ERROR, "DeviceManager",
                                              "Failed to load configuration: " + std::string(e.what()));
                return false;
            }
        }

        pimpl_->initialized_ = true;
        utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager", "Device manager initialized successfully");

        return true;
    }

    void DeviceManager::shutdown()
    {
        std::lock_guard<std::mutex> lock(pimpl_->devices_mutex_);

        if (!pimpl_->initialized_)
        {
            return;
        }

        utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager", "Shutting down device manager");

        // Clear all data
        pimpl_->devices_.clear();
        pimpl_->active_challenges_.clear();
        pimpl_->authenticated_devices_.clear();

        pimpl_->initialized_ = false;

        utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager", "Device manager shut down");
    }

    bool DeviceManager::register_device(const Device &device)
    {
        if (!pimpl_->initialized_)
        {
            utils::Logger::instance().log(utils::LogLevel::ERROR, "DeviceManager",
                                          "Cannot register device: manager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(pimpl_->devices_mutex_);

        // Check if device already exists
        if (pimpl_->devices_.find(device.device_id) != pimpl_->devices_.end())
        {
            utils::Logger::instance().log(utils::LogLevel::WARNING, "DeviceManager",
                                          "Device already registered: " + device.device_id);
            return false;
        }

        // Validate device information
        if (device.device_id.empty() || device.public_key.empty())
        {
            utils::Logger::instance().log(utils::LogLevel::ERROR, "DeviceManager",
                                          "Invalid device information: missing required fields");
            return false;
        }

        // Create device copy with additional computed fields
        auto new_device = std::make_shared<Device>(device);
        new_device->registration_time = std::chrono::duration_cast<std::chrono::seconds>(
                                            std::chrono::system_clock::now().time_since_epoch())
                                            .count();
        new_device->last_seen = new_device->registration_time;
        new_device->status = DeviceStatus::ONLINE;
        new_device->cardano_address = pimpl_->generate_device_address(device.public_key);

        // Store device
        pimpl_->devices_[device.device_id] = new_device;
        pimpl_->total_registrations_++;

        utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager",
                                      "Device registered successfully: " + device.device_id +
                                          " (address: " + new_device->cardano_address + ")");

        // Notify event
        pimpl_->notify_event(device.device_id, DeviceEvent::REGISTERED, "Device registered successfully");

        return true;
    }

    bool DeviceManager::unregister_device(const std::string &device_id)
    {
        std::lock_guard<std::mutex> lock(pimpl_->devices_mutex_);

        auto it = pimpl_->devices_.find(device_id);
        if (it == pimpl_->devices_.end())
        {
            utils::Logger::instance().log(utils::LogLevel::WARNING, "DeviceManager",
                                          "Cannot unregister: device not found: " + device_id);
            return false;
        }

        // Remove from authenticated devices
        pimpl_->authenticated_devices_.erase(device_id);

        // Remove active challenges
        {
            std::lock_guard<std::mutex> challenges_lock(pimpl_->challenges_mutex_);
            pimpl_->active_challenges_.erase(device_id);
        }

        // Remove device
        pimpl_->devices_.erase(it);

        utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager",
                                      "Device unregistered: " + device_id);

        return true;
    }

    bool DeviceManager::authenticate_device(const std::string &device_id,
                                            const std::string &challenge,
                                            const std::string &signature)
    {
        // Check if device exists
        auto device = get_device(device_id);
        if (!device)
        {
            utils::Logger::instance().log(utils::LogLevel::ERROR, "DeviceManager",
                                          "Authentication failed: device not found: " + device_id);
            pimpl_->failed_authentications_++;
            return false;
        }

        // Check if challenge is valid
        {
            std::lock_guard<std::mutex> lock(pimpl_->challenges_mutex_);
            auto challenge_it = pimpl_->active_challenges_.find(device_id);
            if (challenge_it == pimpl_->active_challenges_.end() ||
                challenge_it->second != challenge)
            {
                utils::Logger::instance().log(utils::LogLevel::ERROR, "DeviceManager",
                                              "Authentication failed: invalid challenge for device: " + device_id);
                pimpl_->failed_authentications_++;
                return false;
            }

            // Remove used challenge
            pimpl_->active_challenges_.erase(challenge_it);
        }

        // In a real implementation, we would verify the signature here
        // using the device's public key and proper cryptographic verification

        // For this demo, we'll do a simple mock verification
        bool signature_valid = !signature.empty() && signature.length() > 10;

        if (signature_valid)
        {
            pimpl_->authenticated_devices_[device_id] = true;
            pimpl_->total_authentications_++;

            // Update last seen time
            device->last_seen = std::chrono::duration_cast<std::chrono::seconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count();

            utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager",
                                          "Device authenticated successfully: " + device_id);

            pimpl_->notify_event(device_id, DeviceEvent::AUTHENTICATED, "Authentication successful");

            return true;
        }
        else
        {
            pimpl_->failed_authentications_++;
            utils::Logger::instance().log(utils::LogLevel::ERROR, "DeviceManager",
                                          "Authentication failed: invalid signature for device: " + device_id);

            return false;
        }
    }

    std::string DeviceManager::generate_challenge(const std::string &device_id)
    {
        auto device = get_device(device_id);
        if (!device)
        {
            utils::Logger::instance().log(utils::LogLevel::ERROR, "DeviceManager",
                                          "Cannot generate challenge: device not found: " + device_id);
            return "";
        }

        // Generate random challenge
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);

        std::stringstream challenge;
        challenge << "challenge_" << device_id << "_";
        for (int i = 0; i < 32; ++i)
        {
            challenge << std::hex << dis(gen);
        }

        std::string challenge_str = challenge.str();

        // Store challenge
        {
            std::lock_guard<std::mutex> lock(pimpl_->challenges_mutex_);
            pimpl_->active_challenges_[device_id] = challenge_str;
        }

        utils::Logger::instance().log(utils::LogLevel::DEBUG, "DeviceManager",
                                      "Generated challenge for device: " + device_id);

        return challenge_str;
    }

    bool DeviceManager::update_device_status(const std::string &device_id, DeviceStatus status)
    {
        auto device = get_device(device_id);
        if (!device)
        {
            return false;
        }

        DeviceStatus old_status = device->status;
        device->status = status;

        utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager",
                                      "Device status updated: " + device_id + " -> " + status_to_string(status));

        if (old_status != status)
        {
            pimpl_->notify_event(device_id, DeviceEvent::STATUS_CHANGED,
                                 "Status changed from " + status_to_string(old_status) + " to " + status_to_string(status));
        }

        return true;
    }

    bool DeviceManager::update_device_metadata(const std::string &device_id,
                                               const std::map<std::string, std::string> &metadata)
    {
        auto device = get_device(device_id);
        if (!device)
        {
            return false;
        }

        // Update metadata
        for (const auto &[key, value] : metadata)
        {
            device->metadata[key] = value;
        }

        utils::Logger::instance().log(utils::LogLevel::DEBUG, "DeviceManager",
                                      "Device metadata updated: " + device_id);

        return true;
    }

    std::shared_ptr<Device> DeviceManager::get_device(const std::string &device_id) const
    {
        std::lock_guard<std::mutex> lock(pimpl_->devices_mutex_);

        auto it = pimpl_->devices_.find(device_id);
        if (it != pimpl_->devices_.end())
        {
            return it->second;
        }

        return nullptr;
    }

    std::vector<std::string> DeviceManager::get_device_list() const
    {
        std::lock_guard<std::mutex> lock(pimpl_->devices_mutex_);

        std::vector<std::string> device_ids;
        device_ids.reserve(pimpl_->devices_.size());

        for (const auto &[device_id, device] : pimpl_->devices_)
        {
            device_ids.push_back(device_id);
        }

        return device_ids;
    }

    std::vector<std::string> DeviceManager::get_devices_by_status(DeviceStatus status) const
    {
        std::lock_guard<std::mutex> lock(pimpl_->devices_mutex_);

        std::vector<std::string> matching_devices;

        for (const auto &[device_id, device] : pimpl_->devices_)
        {
            if (device->status == status)
            {
                matching_devices.push_back(device_id);
            }
        }

        return matching_devices;
    }

    std::vector<std::string> DeviceManager::get_devices_by_capability(DeviceCapability capability) const
    {
        std::lock_guard<std::mutex> lock(pimpl_->devices_mutex_);

        std::vector<std::string> matching_devices;
        uint32_t capability_flag = static_cast<uint32_t>(capability);

        for (const auto &[device_id, device] : pimpl_->devices_)
        {
            if (device->capabilities & capability_flag)
            {
                matching_devices.push_back(device_id);
            }
        }

        return matching_devices;
    }

    bool DeviceManager::is_device_registered(const std::string &device_id) const
    {
        return get_device(device_id) != nullptr;
    }

    bool DeviceManager::is_device_authenticated(const std::string &device_id) const
    {
        auto it = pimpl_->authenticated_devices_.find(device_id);
        return it != pimpl_->authenticated_devices_.end() && it->second;
    }

    void DeviceManager::set_event_callback(DeviceEventCallback callback)
    {
        pimpl_->event_callback_ = std::move(callback);
    }

    void DeviceManager::update_heartbeat(const std::string &device_id)
    {
        auto device = get_device(device_id);
        if (device)
        {
            device->last_seen = std::chrono::duration_cast<std::chrono::seconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count();
        }
    }

    std::vector<std::string> DeviceManager::get_stale_devices(uint32_t timeout_seconds) const
    {
        std::lock_guard<std::mutex> lock(pimpl_->devices_mutex_);

        std::vector<std::string> stale_devices;
        uint64_t current_time = std::chrono::duration_cast<std::chrono::seconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count();

        for (const auto &[device_id, device] : pimpl_->devices_)
        {
            if (current_time - device->last_seen >= timeout_seconds)
            {
                stale_devices.push_back(device_id);
            }
        }

        return stale_devices;
    }

    bool DeviceManager::set_low_power_mode(const std::string &device_id, bool enable)
    {
        auto device = get_device(device_id);
        if (!device)
        {
            return false;
        }

        device->low_power_mode = enable;

        utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager",
                                      "Low power mode " + std::string(enable ? "enabled" : "disabled") + " for device: " + device_id);

        return true;
    }

    bool DeviceManager::update_battery_level(const std::string &device_id, double battery_level)
    {
        auto device = get_device(device_id);
        if (!device)
        {
            return false;
        }

        device->battery_level = std::clamp(battery_level, 0.0, 1.0);

        // Check for low battery
        if (battery_level < 0.2)
        {
            pimpl_->notify_event(device_id, DeviceEvent::BATTERY_LOW,
                                 "Battery level: " + std::to_string(static_cast<int>(battery_level * 100)) + "%");
        }

        return true;
    }

    std::map<std::string, uint32_t> DeviceManager::get_statistics() const
    {
        std::lock_guard<std::mutex> lock(pimpl_->devices_mutex_);

        std::map<std::string, uint32_t> stats;
        stats["total_devices"] = pimpl_->devices_.size();
        stats["total_registrations"] = pimpl_->total_registrations_.load();
        stats["total_authentications"] = pimpl_->total_authentications_.load();
        stats["failed_authentications"] = pimpl_->failed_authentications_.load();
        stats["authenticated_devices"] = pimpl_->authenticated_devices_.size();
        stats["active_challenges"] = pimpl_->active_challenges_.size();

        // Count devices by status
        std::map<DeviceStatus, uint32_t> status_counts;
        for (const auto &[device_id, device] : pimpl_->devices_)
        {
            status_counts[device->status]++;
        }

        stats["online_devices"] = status_counts[DeviceStatus::ONLINE];
        stats["offline_devices"] = status_counts[DeviceStatus::OFFLINE];
        stats["maintenance_devices"] = status_counts[DeviceStatus::MAINTENANCE];
        stats["error_devices"] = status_counts[DeviceStatus::ERROR];
        stats["low_power_devices"] = status_counts[DeviceStatus::LOW_POWER];

        return stats;
    }

    bool DeviceManager::export_device_registry(const std::string &file_path) const
    {
        // Implementation would serialize device registry to file
        // This is a simplified stub
        utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager",
                                      "Exporting device registry to: " + file_path);
        return true;
    }

    bool DeviceManager::import_device_registry(const std::string &file_path)
    {
        // Implementation would load device registry from file
        // This is a simplified stub
        utils::Logger::instance().log(utils::LogLevel::INFO, "DeviceManager",
                                      "Importing device registry from: " + file_path);
        return true;
    }

    // Utility functions
    std::string capability_to_string(DeviceCapability capability)
    {
        switch (capability)
        {
        case DeviceCapability::SENSOR_DATA:
            return "sensor_data";
        case DeviceCapability::ACTUATOR_CONTROL:
            return "actuator_control";
        case DeviceCapability::SMART_CONTRACT_EXECUTION:
            return "smart_contract_execution";
        case DeviceCapability::PEER_TO_PEER_COMMUNICATION:
            return "peer_to_peer_communication";
        case DeviceCapability::ENERGY_HARVESTING:
            return "energy_harvesting";
        case DeviceCapability::CRYPTOGRAPHIC_OPERATIONS:
            return "cryptographic_operations";
        case DeviceCapability::DATA_STORAGE:
            return "data_storage";
        case DeviceCapability::FIRMWARE_UPDATE:
            return "firmware_update";
        default:
            return "unknown";
        }
    }

    std::string status_to_string(DeviceStatus status)
    {
        switch (status)
        {
        case DeviceStatus::OFFLINE:
            return "offline";
        case DeviceStatus::ONLINE:
            return "online";
        case DeviceStatus::MAINTENANCE:
            return "maintenance";
        case DeviceStatus::ERROR:
            return "error";
        case DeviceStatus::LOW_POWER:
            return "low_power";
        case DeviceStatus::UPDATING:
            return "updating";
        default:
            return "unknown";
        }
    }

    std::string event_to_string(DeviceEvent event)
    {
        switch (event)
        {
        case DeviceEvent::REGISTERED:
            return "registered";
        case DeviceEvent::AUTHENTICATED:
            return "authenticated";
        case DeviceEvent::STATUS_CHANGED:
            return "status_changed";
        case DeviceEvent::DATA_RECEIVED:
            return "data_received";
        case DeviceEvent::ERROR_OCCURRED:
            return "error_occurred";
        case DeviceEvent::BATTERY_LOW:
            return "battery_low";
        case DeviceEvent::FIRMWARE_UPDATED:
            return "firmware_updated";
        default:
            return "unknown";
        }
    }

    std::string generate_device_id(const std::string &manufacturer,
                                   const std::string &model,
                                   const std::string &serial_number)
    {
        std::string combined = manufacturer + "_" + model + "_" + serial_number;

        // Simple hash-based ID generation
        std::hash<std::string> hasher;
        auto hash = hasher(combined);

        std::stringstream ss;
        ss << "device_" << std::hex << hash;
        return ss.str();
    }

} // namespace cardano_iot::core
