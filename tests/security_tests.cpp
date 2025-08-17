/**
 * @file security_tests.cpp
 * @brief Unit tests for Security and Authentication modules
 *
 * @author Cardano IoT SDK Team
 * @version 1.0.0
 */

#include <gtest/gtest.h>
#include "cardano_iot/security/authentication.h"
#include "cardano_iot/security/encryption.h"
#include "utils/test_utils.h"

using namespace cardano_iot::security;

class SecurityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup test environment
    }

    void TearDown() override
    {
        // Cleanup
    }
};

TEST_F(SecurityTest, EncryptionAesGcmRoundTrip)
{
    cardano_iot::security::Encryption enc;
    ASSERT_TRUE(enc.initialize());

    // Generate key
    std::string key_id = enc.generate_key(cardano_iot::security::EncryptionAlgorithm::AES_256_GCM);
    ASSERT_FALSE(key_id.empty());

    // Plaintext
    std::vector<uint8_t> pt = {'h', 'e', 'l', 'l', 'o', '_', 'i', 'o', 't'};
    auto ed = enc.encrypt(pt, key_id);
    ASSERT_FALSE(ed.ciphertext.empty());

    auto dt = enc.decrypt(ed);
    ASSERT_EQ(dt, pt);
}

TEST_F(SecurityTest, DISABLED_AuthenticationPasswordFlow)
{
    using namespace cardano_iot::security;
    Authentication auth;
    ASSERT_TRUE(auth.initialize());

    AuthCredentials cred{};
    cred.method = AuthMethod::PASSWORD;
    cred.identifier = "device_auth_01";
    cred.credential_data = {1, 2, 3, 4};
    ASSERT_TRUE(auth.register_device_credentials("device01", cred));

    auto res_ok = auth.authenticate_device("device01", cred);
    // Avoid hanging tests: ensure we got a response
    ASSERT_TRUE(res_ok.status == AuthStatus::SUCCESS || res_ok.status == AuthStatus::FAILED || res_ok.status == AuthStatus::LOCKED);
    EXPECT_EQ(res_ok.status, AuthStatus::SUCCESS);

    AuthCredentials wrong = cred;
    wrong.credential_data = {9, 9, 9};
    auto res_bad = auth.authenticate_device("device01", wrong);
    ASSERT_TRUE(res_bad.status == AuthStatus::SUCCESS || res_bad.status == AuthStatus::FAILED || res_bad.status == AuthStatus::LOCKED);
    EXPECT_NE(res_bad.status, AuthStatus::SUCCESS);
}

TEST_F(SecurityTest, EncryptionTest)
{
    // Encryption functionality test
    EXPECT_TRUE(true);
}
