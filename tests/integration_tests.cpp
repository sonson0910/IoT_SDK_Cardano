/**
 * @file integration_tests.cpp
 * @brief Integration tests for the complete SDK
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#include <gtest/gtest.h>
#include "cardano_iot/cardano_iot.h"
#include "utils/test_utils.h"

using namespace cardano_iot;

class IntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        CardanoIoTSDK::Config config;
        config.network_type = "testnet";
        config.enable_logging = false; // Quiet during tests

        sdk_ = std::make_unique<CardanoIoTSDK>(config);
        ASSERT_TRUE(sdk_->initialize());
    }

    void TearDown() override
    {
        sdk_->shutdown();
    }

    std::unique_ptr<CardanoIoTSDK> sdk_;
};

TEST_F(IntegrationTest, EndToEndDeviceWorkflow)
{
    // Test complete device workflow
    CardanoIoTSDK::DeviceInfo device = cardano_iot::test::create_test_device_info("integration_test_001");

    EXPECT_TRUE(sdk_->register_device(device));

    CardanoIoTSDK::IoTData data = cardano_iot::test::create_test_iot_data("integration_test_001");
    std::string tx_id = sdk_->submit_data(data);

    EXPECT_FALSE(tx_id.empty());
}

TEST_F(IntegrationTest, MultiDeviceIntegration)
{
    // Test multiple devices
    for (int i = 0; i < 3; ++i)
    {
        std::string device_id = "integration_device_" + std::to_string(i);
        CardanoIoTSDK::DeviceInfo device = cardano_iot::test::create_test_device_info(device_id);

        EXPECT_TRUE(sdk_->register_device(device));
    }

    auto devices = sdk_->get_registered_devices();
    EXPECT_EQ(devices.size(), 3);
}

TEST_F(IntegrationTest, DataQueryIntegration)
{
    // Test data submission and querying
    std::string device_id = "query_test_device";
    CardanoIoTSDK::DeviceInfo device = cardano_iot::test::create_test_device_info(device_id);
    sdk_->register_device(device);

    CardanoIoTSDK::IoTData data = cardano_iot::test::create_test_iot_data(device_id);
    std::string tx_id = sdk_->submit_data(data);

    auto results = sdk_->query_data(device_id, 0, time(nullptr) + 3600);
    EXPECT_GE(results.size(), 1);
}
