/**
 * @file power_manager.h
 * @brief Energy Management Module for Cardano IoT SDK
 *
 * This module provides comprehensive power management for IoT devices,
 * including battery monitoring, power optimization, and energy-efficient
 * blockchain operations.
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#ifndef CARDANO_IOT_POWER_MANAGER_H
#define CARDANO_IOT_POWER_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cstdint>

namespace cardano_iot::energy
{

    /**
     * @brief Power states for IoT devices
     */
    enum class PowerState
    {
        ACTIVE,      // Full power operation
        IDLE,        // Reduced power, ready to wake
        SLEEP,       // Minimal power, periodic wake
        DEEP_SLEEP,  // Minimal power, external wake only
        HIBERNATION, // Near zero power, manual wake
        CHARGING,    // Device is charging
        CRITICAL     // Emergency low power mode
    };

    /**
     * @brief Power source types
     */
    enum class PowerSource
    {
        BATTERY,       // Battery powered
        AC_POWER,      // AC mains power
        SOLAR,         // Solar panel
        WIND,          // Wind power
        THERMAL,       // Thermal energy harvesting
        KINETIC,       // Kinetic energy harvesting
        RF_HARVESTING, // RF energy harvesting
        HYBRID         // Multiple sources
    };

    /**
     * @brief Battery information
     */
    struct BatteryInfo
    {
        double voltage;        // Current voltage (V)
        double current;        // Current draw (A)
        double capacity_mah;   // Battery capacity (mAh)
        double remaining_mah;  // Remaining capacity (mAh)
        double charge_level;   // Charge level (0.0 - 1.0)
        double temperature;    // Battery temperature (°C)
        uint32_t cycle_count;  // Charge cycles
        std::string chemistry; // Battery chemistry (Li-Ion, etc.)
        uint64_t last_update;  // Last update timestamp
    };

    /**
     * @brief Power consumption profile
     */
    struct PowerProfile
    {
        std::string device_id;
        PowerState current_state;
        PowerSource power_source;
        double power_consumption_mw; // Current power consumption (mW)
        double avg_power_1h;         // Average power last hour (mW)
        double avg_power_24h;        // Average power last 24h (mW)
        BatteryInfo battery;
        std::map<std::string, double> component_power; // Power per component
        uint64_t uptime_seconds;                       // Total uptime
        uint64_t sleep_time_seconds;                   // Total sleep time
    };

    /**
     * @brief Power optimization settings
     */
    struct PowerSettings
    {
        bool enable_optimization = true;
        double low_power_threshold = 0.2;            // Switch to low power at 20%
        double critical_threshold = 0.05;            // Critical power at 5%
        uint32_t sleep_timeout_minutes = 30;         // Auto sleep after inactivity
        uint32_t deep_sleep_timeout_hours = 2;       // Auto deep sleep
        bool enable_dynamic_frequency = true;        // Dynamic CPU frequency
        bool enable_tx_power_control = true;         // Dynamic TX power
        uint32_t heartbeat_interval_low_power = 300; // Heartbeat in low power (sec)
        uint32_t heartbeat_interval_normal = 60;     // Normal heartbeat (sec)
    };

    /**
     * @brief Energy harvesting configuration
     */
    struct HarvestingConfig
    {
        bool enable_solar = false;
        bool enable_kinetic = false;
        bool enable_thermal = false;
        bool enable_rf = false;
        double solar_efficiency = 0.2;       // Solar panel efficiency
        double kinetic_threshold = 0.1;      // Minimum movement for kinetic
        double thermal_diff_threshold = 5.0; // Min temp diff for thermal (°C)
        double rf_power_threshold = -60;     // Min RF power (dBm)
    };

    /**
     * @brief Power Manager class
     *
     * Manages energy consumption and optimization for IoT devices,
     * providing battery monitoring, power state management, and
     * energy harvesting coordination.
     */
    class PowerManager
    {
    public:
        using PowerEventCallback = std::function<void(const std::string &, PowerState, double)>;
        using BatteryEventCallback = std::function<void(const std::string &, const BatteryInfo &)>;

        /**
         * @brief Constructor
         */
        PowerManager();

        /**
         * @brief Destructor
         */
        ~PowerManager();

        /**
         * @brief Initialize power manager
         * @param config_params Configuration parameters
         * @return true if initialization successful
         */
        bool initialize(const std::map<std::string, std::string> &config_params = {});

        /**
         * @brief Shutdown power manager
         */
        void shutdown();

        // Device Power Management
        /**
         * @brief Register device for power management
         * @param device_id Device identifier
         * @param settings Power management settings
         * @return true if registration successful
         */
        bool register_device(const std::string &device_id, const PowerSettings &settings);

        /**
         * @brief Unregister device from power management
         * @param device_id Device identifier
         * @return true if unregistration successful
         */
        bool unregister_device(const std::string &device_id);

        /**
         * @brief Set device power state
         * @param device_id Device identifier
         * @param state New power state
         * @return true if state change successful
         */
        bool set_power_state(const std::string &device_id, PowerState state);

        /**
         * @brief Get device power state
         * @param device_id Device identifier
         * @return Current power state
         */
        PowerState get_power_state(const std::string &device_id) const;

        /**
         * @brief Get device power profile
         * @param device_id Device identifier
         * @return Power profile information
         */
        std::shared_ptr<PowerProfile> get_power_profile(const std::string &device_id) const;

        // Battery Management
        /**
         * @brief Update battery information
         * @param device_id Device identifier
         * @param battery_info Current battery information
         * @return true if update successful
         */
        bool update_battery_info(const std::string &device_id, const BatteryInfo &battery_info);

        /**
         * @brief Get battery level
         * @param device_id Device identifier
         * @return Battery charge level (0.0 - 1.0)
         */
        double get_battery_level(const std::string &device_id) const;

        /**
         * @brief Get estimated battery runtime
         * @param device_id Device identifier
         * @return Estimated runtime in hours
         */
        double get_estimated_runtime(const std::string &device_id) const;

        /**
         * @brief Check if device battery is critical
         * @param device_id Device identifier
         * @return true if battery is in critical state
         */
        bool is_battery_critical(const std::string &device_id) const;

        // Power Optimization
        /**
         * @brief Enable/disable power optimization
         * @param device_id Device identifier
         * @param enable Enable optimization
         * @return true if successful
         */
        bool enable_power_optimization(const std::string &device_id, bool enable);

        /**
         * @brief Update power consumption
         * @param device_id Device identifier
         * @param power_mw Current power consumption in milliwatts
         * @return true if update successful
         */
        bool update_power_consumption(const std::string &device_id, double power_mw);

        /**
         * @brief Get power optimization recommendations
         * @param device_id Device identifier
         * @return Map of optimization recommendations
         */
        std::map<std::string, std::string> get_optimization_recommendations(const std::string &device_id) const;

        /**
         * @brief Set component power consumption
         * @param device_id Device identifier
         * @param component Component name (cpu, radio, sensors, etc.)
         * @param power_mw Power consumption in milliwatts
         * @return true if successful
         */
        bool set_component_power(const std::string &device_id,
                                 const std::string &component,
                                 double power_mw);

        // Energy Harvesting
        /**
         * @brief Configure energy harvesting
         * @param device_id Device identifier
         * @param config Harvesting configuration
         * @return true if configuration successful
         */
        bool configure_energy_harvesting(const std::string &device_id,
                                         const HarvestingConfig &config);

        /**
         * @brief Update harvested energy
         * @param device_id Device identifier
         * @param source Energy source
         * @param energy_mwh Harvested energy in milliwatt-hours
         * @return true if update successful
         */
        bool update_harvested_energy(const std::string &device_id,
                                     PowerSource source,
                                     double energy_mwh);

        /**
         * @brief Get harvesting statistics
         * @param device_id Device identifier
         * @return Harvesting statistics map
         */
        std::map<std::string, double> get_harvesting_stats(const std::string &device_id) const;

        // Scheduling and Automation
        /**
         * @brief Schedule power state change
         * @param device_id Device identifier
         * @param state Target power state
         * @param delay_seconds Delay before state change
         * @return Schedule ID for cancellation
         */
        std::string schedule_power_state(const std::string &device_id,
                                         PowerState state,
                                         uint32_t delay_seconds);

        /**
         * @brief Cancel scheduled power state change
         * @param schedule_id Schedule identifier
         * @return true if cancellation successful
         */
        bool cancel_scheduled_state(const std::string &schedule_id);

        /**
         * @brief Set automatic sleep mode
         * @param device_id Device identifier
         * @param enable Enable automatic sleep
         * @param timeout_minutes Timeout before sleep
         * @return true if successful
         */
        bool set_auto_sleep(const std::string &device_id, bool enable, uint32_t timeout_minutes = 30);

        // Monitoring and Statistics
        /**
         * @brief Get power consumption history
         * @param device_id Device identifier
         * @param hours Hours of history to retrieve
         * @return Vector of (timestamp, power_mw) pairs
         */
        std::vector<std::pair<uint64_t, double>> get_power_history(const std::string &device_id,
                                                                   uint32_t hours = 24) const;

        /**
         * @brief Get energy efficiency metrics
         * @param device_id Device identifier
         * @return Efficiency metrics map
         */
        std::map<std::string, double> get_efficiency_metrics(const std::string &device_id) const;

        /**
         * @brief Get power management statistics
         * @return Global power management statistics
         */
        std::map<std::string, uint64_t> get_statistics() const;

        // Event Handling
        /**
         * @brief Set power event callback
         * @param callback Function to call on power events
         */
        void set_power_event_callback(PowerEventCallback callback);

        /**
         * @brief Set battery event callback
         * @param callback Function to call on battery events
         */
        void set_battery_event_callback(BatteryEventCallback callback);

        // Blockchain Integration
        /**
         * @brief Get transaction power cost
         * @param tx_type Transaction type
         * @param data_size Data size in bytes
         * @return Estimated power cost in milliwatts
         */
        double get_transaction_power_cost(const std::string &tx_type, uint32_t data_size) const;

        /**
         * @brief Optimize for blockchain operations
         * @param device_id Device identifier
         * @param enable Enable blockchain optimization
         * @return true if successful
         */
        bool optimize_for_blockchain(const std::string &device_id, bool enable);

        /**
         * @brief Check if device can perform blockchain operation
         * @param device_id Device identifier
         * @param estimated_power_cost Estimated power cost in milliwatts
         * @return true if device has sufficient power
         */
        bool can_perform_blockchain_op(const std::string &device_id, double estimated_power_cost) const;

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl_;
    };

    /**
     * @brief Convert power state to string
     * @param state Power state
     * @return String representation
     */
    std::string power_state_to_string(PowerState state);

    /**
     * @brief Convert power source to string
     * @param source Power source
     * @return String representation
     */
    std::string power_source_to_string(PowerSource source);

    /**
     * @brief Calculate estimated runtime
     * @param remaining_mah Remaining battery capacity (mAh)
     * @param current_draw_ma Current draw (mA)
     * @return Estimated runtime in hours
     */
    double calculate_runtime_hours(double remaining_mah, double current_draw_ma);

    /**
     * @brief Calculate power efficiency
     * @param useful_work Work performed (arbitrary units)
     * @param power_consumed Power consumed (mW)
     * @return Efficiency ratio
     */
    double calculate_efficiency(double useful_work, double power_consumed);

} // namespace cardano_iot::energy

#endif // CARDANO_IOT_POWER_MANAGER_H
