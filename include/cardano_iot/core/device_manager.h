/**
 * @file device_manager.h
 * @brief Device Management Module for Cardano IoT SDK
 *
 * This module handles device registration, authentication, and lifecycle management
 * within the Cardano blockchain ecosystem.
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#ifndef CARDANO_IOT_DEVICE_MANAGER_H
#define CARDANO_IOT_DEVICE_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace cardano_iot::core
{

    /**
     * @brief Device capability flags
     */
    enum class DeviceCapability
    {
        SENSOR_DATA = 1 << 0,
        ACTUATOR_CONTROL = 1 << 1,
        SMART_CONTRACT_EXECUTION = 1 << 2,
        PEER_TO_PEER_COMMUNICATION = 1 << 3,
        ENERGY_HARVESTING = 1 << 4,
        CRYPTOGRAPHIC_OPERATIONS = 1 << 5,
        DATA_STORAGE = 1 << 6,
        FIRMWARE_UPDATE = 1 << 7
    };

    /**
     * @brief Device status enumeration
     */
    enum class DeviceStatus
    {
        OFFLINE,
        ONLINE,
        MAINTENANCE,
        ERROR,
        LOW_POWER,
        UPDATING
    };

    /**
     * @brief Device information structure
     */
    struct Device
    {
        std::string device_id;
        std::string device_type;
        std::string manufacturer;
        std::string model;
        std::string firmware_version;
        std::string hardware_revision;
        std::string public_key;
        std::string private_key_hash; // Hash only, never store actual private key
        uint32_t capabilities;        // Bitfield of DeviceCapability flags
        DeviceStatus status;
        uint64_t registration_time;
        uint64_t last_seen;
        std::string cardano_address;
        std::map<std::string, std::string> metadata;
        bool low_power_mode;
        double battery_level; // 0.0 - 1.0
        std::string location;
    };

    /**
     * @brief Device event types
     */
    enum class DeviceEvent
    {
        REGISTERED,
        AUTHENTICATED,
        STATUS_CHANGED,
        DATA_RECEIVED,
        ERROR_OCCURRED,
        BATTERY_LOW,
        FIRMWARE_UPDATED
    };

    /**
     * @brief Device Manager class
     *
     * Manages IoT device registration, authentication, and lifecycle within
     * the Cardano blockchain ecosystem.
     */
    class DeviceManager
    {
    public:
        using DeviceEventCallback = std::function<void(const std::string &, DeviceEvent, const std::string &)>;

        /**
         * @brief Constructor
         */
        DeviceManager();

        /**
         * @brief Destructor
         */
        ~DeviceManager();

        /**
         * @brief Initialize the device manager
         * @param config_path Path to configuration file
         * @return true if initialization successful
         */
        bool initialize(const std::string &config_path = "");

        /**
         * @brief Shutdown the device manager
         */
        void shutdown();

        /**
         * @brief Register a new IoT device
         * @param device Device information
         * @return true if registration successful
         */
        bool register_device(const Device &device);

        /**
         * @brief Unregister a device
         * @param device_id Device identifier
         * @return true if unregistration successful
         */
        bool unregister_device(const std::string &device_id);

        /**
         * @brief Authenticate a device using cryptographic challenge
         * @param device_id Device identifier
         * @param challenge Random challenge string
         * @param signature Device signature of challenge
         * @return true if authentication successful
         */
        bool authenticate_device(const std::string &device_id,
                                 const std::string &challenge,
                                 const std::string &signature);

        /**
         * @brief Generate authentication challenge for device
         * @param device_id Device identifier
         * @return Challenge string, empty if device not found
         */
        std::string generate_challenge(const std::string &device_id);

        /**
         * @brief Update device status
         * @param device_id Device identifier
         * @param status New status
         * @return true if update successful
         */
        bool update_device_status(const std::string &device_id, DeviceStatus status);

        /**
         * @brief Update device metadata
         * @param device_id Device identifier
         * @param metadata New metadata
         * @return true if update successful
         */
        bool update_device_metadata(const std::string &device_id,
                                    const std::map<std::string, std::string> &metadata);

        /**
         * @brief Get device information
         * @param device_id Device identifier
         * @return Device information, nullptr if not found
         */
        std::shared_ptr<Device> get_device(const std::string &device_id) const;

        /**
         * @brief Get all registered devices
         * @return Vector of device IDs
         */
        std::vector<std::string> get_device_list() const;

        /**
         * @brief Get devices by status
         * @param status Device status to filter by
         * @return Vector of device IDs
         */
        std::vector<std::string> get_devices_by_status(DeviceStatus status) const;

        /**
         * @brief Get devices by capability
         * @param capability Capability to filter by
         * @return Vector of device IDs
         */
        std::vector<std::string> get_devices_by_capability(DeviceCapability capability) const;

        /**
         * @brief Check if device is registered
         * @param device_id Device identifier
         * @return true if device is registered
         */
        bool is_device_registered(const std::string &device_id) const;

        /**
         * @brief Check if device is authenticated
         * @param device_id Device identifier
         * @return true if device is authenticated
         */
        bool is_device_authenticated(const std::string &device_id) const;

        /**
         * @brief Set device event callback
         * @param callback Callback function for device events
         */
        void set_event_callback(DeviceEventCallback callback);

        /**
         * @brief Update device heartbeat (last seen time)
         * @param device_id Device identifier
         */
        void update_heartbeat(const std::string &device_id);

        /**
         * @brief Get devices that haven't been seen for specified time
         * @param timeout_seconds Timeout in seconds
         * @return Vector of device IDs
         */
        std::vector<std::string> get_stale_devices(uint32_t timeout_seconds) const;

        /**
         * @brief Enable/disable low power mode for device
         * @param device_id Device identifier
         * @param enable Enable low power mode
         * @return true if successful
         */
        bool set_low_power_mode(const std::string &device_id, bool enable);

        /**
         * @brief Update device battery level
         * @param device_id Device identifier
         * @param battery_level Battery level (0.0 - 1.0)
         * @return true if successful
         */
        bool update_battery_level(const std::string &device_id, double battery_level);

        /**
         * @brief Get device statistics
         * @return Statistics map
         */
        std::map<std::string, uint32_t> get_statistics() const;

        /**
         * @brief Export device registry to file
         * @param file_path Output file path
         * @return true if export successful
         */
        bool export_device_registry(const std::string &file_path) const;

        /**
         * @brief Import device registry from file
         * @param file_path Input file path
         * @return true if import successful
         */
        bool import_device_registry(const std::string &file_path);

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl_;
    };

    /**
     * @brief Convert device capability to string
     * @param capability Device capability
     * @return String representation
     */
    std::string capability_to_string(DeviceCapability capability);

    /**
     * @brief Convert device status to string
     * @param status Device status
     * @return String representation
     */
    std::string status_to_string(DeviceStatus status);

    /**
     * @brief Convert device event to string
     * @param event Device event
     * @return String representation
     */
    std::string event_to_string(DeviceEvent event);

    /**
     * @brief Generate unique device ID
     * @param manufacturer Manufacturer name
     * @param model Device model
     * @param serial_number Device serial number
     * @return Unique device ID
     */
    std::string generate_device_id(const std::string &manufacturer,
                                   const std::string &model,
                                   const std::string &serial_number);

} // namespace cardano_iot::core

#endif // CARDANO_IOT_DEVICE_MANAGER_H
