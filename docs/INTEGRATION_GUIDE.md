# 🔗 Integration Guide - Cardano IoT SDK

![Integration Banner](https://img.shields.io/badge/Integration-Ready-brightgreen) ![C++17](https://img.shields.io/badge/C++-17-blue) ![Cardano](https://img.shields.io/badge/Cardano-Compatible-blue)

## 🚀 **Quick Start Integration**

### **⚡ 5-Minute Setup**

```bash
# 1. Clone và build SDK
git clone https://github.com/cardano-iot/sdk.git
cd cardano-iot-sdk
make build

# 2. Tạo ứng dụng đầu tiên
mkdir my-iot-app && cd my-iot-app
```

**Tạo file `main.cpp`:**
```cpp
#include <cardano_iot/cardano_iot.h>
#include <iostream>

int main() {
    // Bước 1: Khởi tạo SDK
    cardano_iot::CardanoIoTSDK::Config config;
    config.network_type = "testnet";
    config.enable_logging = true;
    
    cardano_iot::CardanoIoTSDK sdk(config);
    
    if (!sdk.initialize()) {
        std::cerr << "❌ Failed to initialize SDK" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Cardano IoT SDK Ready!" << std::endl;
    
    // Bước 2: Đăng ký thiết bị
    cardano_iot::CardanoIoTSDK::DeviceInfo device;
    device.device_id = "my_sensor_001";
    device.device_type = "temperature_sensor";
    device.manufacturer = "My Company";
    device.capabilities = {"sensor_data"};
    
    if (sdk.register_device(device)) {
        std::cout << "🤖 Device registered successfully!" << std::endl;
    }
    
    // Bước 3: Gửi dữ liệu
    cardano_iot::CardanoIoTSDK::IoTData data;
    data.device_id = "my_sensor_001";
    data.data_type = "temperature";
    data.payload = R"({"temperature": 25.0, "unit": "celsius"})";
    data.timestamp = time(nullptr);
    
    std::string tx_id = sdk.submit_data(data);
    std::cout << "📡 Data submitted: " << tx_id << std::endl;
    
    return 0;
}
```

**Build và chạy:**
```bash
g++ -std=c++17 -I../include main.cpp -L../build -lcardano_iot_sdk -lcurl -lssl -lcrypto -o my-iot-app
./my-iot-app
```

---

## 📋 **Integration Patterns**

### **1. 🏭 Industrial IoT Integration**

```cpp
class IndustrialMonitor {
private:
    cardano_iot::CardanoIoTSDK sdk_;
    std::vector<std::string> sensor_ids_;
    
public:
    IndustrialMonitor() : sdk_(create_config()) {
        if (!sdk_.initialize()) {
            throw std::runtime_error("SDK initialization failed");
        }
    }
    
    // Đăng ký nhiều sensor
    void register_sensors(const std::vector<SensorSpec>& sensors) {
        for (const auto& spec : sensors) {
            cardano_iot::CardanoIoTSDK::DeviceInfo device;
            device.device_id = spec.id;
            device.device_type = spec.type;
            device.manufacturer = "Industrial Corp";
            device.capabilities = {"sensor_data", "actuator_control"};
            
            if (sdk_.register_device(device)) {
                sensor_ids_.push_back(spec.id);
                std::cout << "✅ Registered: " << spec.id << std::endl;
            }
        }
    }
    
    // Thu thập dữ liệu theo batch
    void collect_batch_data() {
        for (const auto& sensor_id : sensor_ids_) {
            auto sensor_data = read_sensor_data(sensor_id);
            
            cardano_iot::CardanoIoTSDK::IoTData data;
            data.device_id = sensor_id;
            data.data_type = sensor_data.type;
            data.payload = sensor_data.to_json();
            data.timestamp = time(nullptr);
            
            // Metadata cho tracking
            data.metadata["batch_id"] = current_batch_id_;
            data.metadata["quality_check"] = "passed";
            data.metadata["calibration_date"] = last_calibration_;
            
            std::string tx_id = sdk_.submit_data(data);
            log_transaction(sensor_id, tx_id);
        }
    }
    
private:
    cardano_iot::CardanoIoTSDK::Config create_config() {
        cardano_iot::CardanoIoTSDK::Config config;
        config.network_type = "mainnet";  // Production
        config.enable_logging = true;
        config.enable_power_management = false;  // AC powered
        config.max_devices = 500;  // Large deployment
        return config;
    }
};
```

### **2. 🏠 Smart Home Integration**

```cpp
class SmartHomeController {
private:
    cardano_iot::CardanoIoTSDK sdk_;
    std::map<std::string, std::string> deployed_contracts_;
    
public:
    // Automation setup
    void setup_automation() {
        // Deploy temperature control contract
        std::string temp_contract = deploy_temperature_contract();
        deployed_contracts_["temperature"] = temp_contract;
        
        // Deploy security contract  
        std::string security_contract = deploy_security_contract();
        deployed_contracts_["security"] = security_contract;
        
        std::cout << "🏠 Smart home automation deployed!" << std::endl;
    }
    
    // Device event handler
    void setup_event_handlers() {
        sdk_.set_device_event_callback([this](const std::string& device_id, const std::string& event) {
            handle_device_event(device_id, event);
        });
        
        sdk_.set_data_event_callback([this](const cardano_iot::CardanoIoTSDK::IoTData& data) {
            handle_sensor_data(data);
        });
    }
    
private:
    void handle_sensor_data(const cardano_iot::CardanoIoTSDK::IoTData& data) {
        if (data.data_type == "temperature") {
            auto temp_data = parse_json(data.payload);
            double temperature = temp_data["temperature"];
            
            // Trigger automation
            if (temperature > 25.0) {
                execute_cooling_automation(data.device_id);
            } else if (temperature < 18.0) {
                execute_heating_automation(data.device_id);
            }
        }
        
        if (data.data_type == "motion") {
            execute_security_automation(data.device_id);
        }
    }
    
    void execute_cooling_automation(const std::string& sensor_id) {
        std::map<std::string, std::string> params;
        params["action"] = "COOL";
        params["trigger_device"] = sensor_id;
        params["target_temp"] = "22";
        
        std::string result = sdk_.execute_contract(
            deployed_contracts_["temperature"], 
            "control_hvac", 
            params
        );
        
        std::cout << "❄️ Cooling activated by " << sensor_id << std::endl;
    }
};
```

### **3. 🌱 Agricultural IoT Integration**

```cpp
class AgricultureMonitor {
private:
    cardano_iot::CardanoIoTSDK sdk_;
    std::string irrigation_contract_;
    
public:
    // Thiết lập monitoring cho farm
    void setup_farm_monitoring() {
        // Đăng ký soil sensors
        register_soil_sensors();
        
        // Đăng ký weather station
        register_weather_station();
        
        // Đăng ký irrigation actuators
        register_irrigation_system();
        
        // Deploy irrigation automation contract
        irrigation_contract_ = deploy_irrigation_contract();
    }
    
    // Monitoring loop chính
    void run_monitoring_cycle() {
        while (monitoring_active_) {
            // Thu thập dữ liệu từ sensors
            auto soil_data = collect_soil_data();
            auto weather_data = collect_weather_data();
            
            // Phân tích và ra quyết định
            IrrigationDecision decision = analyze_irrigation_needs(soil_data, weather_data);
            
            if (decision.needs_irrigation) {
                execute_irrigation(decision);
            }
            
            // Log lên blockchain
            log_agriculture_data(soil_data, weather_data, decision);
            
            // Chờ chu kỳ tiếp theo
            std::this_thread::sleep_for(std::chrono::minutes(15));
        }
    }
    
private:
    struct IrrigationDecision {
        bool needs_irrigation;
        std::string zone_id;
        int duration_minutes;
        std::string reason;
    };
    
    IrrigationDecision analyze_irrigation_needs(
        const SoilData& soil, 
        const WeatherData& weather
    ) {
        IrrigationDecision decision;
        
        // Logic phân tích đất và thời tiết
        if (soil.moisture_level < 0.3 && weather.rainfall_24h < 5.0) {
            decision.needs_irrigation = true;
            decision.zone_id = soil.zone_id;
            decision.duration_minutes = calculate_irrigation_duration(soil);
            decision.reason = "Low soil moisture + no rain";
        } else {
            decision.needs_irrigation = false;
            decision.reason = "Adequate moisture levels";
        }
        
        return decision;
    }
    
    void execute_irrigation(const IrrigationDecision& decision) {
        std::map<std::string, std::string> params;
        params["zone_id"] = decision.zone_id;
        params["duration"] = std::to_string(decision.duration_minutes);
        params["reason"] = decision.reason;
        
        std::string result = sdk_.execute_contract(
            irrigation_contract_,
            "start_irrigation",
            params
        );
        
        std::cout << "💧 Irrigation started for zone " << decision.zone_id 
                  << " for " << decision.duration_minutes << " minutes" << std::endl;
    }
};
```

---

## ⚙️ **Configuration Patterns**

### **1. 📋 Environment-Based Configuration**

```cpp
// config/development.json
{
    "network": {"type": "testnet"},
    "logging": {"level": "DEBUG", "enable_console": true},
    "performance": {"max_devices": 10},
    "security": {"enable_encryption": false}
}

// config/production.json  
{
    "network": {"type": "mainnet"},
    "logging": {"level": "INFO", "file_path": "/var/log/iot.log"},
    "performance": {"max_devices": 1000},
    "security": {"enable_encryption": true}
}

// C++ Configuration Loading
class ConfigManager {
public:
    static cardano_iot::CardanoIoTSDK::Config load_for_environment(const std::string& env) {
        std::string config_file = "config/" + env + ".json";
        
        cardano_iot::utils::Config config_loader;
        config_loader.load_from_file(config_file);
        
        cardano_iot::CardanoIoTSDK::Config sdk_config;
        sdk_config.network_type = config_loader.get_string("network.type");
        sdk_config.enable_logging = config_loader.get_bool("logging.enable_console");
        sdk_config.max_devices = config_loader.get_int("performance.max_devices");
        
        return sdk_config;
    }
};

// Usage
auto config = ConfigManager::load_for_environment("production");
cardano_iot::CardanoIoTSDK sdk(config);
```

### **2. 🔐 Security Configuration**

```cpp
class SecureIoTApp {
private:
    void setup_secure_sdk() {
        cardano_iot::CardanoIoTSDK::Config config;
        
        // Network security
        config.network_type = "mainnet";
        config.node_socket_path = "/secure/cardano-node.socket";
        
        // Enable all security features
        config.enable_encryption = true;
        config.transaction_timeout_ms = 30000;
        
        // Device authentication requirements
        setup_device_authentication();
        
        sdk_ = std::make_unique<cardano_iot::CardanoIoTSDK>(config);
    }
    
    void setup_device_authentication() {
        // Set strong authentication callback
        sdk_->set_device_event_callback([this](const std::string& device_id, const std::string& event) {
            if (event == "authentication_failed") {
                handle_security_breach(device_id);
            }
        });
    }
    
    void handle_security_breach(const std::string& device_id) {
        // Log security incident
        log_security_event("AUTH_FAILURE", device_id);
        
        // Disable device temporarily
        blacklist_device(device_id);
        
        // Alert security team
        send_security_alert("Authentication failure for device: " + device_id);
    }
};
```

---

## 🔌 **Integration with External Systems**

### **1. 🗄️ Database Integration**

```cpp
class DatabaseIntegratedIoT {
private:
    cardano_iot::CardanoIoTSDK sdk_;
    std::unique_ptr<DatabaseConnection> db_;
    
public:
    void setup_dual_storage() {
        // Setup database connection
        db_ = std::make_unique<DatabaseConnection>("postgresql://iot:pass@localhost/iot_db");
        
        // Setup blockchain callback
        sdk_.set_data_event_callback([this](const cardano_iot::CardanoIoTSDK::IoTData& data) {
            // Store to both database and blockchain
            store_to_database(data);
            // Blockchain storage happens automatically via SDK
        });
    }
    
private:
    void store_to_database(const cardano_iot::CardanoIoTSDK::IoTData& data) {
        std::string sql = R"(
            INSERT INTO iot_data (device_id, data_type, payload, timestamp, blockchain_tx)
            VALUES ($1, $2, $3, $4, $5)
        )";
        
        db_->execute(sql, {
            data.device_id,
            data.data_type, 
            data.payload,
            std::to_string(data.timestamp),
            data.hash  // Blockchain reference
        });
    }
};
```

### **2. 📊 Analytics Integration**

```cpp
class AnalyticsIntegratedIoT {
private:
    cardano_iot::CardanoIoTSDK sdk_;
    std::unique_ptr<AnalyticsEngine> analytics_;
    
public:
    void setup_analytics_pipeline() {
        analytics_ = std::make_unique<AnalyticsEngine>();
        
        sdk_.set_data_event_callback([this](const cardano_iot::CardanoIoTSDK::IoTData& data) {
            // Real-time analytics
            process_for_analytics(data);
            
            // Anomaly detection
            if (analytics_->detect_anomaly(data)) {
                handle_anomaly(data);
            }
        });
    }
    
private:
    void process_for_analytics(const cardano_iot::CardanoIoTSDK::IoTData& data) {
        // Parse data for analytics
        auto parsed = parse_iot_data(data.payload);
        
        // Feed to analytics engine
        analytics_->ingest_data_point({
            .device_id = data.device_id,
            .timestamp = data.timestamp,
            .value = parsed.primary_value,
            .metadata = data.metadata
        });
        
        // Generate insights
        auto insights = analytics_->generate_insights(data.device_id);
        if (!insights.empty()) {
            publish_insights(insights);
        }
    }
};
```

---

## 🚀 **Production Deployment Patterns**

### **1. 🐳 Containerized Deployment**

**Dockerfile:**
```dockerfile
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libcurl4-openssl-dev \
    nlohmann-json3-dev

# Copy SDK
COPY cardano-iot-sdk /app/sdk
WORKDIR /app/sdk

# Build SDK
RUN make build && make install

# Copy application
COPY my-iot-app /app/
WORKDIR /app

# Build application
RUN g++ -std=c++17 -o iot-app main.cpp -lcardano_iot_sdk

CMD ["./iot-app"]
```

**docker-compose.yml:**
```yaml
version: '3.8'
services:
  iot-app:
    build: .
    environment:
      - IOT_NETWORK=mainnet
      - IOT_LOG_LEVEL=INFO
    volumes:
      - ./config:/app/config
      - ./logs:/app/logs
    restart: unless-stopped
    
  cardano-node:
    image: inputoutput/cardano-node:latest
    volumes:
      - cardano-data:/data
    ports:
      - "3001:3001"
```

### **2. 🏗️ Microservices Architecture**

```cpp
// Device Service
class DeviceService {
    void run() {
        auto config = load_device_service_config();
        cardano_iot::CardanoIoTSDK sdk(config);
        
        // Handle device registration requests
        setup_device_endpoints(sdk);
    }
};

// Data Service  
class DataService {
    void run() {
        auto config = load_data_service_config();
        cardano_iot::CardanoIoTSDK sdk(config);
        
        // Handle data submission requests
        setup_data_endpoints(sdk);
    }
};

// Contract Service
class ContractService {
    void run() {
        auto config = load_contract_service_config();
        cardano_iot::CardanoIoTSDK sdk(config);
        
        // Handle smart contract operations
        setup_contract_endpoints(sdk);
    }
};
```

---

## 📈 **Performance Optimization**

### **1. ⚡ Batch Processing**

```cpp
class BatchProcessor {
private:
    cardano_iot::CardanoIoTSDK sdk_;
    std::queue<cardano_iot::CardanoIoTSDK::IoTData> data_queue_;
    std::mutex queue_mutex_;
    
public:
    void add_data(const cardano_iot::CardanoIoTSDK::IoTData& data) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        data_queue_.push(data);
        
        // Process batch when threshold reached
        if (data_queue_.size() >= BATCH_SIZE) {
            process_batch();
        }
    }
    
private:
    void process_batch() {
        std::vector<cardano_iot::CardanoIoTSDK::IoTData> batch;
        
        // Extract batch
        while (!data_queue_.empty() && batch.size() < BATCH_SIZE) {
            batch.push_back(data_queue_.front());
            data_queue_.pop();
        }
        
        // Submit batch efficiently
        for (const auto& data : batch) {
            sdk_.submit_data(data);
        }
        
        std::cout << "📦 Processed batch of " << batch.size() << " items" << std::endl;
    }
    
    static constexpr size_t BATCH_SIZE = 50;
};
```

### **2. 🔄 Connection Pooling**

```cpp
class ConnectionPooledSDK {
private:
    std::vector<std::unique_ptr<cardano_iot::CardanoIoTSDK>> sdk_pool_;
    std::queue<size_t> available_connections_;
    std::mutex pool_mutex_;
    
public:
    ConnectionPooledSDK(size_t pool_size) {
        for (size_t i = 0; i < pool_size; ++i) {
            auto config = create_config();
            auto sdk = std::make_unique<cardano_iot::CardanoIoTSDK>(config);
            sdk->initialize();
            
            sdk_pool_.push_back(std::move(sdk));
            available_connections_.push(i);
        }
    }
    
    template<typename Operation>
    auto execute_with_pool(Operation op) {
        // Get connection from pool
        auto connection_id = acquire_connection();
        
        // Execute operation
        auto result = op(*sdk_pool_[connection_id]);
        
        // Return connection to pool
        release_connection(connection_id);
        
        return result;
    }
    
private:
    size_t acquire_connection() {
        std::unique_lock<std::mutex> lock(pool_mutex_);
        
        // Wait for available connection
        while (available_connections_.empty()) {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            lock.lock();
        }
        
        size_t id = available_connections_.front();
        available_connections_.pop();
        return id;
    }
    
    void release_connection(size_t id) {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        available_connections_.push(id);
    }
};
```

---

## 🔧 **Development Tools Integration**

### **1. 📊 Monitoring và Metrics**

```cpp
class MonitoredIoTApp {
private:
    cardano_iot::CardanoIoTSDK sdk_;
    std::unique_ptr<MetricsCollector> metrics_;
    
public:
    void setup_monitoring() {
        metrics_ = std::make_unique<MetricsCollector>();
        
        sdk_.set_device_event_callback([this](const std::string& device_id, const std::string& event) {
            metrics_->increment_counter("device_events", {{"event", event}});
        });
        
        sdk_.set_transaction_event_callback([this](const std::string& tx_id, bool success) {
            if (success) {
                metrics_->increment_counter("successful_transactions");
            } else {
                metrics_->increment_counter("failed_transactions");
            }
        });
    }
    
    void report_health() {
        auto stats = sdk_.get_network_status();
        
        metrics_->set_gauge("connected_devices", std::stoi(stats["devices"]));
        metrics_->set_gauge("blockchain_sync", stats["sync_progress"] == "100%" ? 1 : 0);
        metrics_->set_gauge("pending_transactions", std::stoi(stats["pending"]));
    }
};
```

### **2. 🧪 Testing Integration**

```cpp
class TestableIoTApp {
private:
    cardano_iot::CardanoIoTSDK sdk_;
    bool test_mode_;
    
public:
    TestableIoTApp(bool test_mode = false) : test_mode_(test_mode) {
        auto config = create_config();
        if (test_mode_) {
            config.network_type = "testnet";
            config.enable_logging = false;  // Quiet during tests
        }
        
        sdk_ = cardano_iot::CardanoIoTSDK(config);
    }
    
    // Test-friendly methods
    bool register_test_device(const std::string& device_id) {
        cardano_iot::CardanoIoTSDK::DeviceInfo device;
        device.device_id = device_id;
        device.device_type = "test_device";
        device.manufacturer = "Test Corp";
        
        return sdk_.register_device(device);
    }
    
    std::string submit_test_data(const std::string& device_id, const std::string& payload) {
        cardano_iot::CardanoIoTSDK::IoTData data;
        data.device_id = device_id;
        data.data_type = "test_data";
        data.payload = payload;
        data.timestamp = time(nullptr);
        
        return sdk_.submit_data(data);
    }
};

// Unit Test Example
#include <gtest/gtest.h>

TEST(IoTAppTest, DeviceRegistration) {
    TestableIoTApp app(true);  // Test mode
    
    ASSERT_TRUE(app.initialize());
    EXPECT_TRUE(app.register_test_device("test_001"));
    EXPECT_FALSE(app.register_test_device("test_001"));  // Duplicate should fail
}

TEST(IoTAppTest, DataSubmission) {
    TestableIoTApp app(true);
    
    app.register_test_device("test_002");
    
    std::string tx_id = app.submit_test_data("test_002", R"({"value": 42})");
    EXPECT_FALSE(tx_id.empty());
    EXPECT_TRUE(tx_id.find("tx_") == 0);  // Should start with "tx_"
}
```

---

## 🎯 **Integration Checklist**

### **✅ Pre-Integration**
- [ ] SDK dependencies installed (OpenSSL, libcurl, nlohmann/json)
- [ ] Cardano testnet access configured
- [ ] Development environment setup
- [ ] Example applications tested

### **✅ Basic Integration**  
- [ ] SDK initialization working
- [ ] Device registration successful
- [ ] Data submission functional
- [ ] Event callbacks implemented

### **✅ Advanced Integration**
- [ ] Smart contract deployment tested
- [ ] Power management configured
- [ ] Error handling implemented
- [ ] Logging and monitoring setup

### **✅ Production Ready**
- [ ] Security configuration reviewed
- [ ] Performance testing completed
- [ ] Monitoring dashboards created
- [ ] Backup and recovery tested
- [ ] Documentation completed

---

## 🚀 **Next Steps**

1. **📖 Review Examples**: Study the demo applications code
2. **🔧 Customize Configuration**: Adapt to your specific needs  
3. **🧪 Build Prototype**: Create a minimal working version
4. **📈 Add Monitoring**: Implement metrics and alerts
5. **🚀 Deploy to Production**: Follow deployment best practices

---

**🎯 Ready to integrate? Start with the Quick Start section and gradually add more advanced features!**

For more detailed examples, check out the `/examples` directory and explore the comprehensive API documentation.
