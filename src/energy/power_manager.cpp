/**
 * @file power_manager.cpp
 * @brief Implementation of Energy Management Module for Cardano IoT SDK
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#include "cardano_iot/energy/power_manager.h"
#include "cardano_iot/utils/logger.h"

#include <unordered_map>
#include <mutex>
#include <chrono>
#include <cmath>
#include <algorithm>

namespace cardano_iot::energy
{

    class PowerManager::Impl
    {
    public:
        std::unordered_map<std::string, std::shared_ptr<PowerProfile>> device_profiles_;
        std::unordered_map<std::string, PowerSettings> device_settings_;
        std::unordered_map<std::string, HarvestingConfig> harvesting_configs_;

        // Power history: device_id -> vector of (timestamp, power_mw)
        std::unordered_map<std::string, std::vector<std::pair<uint64_t, double>>> power_history_;

        PowerEventCallback power_event_callback_;
        BatteryEventCallback battery_event_callback_;

        mutable std::mutex data_mutex_;
        bool initialized_ = false;

        // Statistics
        std::atomic<uint64_t> total_devices_{0};
        std::atomic<uint64_t> total_power_events_{0};
        std::atomic<uint64_t> total_optimizations_{0};

        uint64_t get_current_timestamp() const
        {
            return std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                .count();
        }

        void notify_power_event(const std::string &device_id, PowerState state, double battery_level)
        {
            if (power_event_callback_)
            {
                power_event_callback_(device_id, state, battery_level);
            }
            total_power_events_++;

            utils::Logger::instance().log(utils::LogLevel::INFO, "PowerManager",
                                          "Power event: " + device_id + " -> " + power_state_to_string(state) +
                                              " (battery: " + std::to_string(static_cast<int>(battery_level * 100)) + "%)");
        }

        void notify_battery_event(const std::string &device_id, const BatteryInfo &battery_info)
        {
            if (battery_event_callback_)
            {
                battery_event_callback_(device_id, battery_info);
            }

            if (battery_info.charge_level < 0.2)
            {
                utils::Logger::instance().log(utils::LogLevel::WARNING, "PowerManager",
                                              "Low battery warning: " + device_id + " (" +
                                                  std::to_string(static_cast<int>(battery_info.charge_level * 100)) + "%)");
            }
        }

        void update_power_history(const std::string &device_id, double power_mw)
        {
            uint64_t timestamp = get_current_timestamp();

            auto &history = power_history_[device_id];
            history.emplace_back(timestamp, power_mw);

            // Keep only last 7 days of data
            uint64_t week_ago = timestamp - (7 * 24 * 3600);
            history.erase(
                std::remove_if(history.begin(), history.end(),
                               [week_ago](const auto &entry)
                               { return entry.first < week_ago; }),
                history.end());
        }

        double calculate_average_power(const std::string &device_id, uint32_t hours) const
        {
            auto it = power_history_.find(device_id);
            if (it == power_history_.end())
            {
                return 0.0;
            }

            uint64_t current_time = get_current_timestamp();
            uint64_t start_time = current_time - (hours * 3600);

            double total_power = 0.0;
            uint32_t count = 0;

            for (const auto &[timestamp, power] : it->second)
            {
                if (timestamp >= start_time)
                {
                    total_power += power;
                    count++;
                }
            }

            return count > 0 ? total_power / count : 0.0;
        }
    };

    PowerManager::PowerManager() : pimpl_(std::make_unique<Impl>()) {}

    PowerManager::~PowerManager() = default;

    bool PowerManager::initialize(const std::map<std::string, std::string> &config_params)
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        if (pimpl_->initialized_)
        {
            return true;
        }

        utils::Logger::instance().log(utils::LogLevel::INFO, "PowerManager", "Initializing power manager");

        // Process configuration parameters
        for (const auto &[key, value] : config_params)
        {
            utils::Logger::instance().log(utils::LogLevel::DEBUG, "PowerManager",
                                          "Config: " + key + " = " + value);
        }

        pimpl_->initialized_ = true;
        utils::Logger::instance().log(utils::LogLevel::INFO, "PowerManager", "Power manager initialized");

        return true;
    }

    void PowerManager::shutdown()
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        if (!pimpl_->initialized_)
        {
            return;
        }

        utils::Logger::instance().log(utils::LogLevel::INFO, "PowerManager", "Shutting down power manager");

        pimpl_->device_profiles_.clear();
        pimpl_->device_settings_.clear();
        pimpl_->harvesting_configs_.clear();
        pimpl_->power_history_.clear();

        pimpl_->initialized_ = false;

        utils::Logger::instance().log(utils::LogLevel::INFO, "PowerManager", "Power manager shut down");
    }

    bool PowerManager::register_device(const std::string &device_id, const PowerSettings &settings)
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        if (!pimpl_->initialized_)
        {
            return false;
        }

        // Check if device already exists
        if (pimpl_->device_profiles_.find(device_id) != pimpl_->device_profiles_.end())
        {
            utils::Logger::instance().log(utils::LogLevel::WARNING, "PowerManager",
                                          "Device already registered for power management: " + device_id);
            return false;
        }

        // Create power profile for device
        auto profile = std::make_shared<PowerProfile>();
        profile->device_id = device_id;
        profile->current_state = PowerState::ACTIVE;
        profile->power_source = PowerSource::BATTERY;
        profile->power_consumption_mw = 100.0; // Default 100mW
        profile->avg_power_1h = 0.0;
        profile->avg_power_24h = 0.0;
        profile->uptime_seconds = 0;
        profile->sleep_time_seconds = 0;

        // Initialize battery info
        profile->battery.voltage = 3.7;
        profile->battery.current = 0.1;
        profile->battery.capacity_mah = 2000;
        profile->battery.remaining_mah = 2000;
        profile->battery.charge_level = 1.0;
        profile->battery.temperature = 25.0;
        profile->battery.cycle_count = 0;
        profile->battery.chemistry = "Li-Ion";
        profile->battery.last_update = pimpl_->get_current_timestamp();

        pimpl_->device_profiles_[device_id] = profile;
        pimpl_->device_settings_[device_id] = settings;
        pimpl_->total_devices_++;

        utils::Logger::instance().log(utils::LogLevel::INFO, "PowerManager",
                                      "Device registered for power management: " + device_id);

        return true;
    }

    bool PowerManager::unregister_device(const std::string &device_id)
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        pimpl_->device_profiles_.erase(device_id);
        pimpl_->device_settings_.erase(device_id);
        pimpl_->harvesting_configs_.erase(device_id);
        pimpl_->power_history_.erase(device_id);

        utils::Logger::instance().log(utils::LogLevel::INFO, "PowerManager",
                                      "Device unregistered from power management: " + device_id);

        return true;
    }

    bool PowerManager::set_power_state(const std::string &device_id, PowerState state)
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        auto it = pimpl_->device_profiles_.find(device_id);
        if (it == pimpl_->device_profiles_.end())
        {
            return false;
        }

        PowerState old_state = it->second->current_state;
        it->second->current_state = state;

        // Adjust power consumption based on state
        double power_multiplier = 1.0;
        switch (state)
        {
        case PowerState::ACTIVE:
            power_multiplier = 1.0;
            break;
        case PowerState::IDLE:
            power_multiplier = 0.7;
            break;
        case PowerState::SLEEP:
            power_multiplier = 0.1;
            break;
        case PowerState::DEEP_SLEEP:
            power_multiplier = 0.05;
            break;
        case PowerState::HIBERNATION:
            power_multiplier = 0.01;
            break;
        case PowerState::CHARGING:
            power_multiplier = 1.2;
            break;
        case PowerState::CRITICAL:
            power_multiplier = 0.2;
            break;
        }

        it->second->power_consumption_mw *= power_multiplier;

        utils::Logger::instance().log(utils::LogLevel::INFO, "PowerManager",
                                      "Power state changed: " + device_id + " from " + power_state_to_string(old_state) +
                                          " to " + power_state_to_string(state));

        pimpl_->notify_power_event(device_id, state, it->second->battery.charge_level);

        return true;
    }

    PowerState PowerManager::get_power_state(const std::string &device_id) const
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        auto it = pimpl_->device_profiles_.find(device_id);
        if (it != pimpl_->device_profiles_.end())
        {
            return it->second->current_state;
        }

        return PowerState::ACTIVE; // Default state
    }

    std::shared_ptr<PowerProfile> PowerManager::get_power_profile(const std::string &device_id) const
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        auto it = pimpl_->device_profiles_.find(device_id);
        if (it != pimpl_->device_profiles_.end())
        {
            // Update averages before returning
            auto profile = it->second;
            profile->avg_power_1h = pimpl_->calculate_average_power(device_id, 1);
            profile->avg_power_24h = pimpl_->calculate_average_power(device_id, 24);
            return profile;
        }

        return nullptr;
    }

    bool PowerManager::update_battery_info(const std::string &device_id, const BatteryInfo &battery_info)
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        auto it = pimpl_->device_profiles_.find(device_id);
        if (it == pimpl_->device_profiles_.end())
        {
            return false;
        }

        it->second->battery = battery_info;

        // Check for critical battery
        if (battery_info.charge_level < 0.05)
        {
            set_power_state(device_id, PowerState::CRITICAL);
        }

        pimpl_->notify_battery_event(device_id, battery_info);

        return true;
    }

    double PowerManager::get_battery_level(const std::string &device_id) const
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        auto it = pimpl_->device_profiles_.find(device_id);
        if (it != pimpl_->device_profiles_.end())
        {
            return it->second->battery.charge_level;
        }

        return 0.0;
    }

    double PowerManager::get_estimated_runtime(const std::string &device_id) const
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        auto it = pimpl_->device_profiles_.find(device_id);
        if (it == pimpl_->device_profiles_.end())
        {
            return 0.0;
        }

        const auto &profile = it->second;
        double current_draw_ma = profile->power_consumption_mw / profile->battery.voltage;

        return calculate_runtime_hours(profile->battery.remaining_mah, current_draw_ma);
    }

    bool PowerManager::is_battery_critical(const std::string &device_id) const
    {
        double battery_level = get_battery_level(device_id);
        return battery_level < 0.05;
    }

    bool PowerManager::enable_power_optimization(const std::string &device_id, bool enable)
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        auto settings_it = pimpl_->device_settings_.find(device_id);
        if (settings_it != pimpl_->device_settings_.end())
        {
            settings_it->second.enable_optimization = enable;

            if (enable)
            {
                pimpl_->total_optimizations_++;
            }

            utils::Logger::instance().log(utils::LogLevel::INFO, "PowerManager",
                                          "Power optimization " + std::string(enable ? "enabled" : "disabled") +
                                              " for device: " + device_id);

            return true;
        }

        return false;
    }

    bool PowerManager::update_power_consumption(const std::string &device_id, double power_mw)
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        auto it = pimpl_->device_profiles_.find(device_id);
        if (it == pimpl_->device_profiles_.end())
        {
            return false;
        }

        it->second->power_consumption_mw = power_mw;
        pimpl_->update_power_history(device_id, power_mw);

        return true;
    }

    std::map<std::string, std::string> PowerManager::get_optimization_recommendations(const std::string &device_id) const
    {
        std::map<std::string, std::string> recommendations;

        auto profile = get_power_profile(device_id);
        if (!profile)
        {
            return recommendations;
        }

        // Generate recommendations based on current state
        if (profile->battery.charge_level < 0.3)
        {
            recommendations["power_state"] = "Consider switching to sleep mode to conserve battery";
        }

        if (profile->power_consumption_mw > 500)
        {
            recommendations["high_consumption"] = "Current power consumption is high, check component usage";
        }

        if (profile->avg_power_24h > profile->power_consumption_mw * 1.5)
        {
            recommendations["power_trend"] = "Power consumption has increased, consider optimization";
        }

        return recommendations;
    }

    bool PowerManager::set_component_power(const std::string &device_id,
                                           const std::string &component,
                                           double power_mw)
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        auto it = pimpl_->device_profiles_.find(device_id);
        if (it == pimpl_->device_profiles_.end())
        {
            return false;
        }

        it->second->component_power[component] = power_mw;

        // Update total consumption
        double total_power = 0.0;
        for (const auto &[comp, power] : it->second->component_power)
        {
            total_power += power;
        }

        it->second->power_consumption_mw = total_power;

        return true;
    }

    std::map<std::string, uint64_t> PowerManager::get_statistics() const
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        std::map<std::string, uint64_t> stats;
        stats["total_devices"] = pimpl_->total_devices_.load();
        stats["total_power_events"] = pimpl_->total_power_events_.load();
        stats["total_optimizations"] = pimpl_->total_optimizations_.load();
        stats["active_devices"] = pimpl_->device_profiles_.size();

        // Count devices by power state
        std::map<PowerState, uint64_t> state_counts;
        for (const auto &[device_id, profile] : pimpl_->device_profiles_)
        {
            state_counts[profile->current_state]++;
        }

        stats["active_state_devices"] = state_counts[PowerState::ACTIVE];
        stats["sleep_state_devices"] = state_counts[PowerState::SLEEP];
        stats["critical_state_devices"] = state_counts[PowerState::CRITICAL];

        return stats;
    }

    void PowerManager::set_power_event_callback(PowerEventCallback callback)
    {
        pimpl_->power_event_callback_ = std::move(callback);
    }

    void PowerManager::set_battery_event_callback(BatteryEventCallback callback)
    {
        pimpl_->battery_event_callback_ = std::move(callback);
    }

    double PowerManager::get_transaction_power_cost(const std::string &tx_type, uint32_t data_size) const
    {
        // Estimate power cost for blockchain transactions
        double base_cost = 50.0; // 50mW base cost

        if (tx_type == "data_submission")
        {
            return base_cost + (data_size * 0.1); // 0.1mW per byte
        }
        else if (tx_type == "smart_contract")
        {
            return base_cost * 2.0 + (data_size * 0.2); // Higher cost for smart contracts
        }
        else if (tx_type == "ada_transfer")
        {
            return base_cost * 0.5; // Lower cost for simple transfers
        }

        return base_cost;
    }

    bool PowerManager::optimize_for_blockchain(const std::string &device_id, bool enable)
    {
        std::lock_guard<std::mutex> lock(pimpl_->data_mutex_);

        auto settings_it = pimpl_->device_settings_.find(device_id);
        if (settings_it != pimpl_->device_settings_.end())
        {
            if (enable)
            {
                // Enable blockchain optimizations
                settings_it->second.enable_tx_power_control = true;
                settings_it->second.heartbeat_interval_normal = 120; // Longer intervals
            }

            return true;
        }

        return false;
    }

    bool PowerManager::can_perform_blockchain_op(const std::string &device_id, double estimated_power_cost) const
    {
        auto profile = get_power_profile(device_id);
        if (!profile)
        {
            return false;
        }

        // Check if device has sufficient battery for the operation
        double estimated_runtime = get_estimated_runtime(device_id);
        double operation_time_hours = estimated_power_cost / profile->power_consumption_mw;

        return estimated_runtime > operation_time_hours * 2; // 2x safety margin
    }

    // Utility functions
    std::string power_state_to_string(PowerState state)
    {
        switch (state)
        {
        case PowerState::ACTIVE:
            return "active";
        case PowerState::IDLE:
            return "idle";
        case PowerState::SLEEP:
            return "sleep";
        case PowerState::DEEP_SLEEP:
            return "deep_sleep";
        case PowerState::HIBERNATION:
            return "hibernation";
        case PowerState::CHARGING:
            return "charging";
        case PowerState::CRITICAL:
            return "critical";
        default:
            return "unknown";
        }
    }

    std::string power_source_to_string(PowerSource source)
    {
        switch (source)
        {
        case PowerSource::BATTERY:
            return "battery";
        case PowerSource::AC_POWER:
            return "ac_power";
        case PowerSource::SOLAR:
            return "solar";
        case PowerSource::WIND:
            return "wind";
        case PowerSource::THERMAL:
            return "thermal";
        case PowerSource::KINETIC:
            return "kinetic";
        case PowerSource::RF_HARVESTING:
            return "rf_harvesting";
        case PowerSource::HYBRID:
            return "hybrid";
        default:
            return "unknown";
        }
    }

    double calculate_runtime_hours(double remaining_mah, double current_draw_ma)
    {
        if (current_draw_ma <= 0)
        {
            return 0.0;
        }
        return remaining_mah / current_draw_ma;
    }

    double calculate_efficiency(double useful_work, double power_consumed)
    {
        if (power_consumed <= 0)
        {
            return 0.0;
        }
        return useful_work / power_consumed;
    }

} // namespace cardano_iot::energy
