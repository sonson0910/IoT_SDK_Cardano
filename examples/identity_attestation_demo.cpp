#include <cardano_iot/cardano_iot.h>
#include <cardano_iot/identity/did.h>
#include <cardano_iot/security/attestation.h>
#include <cardano_iot/utils/logger.h>

#include <iostream>

using namespace cardano_iot;

int main()
{
    // Configure logger rotation defaults (2MB, 5 backups)
    utils::Logger::instance().set_max_file_size_bytes(2 * 1024 * 1024);
    utils::Logger::instance().set_max_backup_files(5);

    // Init SDK
    CardanoIoTSDK::Config cfg;
    cfg.network_type = "testnet";
    CardanoIoTSDK sdk(cfg);
    if (!sdk.initialize())
    {
        std::cerr << "Failed to initialize SDK" << std::endl;
        return 1;
    }

    // Device info
    CardanoIoTSDK::DeviceInfo device{};
    device.device_id = "did_demo_device_01";
    device.device_type = "sensor";
    device.manufacturer = "DemoCorp";
    device.firmware_version = "v1.0.0";
    device.capabilities = {"sensor_data"};
    device.public_key = "aabbccddeeff00112233445566778899"; // mock hex

    // Create DID from public key
    auto did = identity::DIDRegistry::create_did_from_public_key(cfg.network_type, device.public_key);
    auto doc = identity::DIDRegistry::build_document(
        did,
        {{"device_key", device.public_key}},
        {{"telemetry", "wss://example.com/telemetry"}},
        {{"fw", device.firmware_version}});

    std::cout << "Created DID: " << doc.id << std::endl;

    // Verify attestation (mock)
    security::AttestationEvidence ev{};
    ev.device_id = device.device_id;
    ev.nonce = "123456";
    ev.quote = "QU0xLW1vY2stYXR0ZXN0YXRpb24="; // base64 mock
    ev.claims["fw"] = device.firmware_version;
    auto att = security::AttestationVerifier::verify_quote(ev, "123456");
    if (!att.valid)
    {
        std::cerr << "Attestation failed: " << att.error << std::endl;
        return 1;
    }

    // Register device only after attestation passes
    if (!sdk.register_device(device))
    {
        std::cerr << "Device registration failed" << std::endl;
        return 1;
    }

    std::cout << "Device registered with DID: " << doc.id << std::endl;
    return 0;
}
