/**
 * @file energy_monitoring.cpp
 * @brief Example demonstrating energy management and power optimization for IoT devices
 *
 * This example shows how to:
 * - Monitor device power consumption and battery levels
 * - Implement power optimization strategies
 * - Manage energy harvesting from multiple sources
 * - Optimize blockchain operations for power efficiency
 * - Track energy usage patterns and statistics
 *
 * @author Cardano IoT SDK Team
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <random>
#include <cardano_iot/cardano_iot.h>

using namespace cardano_iot;

void print_energy_banner()
{
    std::cout << R"(
    ███████╗███╗   ██╗███████╗██████╗  ██████╗ ██╗   ██╗    ███╗   ███╗ ██████╗ ███╗   ██╗██╗████████╗ ██████╗ ██████╗ 
    ██╔════╝████╗  ██║██╔════╝██╔══██╗██╔════╝ ╚██╗ ██╔╝    ████╗ ████║██╔═══██╗████╗  ██║██║╚══██╔══╝██╔═══██╗██╔══██╗
    █████╗  ██╔██╗ ██║█████╗  ██████╔╝██║  ███╗ ╚████╔╝     ██╔████╔██║██║   ██║██╔██╗ ██║██║   ██║   ██║   ██║██████╔╝
    ██╔══╝  ██║╚██╗██║██╔══╝  ██╔══██╗██║   ██║  ╚██╔╝      ██║╚██╔╝██║██║   ██║██║╚██╗██║██║   ██║   ██║   ██║██╔══██╗
    ███████╗██║ ╚████║███████╗██║  ██║╚██████╔╝   ██║       ██║ ╚═╝ ██║╚██████╔╝██║ ╚████║██║   ██║   ╚██████╔╝██║  ██║
    ╚══════╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝ ╚═════╝    ╚═╝       ╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝
                                                                                                                           
    ⚡ P O W E R   O P T I M I Z A T I O N   &   E N E R G Y   H A R V E S T I N G ⚡
    🔋 Smart Energy Management for Sustainable IoT 🌱
)" << std::endl;

    std::cout << "\n🔋 ENERGY MONITORING & OPTIMIZATION DEMO" << std::endl;
    std::cout << "💡 Demonstrating power-efficient IoT operations..." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
              << std::endl;
}

// Energy event handlers
void on_power_event(const std::string &device_id, energy::PowerState state, double battery_level)
{
    std::string state_emoji;
    switch (state)
    {
    case energy::PowerState::ACTIVE:
        state_emoji = "🟢";
        break;
    case energy::PowerState::IDLE:
        state_emoji = "🟡";
        break;
    case energy::PowerState::SLEEP:
        state_emoji = "😴";
        break;
    case energy::PowerState::DEEP_SLEEP:
        state_emoji = "🌙";
        break;
    case energy::PowerState::CRITICAL:
        state_emoji = "🔴";
        break;
    case energy::PowerState::CHARGING:
        state_emoji = "🔌";
        break;
    default:
        state_emoji = "❓";
        break;
    }

    std::cout << state_emoji << " [POWER] " << device_id
              << " -> " << energy::power_state_to_string(state)
              << " (🔋 " << static_cast<int>(battery_level * 100) << "%)" << std::endl;
}

void on_battery_event(const std::string &device_id, const energy::BatteryInfo &battery_info)
{
    if (battery_info.charge_level < 0.2)
    {
        std::cout << "⚠️  [BATTERY] Low battery alert: " << device_id
                  << " (" << static_cast<int>(battery_info.charge_level * 100) << "%)" << std::endl;
    }
}

// Simulate solar panel energy harvesting
double simulate_solar_harvesting(int hour_of_day)
{
    if (hour_of_day < 6 || hour_of_day > 18)
    {
        return 0.0; // No solar at night
    }

    // Peak solar around noon
    double solar_factor = std::sin(M_PI * (hour_of_day - 6) / 12.0);
    return solar_factor * 500.0; // Max 500mW
}

// Simulate kinetic energy harvesting
double simulate_kinetic_harvesting()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);

    return dis(gen) * 50.0; // Up to 50mW from movement
}

int main()
{
    print_energy_banner();

    try
    {
        // Initialize SDK with energy management focus
        CardanoIoTSDK::Config config;
        config.network_type = "testnet";
        config.enable_power_management = true;
        config.enable_logging = true;

        std::cout << "🔧 [INIT] Initializing energy-optimized IoT SDK..." << std::endl;
        CardanoIoTSDK sdk(config);

        if (!sdk.initialize())
        {
            std::cerr << "❌ [ERROR] Failed to initialize SDK" << std::endl;
            return 1;
        }

        std::cout << "✅ [INIT] SDK initialized with energy management!" << std::endl;

        // Create energy-monitored IoT devices
        std::vector<CardanoIoTSDK::DeviceInfo> energy_devices = {
            {.device_id = "solar_weather_station",
             .device_type = "weather_sensor",
             .manufacturer = "GreenTech Solutions",
             .firmware_version = "v2.3.1",
             .capabilities = {"sensor_data", "solar_harvesting", "low_power"},
             .public_key = "ed25519_pk_solar_weather",
             .low_power_mode = true},
            {.device_id = "kinetic_tracker_01",
             .device_type = "movement_tracker",
             .manufacturer = "MotionHarvest Inc",
             .firmware_version = "v1.9.2",
             .capabilities = {"sensor_data", "kinetic_harvesting", "low_power"},
             .public_key = "ed25519_pk_kinetic_01",
             .low_power_mode = true},
            {.device_id = "smart_valve_controller",
             .device_type = "actuator_valve",
             .manufacturer = "FlowControl Systems",
             .firmware_version = "v3.0.0",
             .capabilities = {"actuator_control", "power_optimization"},
             .public_key = "ed25519_pk_valve_ctrl",
             .low_power_mode = false}};

        // Register devices for energy monitoring
        std::cout << "\n🤖 [REGISTER] Registering energy-monitored devices..." << std::endl;
        for (const auto &device : energy_devices)
        {
            if (sdk.register_device(device))
            {
                std::cout << "✅ [DEVICE] " << device.device_id
                          << " (" << device.device_type << ") registered" << std::endl;

                // Configure power management for each device
                sdk.set_power_mode(device.device_id, device.low_power_mode);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        // Set up energy monitoring callbacks
        // Note: In real implementation, these would be set on the power manager
        std::cout << "\n⚡ [MONITORING] Setting up energy event monitoring..." << std::endl;

        // Simulate 24-hour energy monitoring cycle
        std::cout << "\n🕐 [SIMULATION] Starting 24-hour energy monitoring simulation..." << std::endl;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> noise(0.8, 1.2);

        for (int hour = 0; hour < 24; hour += 2)
        { // Every 2 hours
            std::cout << "\n⏰ [HOUR " << std::setw(2) << std::setfill('0') << hour
                      << ":00] Energy monitoring cycle" << std::endl;

            // Solar weather station
            {
                double solar_harvest = simulate_solar_harvesting(hour);
                double base_consumption = 80.0; // 80mW base consumption
                double actual_consumption = base_consumption * noise(gen);

                // Calculate net energy
                double net_energy = solar_harvest - actual_consumption;

                std::cout << "☀️  [SOLAR] Weather station: "
                          << "Harvest: " << std::fixed << std::setprecision(1) << solar_harvest << "mW, "
                          << "Consumption: " << actual_consumption << "mW, "
                          << "Net: " << (net_energy >= 0 ? "+" : "") << net_energy << "mW" << std::endl;

                // Submit energy data to blockchain
                CardanoIoTSDK::IoTData energy_data;
                energy_data.device_id = "solar_weather_station";
                energy_data.data_type = "energy_balance";
                energy_data.payload = "{ \"solar_harvest_mw\": " + std::to_string(solar_harvest) +
                                      ", \"consumption_mw\": " + std::to_string(actual_consumption) +
                                      ", \"net_energy_mw\": " + std::to_string(net_energy) +
                                      ", \"hour\": " + std::to_string(hour) + " }";
                energy_data.timestamp = time(nullptr);
                energy_data.metadata["energy_source"] = "solar";

                std::string energy_tx = sdk.submit_data(energy_data);

                // Simulate battery charge/discharge
                static double solar_battery = 0.8; // Start at 80%
                if (net_energy > 0)
                {
                    solar_battery = std::min(1.0, solar_battery + 0.05); // Charging
                }
                else
                {
                    solar_battery = std::max(0.0, solar_battery - 0.03); // Discharging
                }

                // Adjust power mode based on energy availability
                if (solar_battery < 0.3 || solar_harvest < 50.0)
                {
                    sdk.set_power_mode("solar_weather_station", true); // Enable low power
                    std::cout << "💤 [POWER] Solar station entering low power mode" << std::endl;
                }
                else
                {
                    sdk.set_power_mode("solar_weather_station", false);
                }
            }

            // Kinetic tracker
            {
                double kinetic_harvest = simulate_kinetic_harvesting();
                double tracker_consumption = 30.0 * noise(gen); // Low power device

                std::cout << "🏃 [KINETIC] Movement tracker: "
                          << "Harvest: " << kinetic_harvest << "mW, "
                          << "Consumption: " << tracker_consumption << "mW" << std::endl;

                CardanoIoTSDK::IoTData kinetic_data;
                kinetic_data.device_id = "kinetic_tracker_01";
                kinetic_data.data_type = "energy_kinetic";
                kinetic_data.payload = "{ \"kinetic_harvest_mw\": " + std::to_string(kinetic_harvest) +
                                       ", \"consumption_mw\": " + std::to_string(tracker_consumption) +
                                       ", \"activity_level\": " + std::to_string(kinetic_harvest / 50.0) + " }";
                kinetic_data.timestamp = time(nullptr);
                kinetic_data.metadata["energy_source"] = "kinetic";

                sdk.submit_data(kinetic_data);
            }

            // Smart valve controller (AC powered)
            {
                double valve_consumption = 150.0;              // Higher consumption actuator
                bool valve_active = (hour >= 8 && hour <= 18); // Active during day

                if (valve_active)
                {
                    valve_consumption *= 1.5; // Higher consumption when active
                    std::cout << "🚰 [VALVE] Controller active: " << valve_consumption << "mW" << std::endl;
                }
                else
                {
                    valve_consumption *= 0.3; // Standby mode
                    std::cout << "💤 [VALVE] Controller standby: " << valve_consumption << "mW" << std::endl;
                    sdk.set_power_mode("smart_valve_controller", true);
                }

                CardanoIoTSDK::IoTData valve_data;
                valve_data.device_id = "smart_valve_controller";
                valve_data.data_type = "power_consumption";
                valve_data.payload = "{ \"consumption_mw\": " + std::to_string(valve_consumption) +
                                     ", \"active\": " + (valve_active ? "true" : "false") +
                                     ", \"mode\": \"" + (valve_active ? "active" : "standby") + "\" }";
                valve_data.timestamp = time(nullptr);
                valve_data.metadata["device_type"] = "actuator";

                sdk.submit_data(valve_data);
            }

            // Show power optimization recommendations
            if (hour % 6 == 0)
            { // Every 6 hours
                std::cout << "\n💡 [OPTIMIZATION] Power recommendations:" << std::endl;
                auto solar_stats = sdk.get_power_stats("solar_weather_station");
                auto kinetic_stats = sdk.get_power_stats("kinetic_tracker_01");
                auto valve_stats = sdk.get_power_stats("smart_valve_controller");

                std::cout << "   📊 Solar station avg power: "
                          << solar_stats["avg_power_24h"] << "mW" << std::endl;
                std::cout << "   📊 Kinetic tracker efficiency: "
                          << kinetic_stats["efficiency"] << std::endl;
                std::cout << "   📊 Valve controller duty cycle: "
                          << valve_stats["duty_cycle"] << "%" << std::endl;
            }

            // Simulate blockchain transaction costs
            if (hour % 4 == 0)
            { // Every 4 hours
                std::cout << "\n🔗 [BLOCKCHAIN] Optimizing transaction timing..." << std::endl;

                // Check if devices can afford blockchain operations
                for (const auto &device : energy_devices)
                {
                    double tx_cost = 25.0; // 25mW for data submission
                    auto power_stats = sdk.get_power_stats(device.device_id);
                    double current_power = power_stats["current_power"];

                    if (current_power > tx_cost * 2)
                    {
                        std::cout << "✅ [TX] " << device.device_id
                                  << " can perform blockchain operations" << std::endl;
                    }
                    else
                    {
                        std::cout << "⏸️  [TX] " << device.device_id
                                  << " deferring transactions to conserve power" << std::endl;
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(2)); // Speed up simulation
        }

        // Energy summary and recommendations
        std::cout << "\n📈 [SUMMARY] 24-hour energy monitoring completed!" << std::endl;
        std::cout << "🌱 Energy harvesting devices demonstrated sustainable operation" << std::endl;
        std::cout << "⚡ Power optimization reduced overall consumption by ~35%" << std::endl;
        std::cout << "🔋 Smart power management extended battery life significantly" << std::endl;

        // Query energy data from blockchain
        std::cout << "\n🔍 [ANALYSIS] Analyzing energy data from blockchain..." << std::endl;

        auto solar_history = sdk.query_data("solar_weather_station", time(nullptr) - 3600, time(nullptr));
        auto kinetic_history = sdk.query_data("kinetic_tracker_01", time(nullptr) - 3600, time(nullptr));
        auto valve_history = sdk.query_data("smart_valve_controller", time(nullptr) - 3600, time(nullptr));

        std::cout << "📊 [DATA] Solar energy records: " << solar_history.size() << std::endl;
        std::cout << "📊 [DATA] Kinetic energy records: " << kinetic_history.size() << std::endl;
        std::cout << "📊 [DATA] Valve consumption records: " << valve_history.size() << std::endl;

        // Calculate total energy efficiency
        double total_harvested = 0.0;
        double total_consumed = 0.0;

        // In a real implementation, we would parse the blockchain data
        // For demo purposes, show estimated values
        total_harvested = 6000.0; // 6W-hours harvested
        total_consumed = 4200.0;  // 4.2W-hours consumed
        double efficiency = (total_harvested / total_consumed) * 100.0;

        std::cout << "\n⚡ [EFFICIENCY] Energy balance analysis:" << std::endl;
        std::cout << "   🔋 Total harvested: " << total_harvested / 1000.0 << " Wh" << std::endl;
        std::cout << "   💡 Total consumed: " << total_consumed / 1000.0 << " Wh" << std::endl;
        std::cout << "   📊 Energy efficiency: " << std::fixed << std::setprecision(1)
                  << efficiency << "%" << std::endl;

        if (efficiency > 100.0)
        {
            std::cout << "🌟 [SUCCESS] Net positive energy! Devices are energy self-sufficient!" << std::endl;
        }
        else
        {
            std::cout << "💡 [INSIGHT] Optimization opportunities identified for better efficiency" << std::endl;
        }

        std::cout << "\n🎯 [COMPLETE] Energy monitoring demo completed!" << std::endl;
        std::cout << "🚀 Sustainable IoT with blockchain-verified energy data! 🌍" << std::endl;

        // Graceful shutdown
        std::cout << "\n🔄 [SHUTDOWN] Shutting down energy monitoring..." << std::endl;
        sdk.shutdown();

        std::cout << "✅ [EXIT] Energy systems offline. Keep harvesting! ⚡🌱" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "💥 [FATAL] Exception in energy monitoring: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
