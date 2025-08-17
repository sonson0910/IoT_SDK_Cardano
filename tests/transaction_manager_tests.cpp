/**
 * @file transaction_manager_tests.cpp
 * @brief Unit tests for Transaction Manager module
 */

#include <gtest/gtest.h>
#include "cardano_iot/core/transaction_manager.h"

using namespace cardano_iot::core;

class TransactionManagerTest : public ::testing::Test
{
protected:
    TransactionManager tm_;

    void SetUp() override { ASSERT_TRUE(tm_.initialize("testnet")); }
    void TearDown() override { tm_.shutdown(); }
};

TEST_F(TransactionManagerTest, CreateSignSubmitPayment)
{
    // Load a mock wallet
    ASSERT_TRUE(tm_.import_wallet("test test test test test test test test test test test ball"));

    // From address derived from wallet key; for tests, generate via API
    std::string from_addr = tm_.address_from_public_key("vk_mock", "testnet");
    std::string to_addr = tm_.address_from_public_key("vk_mock_dest", "testnet");

    auto tx = tm_.create_payment_transaction(from_addr, to_addr, 1'000'000 /* 1 ADA */);
    ASSERT_NE(tx, nullptr);
    EXPECT_EQ(tx->type, TransactionType::PAYMENT);
    EXPECT_EQ(tx->status, TransactionStatus::PENDING);
    EXPECT_GE(tx->inputs.size(), 1u);
    EXPECT_GE(tx->outputs.size(), 1u);

    // Sign
    ASSERT_TRUE(tm_.sign_transaction(*tx, "signing_key_mock"));
    EXPECT_FALSE(tx->signed_cbor.empty());
    EXPECT_GE(tx->witnesses.size(), 1u);

    // Submit (mocked)
    std::string submit_res = tm_.submit_transaction(*tx);
    if (!submit_res.empty())
    {
        // Track status; may or may not be confirmed yet
        auto status = tm_.get_transaction_status(tx->tx_id);
        EXPECT_TRUE(status == TransactionStatus::SUBMITTED || status == TransactionStatus::CONFIRMED);
    }
    else
    {
        // Failure path should mark FAILED
        auto status = tm_.get_transaction_status(tx->tx_id);
        EXPECT_EQ(status, TransactionStatus::FAILED);
    }
}

TEST_F(TransactionManagerTest, EstimateAndCalculateFee)
{
    auto tx = std::make_unique<Transaction>();
    tx->inputs.resize(2);
    tx->outputs.resize(2);
    tx->metadata = std::make_unique<TransactionMetadata>();
    tx->metadata->json_metadata = std::string(128, 'x');

    auto est = tm_.estimate_fee(2, 2, 128);
    auto calc = tm_.calculate_fee(*tx);

    EXPECT_GT(est, 0u);
    EXPECT_GT(calc, 0u);
}
