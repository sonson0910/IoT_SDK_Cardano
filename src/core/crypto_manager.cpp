#include "cardano_iot/core/crypto_manager.h"
#include "cardano_iot/utils/logger.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/aes.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <chrono>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <algorithm>
#include <random>

namespace cardano_iot
{
    namespace core
    {
        // PIMPL implementation
        struct CryptoManager::Impl
        {
            bool initialized_ = false;
            std::mutex crypto_mutex_;
            std::map<std::string, std::shared_ptr<KeyPair>> device_keys_;

            // Statistics
            mutable std::mutex stats_mutex_;
            CryptoStats stats_ = {};

            // Timing helpers
            std::chrono::high_resolution_clock::time_point start_time_;

            void start_timing()
            {
                start_time_ = std::chrono::high_resolution_clock::now();
            }

            double get_elapsed_ms()
            {
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time_);
                return duration.count() / 1000.0; // Convert to milliseconds
            }

            // Generate unique key ID
            std::string generate_key_id()
            {
                auto now = std::chrono::system_clock::now();
                auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1000, 9999);

                std::stringstream ss;
                ss << "key_" << timestamp << "_" << dis(gen);
                return ss.str();
            }

            // Convert bytes to hex string
            std::string bytes_to_hex(const std::vector<uint8_t> &bytes)
            {
                std::stringstream ss;
                ss << std::hex << std::setfill('0');
                for (uint8_t byte : bytes)
                {
                    ss << std::setw(2) << static_cast<int>(byte);
                }
                return ss.str();
            }

            // Convert hex string to bytes
            std::vector<uint8_t> hex_to_bytes(const std::string &hex)
            {
                std::vector<uint8_t> bytes;
                for (size_t i = 0; i < hex.length(); i += 2)
                {
                    std::string byteString = hex.substr(i, 2);
                    uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
                    bytes.push_back(byte);
                }
                return bytes;
            }

            // Base64 encoding/decoding
            std::string base64_encode(const std::vector<uint8_t> &data)
            {
                BIO *bio = BIO_new(BIO_s_mem());
                BIO *b64 = BIO_new(BIO_f_base64());
                BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
                bio = BIO_push(b64, bio);

                BIO_write(bio, data.data(), data.size());
                BIO_flush(bio);

                BUF_MEM *buffer_ptr;
                BIO_get_mem_ptr(bio, &buffer_ptr);

                std::string result(buffer_ptr->data, buffer_ptr->length);
                BIO_free_all(bio);

                return result;
            }

            std::vector<uint8_t> base64_decode(const std::string &encoded)
            {
                BIO *bio = BIO_new_mem_buf(encoded.c_str(), -1);
                BIO *b64 = BIO_new(BIO_f_base64());
                BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
                bio = BIO_push(b64, bio);

                std::vector<uint8_t> result(encoded.length());
                int decoded_length = BIO_read(bio, result.data(), encoded.length());
                result.resize(decoded_length);

                BIO_free_all(bio);
                return result;
            }

            // Generate ED25519 key pair
            std::unique_ptr<KeyPair> generate_ed25519_keypair()
            {
                EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr);
                if (!ctx)
                    return nullptr;

                if (EVP_PKEY_keygen_init(ctx) <= 0)
                {
                    EVP_PKEY_CTX_free(ctx);
                    return nullptr;
                }

                EVP_PKEY *pkey = nullptr;
                if (EVP_PKEY_keygen(ctx, &pkey) <= 0)
                {
                    EVP_PKEY_CTX_free(ctx);
                    return nullptr;
                }

                // Extract public key
                size_t pub_len = 32;
                std::vector<uint8_t> pub_key(pub_len);
                if (EVP_PKEY_get_raw_public_key(pkey, pub_key.data(), &pub_len) <= 0)
                {
                    EVP_PKEY_free(pkey);
                    EVP_PKEY_CTX_free(ctx);
                    return nullptr;
                }

                // Extract private key
                size_t priv_len = 32;
                std::vector<uint8_t> priv_key(priv_len);
                if (EVP_PKEY_get_raw_private_key(pkey, priv_key.data(), &priv_len) <= 0)
                {
                    EVP_PKEY_free(pkey);
                    EVP_PKEY_CTX_free(ctx);
                    return nullptr;
                }

