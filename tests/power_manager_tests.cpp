/**
 * @file power_manager_tests.cpp
 * @brief Unit tests for Power Manager module
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#include <gtest/gtest.h>
#include "cardano_iot/energy/power_manager.h"
#include "utils/test_utils.h"

using namespace cardano_iot::energy;

class PowerManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        power_manager_ = std::make_unique<PowerManager>();
        ASSERT_TRUE(power_manager_->initialize());
    }

    void TearDown() override
    {
        power_manager_->shutdown();
    }

    std::unique_ptr<PowerManager> power_manager_;
};

TEST_F(PowerManagerTest, DeviceRegistration)
{
    PowerSettings settings;
    settings.enable_optimization = true;

    EXPECT_TRUE(power_manager_->register_device("test_device", settings));
    EXPECT_FALSE(power_manager_->register_device("test_device", settings)); // Duplicate
}

TEST_F(PowerManagerTest, PowerStateManagement)
{
    PowerSettings settings;
    power_manager_->register_device("test_device", settings);

    EXPECT_TRUE(power_manager_->set_power_state("test_device", PowerState::SLEEP));
    EXPECT_EQ(power_manager_->get_power_state("test_device"), PowerState::SLEEP);
}

TEST_F(PowerManagerTest, BatteryManagement)
{
    PowerSettings settings;
    power_manager_->register_device("test_device", settings);

    BatteryInfo battery;
    battery.charge_level = 0.8;
    battery.voltage = 3.7;

    EXPECT_TRUE(power_manager_->update_battery_info("test_device", battery));
    EXPECT_DOUBLE_EQ(power_manager_->get_battery_level("test_device"), 0.8);
}
