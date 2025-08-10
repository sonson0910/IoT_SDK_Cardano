#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace cardano_iot
{
    namespace core
    {
        // Cryptographic algorithm types
        enum class CryptoAlgorithm
        {
            ED25519,          // Edwards-curve Digital Signature Algorithm
            ECDSA_SECP256K1,  // Elliptic Curve Digital Signature Algorithm
            BLS12_381,        // Boneh-Lynn-Shacham signature scheme
            AES_256_GCM,      // Advanced Encryption Standard with Galois/Counter Mode
            CHACHA20_POLY1305 // ChaCha20-Poly1305 authenticated encryption
        };

        // Key pair structure
        struct KeyPair
        {
            std::string public_key;
            std::string private_key;
            CryptoAlgorithm algorithm;
            uint64_t created_timestamp;
            std::string key_id;
        };

        // Digital signature structure
        struct DigitalSignature
        {
            std::string signature;
            std::string public_key;
            CryptoAlgorithm algorithm;
            std::string message_hash;
            uint64_t timestamp;
        };

        // Encryption result structure
        struct EncryptionResult
        {
            std::string encrypted_data;
            std::string nonce;
            std::string auth_tag;
            CryptoAlgorithm algorithm;
        };

        /**
         * @brief Cryptographic Manager for IoT devices
         *
         * Provides comprehensive cryptographic operations including:
         * - Key generation and management
         * - Digital signatures (ED25519, ECDSA, BLS)
         * - Symmetric encryption (AES, ChaCha20)
         * - Hash functions and message authentication
         * - Cardano-compatible cryptographic operations
         */
        class CryptoManager
        {
        public:
            CryptoManager();
            ~CryptoManager();

            // Initialize/shutdown
            bool initialize();
            void shutdown();

            // Key management
            std::unique_ptr<KeyPair> generate_key_pair(CryptoAlgorithm algorithm);
            bool store_key_pair(const std::string &device_id, std::unique_ptr<KeyPair> key_pair);
            std::shared_ptr<KeyPair> get_key_pair(const std::string &device_id);
            bool delete_key_pair(const std::string &device_id);
            std::vector<std::string> list_device_keys() const;

            // Digital signatures
            std::unique_ptr<DigitalSignature> sign_message(
                const std::string &message,
                const std::string &private_key,
                CryptoAlgorithm algorithm);
            bool verify_signature(const DigitalSignature &signature, const std::string &message);

            // Symmetric encryption
            std::unique_ptr<EncryptionResult> encrypt_data(
                const std::vector<uint8_t> &data,
                const std::string &key,
                CryptoAlgorithm algorithm);
            std::vector<uint8_t> decrypt_data(
                const EncryptionResult &encrypted,
                const std::string &key);

            // Hash functions
            std::string compute_hash(const std::string &data, const std::string &algorithm = "SHA256");
            std::string compute_hmac(const std::string &data, const std::string &key);

            // Cardano-specific operations
            std::string generate_cardano_address(const std::string &public_key, const std::string &network = "testnet");
            bool verify_cardano_signature(const std::string &signature, const std::string &message, const std::string &public_key);
            std::string create_cardano_transaction_hash(const std::string &transaction_data);

            // Random generation
            std::vector<uint8_t> generate_random_bytes(size_t length);
            std::string generate_secure_token(size_t length = 32);

            // Key derivation
            std::string derive_key(const std::string &master_key, const std::string &derivation_path);
            std::vector<uint8_t> pbkdf2(const std::string &password, const std::vector<uint8_t> &salt, int iterations);

            // Utility functions
            std::string encode_base64(const std::vector<uint8_t> &data);
            std::vector<uint8_t> decode_base64(const std::string &encoded);
            std::string encode_hex(const std::vector<uint8_t> &data);
            std::vector<uint8_t> decode_hex(const std::string &hex);

            // Statistics and diagnostics
            struct CryptoStats
            {
                uint64_t keys_generated;
                uint64_t signatures_created;
                uint64_t signatures_verified;
                uint64_t encryptions_performed;
                uint64_t decryptions_performed;
                uint64_t hashes_computed;
                uint64_t random_bytes_generated;
                double avg_sign_time_ms;
                double avg_verify_time_ms;
                double avg_encrypt_time_ms;
                double avg_decrypt_time_ms;
            };

            CryptoStats get_statistics() const;
            void reset_statistics();

        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl_;
        };

    } // namespace core
} // namespace cardano_iot
