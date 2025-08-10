/**
 * @file test_utils.cpp
 * @brief Test utilities for Cardano IoT SDK tests
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#include "test_utils.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace cardano_iot::test
{

    std::string generate_random_device_id()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);

        std::stringstream ss;
        ss << "test_device_";
        for (int i = 0; i < 8; ++i)
        {
            ss << std::hex << dis(gen);
        }

        return ss.str();
    }

    std::string generate_random_public_key()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);

        std::stringstream ss;
        ss << "ed25519_pk_";
        for (int i = 0; i < 64; ++i)
        {
            ss << std::hex << dis(gen);
        }

        return ss.str();
    }

    core::Device create_test_device(const std::string &device_id)
    {
        core::Device device;

        device.device_id = device_id.empty() ? generate_random_device_id() : device_id;
        device.device_type = "test_sensor";
        device.manufacturer = "Test Corp";
        device.model = "TEST-100";
        device.firmware_version = "v1.0.0";
        device.hardware_revision = "rev1";
        device.public_key = generate_random_public_key();
        device.capabilities = static_cast<uint32_t>(core::DeviceCapability::SENSOR_DATA);
        device.status = core::DeviceStatus::ONLINE;
        device.low_power_mode = false;
        device.battery_level = 1.0;
        device.location = "test_location";

        return device;
    }

    CardanoIoTSDK::DeviceInfo create_test_device_info(const std::string &device_id)
    {
        CardanoIoTSDK::DeviceInfo info;

        info.device_id = device_id.empty() ? generate_random_device_id() : device_id;
        info.device_type = "test_sensor";
        info.manufacturer = "Test Corp";
        info.firmware_version = "v1.0.0";
        info.capabilities = {"sensor_data", "low_power"};
        info.public_key = generate_random_public_key();
        info.low_power_mode = false;

        return info;
    }

    CardanoIoTSDK::IoTData create_test_iot_data(const std::string &device_id)
    {
        CardanoIoTSDK::IoTData data;

        data.device_id = device_id.empty() ? generate_random_device_id() : device_id;
        data.data_type = "temperature";
        data.payload = R"({"temperature": 23.5, "humidity": 65.0, "unit": "celsius"})";
        data.timestamp = std::time(nullptr);
        data.metadata["test"] = "true";
        data.metadata["version"] = "1.0";

        return data;
    }

    energy::PowerProfile create_test_power_profile(const std::string &device_id)
    {
        energy::PowerProfile profile;

        profile.device_id = device_id.empty() ? generate_random_device_id() : device_id;
        profile.current_state = energy::PowerState::ACTIVE;
        profile.power_source = energy::PowerSource::BATTERY;
        profile.power_consumption_mw = 100.0;
        profile.avg_power_1h = 95.0;
        profile.avg_power_24h = 90.0;
        profile.uptime_seconds = 3600;
        profile.sleep_time_seconds = 0;

        // Battery info
        profile.battery.voltage = 3.7;
        profile.battery.current = 0.1;
        profile.battery.capacity_mah = 2000;
        profile.battery.remaining_mah = 1800;
        profile.battery.charge_level = 0.9;
        profile.battery.temperature = 25.0;
        profile.battery.cycle_count = 10;
        profile.battery.chemistry = "Li-Ion";
        profile.battery.last_update = std::time(nullptr);

        return profile;
    }

    energy::PowerSettings create_test_power_settings()
    {
        energy::PowerSettings settings;

        settings.enable_optimization = true;
        settings.low_power_threshold = 0.2;
        settings.critical_threshold = 0.05;
        settings.sleep_timeout_minutes = 30;
        settings.deep_sleep_timeout_hours = 2;
        settings.enable_dynamic_frequency = true;
        settings.enable_tx_power_control = true;
        settings.heartbeat_interval_low_power = 300;
        settings.heartbeat_interval_normal = 60;

        return settings;
    }

    MockCardanoClient::MockCardanoClient() : connected_(false) {}

    bool MockCardanoClient::initialize()
    {
        connected_ = true;
        return true;
    }

    void MockCardanoClient::shutdown()
    {
        connected_ = false;
    }

    bool MockCardanoClient::is_connected() const
    {
        return connected_;
    }

    std::string MockCardanoClient::submit_transaction(const std::string &tx_data)
    {
        if (!connected_)
        {
            return "";
        }

        // Generate mock transaction ID
        std::stringstream ss;
        ss << "mock_tx_" << std::hex << std::hash<std::string>{}(tx_data);
        return ss.str();
    }

    uint64_t MockCardanoClient::get_balance(const std::string &address) const
    {
        if (!connected_)
        {
            return 0;
        }

        // Return mock balance based on address hash
        return std::hash<std::string>{}(address) % 1000000; // Up to 1M lovelace
    }

    bool wait_for_condition(std::function<bool()> condition,
                            uint32_t timeout_ms,
                            uint32_t check_interval_ms)
    {
        auto start_time = std::chrono::steady_clock::now();
        auto timeout_duration = std::chrono::milliseconds(timeout_ms);

        while (std::chrono::steady_clock::now() - start_time < timeout_duration)
        {
            if (condition())
            {
                return true;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(check_interval_ms));
        }

        return false;
    }

    void simulate_time_passage(uint32_t seconds)
    {
        // In a real implementation, this might advance a mock clock
        // For now, just sleep for a shorter duration to speed up tests
        std::this_thread::sleep_for(std::chrono::milliseconds(seconds * 10));
    }

} // namespace cardano_iot::test