                auto keypair = std::make_unique<KeyPair>();
                keypair->public_key = bytes_to_hex(pub_key);
                keypair->private_key = bytes_to_hex(priv_key);
                keypair->algorithm = CryptoAlgorithm::ED25519;
                keypair->created_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                                 std::chrono::system_clock::now().time_since_epoch())
                                                 .count();
                keypair->key_id = generate_key_id();

                EVP_PKEY_free(pkey);
                EVP_PKEY_CTX_free(ctx);

                return keypair;
            }

            // Generate random bytes using OpenSSL
            std::vector<uint8_t> generate_secure_random(size_t length)
            {
                std::vector<uint8_t> buffer(length);
                if (RAND_bytes(buffer.data(), length) != 1)
                {
                    // Fallback to system random if OpenSSL fails
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dis(0, 255);
                    for (size_t i = 0; i < length; ++i)
                    {
                        buffer[i] = static_cast<uint8_t>(dis(gen));
                    }
                }
                return buffer;
            }

            // Compute SHA256 hash
            std::string compute_sha256(const std::string &data)
            {
                std::vector<uint8_t> hash(SHA256_DIGEST_LENGTH);
                SHA256(reinterpret_cast<const unsigned char *>(data.c_str()), data.length(), hash.data());
                return bytes_to_hex(hash);
            }

            // Generate mock Cardano address (simplified for demo)
            std::string generate_mock_cardano_address(const std::string &public_key, const std::string &network)
            {
                std::string prefix = (network == "mainnet") ? "addr1" : "addr_test1";

                // Simplified address generation - in real implementation would follow Cardano spec
                std::string combined = prefix + public_key.substr(0, 16);
                std::string hash = compute_sha256(combined);

                return prefix + hash.substr(0, 32);
            }
        };

        // Constructor/Destructor
        CryptoManager::CryptoManager() : pimpl_(std::make_unique<Impl>()) {}
        CryptoManager::~CryptoManager() = default;

        bool CryptoManager::initialize()
        {
            std::lock_guard<std::mutex> lock(pimpl_->crypto_mutex_);

            if (pimpl_->initialized_)
            {
                return true;
            }

            // Initialize OpenSSL
            if (!RAND_status())
            {
                utils::Logger::instance().log(utils::LogLevel::WARNING, "CryptoManager",
                                              "OpenSSL random generator not properly seeded");
            }

            pimpl_->initialized_ = true;

            utils::Logger::instance().log(utils::LogLevel::INFO, "CryptoManager",
                                          "Crypto manager initialized successfully");
            return true;
        }

        void CryptoManager::shutdown()
        {
            std::lock_guard<std::mutex> lock(pimpl_->crypto_mutex_);

            if (!pimpl_->initialized_)
            {
                return;
            }

            // Clear all stored keys
            pimpl_->device_keys_.clear();
            pimpl_->initialized_ = false;

            utils::Logger::instance().log(utils::LogLevel::INFO, "CryptoManager",
                                          "Crypto manager shut down");
        }

        std::unique_ptr<KeyPair> CryptoManager::generate_key_pair(CryptoAlgorithm algorithm)
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
            pimpl_->start_timing();

            std::unique_ptr<KeyPair> result;

            switch (algorithm)
            {
            case CryptoAlgorithm::ED25519:
                result = pimpl_->generate_ed25519_keypair();
                break;

            case CryptoAlgorithm::ECDSA_SECP256K1:
                // Simplified implementation - would use proper ECDSA in real version
                result = pimpl_->generate_ed25519_keypair(); // Fallback for demo
                if (result)
                    result->algorithm = CryptoAlgorithm::ECDSA_SECP256K1;
                break;

            case CryptoAlgorithm::BLS12_381:
                // Simplified implementation - would use proper BLS in real version
                result = pimpl_->generate_ed25519_keypair(); // Fallback for demo
                if (result)
                    result->algorithm = CryptoAlgorithm::BLS12_381;
                break;

            default:
                utils::Logger::instance().log(utils::LogLevel::ERROR, "CryptoManager",
                                              "Unsupported key generation algorithm");
                return nullptr;
            }

            if (result)
            {
                pimpl_->stats_.keys_generated++;
                utils::Logger::instance().log(utils::LogLevel::INFO, "CryptoManager",
                                              "Generated key pair: " + result->key_id);
            }

            return result;
        }

        bool CryptoManager::store_key_pair(const std::string &device_id, std::unique_ptr<KeyPair> key_pair)
        {
            if (!pimpl_->initialized_ || !key_pair)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->crypto_mutex_);

            auto shared_key = std::shared_ptr<KeyPair>(key_pair.release());
            pimpl_->device_keys_[device_id] = shared_key;

            utils::Logger::instance().log(utils::LogLevel::INFO, "CryptoManager",
                                          "Stored key pair for device: " + device_id);
            return true;
        }

        std::shared_ptr<KeyPair> CryptoManager::get_key_pair(const std::string &device_id)
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            std::lock_guard<std::mutex> lock(pimpl_->crypto_mutex_);

            auto it = pimpl_->device_keys_.find(device_id);
            if (it != pimpl_->device_keys_.end())
            {
                return it->second;
            }

            return nullptr;
        }

        bool CryptoManager::delete_key_pair(const std::string &device_id)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->crypto_mutex_);

            auto it = pimpl_->device_keys_.find(device_id);
            if (it != pimpl_->device_keys_.end())
            {
                pimpl_->device_keys_.erase(it);
                utils::Logger::instance().log(utils::LogLevel::INFO, "CryptoManager",
                                              "Deleted key pair for device: " + device_id);
                return true;
            }

            return false;
        }

        std::vector<std::string> CryptoManager::list_device_keys() const
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->crypto_mutex_);

            std::vector<std::string> device_ids;
            for (const auto &[device_id, key_pair] : pimpl_->device_keys_)
            {
                device_ids.push_back(device_id);
            }

            return device_ids;
        }

        std::unique_ptr<DigitalSignature> CryptoManager::sign_message(
            const std::string &message,
            const std::string &private_key,
            CryptoAlgorithm algorithm)
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
            pimpl_->start_timing();

            auto signature = std::make_unique<DigitalSignature>();

            // Simplified signature generation for demo
            std::string combined = message + private_key;
            signature->signature = pimpl_->compute_sha256(combined);
            signature->algorithm = algorithm;
            signature->message_hash = pimpl_->compute_sha256(message);
            signature->timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                       std::chrono::system_clock::now().time_since_epoch())
                                       .count();

            double elapsed = pimpl_->get_elapsed_ms();
            pimpl_->stats_.signatures_created++;
            pimpl_->stats_.avg_sign_time_ms = (pimpl_->stats_.avg_sign_time_ms + elapsed) / 2.0;

            utils::Logger::instance().log(utils::LogLevel::INFO, "CryptoManager",
                                          "Created digital signature");

            return signature;
        }

        bool CryptoManager::verify_signature(const DigitalSignature &signature, const std::string &message)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
            pimpl_->start_timing();

            // Simplified verification for demo - check message hash
            std::string computed_hash = pimpl_->compute_sha256(message);
            bool valid = (computed_hash == signature.message_hash);

            double elapsed = pimpl_->get_elapsed_ms();
            pimpl_->stats_.signatures_verified++;
            pimpl_->stats_.avg_verify_time_ms = (pimpl_->stats_.avg_verify_time_ms + elapsed) / 2.0;

            utils::Logger::instance().log(utils::LogLevel::INFO, "CryptoManager",
                                          valid ? "Signature verified successfully" : "Signature verification failed");

            return valid;
        }

        std::unique_ptr<EncryptionResult> CryptoManager::encrypt_data(
            const std::vector<uint8_t> &data,
            const std::string &key,
            CryptoAlgorithm algorithm)
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
            pimpl_->start_timing();

            auto result = std::make_unique<EncryptionResult>();

            // Generate random nonce
            auto nonce = pimpl_->generate_secure_random(16);
            result->nonce = pimpl_->bytes_to_hex(nonce);

            // Simplified encryption (XOR with key hash for demo)
            std::string key_hash = pimpl_->compute_sha256(key);
            auto key_bytes = pimpl_->hex_to_bytes(key_hash);

            std::vector<uint8_t> encrypted(data.size());
            for (size_t i = 0; i < data.size(); ++i)
            {
                encrypted[i] = data[i] ^ key_bytes[i % key_bytes.size()];
            }

            result->encrypted_data = pimpl_->bytes_to_hex(encrypted);
            result->auth_tag = pimpl_->compute_sha256(result->encrypted_data + result->nonce);
            result->algorithm = algorithm;

            double elapsed = pimpl_->get_elapsed_ms();
            pimpl_->stats_.encryptions_performed++;
            pimpl_->stats_.avg_encrypt_time_ms = (pimpl_->stats_.avg_encrypt_time_ms + elapsed) / 2.0;

            utils::Logger::instance().log(utils::LogLevel::INFO, "CryptoManager",
                                          "Data encrypted successfully");

            return result;
        }

        std::vector<uint8_t> CryptoManager::decrypt_data(
            const EncryptionResult &encrypted,
            const std::string &key)
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
            pimpl_->start_timing();

            // Verify auth tag
            std::string computed_tag = pimpl_->compute_sha256(encrypted.encrypted_data + encrypted.nonce);
            if (computed_tag != encrypted.auth_tag)
            {
                utils::Logger::instance().log(utils::LogLevel::ERROR, "CryptoManager",
                                              "Authentication tag verification failed");
                return {};
            }

            // Decrypt data (reverse XOR operation)
            std::string key_hash = pimpl_->compute_sha256(key);
            auto key_bytes = pimpl_->hex_to_bytes(key_hash);
            auto encrypted_bytes = pimpl_->hex_to_bytes(encrypted.encrypted_data);

            std::vector<uint8_t> decrypted(encrypted_bytes.size());
            for (size_t i = 0; i < encrypted_bytes.size(); ++i)
            {
                decrypted[i] = encrypted_bytes[i] ^ key_bytes[i % key_bytes.size()];
            }

            double elapsed = pimpl_->get_elapsed_ms();
            pimpl_->stats_.decryptions_performed++;
            pimpl_->stats_.avg_decrypt_time_ms = (pimpl_->stats_.avg_decrypt_time_ms + elapsed) / 2.0;

            utils::Logger::instance().log(utils::LogLevel::INFO, "CryptoManager",
                                          "Data decrypted successfully");

            return decrypted;
        }

        std::string CryptoManager::compute_hash(const std::string &data, const std::string &algorithm)
        {
            if (!pimpl_->initialized_)
            {
                return "";
            }

            std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);

            std::string result;
            if (algorithm == "SHA256" || algorithm == "sha256")
            {
                result = pimpl_->compute_sha256(data);
            }
            else
            {
                // Default to SHA256
                result = pimpl_->compute_sha256(data);
            }

            pimpl_->stats_.hashes_computed++;
            return result;
        }

        std::string CryptoManager::compute_hmac(const std::string &data, const std::string &key)
        {
            if (!pimpl_->initialized_)
            {
                return "";
            }

            std::vector<uint8_t> result(SHA256_DIGEST_LENGTH);
            unsigned int len;

            HMAC(EVP_sha256(), key.c_str(), key.length(),
                 reinterpret_cast<const unsigned char *>(data.c_str()), data.length(),
                 result.data(), &len);

            return pimpl_->bytes_to_hex(result);
        }

        std::string CryptoManager::generate_cardano_address(const std::string &public_key, const std::string &network)
        {
            return pimpl_->generate_mock_cardano_address(public_key, network);
        }

        bool CryptoManager::verify_cardano_signature(const std::string &signature, const std::string &message, const std::string &public_key)
        {
            // Simplified verification for demo
            std::string expected = pimpl_->compute_sha256(message + public_key);
            return (signature.find(expected.substr(0, 16)) != std::string::npos);
        }

        std::string CryptoManager::create_cardano_transaction_hash(const std::string &transaction_data)
        {
            return pimpl_->compute_sha256(transaction_data);
        }

        std::vector<uint8_t> CryptoManager::generate_random_bytes(size_t length)
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
            auto result = pimpl_->generate_secure_random(length);
            pimpl_->stats_.random_bytes_generated += length;
            return result;
        }

        std::string CryptoManager::generate_secure_token(size_t length)
        {
            auto bytes = generate_random_bytes(length);
            return pimpl_->bytes_to_hex(bytes);
        }

        std::string CryptoManager::derive_key(const std::string &master_key, const std::string &derivation_path)
        {
            // Simplified key derivation
            return pimpl_->compute_sha256(master_key + derivation_path);
        }

        std::vector<uint8_t> CryptoManager::pbkdf2(const std::string &password, const std::vector<uint8_t> &salt, int iterations)
        {
            std::vector<uint8_t> result(32); // 256-bit key

            if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                                  salt.data(), salt.size(),
                                  iterations, EVP_sha256(),
                                  result.size(), result.data()) != 1)
            {
                utils::Logger::instance().log(utils::LogLevel::ERROR, "CryptoManager",
                                              "PBKDF2 key derivation failed");
                return {};
            }

            return result;
        }

        std::string CryptoManager::encode_base64(const std::vector<uint8_t> &data)
        {
            return pimpl_->base64_encode(data);
        }

        std::vector<uint8_t> CryptoManager::decode_base64(const std::string &encoded)
        {
            return pimpl_->base64_decode(encoded);
        }

        std::string CryptoManager::encode_hex(const std::vector<uint8_t> &data)
        {
            return pimpl_->bytes_to_hex(data);
        }

        std::vector<uint8_t> CryptoManager::decode_hex(const std::string &hex)
        {
            return pimpl_->hex_to_bytes(hex);
        }

        CryptoManager::CryptoStats CryptoManager::get_statistics() const
        {
            std::lock_guard<std::mutex> lock(pimpl_->stats_mutex_);
            return pimpl_->stats_;
        }

        void CryptoManager::reset_statistics()
        {
            std::lock_guard<std::mutex> lock(pimpl_->stats_mutex_);
            pimpl_->stats_ = {};
            utils::Logger::instance().log(utils::LogLevel::INFO, "CryptoManager",
                                          "Statistics reset");
        }

    } // namespace core
} // namespace cardano_iot
