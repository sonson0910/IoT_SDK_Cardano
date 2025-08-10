/**
 * @file smart_contract_demo.cpp
 * @brief Example demonstrating smart contract integration with IoT devices
 *
 * This example shows how to:
 * - Deploy smart contracts for IoT automation
 * - Execute contract functions based on sensor data
 * - Create automated IoT workflows
 * - Monitor contract execution results
 *
 * @author Cardano IoT SDK Team
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cardano_iot/cardano_iot.h>

using namespace cardano_iot;

void print_cyberpunk_header()
{
    std::cout << R"(
    ███████╗███╗   ███╗ █████╗ ██████╗ ████████╗    ██████╗ ██████╗ ███╗   ██╗████████╗██████╗  █████╗  ██████╗████████╗
    ██╔════╝████╗ ████║██╔══██╗██╔══██╗╚══██╔══╝   ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔══██╗██╔════╝╚══██╔══╝
    ███████╗██╔████╔██║███████║██████╔╝   ██║      ██║     ██║   ██║██╔██╗ ██║   ██║   ██████╔╝███████║██║        ██║   
    ╚════██║██║╚██╔╝██║██╔══██║██╔══██╗   ██║      ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██╗██╔══██║██║        ██║   
    ███████║██║ ╚═╝ ██║██║  ██║██║  ██║   ██║      ╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║██║  ██║╚██████╗   ██║   
    ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝       ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝   ╚═╝   
                                                                                                                          
    ┌─┐ ┬ ┬ ┌┬┐ ┌─┐ ┌┬┐ ┌─┐ ┌┬┐ ┬ ┌─┐ ┌┐┌  ╦  ╔═╗ ╔╗ 
    ├─┤ │ │  │  │ │ │││ ├─┤  │  ││ │ │ │││  ║  ╠═╣ ╠╩╗
    ┴ ┴ └─┘  ┴  └─┘ ┴ ┴ ┴ ┴  ┴  ┴└─┘ ┘└┘  ╩═╝╩ ┴ ╚═╝
)" << std::endl;

    std::cout << "\n🤖 SMART CONTRACT AUTOMATION DEMO" << std::endl;
    std::cout << "💾 Deploying IoT automation contracts on Cardano..." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
              << std::endl;
}

// Smart contract event handler
void on_contract_event(const std::string &contract_address, const std::string &event_data)
{
    std::cout << "🔮 [CONTRACT] Event from " << contract_address.substr(0, 16) << "..."
              << " -> " << event_data << std::endl;
}

// Temperature monitoring smart contract (mock Plutus code)
const std::string TEMPERATURE_MONITOR_CONTRACT = R"(
{-# INLINABLE temperatureMonitor #-}
temperatureMonitor :: BuiltinData -> BuiltinData -> BuiltinData -> ()
temperatureMonitor datum redeemer context = 
  if temperature > 30 
  then traceError "Temperature too high - activating cooling"
  else if temperature < 10
  then traceError "Temperature too low - activating heating"
  else ()
  where
    temperature = getTemperature datum
)";

// HVAC control smart contract
const std::string HVAC_CONTROL_CONTRACT = R"(
{-# INLINABLE hvacControl #-}
hvacControl :: BuiltinData -> BuiltinData -> BuiltinData -> ()
hvacControl datum redeemer context = 
  case getAction redeemer of
    "COOL" -> if hasPermission context 
              then activateCooling
              else traceError "Unauthorized"
    "HEAT" -> if hasPermission context
              then activateHeating  
              else traceError "Unauthorized"
    "OFF"  -> deactivateSystem
)";

// Energy management contract
const std::string ENERGY_MGMT_CONTRACT = R"(
{-# INLINABLE energyManager #-}
energyManager :: BuiltinData -> BuiltinData -> BuiltinData -> ()
energyManager datum redeemer context = 
  if totalConsumption > energyBudget
  then optimizePowerConsumption
  else if batteryLevel < 0.2
  then activatePowerSaving
  else normalOperation
  where
    totalConsumption = sum $ map getPowerUsage connectedDevices
    batteryLevel = getBatteryLevel datum
)";

int main()
{
    print_cyberpunk_header();

    try
    {
        // Initialize SDK
        CardanoIoTSDK::Config config;
        config.network_type = "testnet";
        config.enable_logging = true;

        std::cout << "🔧 [INIT] Initializing Cardano IoT SDK..." << std::endl;
        CardanoIoTSDK sdk(config);

        if (!sdk.initialize())
        {
            std::cerr << "❌ [ERROR] Failed to initialize SDK" << std::endl;
            return 1;
        }

        std::cout << "✅ [INIT] SDK initialized successfully!" << std::endl;

        // Register IoT devices for automation
        std::vector<CardanoIoTSDK::DeviceInfo> iot_devices = {
            {.device_id = "temp_sensor_lab_01",
             .device_type = "temperature_humidity_sensor",
             .manufacturer = "SensorTech Pro",
             .firmware_version = "v3.1.2",
             .capabilities = {"sensor_data", "smart_contract", "low_power"},
             .public_key = "ed25519_pk_temp_lab_01",
             .low_power_mode = true},
            {.device_id = "hvac_controller_01",
             .device_type = "hvac_actuator",
             .manufacturer = "ClimateControl Systems",
             .firmware_version = "v2.0.5",
             .capabilities = {"actuator_control", "smart_contract", "energy_monitoring"},
             .public_key = "ed25519_pk_hvac_01",
             .low_power_mode = false},
            {.device_id = "energy_monitor_01",
             .device_type = "power_meter",
             .manufacturer = "PowerMetrics Inc",
             .firmware_version = "v1.8.0",
             .capabilities = {"sensor_data", "smart_contract", "data_aggregation"},
             .public_key = "ed25519_pk_energy_01",
             .low_power_mode = false}};

        // Register all devices
        std::cout << "\n🤖 [REGISTER] Registering IoT automation devices..." << std::endl;
        for (const auto &device : iot_devices)
        {
            if (sdk.register_device(device))
            {
                std::cout << "✅ [DEVICE] " << device.device_id
                          << " (" << device.device_type << ") registered" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        // Deploy smart contracts
        std::cout << "\n📡 [DEPLOY] Deploying automation smart contracts..." << std::endl;

        std::map<std::string, std::string> contract_params;
        contract_params["max_temperature"] = "30";
        contract_params["min_temperature"] = "10";
        contract_params["energy_budget"] = "5000"; // 5kW budget

        // Deploy temperature monitoring contract
        std::cout << "🔮 [CONTRACT] Deploying temperature monitor..." << std::endl;
        std::string temp_contract_addr = sdk.deploy_contract(TEMPERATURE_MONITOR_CONTRACT, contract_params);
        if (!temp_contract_addr.empty())
        {
            std::cout << "✅ [SUCCESS] Temperature monitor deployed: "
                      << temp_contract_addr.substr(0, 16) << "..." << std::endl;
        }

        // Deploy HVAC control contract
        std::cout << "🔮 [CONTRACT] Deploying HVAC controller..." << std::endl;
        std::string hvac_contract_addr = sdk.deploy_contract(HVAC_CONTROL_CONTRACT, contract_params);
        if (!hvac_contract_addr.empty())
        {
            std::cout << "✅ [SUCCESS] HVAC controller deployed: "
                      << hvac_contract_addr.substr(0, 16) << "..." << std::endl;
        }

        // Deploy energy management contract
        std::cout << "🔮 [CONTRACT] Deploying energy manager..." << std::endl;
        std::string energy_contract_addr = sdk.deploy_contract(ENERGY_MGMT_CONTRACT, contract_params);
        if (!energy_contract_addr.empty())
        {
            std::cout << "✅ [SUCCESS] Energy manager deployed: "
                      << energy_contract_addr.substr(0, 16) << "..." << std::endl;
        }

        // Simulate IoT automation workflow
        std::cout << "\n🌡️  [SIMULATION] Starting automation simulation..." << std::endl;

        for (int cycle = 1; cycle <= 5; ++cycle)
        {
            std::cout << "\n🔄 [CYCLE " << cycle << "] Automation cycle " << cycle << std::endl;

            // Simulate temperature reading
            double temperature = 15.0 + (cycle * 4.0) + (rand() % 100) / 100.0; // Gradually increasing
            double humidity = 50.0 + (rand() % 200) / 10.0;                     // 50-70%

            // Submit temperature data
            CardanoIoTSDK::IoTData temp_data;
            temp_data.device_id = "temp_sensor_lab_01";
            temp_data.data_type = "temperature_humidity";
            temp_data.payload = "{ \"temperature\": " + std::to_string(temperature) +
                                ", \"humidity\": " + std::to_string(humidity) +
                                ", \"unit\": \"celsius\" }";
            temp_data.timestamp = time(nullptr);
            temp_data.metadata["cycle"] = std::to_string(cycle);
            temp_data.metadata["location"] = "laboratory_01";

            std::string data_tx = sdk.submit_data(temp_data);
            std::cout << "📊 [DATA] Temperature: " << std::fixed << std::setprecision(1)
                      << temperature << "°C, Humidity: " << humidity << "%" << std::endl;

            // Execute temperature monitoring contract
            std::map<std::string, std::string> temp_params;
            temp_params["temperature"] = std::to_string(temperature);
            temp_params["humidity"] = std::to_string(humidity);
            temp_params["device_id"] = "temp_sensor_lab_01";

            std::string monitor_result = sdk.execute_contract(temp_contract_addr, "checkTemperature", temp_params);

            // Determine HVAC action based on temperature
            std::string hvac_action = "OFF";
            if (temperature > 30.0)
            {
                hvac_action = "COOL";
                std::cout << "🧊 [HVAC] Temperature high -> Activating cooling" << std::endl;
            }
            else if (temperature < 10.0)
            {
                hvac_action = "HEAT";
                std::cout << "🔥 [HVAC] Temperature low -> Activating heating" << std::endl;
            }
            else
            {
                std::cout << "✅ [HVAC] Temperature optimal -> System idle" << std::endl;
            }

            // Execute HVAC control if needed
            if (hvac_action != "OFF")
            {
                std::map<std::string, std::string> hvac_params;
                hvac_params["action"] = hvac_action;
                hvac_params["device_id"] = "hvac_controller_01";
                hvac_params["authorized_by"] = "temp_sensor_lab_01";

                std::string hvac_result = sdk.execute_contract(hvac_contract_addr, "controlHVAC", hvac_params);

                // Simulate HVAC power consumption
                double power_consumption = (hvac_action == "COOL") ? 2500.0 : 2000.0; // Watts

                CardanoIoTSDK::IoTData power_data;
                power_data.device_id = "energy_monitor_01";
                power_data.data_type = "power_consumption";
                power_data.payload = "{ \"power_watts\": " + std::to_string(power_consumption) +
                                     ", \"action\": \"" + hvac_action + "\" }";
                power_data.timestamp = time(nullptr);

                std::string power_tx = sdk.submit_data(power_data);
                std::cout << "⚡ [ENERGY] HVAC power consumption: " << power_consumption << "W" << std::endl;

                // Execute energy management contract
                std::map<std::string, std::string> energy_params;
                energy_params["power_consumption"] = std::to_string(power_consumption);
                energy_params["device_id"] = "hvac_controller_01";
                energy_params["total_budget"] = "5000";

                std::string energy_result = sdk.execute_contract(energy_contract_addr, "managePower", energy_params);
            }

            // Simulate device battery levels
            for (const auto &device : iot_devices)
            {
                if (device.low_power_mode)
                {
                    double battery_drain = 0.02 + (rand() % 30) / 1000.0; // 2-5% drain per cycle
                    double battery_level = 1.0 - (cycle * battery_drain);
                    battery_level = std::max(0.0, battery_level);

                    sdk.set_power_mode(device.device_id, battery_level < 0.3);

                    if (battery_level < 0.2)
                    {
                        std::cout << "🔋 [BATTERY] Low battery warning: " << device.device_id
                                  << " (" << static_cast<int>(battery_level * 100) << "%)" << std::endl;
                    }
                }
            }

            // Show network status
            if (cycle % 2 == 0)
            {
                auto network_status = sdk.get_network_status();
                std::cout << "🌐 [NETWORK] Tip: " << network_status["tip"]
                          << ", Connected peers: " << network_status["peers"] << std::endl;
            }

            std::cout << "⏱️  [WAIT] Waiting for next cycle..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }

        // Show automation summary
        std::cout << "\n📈 [SUMMARY] Automation cycle completed!" << std::endl;
        std::cout << "🤖 Smart contracts successfully automated IoT workflow" << std::endl;
        std::cout << "💡 Temperature monitoring -> HVAC control -> Energy management" << std::endl;
        std::cout << "🔗 All actions recorded on Cardano blockchain" << std::endl;

        // Query some submitted data
        std::cout << "\n🔍 [QUERY] Retrieving automation history..." << std::endl;
        auto temp_history = sdk.query_data("temp_sensor_lab_01", time(nullptr) - 3600, time(nullptr));
        std::cout << "📊 [HISTORY] Found " << temp_history.size() << " temperature readings" << std::endl;

        auto power_history = sdk.query_data("energy_monitor_01", time(nullptr) - 3600, time(nullptr));
        std::cout << "⚡ [HISTORY] Found " << power_history.size() << " power consumption records" << std::endl;

        std::cout << "\n🎯 [COMPLETE] Smart contract automation demo completed!" << std::endl;
        std::cout << "🚀 The future of autonomous IoT is here! 🌆" << std::endl;

        // Graceful shutdown
        std::cout << "\n🔄 [SHUTDOWN] Shutting down automation system..." << std::endl;
        sdk.shutdown();

        std::cout << "✅ [EXIT] Automation system offline. See you in the matrix! 🕶️" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "💥 [FATAL] Exception in automation: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
