/**
 * @file network_tests.cpp
 * @brief Unit tests for Network modules
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#include <gtest/gtest.h>
#include "cardano_iot/network/cardano_client.h"

using namespace cardano_iot::network;

class NetworkTest : public ::testing::Test
{
protected:
    CardanoClient client_;

    void SetUp() override
    {
        ASSERT_TRUE(client_.initialize("/tmp/cardano-node.socket", Network::PREPROD));
    }

    void TearDown() override { client_.shutdown(); }
};

TEST_F(NetworkTest, InitializeAndStatus)
{
    // Should start disconnected
    EXPECT_FALSE(client_.is_connected());
    EXPECT_EQ(client_.get_connection_status(), ConnectionStatus::DISCONNECTED);
}

TEST_F(NetworkTest, ConnectAndQueryUTXOs)
{
    bool connected = client_.connect();

    if (connected)
    {
        EXPECT_TRUE(client_.is_connected());

        // Node info should be populated
        auto info = client_.get_node_info();
        EXPECT_FALSE(info.version.empty());
        EXPECT_EQ(info.network, "preprod");

        // Query UTXOs for a mock address
        std::string addr = "addr_test1qpsometestaddress000000000000000";
        auto utxos = client_.query_utxos(addr);
        EXPECT_GE(utxos.size(), 1u);

        // Balance should be sum of UTXOs
        auto balance = client_.get_address_balance(addr);
        uint64_t manual_sum = 0;
        for (const auto &u : utxos)
            manual_sum += u.amount;
        EXPECT_EQ(balance, manual_sum);
    }
    else
    {
        // On failure path, still stable behavior
        EXPECT_FALSE(client_.is_connected());
        auto info = client_.get_node_info();
        EXPECT_TRUE(info.version.empty());
        auto utxos = client_.query_utxos("addr_test1qpfail");
        EXPECT_EQ(utxos.size(), 0u);
    }
}

TEST_F(NetworkTest, SubmitTransactionResultShape)
{
    bool connected = client_.connect();
    auto res = client_.submit_transaction("cbor_deadbeef");
    if (connected)
    {
        // May succeed or fail; check shape
        if (res.success)
        {
            EXPECT_FALSE(res.tx_hash.empty());
            EXPECT_TRUE(res.error_message.empty());
        }
        else
        {
            EXPECT_TRUE(res.tx_hash.empty());
            EXPECT_FALSE(res.error_message.empty());
        }
    }
    else
    {
        EXPECT_FALSE(res.success);
        EXPECT_TRUE(res.tx_hash.empty());
        EXPECT_FALSE(res.error_message.empty());
    }
}

TEST_F(NetworkTest, SlotToTimestamp)
{
    // Genesis time per implementation
    const uint64_t genesis = 1596059091ULL;

    // Slot 0 => genesis
    EXPECT_EQ(client_.slot_to_timestamp(0), genesis);

    // A few arbitrary slots
    EXPECT_EQ(client_.slot_to_timestamp(1), genesis + 1);
    EXPECT_EQ(client_.slot_to_timestamp(10), genesis + 10);
    EXPECT_EQ(client_.slot_to_timestamp(12345), genesis + 12345);
}

TEST_F(NetworkTest, ValidateAddress)
{
    // Valid prefixes
    EXPECT_TRUE(client_.validate_address("addr1qpxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"));
    EXPECT_TRUE(client_.validate_address("addr_test1qpxxxxxxxxxxxxxxxxxxxxxxxxxx"));
    EXPECT_TRUE(client_.validate_address("stake1uxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"));
    EXPECT_TRUE(client_.validate_address("stake_test1uxxxxxxxxxxxxxxxxxxxxxxxxxx"));

    // Too short
    EXPECT_FALSE(client_.validate_address("addr1short"));
    EXPECT_FALSE(client_.validate_address("stake1short"));

    // Wrong prefix but long
    EXPECT_FALSE(client_.validate_address("zzzz1thisislongenoughbutinvalidprefix"));
}
