/**
 * @file data_provenance_demo.cpp
 * @brief Example demonstrating data provenance and traceability
 *
 * This example shows how to:
 * - Track complete data lifecycle
 * - Verify data integrity and authenticity
 * - Trace data lineage and transformations
 * - Generate audit trails
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

void print_provenance_banner()
{
    std::cout << R"(
    ██████╗ ██████╗  ██████╗ ██╗   ██╗███████╗███╗   ██╗ █████╗ ███╗   ██╗ ██████╗███████╗
    ██╔══██╗██╔══██╗██╔═══██╗██║   ██║██╔════╝████╗  ██║██╔══██╗████╗  ██║██╔════╝██╔════╝
    ██████╔╝██████╔╝██║   ██║██║   ██║█████╗  ██╔██╗ ██║███████║██╔██╗ ██║██║     █████╗  
    ██╔═══╝ ██╔══██╗██║   ██║╚██╗ ██╔╝██╔══╝  ██║╚██╗██║██╔══██║██║╚██╗██║██║     ██╔══╝  
    ██║     ██║  ██║╚██████╔╝ ╚████╔╝ ███████╗██║ ╚████║██║  ██║██║ ╚████║╚██████╗███████╗
    ╚═╝     ╚═╝  ╚═╝ ╚═════╝   ╚═══╝  ╚══════╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝╚══════╝
                                                                                          
    ████████╗██████╗  █████╗  ██████╗██╗  ██╗██╗███╗   ██╗ ██████╗ 
    ╚══██╔══╝██╔══██╗██╔══██╗██╔════╝██║ ██╔╝██║████╗  ██║██╔════╝ 
       ██║   ██████╔╝███████║██║     █████╔╝ ██║██╔██╗ ██║██║  ███╗
       ██║   ██╔══██╗██╔══██║██║     ██╔═██╗ ██║██║╚██╗██║██║   ██║
       ██║   ██║  ██║██║  ██║╚██████╗██║  ██╗██║██║ ╚████║╚██████╔╝
       ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝ 
                                                                    
    📊 DATA PROVENANCE & TRACEABILITY DEMO
    🔍 Complete data lifecycle tracking on blockchain
    )" << std::endl;
}

// Data transformation simulation
struct DataTransformation
{
    std::string transformation_id;
    std::string operation;
    std::string input_hash;
    std::string output_hash;
    std::string algorithm;
    uint64_t timestamp;
};

// Generate mock hash
std::string generate_mock_hash(const std::string &data)
{
    std::hash<std::string> hasher;
    auto hash_value = hasher(data + std::to_string(time(nullptr)));
    std::stringstream ss;
    ss << "sha256_" << std::hex << hash_value;
    return ss.str();
}

int main()
{
    print_provenance_banner();

    try
    {
        // Initialize SDK with data provenance enabled
        CardanoIoTSDK::Config config;
        config.network_type = "testnet";
        config.enable_logging = true;
        // Note: Data provenance is enabled by default in the SDK

        std::cout << "🔧 [INIT] Initializing Data Provenance Demo..." << std::endl;
        CardanoIoTSDK sdk(config);

        if (!sdk.initialize())
        {
            std::cerr << "❌ [ERROR] Failed to initialize SDK" << std::endl;
            return 1;
        }

        std::cout << "✅ [INIT] SDK initialized with data provenance tracking!" << std::endl;

        // Register data producing and processing devices
        std::vector<CardanoIoTSDK::DeviceInfo> provenance_devices = {
            {.device_id = "farm_sensor_01",
             .device_type = "soil_moisture_sensor",
             .manufacturer = "AgriTech Solutions",
             .firmware_version = "v2.3.1",
             .capabilities = {"sensor_data", "data_provenance", "gps_location"},
             .public_key = "ed25519_pk_farm_01"},
            {.device_id = "data_processor_01",
             .device_type = "edge_processor",
             .manufacturer = "DataFlow Systems",
             .firmware_version = "v1.9.0",
             .capabilities = {"data_processing", "data_provenance", "data_validation"},
             .public_key = "ed25519_pk_processor_01"},
            {.device_id = "quality_validator",
             .device_type = "data_validator",
             .manufacturer = "ValidateCore Inc",
             .firmware_version = "v3.1.2",
             .capabilities = {"data_validation", "data_provenance", "quality_assessment"},
             .public_key = "ed25519_pk_validator_01"}};

        std::cout << "\n🤖 [REGISTER] Registering provenance tracking devices..." << std::endl;
        for (const auto &device : provenance_devices)
        {
            if (sdk.register_device(device))
            {
                std::cout << "✅ [DEVICE] " << device.device_id << " registered for provenance tracking" << std::endl;
            }
        }

        std::cout << "\n📊 [PROVENANCE] Starting data lifecycle tracking..." << std::endl;

        // Step 1: Original data collection with provenance
        std::cout << "\n1️⃣ [COLLECTION] Original data collection phase..." << std::endl;

        CardanoIoTSDK::IoTData original_data;
        original_data.device_id = "farm_sensor_01";
        original_data.data_type = "soil_moisture_reading";
        original_data.payload = R"({
            "moisture_percentage": 65.3,
            "temperature": 23.7,
            "ph_level": 6.8,
            "nitrogen_ppm": 120,
            "location": {"lat": 40.7128, "lon": -74.0060},
            "measurement_depth_cm": 15
        })";
        original_data.timestamp = time(nullptr);

        // Add provenance metadata
        original_data.metadata["provenance_stage"] = "original_collection";
        original_data.metadata["data_source"] = "primary_sensor";
        original_data.metadata["calibration_date"] = "2024-01-15";
        original_data.metadata["measurement_accuracy"] = "±2%";
        original_data.metadata["environmental_conditions"] = "normal";
        original_data.metadata["data_hash"] = generate_mock_hash(original_data.payload);

        std::string original_tx = sdk.submit_data(original_data);
        std::cout << "🌱 [ORIGINAL] Soil data collected - TX: " << original_tx.substr(0, 16) << "..." << std::endl;
        std::cout << "   📍 Location: 40.7128°N, 74.0060°W" << std::endl;
        std::cout << "   💧 Moisture: 65.3%, pH: 6.8" << std::endl;
        std::cout << "   🔐 Data hash: " << original_data.metadata["data_hash"].substr(0, 20) << "..." << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Step 2: Data processing with transformation tracking
        std::cout << "\n2️⃣ [PROCESSING] Data processing and transformation..." << std::endl;

        CardanoIoTSDK::IoTData processed_data;
        processed_data.device_id = "data_processor_01";
        processed_data.data_type = "processed_soil_analysis";
        processed_data.payload = R"({
            "original_data_reference": ")" +
                                 original_tx + R"(",
            "calculated_irrigation_need": "medium",
            "fertility_index": 0.78,
            "optimal_crop_types": ["tomatoes", "lettuce", "peppers"],
            "next_measurement_recommended": "2024-01-18T10:00:00Z",
            "processing_algorithm": "agricultural_ai_v2.1"
        })";
        processed_data.timestamp = time(nullptr);

        // Add processing provenance
        processed_data.metadata["provenance_stage"] = "data_processing";
        processed_data.metadata["input_data_tx"] = original_tx;
        processed_data.metadata["input_data_hash"] = original_data.metadata["data_hash"];
        processed_data.metadata["processing_algorithm"] = "agricultural_ai_v2.1";
        processed_data.metadata["processing_duration_ms"] = "1250";
        processed_data.metadata["data_hash"] = generate_mock_hash(processed_data.payload);

        // Log transformation details
        DataTransformation transformation;
        transformation.transformation_id = "transform_" + std::to_string(time(nullptr));
        transformation.operation = "agricultural_analysis";
        transformation.input_hash = original_data.metadata["data_hash"];
        transformation.output_hash = processed_data.metadata["data_hash"];
        transformation.algorithm = "agricultural_ai_v2.1";
        transformation.timestamp = time(nullptr);

        std::string processed_tx = sdk.submit_data(processed_data);
        std::cout << "⚙️ [PROCESSED] Data processed - TX: " << processed_tx.substr(0, 16) << "..." << std::endl;
        std::cout << "   🔗 Input reference: " << original_tx.substr(0, 16) << "..." << std::endl;
        std::cout << "   🌾 Irrigation need: medium" << std::endl;
        std::cout << "   📊 Fertility index: 0.78" << std::endl;
        std::cout << "   🔐 New hash: " << processed_data.metadata["data_hash"].substr(0, 20) << "..." << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Step 3: Quality validation with audit trail
        std::cout << "\n3️⃣ [VALIDATION] Data quality validation..." << std::endl;

        CardanoIoTSDK::IoTData validated_data;
        validated_data.device_id = "quality_validator";
        validated_data.data_type = "data_quality_report";
        validated_data.payload = R"({
            "processed_data_reference": ")" +
                                 processed_tx + R"(",
            "validation_result": "passed",
            "quality_score": 0.94,
            "data_completeness": 1.0,
            "data_accuracy": 0.96,
            "data_consistency": 0.92,
            "validation_checks": {
                "range_check": "passed",
                "format_check": "passed", 
                "consistency_check": "passed",
                "business_rules_check": "passed"
            },
            "validation_timestamp": ")" +
                                 std::to_string(time(nullptr)) + R"("
        })";
        validated_data.timestamp = time(nullptr);

        // Add validation provenance
        validated_data.metadata["provenance_stage"] = "data_validation";
        validated_data.metadata["input_data_tx"] = processed_tx;
        validated_data.metadata["input_data_hash"] = processed_data.metadata["data_hash"];
        validated_data.metadata["validation_algorithm"] = "quality_validator_v3.1";
        validated_data.metadata["validator_certification"] = "ISO27001_compliant";
        validated_data.metadata["data_hash"] = generate_mock_hash(validated_data.payload);

        std::string validated_tx = sdk.submit_data(validated_data);
        std::cout << "✅ [VALIDATED] Data validated - TX: " << validated_tx.substr(0, 16) << "..." << std::endl;
        std::cout << "   🔗 Input reference: " << processed_tx.substr(0, 16) << "..." << std::endl;
        std::cout << "   📊 Quality score: 94%" << std::endl;
        std::cout << "   ✅ All validation checks: PASSED" << std::endl;
        std::cout << "   🔐 Final hash: " << validated_data.metadata["data_hash"].substr(0, 20) << "..." << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Step 4: Complete provenance chain verification
        std::cout << "\n4️⃣ [VERIFICATION] Provenance chain verification..." << std::endl;

        std::vector<std::string> provenance_chain = {original_tx, processed_tx, validated_tx};

        for (size_t i = 0; i < provenance_chain.size(); ++i)
        {
            std::cout << "🔍 [VERIFY] Step " << (i + 1) << " - TX: " << provenance_chain[i].substr(0, 16) << "..." << std::endl;

            // In real implementation, this would verify blockchain data
            bool integrity_check = true; // Mock verification
            if (integrity_check)
            {
                std::cout << "   ✅ Data integrity verified" << std::endl;
            }
            else
            {
                std::cout << "   ❌ Data integrity check failed" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        std::cout << "\n🔗 [LINEAGE] Complete data lineage:" << std::endl;
        std::cout << "   1. 🌱 Original Collection → " << original_tx.substr(0, 12) << "..." << std::endl;
        std::cout << "   2. ⚙️ Processing → " << processed_tx.substr(0, 12) << "..." << std::endl;
        std::cout << "   3. ✅ Validation → " << validated_tx.substr(0, 12) << "..." << std::endl;
        std::cout << "   📊 Total provenance steps: 3" << std::endl;
        std::cout << "   🔐 Chain integrity: VERIFIED" << std::endl;

        // Step 5: Generate audit report
        std::cout << "\n5️⃣ [AUDIT] Generating comprehensive audit report..." << std::endl;

        CardanoIoTSDK::IoTData audit_report;
        audit_report.device_id = "quality_validator";
        audit_report.data_type = "provenance_audit_report";
        audit_report.payload = R"({
            "audit_id": "audit_)" +
                               std::to_string(time(nullptr)) + R"(",
            "data_lifecycle_stages": 3,
            "provenance_chain": [")" +
                               original_tx + R"(", ")" + processed_tx + R"(", ")" + validated_tx + R"("],
            "integrity_verified": true,
            "compliance_status": "compliant",
            "data_quality_final": 0.94,
            "audit_completion_time": ")" +
                               std::to_string(time(nullptr)) + R"(",
            "auditor_certification": "blockchain_auditor_certified"
        })";
        audit_report.timestamp = time(nullptr);

        audit_report.metadata["provenance_stage"] = "audit_report";
        audit_report.metadata["audit_scope"] = "complete_lifecycle";
        audit_report.metadata["compliance_framework"] = "gdpr_sox_compliant";
        audit_report.metadata["data_hash"] = generate_mock_hash(audit_report.payload);

        std::string audit_tx = sdk.submit_data(audit_report);
        std::cout << "📋 [AUDIT] Audit report generated - TX: " << audit_tx.substr(0, 16) << "..." << std::endl;
        std::cout << "   📊 Lifecycle stages tracked: 3" << std::endl;
        std::cout << "   ✅ Compliance status: COMPLIANT" << std::endl;
        std::cout << "   🔍 Data quality final: 94%" << std::endl;

        // Query and display provenance history
        std::cout << "\n🔍 [QUERY] Retrieving complete provenance history..." << std::endl;

        for (const auto &device : provenance_devices)
        {
            auto device_history = sdk.query_data(device.device_id, time(nullptr) - 3600, time(nullptr));
            if (!device_history.empty())
            {
                std::cout << "📊 [HISTORY] " << device.device_id << ": " << device_history.size() << " provenance records" << std::endl;

                for (const auto &record : device_history)
                {
                    auto stage = record.metadata.find("provenance_stage");
                    if (stage != record.metadata.end())
                    {
                        std::cout << "   📝 Stage: " << stage->second << " at " << record.timestamp << std::endl;
                    }
                }
            }
        }

        std::cout << "\n📈 [SUMMARY] Data Provenance Demo Completed!" << std::endl;
        std::cout << "🔍 Complete data lifecycle tracked on blockchain" << std::endl;
        std::cout << "📊 Original collection → Processing → Validation → Audit" << std::endl;
        std::cout << "🔐 Data integrity verified at every stage" << std::endl;
        std::cout << "✅ Full compliance audit trail generated" << std::endl;
        std::cout << "🔗 4 blockchain transactions for complete traceability" << std::endl;

        std::cout << "\n🏆 [BENEFITS] Provenance Benefits Achieved:" << std::endl;
        std::cout << "   🔍 100% data traceability" << std::endl;
        std::cout << "   🛡️ Tamper-proof audit trail" << std::endl;
        std::cout << "   📊 Quality assurance verification" << std::endl;
        std::cout << "   ⚖️ Regulatory compliance support" << std::endl;
        std::cout << "   🔗 Blockchain-based trust layer" << std::endl;

        std::cout << "\n🔄 [SHUTDOWN] Shutting down provenance system..." << std::endl;
        sdk.shutdown();

        std::cout << "✅ [EXIT] Data provenance demo completed successfully! 📊🔍" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "💥 [FATAL] Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
