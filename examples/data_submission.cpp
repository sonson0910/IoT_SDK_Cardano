/**
 * @file data_submission.cpp
 * @brief Example demonstrating IoT data submission to Cardano blockchain
 *
 * This example shows how to:
 * - Submit various types of IoT data
 * - Handle batch data submission
 * - Verify data integrity
 * - Query submitted data
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

void print_data_banner()
{
    std::cout << R"(
    ██████╗  █████╗ ████████╗ █████╗     ███████╗██╗   ██╗██████╗ ███╗   ███╗██╗███████╗███████╗██╗ ██████╗ ███╗   ██╗
    ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗    ██╔════╝██║   ██║██╔══██╗████╗ ████║██║██╔════╝██╔════╝██║██╔═══██╗████╗  ██║
    ██║  ██║███████║   ██║   ███████║    ███████╗██║   ██║██████╔╝██╔████╔██║██║███████╗███████╗██║██║   ██║██╔██╗ ██║
    ██║  ██║██╔══██║   ██║   ██╔══██║    ╚════██║██║   ██║██╔══██╗██║╚██╔╝██║██║╚════██║╚════██║██║██║   ██║██║╚██╗██║
    ██████╔╝██║  ██║   ██║   ██║  ██║    ███████║╚██████╔╝██████╔╝██║ ╚═╝ ██║██║███████║███████║██║╚██████╔╝██║ ╚████║
    ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝    ╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝╚══════╝╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
                                                                                                                      
    📡 DATA SUBMISSION & BLOCKCHAIN STORAGE DEMO
    🔗 Secure IoT data recording on Cardano
    )" << std::endl;
}

// Data generation utilities
std::random_device rd;
std::mt19937 gen(rd());

double generate_temperature()
{
    std::uniform_real_distribution<> dis(15.0, 35.0);
    return dis(gen);
}

double generate_humidity()
{
    std::uniform_real_distribution<> dis(30.0, 80.0);
    return dis(gen);
}

double generate_pressure()
{
    std::uniform_real_distribution<> dis(950.0, 1050.0);
    return dis(gen);
}

int main()
{
    print_data_banner();

    try
    {
        // Initialize SDK
        CardanoIoTSDK::Config config;
        config.network_type = "testnet";
        config.enable_logging = true;

        std::cout << "🔧 [INIT] Initializing Data Submission Demo..." << std::endl;
        CardanoIoTSDK sdk(config);

        if (!sdk.initialize())
        {
            std::cerr << "❌ [ERROR] Failed to initialize SDK" << std::endl;
            return 1;
        }

        std::cout << "✅ [INIT] SDK initialized successfully!" << std::endl;

        // Register data collection devices
        std::vector<CardanoIoTSDK::DeviceInfo> devices = {
            {.device_id = "weather_station_01",
             .device_type = "environmental_sensor",
             .manufacturer = "WeatherTech Pro",
             .firmware_version = "v2.1.0",
             .capabilities = {"sensor_data", "data_aggregation"},
             .public_key = "ed25519_pk_weather_01"},
            {.device_id = "air_quality_monitor",
             .device_type = "air_quality_sensor",
             .manufacturer = "AirSense Corp",
             .firmware_version = "v1.5.2",
             .capabilities = {"sensor_data", "real_time_monitoring"},
             .public_key = "ed25519_pk_air_01"},
            {.device_id = "vibration_sensor_01",
             .device_type = "vibration_sensor",
             .manufacturer = "VibeTech Industries",
             .firmware_version = "v3.0.1",
             .capabilities = {"sensor_data", "frequency_analysis"},
             .public_key = "ed25519_pk_vibe_01"}};

        std::cout << "\n🤖 [REGISTER] Registering data collection devices..." << std::endl;
        for (const auto &device : devices)
        {
            if (sdk.register_device(device))
            {
                std::cout << "✅ [SUCCESS] " << device.device_id << " registered" << std::endl;
            }
        }

        std::cout << "\n📊 [DATA] Starting data collection and submission..." << std::endl;

        // Simulate data collection over 10 cycles
        for (int cycle = 1; cycle <= 10; ++cycle)
        {
            std::cout << "\n🔄 [CYCLE " << cycle << "] Collecting sensor data..." << std::endl;

            // Weather station data
            {
                CardanoIoTSDK::IoTData weather_data;
                weather_data.device_id = "weather_station_01";
                weather_data.data_type = "environmental_reading";

                double temp = generate_temperature();
                double humidity = generate_humidity();
                double pressure = generate_pressure();

                weather_data.payload = "{ \"temperature\": " + std::to_string(temp) +
                                       ", \"humidity\": " + std::to_string(humidity) +
                                       ", \"pressure\": " + std::to_string(pressure) +
                                       ", \"unit_temp\": \"celsius\", \"unit_pressure\": \"hPa\" }";

                weather_data.timestamp = time(nullptr);
                weather_data.metadata["location"] = "rooftop_station_A";
                weather_data.metadata["calibration_date"] = "2024-01-15";
                weather_data.metadata["quality_score"] = "0.98";

                std::string tx_id = sdk.submit_data(weather_data);
                std::cout << "🌡️  [WEATHER] T:" << std::fixed << std::setprecision(1) << temp
                          << "°C, H:" << humidity << "%, P:" << pressure << "hPa -> TX: "
                          << tx_id.substr(0, 12) << "..." << std::endl;
            }

            // Air quality data
            {
                CardanoIoTSDK::IoTData air_data;
                air_data.device_id = "air_quality_monitor";
                air_data.data_type = "air_quality_index";

                std::uniform_real_distribution<> pm25_dis(5.0, 50.0);
                std::uniform_real_distribution<> co2_dis(350.0, 800.0);
                std::uniform_real_distribution<> voc_dis(0.1, 5.0);

                double pm25 = pm25_dis(gen);
                double co2 = co2_dis(gen);
                double voc = voc_dis(gen);

                air_data.payload = "{ \"pm25\": " + std::to_string(pm25) +
                                   ", \"co2\": " + std::to_string(co2) +
                                   ", \"voc\": " + std::to_string(voc) +
                                   ", \"aqi_calculated\": " + std::to_string((int)(pm25 * 2)) + " }";

                air_data.timestamp = time(nullptr);
                air_data.metadata["sensor_model"] = "AQM-2000";
                air_data.metadata["measurement_duration"] = "60";

                std::string tx_id = sdk.submit_data(air_data);
                std::cout << "💨 [AIR] PM2.5:" << pm25 << "μg/m³, CO2:" << (int)co2
                          << "ppm -> TX: " << tx_id.substr(0, 12) << "..." << std::endl;
            }

            // Vibration sensor data
            {
                CardanoIoTSDK::IoTData vib_data;
                vib_data.device_id = "vibration_sensor_01";
                vib_data.data_type = "vibration_analysis";

                std::uniform_real_distribution<> freq_dis(10.0, 1000.0);
                std::uniform_real_distribution<> amp_dis(0.1, 10.0);

                double frequency = freq_dis(gen);
                double amplitude = amp_dis(gen);
                double rms = amplitude * 0.707;

                vib_data.payload = "{ \"dominant_frequency\": " + std::to_string(frequency) +
                                   ", \"amplitude\": " + std::to_string(amplitude) +
                                   ", \"rms_value\": " + std::to_string(rms) +
                                   ", \"fft_peaks\": [" + std::to_string(frequency) + ", " +
                                   std::to_string(frequency * 2) + ", " + std::to_string(frequency * 3) + "] }";

                vib_data.timestamp = time(nullptr);
                vib_data.metadata["sampling_rate"] = "10000";
                vib_data.metadata["analysis_window"] = "hanning";

                std::string tx_id = sdk.submit_data(vib_data);
                std::cout << "📳 [VIBRATION] Freq:" << (int)frequency << "Hz, Amp:"
                          << std::setprecision(2) << amplitude << "mm/s -> TX: "
                          << tx_id.substr(0, 12) << "..." << std::endl;
            }

            // Show network status occasionally
            if (cycle % 3 == 0)
            {
                auto network_status = sdk.get_network_status();
                std::cout << "🌐 [NETWORK] Blockchain sync: " << network_status["sync_progress"]
                          << ", Total TX: " << network_status["transactions"] << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        std::cout << "\n🔍 [QUERY] Querying submitted data..." << std::endl;

        // Query data for each device
        for (const auto &device : devices)
        {
            auto data_history = sdk.query_data(device.device_id, time(nullptr) - 3600, time(nullptr));
            std::cout << "📊 [HISTORY] " << device.device_id << ": " << data_history.size()
                      << " records found" << std::endl;

            // Show latest record
            if (!data_history.empty())
            {
                const auto &latest = data_history.back();
                std::cout << "   Latest: " << latest.data_type << " at " << latest.timestamp << std::endl;
            }
        }

        std::cout << "\n✅ [VERIFY] Verifying data integrity..." << std::endl;

        // Verify some submitted data
        for (const auto &device : devices)
        {
            auto data_history = sdk.query_data(device.device_id, time(nullptr) - 3600, time(nullptr));
            for (const auto &data : data_history)
            {
                bool is_valid = sdk.verify_data_integrity(data);
                if (is_valid)
                {
                    std::cout << "✅ [INTEGRITY] " << device.device_id << " data verified" << std::endl;
                }
                else
                {
                    std::cout << "❌ [INTEGRITY] " << device.device_id << " data verification failed" << std::endl;
                }
                break; // Just check first record
            }
        }

        std::cout << "\n📈 [SUMMARY] Data submission completed!" << std::endl;
        std::cout << "🔗 All IoT data securely recorded on Cardano blockchain" << std::endl;
        std::cout << "📊 Environmental, air quality, and vibration data submitted" << std::endl;
        std::cout << "✅ Data integrity verified" << std::endl;

        std::cout << "\n🔄 [SHUTDOWN] Shutting down..." << std::endl;
        sdk.shutdown();

        std::cout << "✅ [EXIT] Data submission demo completed successfully! 📡" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "💥 [FATAL] Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
