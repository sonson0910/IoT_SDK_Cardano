#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <cstdint>

namespace cardano_iot
{
    namespace security
    {
        enum class EncryptionAlgorithm
        {
            AES_256_GCM,
            AES_256_CBC,
            CHACHA20_POLY1305,
            RSA_OAEP,
            ECIES,
            XCHACHA20_POLY1305
        };

        enum class KeyDerivationFunction
        {
            PBKDF2,
            SCRYPT,
            ARGON2,
            HKDF
        };

        struct EncryptionKey
        {
            std::string key_id;
            EncryptionAlgorithm algorithm;
            std::vector<uint8_t> key_data;
            std::vector<uint8_t> salt;
            uint64_t created_timestamp;
            uint64_t expiry_timestamp;
            std::string device_id;
            std::map<std::string, std::string> metadata;
        };

        struct EncryptedData
        {
            EncryptionAlgorithm algorithm;
            std::vector<uint8_t> ciphertext;
            std::vector<uint8_t> nonce;
            std::vector<uint8_t> auth_tag;
            std::string key_id;
            uint64_t timestamp;
        };

        struct KeyDerivationParams
        {
            KeyDerivationFunction function;
            std::vector<uint8_t> salt;
            uint32_t iterations;
            uint32_t memory_cost;
            uint32_t parallelism;
            uint32_t key_length;
        };

        class Encryption
        {
        public:
            Encryption();
            ~Encryption();

            bool initialize();
            void shutdown();

            // Key management
            std::string generate_key(EncryptionAlgorithm algorithm, const std::string& device_id = "");
            bool store_key(const EncryptionKey& key);
            std::shared_ptr<EncryptionKey> get_key(const std::string& key_id) const;
            bool delete_key(const std::string& key_id);
            std::vector<std::string> list_keys(const std::string& device_id = "") const;

            // Key derivation
            std::vector<uint8_t> derive_key(const std::string& password, const KeyDerivationParams& params) const;
            std::vector<uint8_t> derive_key_simple(const std::string& password, const std::vector<uint8_t>& salt, uint32_t iterations = 10000) const;

            // Symmetric encryption
            EncryptedData encrypt(const std::vector<uint8_t>& plaintext, const std::string& key_id);
            std::vector<uint8_t> decrypt(const EncryptedData& encrypted_data);
            EncryptedData encrypt_with_password(const std::vector<uint8_t>& plaintext, const std::string& password);
            std::vector<uint8_t> decrypt_with_password(const EncryptedData& encrypted_data, const std::string& password);

            // Stream encryption
            bool start_stream_encryption(const std::string& stream_id, const std::string& key_id);
            std::vector<uint8_t> encrypt_stream_chunk(const std::string& stream_id, const std::vector<uint8_t>& chunk);
            std::vector<uint8_t> decrypt_stream_chunk(const std::string& stream_id, const std::vector<uint8_t>& encrypted_chunk);
            void end_stream_encryption(const std::string& stream_id);

            // File encryption
            bool encrypt_file(const std::string& input_path, const std::string& output_path, const std::string& key_id);
            bool decrypt_file(const std::string& input_path, const std::string& output_path, const std::string& key_id);
            bool secure_delete_file(const std::string& file_path);

            // Memory encryption
            std::vector<uint8_t> encrypt_memory_region(const void* data, size_t size, const std::string& key_id);
            bool decrypt_to_memory_region(const EncryptedData& encrypted_data, void* output_buffer, size_t buffer_size);
            void secure_zero_memory(void* data, size_t size);

            // Key exchange
            std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_key_pair() const;
            std::vector<uint8_t> perform_key_exchange(const std::vector<uint8_t>& private_key, const std::vector<uint8_t>& public_key) const;

            // Integrity and authentication
            std::vector<uint8_t> compute_hmac(const std::vector<uint8_t>& data, const std::string& key_id) const;
            bool verify_hmac(const std::vector<uint8_t>& data, const std::vector<uint8_t>& hmac, const std::string& key_id) const;
            std::vector<uint8_t> compute_hash(const std::vector<uint8_t>& data, const std::string& algorithm = "SHA256") const;

            // Configuration
            struct EncryptionConfig
            {
                uint32_t default_key_size = 32;
                uint32_t default_iterations = 10000;
                bool secure_random = true;
                bool memory_protection = true;
                bool key_rotation_enabled = false;
                uint32_t key_rotation_interval_hours = 24;
                std::string default_algorithm = "AES_256_GCM";
            };

            void update_config(const EncryptionConfig& config);
            EncryptionConfig get_config() const;

            // Statistics
            struct EncryptionStats
            {
                uint64_t keys_generated;
                uint64_t keys_stored;
                uint64_t keys_deleted;
                uint64_t encryptions_performed;
                uint64_t decryptions_performed;
                uint64_t bytes_encrypted;
                uint64_t bytes_decrypted;
                double avg_encryption_time_ms;
                double avg_decryption_time_ms;
                uint64_t stream_sessions;
                uint64_t key_exchanges;
            };

            EncryptionStats get_statistics() const;
            void reset_statistics();

            // Utility functions
            std::vector<uint8_t> generate_random_bytes(size_t length) const;
            std::string encode_base64(const std::vector<uint8_t>& data) const;
            std::vector<uint8_t> decode_base64(const std::string& encoded) const;
            std::string encode_hex(const std::vector<uint8_t>& data) const;
            std::vector<uint8_t> decode_hex(const std::string& hex) const;

        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl_;
        };

    } // namespace security
} // namespace cardano_iot
