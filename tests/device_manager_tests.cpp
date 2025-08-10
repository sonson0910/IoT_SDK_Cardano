/**
 * @file device_manager_tests.cpp
 * @brief Unit tests for Device Manager module
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#include <gtest/gtest.h>
#include "cardano_iot/core/device_manager.h"
#include "utils/test_utils.h"

using namespace cardano_iot::core;

class DeviceManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        device_manager_ = std::make_unique<DeviceManager>();
        ASSERT_TRUE(device_manager_->initialize());

        // Create test device
        test_device_.device_id = "test_device_001";
        test_device_.device_type = "temperature_sensor";
        test_device_.manufacturer = "Test Corp";
        test_device_.model = "TS-100";
        test_device_.firmware_version = "v1.0.0";
        test_device_.hardware_revision = "rev1";
        test_device_.public_key = "test_public_key_123";
        test_device_.capabilities = static_cast<uint32_t>(DeviceCapability::SENSOR_DATA);
        test_device_.low_power_mode = false;
        test_device_.battery_level = 1.0;
    }

    void TearDown() override
    {
        device_manager_->shutdown();
    }

    std::unique_ptr<DeviceManager> device_manager_;
    Device test_device_;
};

TEST_F(DeviceManagerTest, InitializationAndShutdown)
{
    // Test that manager initializes correctly
    auto manager = std::make_unique<DeviceManager>();
    EXPECT_TRUE(manager->initialize());

    // Test shutdown
    manager->shutdown();

    // Test multiple initializations
    EXPECT_TRUE(manager->initialize());
    EXPECT_TRUE(manager->initialize()); // Should not fail
}

TEST_F(DeviceManagerTest, DeviceRegistration)
{
    // Test successful registration
    EXPECT_TRUE(device_manager_->register_device(test_device_));
    EXPECT_TRUE(device_manager_->is_device_registered(test_device_.device_id));

    // Test duplicate registration should fail
    EXPECT_FALSE(device_manager_->register_device(test_device_));

    // Test registration with invalid data
    Device invalid_device = test_device_;
    invalid_device.device_id = "";
    EXPECT_FALSE(device_manager_->register_device(invalid_device));

    invalid_device.device_id = "valid_id";
    invalid_device.public_key = "";
    EXPECT_FALSE(device_manager_->register_device(invalid_device));
}

TEST_F(DeviceManagerTest, DeviceUnregistration)
{
    // Register device first
    EXPECT_TRUE(device_manager_->register_device(test_device_));

    // Test successful unregistration
    EXPECT_TRUE(device_manager_->unregister_device(test_device_.device_id));
    EXPECT_FALSE(device_manager_->is_device_registered(test_device_.device_id));

    // Test unregistration of non-existent device
    EXPECT_FALSE(device_manager_->unregister_device("non_existent_device"));
}

TEST_F(DeviceManagerTest, DeviceRetrieval)
{
    // Test getting non-existent device
    EXPECT_EQ(device_manager_->get_device("non_existent"), nullptr);

    // Register device
    EXPECT_TRUE(device_manager_->register_device(test_device_));

    // Test getting existing device
    auto retrieved_device = device_manager_->get_device(test_device_.device_id);
    ASSERT_NE(retrieved_device, nullptr);
    EXPECT_EQ(retrieved_device->device_id, test_device_.device_id);
    EXPECT_EQ(retrieved_device->device_type, test_device_.device_type);
    EXPECT_EQ(retrieved_device->manufacturer, test_device_.manufacturer);
}

TEST_F(DeviceManagerTest, DeviceAuthentication)
{
    // Register device first
    EXPECT_TRUE(device_manager_->register_device(test_device_));

    // Generate challenge
    std::string challenge = device_manager_->generate_challenge(test_device_.device_id);
    EXPECT_FALSE(challenge.empty());

    // Test authentication with valid signature (mock)
    std::string mock_signature = "valid_signature_for_testing";
    EXPECT_TRUE(device_manager_->authenticate_device(test_device_.device_id, challenge, mock_signature));
    EXPECT_TRUE(device_manager_->is_device_authenticated(test_device_.device_id));

    // Test authentication with invalid signature
    EXPECT_FALSE(device_manager_->authenticate_device(test_device_.device_id, challenge, ""));

    // Test authentication with invalid challenge
    EXPECT_FALSE(device_manager_->authenticate_device(test_device_.device_id, "invalid_challenge", mock_signature));

    // Test authentication for non-existent device
    EXPECT_FALSE(device_manager_->authenticate_device("non_existent", challenge, mock_signature));
}

TEST_F(DeviceManagerTest, DeviceStatusManagement)
{
    // Register device
    EXPECT_TRUE(device_manager_->register_device(test_device_));

    // Test status update
    EXPECT_TRUE(device_manager_->update_device_status(test_device_.device_id, DeviceStatus::MAINTENANCE));

    auto device = device_manager_->get_device(test_device_.device_id);
    ASSERT_NE(device, nullptr);
    EXPECT_EQ(device->status, DeviceStatus::MAINTENANCE);

    // Test status update for non-existent device
    EXPECT_FALSE(device_manager_->update_device_status("non_existent", DeviceStatus::OFFLINE));
}

TEST_F(DeviceManagerTest, DeviceCapabilityFiltering)
{
    // Register multiple devices with different capabilities
    Device sensor_device = test_device_;
    sensor_device.device_id = "sensor_001";
    sensor_device.capabilities = static_cast<uint32_t>(DeviceCapability::SENSOR_DATA);

    Device actuator_device = test_device_;
    actuator_device.device_id = "actuator_001";
    actuator_device.capabilities = static_cast<uint32_t>(DeviceCapability::ACTUATOR_CONTROL);

    Device hybrid_device = test_device_;
    hybrid_device.device_id = "hybrid_001";
    hybrid_device.capabilities = static_cast<uint32_t>(DeviceCapability::SENSOR_DATA) |
                                 static_cast<uint32_t>(DeviceCapability::ACTUATOR_CONTROL);

    EXPECT_TRUE(device_manager_->register_device(sensor_device));
    EXPECT_TRUE(device_manager_->register_device(actuator_device));
    EXPECT_TRUE(device_manager_->register_device(hybrid_device));

    // Test filtering by sensor capability
    auto sensor_devices = device_manager_->get_devices_by_capability(DeviceCapability::SENSOR_DATA);
    EXPECT_EQ(sensor_devices.size(), 2); // sensor_device and hybrid_device

    // Test filtering by actuator capability
    auto actuator_devices = device_manager_->get_devices_by_capability(DeviceCapability::ACTUATOR_CONTROL);
    EXPECT_EQ(actuator_devices.size(), 2); // actuator_device and hybrid_device
}

TEST_F(DeviceManagerTest, DeviceStatusFiltering)
{
    // Register multiple devices
    for (int i = 0; i < 5; ++i)
    {
        Device device = test_device_;
        device.device_id = "device_" + std::to_string(i);
        EXPECT_TRUE(device_manager_->register_device(device));
    }

    // Set different statuses
    device_manager_->update_device_status("device_0", DeviceStatus::ONLINE);
    device_manager_->update_device_status("device_1", DeviceStatus::ONLINE);
    device_manager_->update_device_status("device_2", DeviceStatus::OFFLINE);
    device_manager_->update_device_status("device_3", DeviceStatus::MAINTENANCE);
    device_manager_->update_device_status("device_4", DeviceStatus::ERROR);

    // Test filtering by status
    auto online_devices = device_manager_->get_devices_by_status(DeviceStatus::ONLINE);
    EXPECT_EQ(online_devices.size(), 2);

    auto offline_devices = device_manager_->get_devices_by_status(DeviceStatus::OFFLINE);
    EXPECT_EQ(offline_devices.size(), 1);
}

TEST_F(DeviceManagerTest, PowerManagement)
{
    // Register device
    EXPECT_TRUE(device_manager_->register_device(test_device_));

    // Test low power mode
    EXPECT_TRUE(device_manager_->set_low_power_mode(test_device_.device_id, true));
    auto device = device_manager_->get_device(test_device_.device_id);
    ASSERT_NE(device, nullptr);
    EXPECT_TRUE(device->low_power_mode);

    // Test battery level update
    EXPECT_TRUE(device_manager_->update_battery_level(test_device_.device_id, 0.5));
    device = device_manager_->get_device(test_device_.device_id);
    ASSERT_NE(device, nullptr);
    EXPECT_DOUBLE_EQ(device->battery_level, 0.5);

    // Test battery level bounds
    EXPECT_TRUE(device_manager_->update_battery_level(test_device_.device_id, 1.5)); // Should clamp to 1.0
    device = device_manager_->get_device(test_device_.device_id);
    EXPECT_DOUBLE_EQ(device->battery_level, 1.0);

    EXPECT_TRUE(device_manager_->update_battery_level(test_device_.device_id, -0.1)); // Should clamp to 0.0
    device = device_manager_->get_device(test_device_.device_id);
    EXPECT_DOUBLE_EQ(device->battery_level, 0.0);
}

TEST_F(DeviceManagerTest, HeartbeatAndStaleDevices)
{
    // Register device
    EXPECT_TRUE(device_manager_->register_device(test_device_));

    // Update heartbeat
    device_manager_->update_heartbeat(test_device_.device_id);

    // Check for stale devices with short timeout
    auto stale_devices = device_manager_->get_stale_devices(0); // 0 seconds timeout
    EXPECT_EQ(stale_devices.size(), 1);                         // Device should be stale immediately

    // Update heartbeat again
    device_manager_->update_heartbeat(test_device_.device_id);

    // Check for stale devices with longer timeout
    stale_devices = device_manager_->get_stale_devices(3600); // 1 hour timeout
    EXPECT_EQ(stale_devices.size(), 0);                       // Device should not be stale
}

TEST_F(DeviceManagerTest, Statistics)
{
    // Register multiple devices
    for (int i = 0; i < 3; ++i)
    {
        Device device = test_device_;
        device.device_id = "device_" + std::to_string(i);
        EXPECT_TRUE(device_manager_->register_device(device));
    }

    // Authenticate one device
    std::string challenge = device_manager_->generate_challenge("device_0");
    device_manager_->authenticate_device("device_0", challenge, "valid_signature");

    // Get statistics
    auto stats = device_manager_->get_statistics();

    EXPECT_EQ(stats["total_devices"], 3);
    EXPECT_EQ(stats["total_registrations"], 3);
    EXPECT_EQ(stats["authenticated_devices"], 1);
    EXPECT_GE(stats["total_authentications"], 1);
}

TEST_F(DeviceManagerTest, EventCallbacks)
{
    bool event_received = false;
    std::string received_device_id;
    DeviceEvent received_event;

    // Set event callback
    device_manager_->set_event_callback(
        [&](const std::string &device_id, DeviceEvent event, const std::string &details)
        {
            event_received = true;
            received_device_id = device_id;
            received_event = event;
        });

    // Register device - should trigger event
    EXPECT_TRUE(device_manager_->register_device(test_device_));

    // Check that event was received
    EXPECT_TRUE(event_received);
    EXPECT_EQ(received_device_id, test_device_.device_id);
    EXPECT_EQ(received_event, DeviceEvent::REGISTERED);
}

TEST_F(DeviceManagerTest, UtilityFunctions)
{
    // Test capability to string conversion
    EXPECT_EQ(capability_to_string(DeviceCapability::SENSOR_DATA), "sensor_data");
    EXPECT_EQ(capability_to_string(DeviceCapability::ACTUATOR_CONTROL), "actuator_control");

    // Test status to string conversion
    EXPECT_EQ(status_to_string(DeviceStatus::ONLINE), "online");
    EXPECT_EQ(status_to_string(DeviceStatus::OFFLINE), "offline");

    // Test event to string conversion
    EXPECT_EQ(event_to_string(DeviceEvent::REGISTERED), "registered");
    EXPECT_EQ(event_to_string(DeviceEvent::AUTHENTICATED), "authenticated");

    // Test device ID generation
    std::string device_id = generate_device_id("TestCorp", "Model123", "SN001");
    EXPECT_FALSE(device_id.empty());
    EXPECT_TRUE(device_id.find("device_") == 0); // Should start with "device_"
}
