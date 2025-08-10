/**
 * @file test_utils.h
 * @brief Test utilities for Cardano IoT SDK tests
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#ifndef CARDANO_IOT_TEST_UTILS_H
#define CARDANO_IOT_TEST_UTILS_H

#include "cardano_iot/cardano_iot.h"
#include <functional>
#include <chrono>
#include <thread>

using namespace cardano_iot;

namespace cardano_iot::test
{

    /**
     * @brief Generate random device ID for testing
     * @return Random device ID
     */
    std::string generate_random_device_id();

    /**
     * @brief Generate random public key for testing
     * @return Random public key string
     */
    std::string generate_random_public_key();

    /**
     * @brief Create test device with default values
     * @param device_id Optional device ID (random if empty)
     * @return Test device structure
     */
    core::Device create_test_device(const std::string &device_id = "");

    /**
     * @brief Create test device info for SDK
     * @param device_id Optional device ID (random if empty)
     * @return Test device info structure
     */
    CardanoIoTSDK::DeviceInfo create_test_device_info(const std::string &device_id = "");

    /**
     * @brief Create test IoT data
     * @param device_id Optional device ID (random if empty)
     * @return Test IoT data structure
     */
    CardanoIoTSDK::IoTData create_test_iot_data(const std::string &device_id = "");

    /**
     * @brief Create test power profile
     * @param device_id Optional device ID (random if empty)
     * @return Test power profile
     */
    energy::PowerProfile create_test_power_profile(const std::string &device_id = "");

    /**
     * @brief Create test power settings
     * @return Test power settings
     */
    energy::PowerSettings create_test_power_settings();

    /**
     * @brief Mock Cardano client for testing
     */
    class MockCardanoClient
    {
    public:
        MockCardanoClient();

        bool initialize();
        void shutdown();
        bool is_connected() const;

        std::string submit_transaction(const std::string &tx_data);
        uint64_t get_balance(const std::string &address) const;

    private:
        bool connected_;
    };

    /**
     * @brief Wait for a condition to become true with timeout
     * @param condition Function that returns true when condition is met
     * @param timeout_ms Timeout in milliseconds
     * @param check_interval_ms Check interval in milliseconds
     * @return true if condition was met within timeout
     */
    bool wait_for_condition(std::function<bool()> condition,
                            uint32_t timeout_ms = 5000,
                            uint32_t check_interval_ms = 100);

    /**
     * @brief Simulate passage of time for testing
     * @param seconds Number of seconds to simulate
     */
    void simulate_time_passage(uint32_t seconds);

    /**
     * @brief RAII helper for test setup/teardown
     */
    template <typename SetupFunc, typename TeardownFunc>
    class TestFixture
    {
    public:
        TestFixture(SetupFunc setup, TeardownFunc teardown)
            : teardown_(teardown)
        {
            setup();
        }

        ~TestFixture()
        {
            teardown_();
        }

    private:
        TeardownFunc teardown_;
    };

    /**
     * @brief Create test fixture with setup/teardown
     */
    template <typename SetupFunc, typename TeardownFunc>
    auto make_test_fixture(SetupFunc &&setup, TeardownFunc &&teardown)
    {
        return TestFixture<SetupFunc, TeardownFunc>(
            std::forward<SetupFunc>(setup),
            std::forward<TeardownFunc>(teardown));
    }

} // namespace cardano_iot::test

#endif // CARDANO_IOT_TEST_UTILS_H
