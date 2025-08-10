/**
 * @file device_registration.cpp
 * @brief Example demonstrating IoT device registration with Cardano blockchain
 *
 * This example shows how to:
 * - Initialize the Cardano IoT SDK
 * - Register IoT devices with cryptographic authentication
 * - Manage device lifecycle and status
 * - Handle device events
 *
 * @author Cardano IoT SDK Team
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <cardano_iot/cardano_iot.h>

using namespace cardano_iot;

void print_cyberpunk_banner()
{
    std::cout << R"(
 ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄   ▄▄▄▄▄▄▄▄▄▄▄  ▄▄        ▄  ▄▄▄▄▄▄▄▄▄▄▄ 
▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░▌ ▐░░░░░░░░░░░▌▐░░▌      ▐░▌▐░░░░░░░░░░░▌
▐░█▀▀▀▀▀▀▀▀▀ ▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀█░▌▐░▌░▌     ▐░▌▐░█▀▀▀▀▀▀▀█░▌
▐░▌          ▐░▌       ▐░▌▐░▌       ▐░▌▐░▌       ▐░▌▐░▌       ▐░▌▐░▌▐░▌    ▐░▌▐░▌       ▐░▌
▐░▌          ▐░█▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄█░▌▐░▌       ▐░▌▐░█▄▄▄▄▄▄▄█░▌▐░▌ ▐░▌   ▐░▌▐░▌       ▐░▌
▐░▌          ▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░▌       ▐░▌▐░░░░░░░░░░░▌▐░▌  ▐░▌  ▐░▌▐░▌       ▐░▌
▐░▌          ▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀█░█▀▀ ▐░▌       ▐░▌▐░█▀▀▀▀▀▀▀█░▌▐░▌   ▐░▌ ▐░▌▐░▌       ▐░▌
▐░▌          ▐░▌       ▐░▌▐░▌     ▐░▌  ▐░▌       ▐░▌▐░▌       ▐░▌▐░▌    ▐░▌▐░▌▐░▌       ▐░▌
▐░█▄▄▄▄▄▄▄▄▄ ▐░▌       ▐░▌▐░▌      ▐░▌ ▐░█▄▄▄▄▄▄▄█░▌▐░▌       ▐░▌▐░▌     ▐░▐░▌▐░█▄▄▄▄▄▄▄█░▌
▐░░░░░░░░░░░▌▐░▌       ▐░▌▐░▌       ▐░▌▐░░░░░░░░░░▌ ▐░▌       ▐░▌▐░▌      ▐░░▌▐░░░░░░░░░░░▌
 ▀▀▀▀▀▀▀▀▀▀▀  ▀         ▀  ▀         ▀  ▀▀▀▀▀▀▀▀▀▀   ▀         ▀  ▀        ▀▀  ▀▀▀▀▀▀▀▀▀▀▀ 
                                                                                             
    ┌─┐ ┌─┐ ┌┬┐  ┌─┐ ┌─┐ ┬┌─   ╔╦╗ ┌─┐ ┬  ┬ ┬ ┌─┐ ┌─┐  ╦═╗ ┌─┐ ┌─┐ ┬ ┌─┐ ┌┬┐ ┬─┐ ┌─┐ ┌┬┐ ┬ ┌─┐ ┌┐┌ 
    │ │ │ │  │   └─┐ │ │ ├┴┐    ║║ ├┤  └┐┌┘ │ │   ├┤   ╠╦╝ ├┤  │ │ │ └─┐  │  ├┬┘ ├─┤  │  │ │ │ │││ 
    └─┘ └─┘  ┴   └─┘ └─┘ ┴ ┴   ═╩╝ └─┘  └┘  ┴ └─┘ └─┘  ╩╚═ └─┘ └─┘ ┴ └─┘  ┴  ┴└─ ┴ ┴  ┴  ┴ └─┘ ┘└┘ 
)" << std::endl;

    std::cout << "\n🌐 CARDANO IoT SDK v1.0.0 - Device Registration Demo" << std::endl;
    std::cout << "💊 Welcome to the future of IoT on blockchain..." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
              << std::endl;
}

// Device event handler with cyberpunk styling
void on_device_event(const std::string &device_id, const std::string &event_type)
{
    std::cout << "🔮 [NEURAL NET] Device Event: " << device_id
              << " -> " << event_type << std::endl;
}

// Transaction event handler
void on_transaction_event(const std::string &tx_id, bool success)
{
    if (success)
    {
        std::cout << "✅ [BLOCKCHAIN] Transaction confirmed: " << tx_id.substr(0, 16) << "..." << std::endl;
    }
    else
    {
        std::cout << "❌ [BLOCKCHAIN] Transaction failed: " << tx_id.substr(0, 16) << "..." << std::endl;
    }
}

int main()
{
    print_cyberpunk_banner();

    try
    {
        // Initialize SDK configuration
        CardanoIoTSDK::Config config;
        config.network_type = "testnet";
        config.enable_logging = true;
        config.enable_encryption = true;
        config.max_devices = 100;

        std::cout << "🔧 [INIT] Configuring Cardano IoT SDK..." << std::endl;

        // Create SDK instance
        CardanoIoTSDK sdk(config);

        // Set event callbacks
        sdk.set_device_event_callback(on_device_event);
        sdk.set_transaction_event_callback(on_transaction_event);

        std::cout << "🚀 [INIT] Initializing connection to Cardano network..." << std::endl;

        if (!sdk.initialize())
        {
            std::cerr << "❌ [ERROR] Failed to initialize SDK" << std::endl;
            return 1;
        }

        std::cout << "✅ [INIT] SDK initialized successfully!" << std::endl;
        std::cout << "🌍 [NETWORK] Connected to: " << config.network_type << std::endl;

        // Create sample IoT devices
        std::vector<CardanoIoTSDK::DeviceInfo> devices = {
            {.device_id = "temp_sensor_001",
             .device_type = "temperature_sensor",
             .manufacturer = "CyberSense Corp",
             .firmware_version = "v2.1.0",
             .capabilities = {"sensor_data", "low_power"},
             .public_key = "ed25519_pk_1abc...def",
             .low_power_mode = true},
            {.device_id = "actuator_motor_002",
             .device_type = "servo_actuator",
             .manufacturer = "NeuroTech Industries",
             .firmware_version = "v1.8.3",
             .capabilities = {"actuator_control", "smart_contract"},
             .public_key = "ed25519_pk_2ghi...jkl",
             .low_power_mode = false},
            {.device_id = "gateway_hub_003",
             .device_type = "iot_gateway",
             .manufacturer = "DataFlow Systems",
             .firmware_version = "v3.0.1",
             .capabilities = {"p2p_communication", "data_aggregation", "edge_computing"},
             .public_key = "ed25519_pk_3mno...pqr",
             .low_power_mode = false}};

        // Register devices
        std::cout << "\n🤖 [REGISTER] Registering IoT devices on blockchain..." << std::endl;

        for (const auto &device : devices)
        {
            std::cout << "📡 [DEVICE] Registering: " << device.device_id
                      << " (" << device.device_type << ")" << std::endl;

            if (sdk.register_device(device))
            {
                std::cout << "✅ [SUCCESS] Device " << device.device_id << " registered!" << std::endl;

                // Set appropriate power mode
                sdk.set_power_mode(device.device_id, device.low_power_mode);

                if (device.low_power_mode)
                {
                    std::cout << "🔋 [POWER] Low power mode enabled for " << device.device_id << std::endl;
                }
            }
            else
            {
                std::cout << "❌ [ERROR] Failed to register device " << device.device_id << std::endl;
            }

            // Small delay for dramatic effect
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        // Display registered devices
        std::cout << "\n📊 [STATUS] Querying registered devices..." << std::endl;
        auto registered_devices = sdk.get_registered_devices();

        std::cout << "🌐 [REGISTRY] Total registered devices: " << registered_devices.size() << std::endl;
        for (const auto &device_id : registered_devices)
        {
            std::cout << "   🔸 " << device_id << std::endl;
        }

        // Simulate device authentication
        std::cout << "\n🔐 [AUTH] Demonstrating device authentication..." << std::endl;

        for (const auto &device : devices)
        {
            std::cout << "🔑 [CHALLENGE] Authenticating " << device.device_id << "..." << std::endl;

            // In a real scenario, the device would sign this challenge
            std::string challenge = "auth_challenge_" + std::to_string(time(nullptr));
            std::string signature = "mock_signature_for_demo"; // Mock signature

            bool auth_result = sdk.authenticate_device(device.device_id, challenge, signature);

            if (auth_result)
            {
                std::cout << "✅ [VERIFIED] Device " << device.device_id << " authenticated!" << std::endl;
            }
            else
            {
                std::cout << "❌ [FAILED] Authentication failed for " << device.device_id << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        // Show network status
        std::cout << "\n🌍 [NETWORK] Current network status:" << std::endl;
        auto network_status = sdk.get_network_status();
        for (const auto &[key, value] : network_status)
        {
            std::cout << "   🔹 " << key << ": " << value << std::endl;
        }

        // Simulate some IoT data submission
        std::cout << "\n📡 [DATA] Submitting sample IoT data..." << std::endl;

        CardanoIoTSDK::IoTData sample_data;
        sample_data.device_id = "temp_sensor_001";
        sample_data.data_type = "temperature";
        sample_data.payload = R"({"temperature": 23.5, "humidity": 65.2, "unit": "celsius"})";
        sample_data.timestamp = time(nullptr);
        sample_data.metadata["location"] = "Building_A_Floor_2";
        sample_data.metadata["sensor_version"] = "v2.1.0";

        std::string tx_id = sdk.submit_data(sample_data);
        if (!tx_id.empty())
        {
            std::cout << "✅ [SUBMITTED] Data submitted with TX: " << tx_id.substr(0, 16) << "..." << std::endl;
        }
        else
        {
            std::cout << "❌ [ERROR] Failed to submit data" << std::endl;
        }

        // Keep running for a bit to show events
        std::cout << "\n⏱️  [MONITOR] Monitoring for 10 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));

        std::cout << "\n🎯 [COMPLETE] Device registration demo completed!" << std::endl;
        std::cout << "🚀 [INFO] Check the Cardano blockchain explorer for your transactions" << std::endl;

        // Graceful shutdown
        std::cout << "\n🔄 [SHUTDOWN] Shutting down SDK..." << std::endl;
        sdk.shutdown();

        std::cout << "✅ [EXIT] Goodbye from the future! 🌆" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "💥 [FATAL] Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
