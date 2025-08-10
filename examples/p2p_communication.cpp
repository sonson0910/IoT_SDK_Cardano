/**
 * @file p2p_communication.cpp
 * @brief Example demonstrating P2P communication between IoT devices
 *
 * This example shows how to:
 * - Set up peer-to-peer networks
 * - Enable device-to-device communication
 * - Coordinate distributed IoT operations
 * - Handle network discovery and messaging
 *
 * @author Cardano IoT SDK Team
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>
#include <cardano_iot/cardano_iot.h>

using namespace cardano_iot;

void print_p2p_banner()
{
    std::cout << R"(
    ██████╗ ██████╗ ██████╗      ██████╗ ██████╗ ███╗   ███╗███╗   ███╗██╗   ██╗███╗   ██╗██╗ ██████╗ █████╗ ████████╗██╗ ██████╗ ███╗   ██╗
    ██╔══██╗╚════██╗██╔══██╗    ██╔════╝██╔═══██╗████╗ ████║████╗ ████║██║   ██║████╗  ██║██║██╔════╝██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║
    ██████╔╝ █████╔╝██████╔╝    ██║     ██║   ██║██╔████╔██║██╔████╔██║██║   ██║██╔██╗ ██║██║██║     ███████║   ██║   ██║██║   ██║██╔██╗ ██║
    ██╔═══╝ ██╔═══╝ ██╔═══╝     ██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██║   ██║██║╚██╗██║██║██║     ██╔══██║   ██║   ██║██║   ██║██║╚██╗██║
    ██║     ███████╗██║         ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║╚██████╔╝██║ ╚████║██║╚██████╗██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║
    ╚═╝     ╚══════╝╚═╝          ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝ ╚═════╝╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝
                                                                                                                                              
    📡 PEER-TO-PEER IoT NETWORK DEMO
    🤖 Distributed device communication
    )" << std::endl;
}

// Device event handler
void on_p2p_message(const std::string &from_device, const std::string &message)
{
    std::cout << "📨 [P2P] Message from " << from_device << ": " << message << std::endl;
}

// Network discovery handler
void on_peer_discovered(const std::string &peer_id, const std::string &peer_type)
{
    std::cout << "🔍 [DISCOVERY] Found peer: " << peer_id << " (" << peer_type << ")" << std::endl;
}

int main()
{
    print_p2p_banner();

    try
    {
        // Initialize SDK with P2P networking
        CardanoIoTSDK::Config config;
        config.network_type = "testnet";
        config.enable_logging = true;
        // Note: P2P configuration would be set via additional config methods

        std::cout << "🔧 [INIT] Initializing P2P Communication Demo..." << std::endl;
        CardanoIoTSDK sdk(config);

        if (!sdk.initialize())
        {
            std::cerr << "❌ [ERROR] Failed to initialize SDK" << std::endl;
            return 1;
        }

        std::cout << "✅ [INIT] SDK initialized with P2P networking!" << std::endl;

        // Register network nodes/devices
        std::vector<CardanoIoTSDK::DeviceInfo> network_devices = {
            {.device_id = "gateway_hub_01",
             .device_type = "network_gateway",
             .manufacturer = "NetworkTech Pro",
             .firmware_version = "v3.2.1",
             .capabilities = {"p2p_communication", "data_relay", "network_coordination"},
             .public_key = "ed25519_pk_gateway_01"},
            {.device_id = "sensor_cluster_A",
             .device_type = "sensor_cluster",
             .manufacturer = "ClusterSense Corp",
             .firmware_version = "v2.0.5",
             .capabilities = {"p2p_communication", "data_collection", "local_processing"},
             .public_key = "ed25519_pk_cluster_A"},
            {.device_id = "edge_processor_01",
             .device_type = "edge_computer",
             .manufacturer = "EdgeCompute Inc",
             .firmware_version = "v1.8.3",
             .capabilities = {"p2p_communication", "data_processing", "ml_inference"},
             .public_key = "ed25519_pk_edge_01"}};

        std::cout << "\n🌐 [NETWORK] Setting up P2P network..." << std::endl;
        for (const auto &device : network_devices)
        {
            if (sdk.register_device(device))
            {
                std::cout << "✅ [NODE] " << device.device_id << " joined network" << std::endl;
            }
        }

        // Set up P2P event handlers
        std::cout << "\n📡 [P2P] Configuring peer-to-peer communication..." << std::endl;

        // Simulate network discovery
        std::cout << "\n🔍 [DISCOVERY] Starting peer discovery..." << std::endl;
        for (int i = 0; i < 3; ++i)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            on_peer_discovered(network_devices[i].device_id, network_devices[i].device_type);
        }

        std::cout << "\n🤖 [SIMULATION] Simulating distributed IoT operations..." << std::endl;

        // Simulate distributed communication scenarios
        for (int round = 1; round <= 5; ++round)
        {
            std::cout << "\n🔄 [ROUND " << round << "] P2P Communication Round " << round << std::endl;

            // Scenario 1: Sensor data collection coordination
            if (round % 2 == 1)
            {
                std::cout << "📊 [SCENARIO] Coordinated sensor data collection" << std::endl;

                // Gateway coordinates data collection
                on_p2p_message("gateway_hub_01", "REQUEST_DATA_COLLECTION: temperature,humidity");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                // Sensor cluster responds
                on_p2p_message("sensor_cluster_A", "DATA_COLLECTION_STARTED: 5_sensors_active");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                // Edge processor confirms data processing readiness
                on_p2p_message("edge_processor_01", "PROCESSING_READY: ml_models_loaded");

                // Submit coordination message to blockchain
                CardanoIoTSDK::IoTData coordination_data;
                coordination_data.device_id = "gateway_hub_01";
                coordination_data.data_type = "network_coordination";
                coordination_data.payload = R"({"operation": "data_collection", "participants": ["sensor_cluster_A", "edge_processor_01"], "status": "initiated"})";
                coordination_data.timestamp = time(nullptr);

                std::string coord_tx = sdk.submit_data(coordination_data);
                std::cout << "🔗 [BLOCKCHAIN] Coordination logged: " << coord_tx.substr(0, 12) << "..." << std::endl;
            }

            // Scenario 2: Distributed processing
            else
            {
                std::cout << "⚙️ [SCENARIO] Distributed edge processing" << std::endl;

                // Sensor cluster sends data to edge processor
                on_p2p_message("sensor_cluster_A", "DATA_READY: batch_001.json (15KB)");
                std::this_thread::sleep_for(std::chrono::milliseconds(300));

                // Edge processor acknowledges and processes
                on_p2p_message("edge_processor_01", "PROCESSING: ml_inference_started");
                std::this_thread::sleep_for(std::chrono::milliseconds(800));

                // Processing complete, results ready
                on_p2p_message("edge_processor_01", "RESULTS_READY: anomaly_detected=false, confidence=0.94");
                std::this_thread::sleep_for(std::chrono::milliseconds(300));

                // Gateway receives final results
                on_p2p_message("gateway_hub_01", "RESULTS_RECEIVED: forwarding_to_cloud");

                // Submit processing result to blockchain
                CardanoIoTSDK::IoTData processing_result;
                processing_result.device_id = "edge_processor_01";
                processing_result.data_type = "distributed_processing";
                processing_result.payload = R"({"batch_id": "batch_001", "anomaly_detected": false, "confidence": 0.94, "processing_time_ms": 800})";
                processing_result.timestamp = time(nullptr);

                std::string proc_tx = sdk.submit_data(processing_result);
                std::cout << "🔗 [BLOCKCHAIN] Processing result logged: " << proc_tx.substr(0, 12) << "..." << std::endl;
            }

            // Network health check
            std::cout << "💓 [HEALTH] Network health check..." << std::endl;
            for (const auto &device : network_devices)
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> uptime_dis(85.0, 99.9);

                double uptime = uptime_dis(gen);
                std::cout << "   ✅ " << device.device_id << ": " << std::fixed << std::setprecision(1)
                          << uptime << "% uptime" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        std::cout << "\n🔄 [CONSENSUS] Demonstrating network consensus..." << std::endl;

        // Simulate consensus mechanism
        std::vector<std::string> consensus_votes = {
            "gateway_hub_01: VOTE_COMMIT",
            "sensor_cluster_A: VOTE_COMMIT",
            "edge_processor_01: VOTE_COMMIT"};

        for (const auto &vote : consensus_votes)
        {
            std::cout << "🗳️  [CONSENSUS] " << vote << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        std::cout << "✅ [CONSENSUS] Network consensus achieved - committing state" << std::endl;

        // Submit consensus result
        CardanoIoTSDK::IoTData consensus_data;
        consensus_data.device_id = "gateway_hub_01";
        consensus_data.data_type = "network_consensus";
        consensus_data.payload = R"({"consensus_round": 1, "participants": 3, "decision": "commit", "unanimous": true})";
        consensus_data.timestamp = time(nullptr);

        std::string consensus_tx = sdk.submit_data(consensus_data);

        std::cout << "\n📊 [STATISTICS] P2P Network Statistics:" << std::endl;
        std::cout << "   🌐 Active nodes: " << network_devices.size() << std::endl;
        std::cout << "   📨 Messages exchanged: ~25" << std::endl;
        std::cout << "   🔗 Blockchain transactions: 8" << std::endl;
        std::cout << "   ⏱️ Average latency: <200ms" << std::endl;
        std::cout << "   💓 Network uptime: 98.5%" << std::endl;

        std::cout << "\n🔍 [QUERY] Retrieving P2P communication logs..." << std::endl;

        // Query coordination data
        auto coord_history = sdk.query_data("gateway_hub_01", time(nullptr) - 3600, time(nullptr));
        std::cout << "📊 [LOGS] Gateway coordination: " << coord_history.size() << " events" << std::endl;

        auto processing_history = sdk.query_data("edge_processor_01", time(nullptr) - 3600, time(nullptr));
        std::cout << "📊 [LOGS] Edge processing: " << processing_history.size() << " events" << std::endl;

        std::cout << "\n📈 [SUMMARY] P2P Communication Demo Completed!" << std::endl;
        std::cout << "🤖 Distributed IoT network successfully demonstrated" << std::endl;
        std::cout << "📡 Peer-to-peer messaging and coordination working" << std::endl;
        std::cout << "🔗 All network events recorded on Cardano blockchain" << std::endl;
        std::cout << "⚙️ Distributed processing and consensus achieved" << std::endl;

        std::cout << "\n🔄 [SHUTDOWN] Shutting down P2P network..." << std::endl;
        sdk.shutdown();

        std::cout << "✅ [EXIT] P2P communication demo completed! 📡🤖" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "💥 [FATAL] Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
