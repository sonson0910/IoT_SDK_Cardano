# Cardano IoT SDK 🚀

<!-- Banner temporarily disabled (asset missing). Uncomment when available. -->
<!-- ![Cardano IoT Banner](docs/assets/banner.png) -->

**A comprehensive C++ SDK for integrating IoT devices with the Cardano blockchain ecosystem.**

[![Build Status](https://github.com/cardano-iot/sdk/workflows/CI/badge.svg)](https://github.com/cardano-iot/sdk/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Cardano](https://img.shields.io/badge/Cardano-Compatible-blue.svg)](https://cardano.org)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)

## 🌟 Overview

The Cardano IoT SDK enables seamless integration of Internet of Things (IoT) devices with the Cardano blockchain. Built with security, scalability, and energy efficiency in mind, this SDK provides everything needed to create blockchain-enabled IoT applications.

### ✨ Key Features

- 🔐 **Secure Device Authentication** - Cryptographic device identity management
- 📊 **Data Provenance** - Complete data lifecycle tracking and integrity verification
- ⚡ **Smart Contract Integration** - Automated IoT workflows via Plutus smart contracts
- 💰 **Microtransactions** - Efficient ADA transfers for device operations
- 🔋 **Energy Management** - Power-optimized operations for battery-powered devices
- 🌐 **P2P Networking** - Decentralized device communication
- 📈 **Scalability** - Support for thousands of concurrent devices
- 🎯 **Cross-Platform** - Linux, Windows, macOS, and embedded systems

## 🏗️ Architecture

```
┌─────────────────┬─────────────────┬─────────────────┐
│   Application   │   Application   │   Application   │
│      Layer      │      Layer      │      Layer      │
├─────────────────┴─────────────────┴─────────────────┤
│                 Cardano IoT SDK                     │
├─────────────────┬─────────────────┬─────────────────┤
│ Device Manager  │ Security Layer  │ Data Provenance │
├─────────────────┼─────────────────┼─────────────────┤
│ Smart Contracts │ Transaction Mgr │ Energy Manager  │
├─────────────────┼─────────────────┼─────────────────┤
│    P2P Network  │ Cardano Client  │    Utilities    │
├─────────────────┴─────────────────┴─────────────────┤
│              Cardano Blockchain                     │
└─────────────────────────────────────────────────────┘
```

## 🚀 Quick Start

### Prerequisites

- **C++17** compatible compiler (GCC 8+, Clang 9+, MSVC 2019+)
- **CMake 3.16+**
- **OpenSSL 1.1+**
- **libcurl 7.50+**
- **nlohmann/json 3.2+**
- **Cardano Node** (for mainnet/testnet connections)

### Installation

#### Option 1: Build from Source

```bash
git clone https://github.com/cardano-iot/sdk.git
cd sdk
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

#### Option 2: Package Manager (Coming Soon)

```bash
# Ubuntu/Debian
sudo apt install libcardano-iot-dev

# macOS
brew install cardano-iot-sdk

# vcpkg
vcpkg install cardano-iot-sdk
```

### Basic Usage

```cpp
#include <cardano_iot/cardano_iot.h>
using namespace cardano_iot;

int main() {
    // Configure SDK
    CardanoIoTSDK::Config config;
    config.network_type = "testnet";
    config.enable_encryption = true;
    
    // Initialize SDK
    CardanoIoTSDK sdk(config);
    if (!sdk.initialize()) {
        return 1;
    }
    
    // Register IoT device
    CardanoIoTSDK::DeviceInfo device;
    device.device_id = "sensor_001";
    device.device_type = "temperature_sensor";
    device.manufacturer = "ACME Corp";
    device.public_key = "ed25519_pk_0123456789abcdef..."; // required
    device.capabilities = {"sensor_data"}; // optional: "low_power" sets low_power_mode
    
    if (!sdk.register_device(device)) {
        return 1; // registration failed (e.g., missing public_key)
    }
    
    // Submit sensor data
    CardanoIoTSDK::IoTData data;
    data.device_id = "sensor_001";
    data.data_type = "temperature";
    data.payload = R"({"temp": 23.5, "unit": "celsius"})";
    data.timestamp = time(nullptr);
    
    std::string tx_id = sdk.submit_data(data);
    std::cout << "Data submitted: " << tx_id << std::endl;
    
    return 0;
}
```

> Note: Current blockchain/network functionality is mocked for demos/tests.
> Real node integration (Ogmios/Kupo/cardano-node) is planned.

## 📚 Documentation

### API Reference

- [Device Management](docs/api/device-management.md)
- [Data Provenance](docs/api/data-provenance.md)
- [Security & Authentication](docs/api/security.md)
- [Smart Contract Integration](docs/api/smart-contracts.md)
- [Network Operations](docs/api/network.md)
- [Energy Management](docs/api/energy.md)

### Guides (coming soon)

- Getting Started
- Device Registration
- Data Submission
- Smart Contract Development
- Security Best Practices
- Deployment Guide

### Examples

- [Basic Device Registration](examples/device_registration.cpp)
- [Data Submission](examples/data_submission.cpp)
- [Smart Contract Demo](examples/smart_contract_demo.cpp)
- [Energy Monitoring](examples/energy_monitoring.cpp)
- [P2P Communication](examples/p2p_communication.cpp)
- [Data Provenance](examples/data_provenance_demo.cpp)

## 🔧 Configuration

### SDK Configuration

```json
{
    "network": {
        "type": "testnet",
        "node_socket_path": "/tmp/cardano-node.socket",
        "node_config_path": "/etc/cardano/testnet-config.json"
    },
    "security": {
        "enable_encryption": true,
        "auth_method": "ed25519",
        "certificate_validation": true
    },
    "performance": {
        "max_devices": 1000,
        "transaction_timeout": 30000,
        "enable_caching": true
    },
    "logging": {
        "level": "INFO",
        "file_path": "/var/log/cardano-iot.log",
        "enable_console": true
    }
}
```

### Device Configuration

```json
{
    "device_id": "temp_sensor_001",
    "device_type": "temperature_sensor",
    "manufacturer": "ACME Corp",
    "model": "TS-2000",
    "firmware_version": "v1.2.3",
    "capabilities": [
        "sensor_data",
        "low_power",
        "firmware_update"
    ],
    "power_settings": {
        "low_power_mode": true,
        "sleep_interval": 60,
        "wake_on_event": true
    },
    "network_settings": {
        "enable_p2p": true,
        "relay_nodes": ["relay1.cardano.org", "relay2.cardano.org"]
    }
}
```

## 🛠️ Development

### Building

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# With tests
cmake -DBUILD_TESTS=ON ..
make test

# With examples
cmake -DBUILD_EXAMPLES=ON ..
make
```

### Testing

```bash
# Run all tests
ctest

# Run specific test suite
./tests/device_manager_tests
./tests/security_tests
./tests/network_tests
```

### Code Style

We follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with some modifications:

```bash
# Format code
clang-format -i src/**/*.cpp include/**/*.h

# Lint code
cpplint src/**/*.cpp include/**/*.h
```

## 🌐 Supported Platforms

| Platform | Architecture | Status |
|----------|-------------|--------|
| Linux | x86_64 | ✅ Supported |
| Linux | ARM64 | ✅ Supported |
| Linux | ARM32 | 🔄 In Progress |
| Windows | x86_64 | ✅ Supported |
| macOS | x86_64 | ✅ Supported |
| macOS | ARM64 (M1/M2) | ✅ Supported |
| Embedded | Various | 🔄 In Progress |

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Setup

```bash
git clone https://github.com/cardano-iot/sdk.git
cd sdk
git submodule update --init --recursive
mkdir build && cd build
cmake -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON ..
make
```

### Submitting Changes

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/awesome-feature`)
3. Commit your changes (`git commit -m 'Add awesome feature'`)
4. Push to the branch (`git push origin feature/awesome-feature`)
5. Open a Pull Request

## 📋 Roadmap

### Version 1.0 (Current)
- ✅ Core device management
- ✅ Basic authentication
- ✅ Data submission
- ✅ Simple smart contract integration

### Version 1.1 (Q2 2024)
- 🔄 Advanced cryptographic features
- 🔄 Multi-signature support
- 🔄 Enhanced data provenance
- 🔄 Performance optimizations

### Version 1.2 (Q3 2024)
- 📅 Native token support
- 📅 DeFi integration
- 📅 Advanced energy management
- 📅 Edge computing features

### Version 2.0 (Q4 2024)
- 📅 Interoperability with other chains
- 📅 AI/ML integration
- 📅 Zero-knowledge proofs
- 📅 Quantum-resistant cryptography

## 🏆 Use Cases

### Smart Agriculture
- Soil sensor monitoring
- Automated irrigation
- Crop yield tracking
- Supply chain verification

### Smart Cities
- Air quality monitoring
- Traffic management
- Energy grid optimization
- Waste management

### Industrial IoT
- Equipment monitoring
- Predictive maintenance
- Quality control
- Asset tracking

### Healthcare
- Patient monitoring
- Medical device tracking
- Drug supply chain
- Clinical data integrity

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [Cardano Foundation](https://cardanofoundation.org/) for the amazing blockchain platform
- [IOHK](https://iohk.io/) for Cardano development and research
- [Plutus](https://plutus.readthedocs.io/) smart contract platform
- Open source IoT community for inspiration and tools

## 📞 Support

- 📧 Email: support@cardano-iot.org
- 💬 Discord: [Cardano IoT Community](https://discord.gg/cardano-iot)
- 🐦 Twitter: [@CardanoIoT](https://twitter.com/CardanoIoT)
- 📖 Wiki: [GitHub Wiki](https://github.com/cardano-iot/sdk/wiki)

## ⚠️ Disclaimer

This SDK is under active development. While we strive for stability, breaking changes may occur between versions. Please review the [CHANGELOG](CHANGELOG.md) before upgrading.

---

Made with ❤️ by the Cardano IoT community
