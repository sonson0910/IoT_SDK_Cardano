#include "cardano_iot/core/transaction_manager.h"
#include "cardano_iot/utils/logger.h"

#include <chrono>
#include <sstream>
#include <random>
#include <algorithm>
#include <mutex>
#include <thread>
#include <iomanip>

namespace cardano_iot
{
    namespace core
    {
        // PIMPL implementation
        struct TransactionManager::Impl
        {
            bool initialized_ = false;
            std::string network_ = "testnet";
            mutable std::mutex transactions_mutex_;
            mutable std::mutex utxos_mutex_;
            mutable std::mutex stats_mutex_;

            // Transaction storage
            std::map<std::string, std::unique_ptr<Transaction>> transactions_;
            std::map<std::string, std::vector<UTXO>> address_utxos_;

            // Configuration
            FeeParameters fee_params_;
            UTXOSelectionStrategy utxo_strategy_ = UTXOSelectionStrategy::LARGEST_FIRST;
            ConfirmationCallback confirmation_callback_;

            // Statistics
            TransactionStats stats_ = {};

            // Wallet information
            struct WalletInfo
            {
                std::string signing_key;
                std::string verification_key;
                std::string address;
                bool is_loaded = false;
            };
            WalletInfo wallet_;

            // Generate transaction ID
            std::string generate_tx_id()
            {
                auto now = std::chrono::system_clock::now();
                auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(100000, 999999);

                std::stringstream ss;
                ss << std::hex << timestamp << dis(gen);
                return ss.str();
            }

            // Generate mock address
            std::string generate_mock_address(const std::string &public_key, const std::string &network)
            {
                std::string prefix = (network == "mainnet") ? "addr1" : "addr_test1";

                // Simplified address generation
                std::hash<std::string> hasher;
                auto hash = hasher(prefix + public_key);

                std::stringstream ss;
                ss << prefix << std::hex << hash;
                std::string address = ss.str();

                // Ensure reasonable length
                if (address.length() > 64)
                {
                    address = address.substr(0, 64);
                }
                else if (address.length() < 32)
                {
                    address += std::string(32 - address.length(), '0');
                }

                return address;
            }

            // Create mock UTXOs for demo
            std::vector<UTXO> create_mock_utxos(const std::string &address)
            {
                std::vector<UTXO> utxos;

                // Create a few mock UTXOs
                for (int i = 0; i < 3; ++i)
                {
                    UTXO utxo;
                    utxo.tx_hash = generate_tx_id();
                    utxo.output_index = i;
                    utxo.address = address;
                    utxo.amount_lovelace = 10000000 + (i * 5000000); // 10-20 ADA

                    // Add some mock native tokens
                    if (i == 1)
                    {
                        utxo.native_tokens["policy1.token1"] = 1000;
                        utxo.native_tokens["policy2.token2"] = 500;
                    }

                    utxos.push_back(utxo);
                }

                return utxos;
            }

            // Calculate transaction size estimate
            size_t estimate_transaction_size(size_t num_inputs, size_t num_outputs, size_t metadata_size)
            {
                // Simplified size calculation (in real implementation would be more accurate)
                size_t base_size = 200;                 // Base transaction overhead
                size_t input_size = num_inputs * 150;   // Each input ~150 bytes
                size_t output_size = num_outputs * 100; // Each output ~100 bytes
                size_t witness_size = num_inputs * 100; // Each witness ~100 bytes

                return base_size + input_size + output_size + witness_size + metadata_size;
            }

            // Mock transaction submission
            std::string submit_to_network(const Transaction &transaction)
            {
                // Simulate network submission delay
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                // For demo, assume 90% success rate
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1, 10);

                if (dis(gen) <= 9)
                {
                    return transaction.tx_id; // Success
                }
                else
                {
                    return ""; // Failure
                }
            }

            // Mock confirmation checking
            bool check_confirmation(const std::string &tx_id)
            {
                // Simulate confirmation after some time
                auto it = transactions_.find(tx_id);
                if (it != transactions_.end())
                {
                    auto &tx = it->second;
                    auto now = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();

                    // Consider confirmed after 30 seconds (for demo)
                    return (now - tx->submitted_timestamp > 30);
                }
                return false;
            }

            // UTXO selection implementation
            std::vector<UTXO> select_utxos_impl(
                const std::vector<UTXO> &available_utxos,
                uint64_t target_amount,
                const std::map<std::string, uint64_t> &required_tokens,
                UTXOSelectionStrategy strategy)
            {

                std::vector<UTXO> candidates = available_utxos;
                std::vector<UTXO> selected;

                // Sort based on strategy
                switch (strategy)
                {
                case UTXOSelectionStrategy::LARGEST_FIRST:
                    std::sort(candidates.begin(), candidates.end(),
                              [](const UTXO &a, const UTXO &b)
                              {
                                  return a.amount_lovelace > b.amount_lovelace;
                              });
                    break;

                case UTXOSelectionStrategy::SMALLEST_FIRST:
                    std::sort(candidates.begin(), candidates.end(),
                              [](const UTXO &a, const UTXO &b)
                              {
                                  return a.amount_lovelace < b.amount_lovelace;
                              });
                    break;

                case UTXOSelectionStrategy::RANDOM:
                {
                    std::random_device rd;
                    std::mt19937 g(rd());
                    std::shuffle(candidates.begin(), candidates.end(), g);
                }
                break;

                case UTXOSelectionStrategy::OPTIMAL_FEE:
                    // For demo, use largest first
                    std::sort(candidates.begin(), candidates.end(),
                              [](const UTXO &a, const UTXO &b)
                              {
                                  return a.amount_lovelace > b.amount_lovelace;
                              });
                    break;
                }

                uint64_t accumulated = 0;
                std::map<std::string, uint64_t> accumulated_tokens;

                for (const auto &utxo : candidates)
                {
                    selected.push_back(utxo);
                    accumulated += utxo.amount_lovelace;

                    // Accumulate tokens
                    for (const auto &[token, amount] : utxo.native_tokens)
                    {
                        accumulated_tokens[token] += amount;
                    }

                    // Check if we have enough ADA
                    bool ada_sufficient = accumulated >= target_amount;

                    // Check if we have enough tokens
                    bool tokens_sufficient = true;
                    for (const auto &[token, required_amount] : required_tokens)
                    {
                        if (accumulated_tokens[token] < required_amount)
                        {
                            tokens_sufficient = false;
                            break;
                        }
                    }

                    if (ada_sufficient && tokens_sufficient)
                    {
                        break;
                    }
                }

                return selected;
            }
        };

        // Constructor/Destructor
        TransactionManager::TransactionManager() : pimpl_(std::make_unique<Impl>()) {}
        TransactionManager::~TransactionManager() = default;

        bool TransactionManager::initialize(const std::string &network)
        {
            std::lock_guard<std::mutex> lock(pimpl_->transactions_mutex_);

            if (pimpl_->initialized_)
            {
                return true;
            }

            pimpl_->network_ = network;

            // Set default fee parameters
            pimpl_->fee_params_ = FeeParameters{};

            pimpl_->initialized_ = true;

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Transaction manager initialized for network: " + network);
            return true;
        }

        void TransactionManager::shutdown()
        {
            std::lock_guard<std::mutex> lock(pimpl_->transactions_mutex_);

            if (!pimpl_->initialized_)
            {
                return;
            }

            // Clear all data
            pimpl_->transactions_.clear();
            pimpl_->address_utxos_.clear();
            pimpl_->wallet_ = {};
            pimpl_->initialized_ = false;

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Transaction manager shut down");
        }

        bool TransactionManager::import_wallet(const std::string &mnemonic, const std::string &passphrase)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            // Simplified wallet import for demo
            std::hash<std::string> hasher;
            auto seed = hasher(mnemonic + passphrase);

            std::stringstream sk_ss, vk_ss;
            sk_ss << "sk_" << std::hex << seed;
            vk_ss << "vk_" << std::hex << (seed ^ 0xDEADBEEF);

            pimpl_->wallet_.signing_key = sk_ss.str();
            pimpl_->wallet_.verification_key = vk_ss.str();
            pimpl_->wallet_.address = pimpl_->generate_mock_address(pimpl_->wallet_.verification_key, pimpl_->network_);
            pimpl_->wallet_.is_loaded = true;

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Wallet imported successfully");
            return true;
        }

        bool TransactionManager::load_wallet_from_keys(const std::string &signing_key, const std::string &verification_key)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            pimpl_->wallet_.signing_key = signing_key;
            pimpl_->wallet_.verification_key = verification_key;
            pimpl_->wallet_.address = pimpl_->generate_mock_address(verification_key, pimpl_->network_);
            pimpl_->wallet_.is_loaded = true;

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Wallet loaded from keys");
            return true;
        }

        WalletBalance TransactionManager::get_wallet_balance(const std::string &address) const
        {
            WalletBalance balance = {};

            if (!pimpl_->initialized_)
            {
                return balance;
            }

            std::lock_guard<std::mutex> lock(pimpl_->utxos_mutex_);

            auto it = pimpl_->address_utxos_.find(address);
            if (it != pimpl_->address_utxos_.end())
            {
                for (const auto &utxo : it->second)
                {
                    balance.total_lovelace += utxo.amount_lovelace;
                    balance.available_lovelace += utxo.amount_lovelace;

                    for (const auto &[token, amount] : utxo.native_tokens)
                    {
                        balance.native_tokens[token] += amount;
                    }
                }
                balance.utxos = it->second;
            }

            return balance;
        }

        std::vector<UTXO> TransactionManager::get_utxos(const std::string &address) const
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->utxos_mutex_);

            auto it = pimpl_->address_utxos_.find(address);
            if (it != pimpl_->address_utxos_.end())
            {
                return it->second;
            }

            return {};
        }

        bool TransactionManager::refresh_utxos(const std::string &address)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->utxos_mutex_);

            // Create mock UTXOs for demo
            pimpl_->address_utxos_[address] = pimpl_->create_mock_utxos(address);

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Refreshed UTXOs for address: " + address.substr(0, 16) + "...");
            return true;
        }

        std::unique_ptr<Transaction> TransactionManager::create_payment_transaction(
            const std::string &from_address,
            const std::string &to_address,
            uint64_t amount_lovelace,
            const std::string &device_id)
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            auto transaction = std::make_unique<Transaction>();
            transaction->tx_id = pimpl_->generate_tx_id();
            transaction->type = TransactionType::PAYMENT;
            transaction->status = TransactionStatus::PENDING;
            transaction->device_id = device_id;
            transaction->created_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                                 std::chrono::system_clock::now().time_since_epoch())
                                                 .count();

            // Get UTXOs for the from address
            auto utxos = get_utxos(from_address);
            if (utxos.empty())
            {
                // Try to refresh UTXOs
                refresh_utxos(from_address);
                utxos = get_utxos(from_address);
            }

            // Select UTXOs
            auto selected_utxos = select_utxos(utxos, amount_lovelace + 200000); // +200k for fees

            if (selected_utxos.empty())
            {
                utils::Logger::instance().log(utils::LogLevel::ERROR, "TransactionManager",
                                              "Insufficient funds for transaction");
                return nullptr;
            }

            // Build inputs
            uint64_t total_input = 0;
            for (const auto &utxo : selected_utxos)
            {
                TransactionInput input;
                input.tx_hash = utxo.tx_hash;
                input.output_index = utxo.output_index;
                input.utxo_info = utxo;
                transaction->inputs.push_back(input);
                total_input += utxo.amount_lovelace;
            }

            // Calculate fee
            transaction->fee = estimate_fee(transaction->inputs.size(), 2); // 2 outputs (payment + change)

            // Build outputs
            TransactionOutput payment_output;
            payment_output.address = to_address;
            payment_output.amount_lovelace = amount_lovelace;
            transaction->outputs.push_back(payment_output);

            // Add change output if needed
            uint64_t change = total_input - amount_lovelace - transaction->fee;
            if (change > pimpl_->fee_params_.min_utxo)
            {
                TransactionOutput change_output;
                change_output.address = from_address;
                change_output.amount_lovelace = change;
                transaction->outputs.push_back(change_output);
            }

            // Set TTL (1 hour from now)
            transaction->ttl = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count() +
                               3600;

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Created payment transaction: " + transaction->tx_id);

            return transaction;
        }

        std::unique_ptr<Transaction> TransactionManager::create_token_transfer(
            const std::string &from_address,
            const std::string &to_address,
            const std::map<std::string, uint64_t> &tokens,
            const std::string &device_id)
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            auto transaction = std::make_unique<Transaction>();
            transaction->tx_id = pimpl_->generate_tx_id();
            transaction->type = TransactionType::TOKEN_TRANSFER;
            transaction->status = TransactionStatus::PENDING;
            transaction->device_id = device_id;
            transaction->created_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                                 std::chrono::system_clock::now().time_since_epoch())
                                                 .count();

            // Get UTXOs and select for token transfer
            auto utxos = get_utxos(from_address);
            if (utxos.empty())
            {
                refresh_utxos(from_address);
                utxos = get_utxos(from_address);
            }

            auto selected_utxos = select_utxos(utxos, pimpl_->fee_params_.min_utxo, tokens);

            if (selected_utxos.empty())
            {
                utils::Logger::instance().log(utils::LogLevel::ERROR, "TransactionManager",
                                              "Insufficient tokens for transfer");
                return nullptr;
            }

            // Build transaction similar to payment but with tokens
            uint64_t total_input = 0;
            std::map<std::string, uint64_t> input_tokens;

            for (const auto &utxo : selected_utxos)
            {
                TransactionInput input;
                input.tx_hash = utxo.tx_hash;
                input.output_index = utxo.output_index;
                input.utxo_info = utxo;
                transaction->inputs.push_back(input);
                total_input += utxo.amount_lovelace;

                for (const auto &[token, amount] : utxo.native_tokens)
                {
                    input_tokens[token] += amount;
                }
            }

            transaction->fee = estimate_fee(transaction->inputs.size(), 2);

            // Payment output with tokens
            TransactionOutput payment_output;
            payment_output.address = to_address;
            payment_output.amount_lovelace = pimpl_->fee_params_.min_utxo;
            payment_output.native_tokens = tokens;
            transaction->outputs.push_back(payment_output);

            // Change output
            TransactionOutput change_output;
            change_output.address = from_address;
            change_output.amount_lovelace = total_input - payment_output.amount_lovelace - transaction->fee;

            // Calculate token change
            for (const auto &[token, input_amount] : input_tokens)
            {
                uint64_t sent_amount = 0;
                auto it = tokens.find(token);
                if (it != tokens.end())
                {
                    sent_amount = it->second;
                }
                if (input_amount > sent_amount)
                {
                    change_output.native_tokens[token] = input_amount - sent_amount;
                }
            }

            transaction->outputs.push_back(change_output);
            transaction->ttl = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count() +
                               3600;

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Created token transfer transaction: " + transaction->tx_id);

            return transaction;
        }

        std::unique_ptr<Transaction> TransactionManager::create_metadata_transaction(
            const std::string &from_address,
            const TransactionMetadata &metadata,
            const std::string &device_id)
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            auto transaction = std::make_unique<Transaction>();
            transaction->tx_id = pimpl_->generate_tx_id();
            transaction->type = TransactionType::METADATA;
            transaction->status = TransactionStatus::PENDING;
            transaction->device_id = device_id;
            transaction->metadata = std::make_unique<TransactionMetadata>(metadata);
            transaction->created_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                                 std::chrono::system_clock::now().time_since_epoch())
                                                 .count();

            // Simple metadata transaction to self
            auto utxos = get_utxos(from_address);
            if (utxos.empty())
            {
                refresh_utxos(from_address);
                utxos = get_utxos(from_address);
            }

            if (!utxos.empty())
            {
                // Use one UTXO
                TransactionInput input;
                input.tx_hash = utxos[0].tx_hash;
                input.output_index = utxos[0].output_index;
                input.utxo_info = utxos[0];
                transaction->inputs.push_back(input);

                transaction->fee = estimate_fee(1, 1, metadata.json_metadata.length());

                // Send back to self
                TransactionOutput output;
                output.address = from_address;
                output.amount_lovelace = utxos[0].amount_lovelace - transaction->fee;
                transaction->outputs.push_back(output);

                transaction->ttl = std::chrono::duration_cast<std::chrono::seconds>(
                                       std::chrono::system_clock::now().time_since_epoch())
                                       .count() +
                                   3600;
            }

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Created metadata transaction: " + transaction->tx_id);

            return transaction;
        }

        std::unique_ptr<Transaction> TransactionManager::create_smart_contract_transaction(
            const std::string &from_address,
            const std::string &contract_address,
            const std::string &function_name,
            const std::vector<std::string> &parameters,
            uint64_t amount_lovelace,
            const std::string &device_id)
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            auto transaction = std::make_unique<Transaction>();
            transaction->tx_id = pimpl_->generate_tx_id();
            transaction->type = TransactionType::SMART_CONTRACT;
            transaction->status = TransactionStatus::PENDING;
            transaction->device_id = device_id;
            transaction->created_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                                 std::chrono::system_clock::now().time_since_epoch())
                                                 .count();

            // Create metadata with contract call info
            TransactionMetadata contract_metadata;
            contract_metadata.labels["function"] = function_name;
            contract_metadata.labels["contract"] = contract_address;

            std::stringstream params_ss;
            for (size_t i = 0; i < parameters.size(); ++i)
            {
                if (i > 0)
                    params_ss << ",";
                params_ss << parameters[i];
            }
            contract_metadata.labels["parameters"] = params_ss.str();

            transaction->metadata = std::make_unique<TransactionMetadata>(contract_metadata);

            // Build transaction similar to payment
            auto payment_tx = create_payment_transaction(from_address, contract_address, amount_lovelace, device_id);
            if (payment_tx)
            {
                transaction->inputs = std::move(payment_tx->inputs);
                transaction->outputs = std::move(payment_tx->outputs);
                transaction->fee = payment_tx->fee + 50000; // Extra fee for contract execution
                transaction->ttl = payment_tx->ttl;
            }

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Created smart contract transaction: " + transaction->tx_id);

            return transaction;
        }

        uint64_t TransactionManager::calculate_fee(const Transaction &transaction) const
        {
            size_t tx_size = pimpl_->estimate_transaction_size(
                transaction.inputs.size(),
                transaction.outputs.size(),
                transaction.metadata ? transaction.metadata->json_metadata.length() : 0);

            return pimpl_->fee_params_.min_fee_a + (pimpl_->fee_params_.min_fee_b * tx_size);
        }

        uint64_t TransactionManager::estimate_fee(
            size_t num_inputs,
            size_t num_outputs,
            size_t metadata_size) const
        {
            size_t tx_size = pimpl_->estimate_transaction_size(num_inputs, num_outputs, metadata_size);
            return pimpl_->fee_params_.min_fee_a + (pimpl_->fee_params_.min_fee_b * tx_size);
        }

        bool TransactionManager::sign_transaction(Transaction &transaction, const std::string &signing_key)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            // Simplified signing for demo
            std::stringstream ss;
            ss << "signed_" << transaction.tx_id << "_" << signing_key.substr(0, 8);
            std::string witness = ss.str();

            transaction.witnesses.push_back(witness);
            transaction.signed_cbor = "cbor_" + transaction.tx_id + "_signed";

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Transaction signed: " + transaction.tx_id);
            return true;
        }

        bool TransactionManager::add_witness(Transaction &transaction, const std::string &witness)
        {
            transaction.witnesses.push_back(witness);

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Added witness to transaction: " + transaction.tx_id);
            return true;
        }

        std::string TransactionManager::submit_transaction(const Transaction &transaction)
        {
            if (!pimpl_->initialized_)
            {
                return "";
            }

            // Store transaction
            {
                std::lock_guard<std::mutex> lock(pimpl_->transactions_mutex_);
                auto tx_copy = std::make_unique<Transaction>(transaction);
                tx_copy->status = TransactionStatus::SUBMITTED;
                tx_copy->submitted_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                                   std::chrono::system_clock::now().time_since_epoch())
                                                   .count();

                pimpl_->transactions_[transaction.tx_id] = std::move(tx_copy);
            }

            // Submit to network (mock)
            std::string result = pimpl_->submit_to_network(transaction);

            if (!result.empty())
            {
                std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                pimpl_->stats_.total_transactions++;
                pimpl_->stats_.pending_transactions++;

                utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                              "Transaction submitted successfully: " + transaction.tx_id);
            }
            else
            {
                std::lock_guard<std::mutex> lock(pimpl_->transactions_mutex_);
                auto it = pimpl_->transactions_.find(transaction.tx_id);
                if (it != pimpl_->transactions_.end())
                {
                    it->second->status = TransactionStatus::FAILED;
                    it->second->error_message = "Network submission failed";
                }

                std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                pimpl_->stats_.failed_transactions++;

                utils::Logger::instance().log(utils::LogLevel::ERROR, "TransactionManager",
                                              "Transaction submission failed: " + transaction.tx_id);
            }

            return result;
        }

        bool TransactionManager::cancel_transaction(const std::string &tx_id)
        {
            std::lock_guard<std::mutex> lock(pimpl_->transactions_mutex_);

            auto it = pimpl_->transactions_.find(tx_id);
            if (it != pimpl_->transactions_.end() && it->second->status == TransactionStatus::PENDING)
            {
                it->second->status = TransactionStatus::CANCELLED;

                utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                              "Transaction cancelled: " + tx_id);
                return true;
            }

            return false;
        }

        TransactionStatus TransactionManager::get_transaction_status(const std::string &tx_id) const
        {
            std::lock_guard<std::mutex> lock(pimpl_->transactions_mutex_);

            auto it = pimpl_->transactions_.find(tx_id);
            if (it != pimpl_->transactions_.end())
            {
                // Check for confirmation
                if (it->second->status == TransactionStatus::SUBMITTED)
                {
                    if (pimpl_->check_confirmation(tx_id))
                    {
                        it->second->status = TransactionStatus::CONFIRMED;
                        it->second->confirmed_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                                              std::chrono::system_clock::now().time_since_epoch())
                                                              .count();
                    }
                }
                return it->second->status;
            }

            return TransactionStatus::FAILED;
        }

        std::unique_ptr<Transaction> TransactionManager::get_transaction(const std::string &tx_id) const
        {
            std::lock_guard<std::mutex> lock(pimpl_->transactions_mutex_);

            auto it = pimpl_->transactions_.find(tx_id);
            if (it != pimpl_->transactions_.end())
            {
                return std::make_unique<Transaction>(*it->second);
            }

            return nullptr;
        }

        std::vector<Transaction> TransactionManager::get_transactions_by_device(const std::string &device_id) const
        {
            std::lock_guard<std::mutex> lock(pimpl_->transactions_mutex_);

            std::vector<Transaction> result;
            for (const auto &[tx_id, tx] : pimpl_->transactions_)
            {
                if (tx->device_id == device_id)
                {
                    result.push_back(*tx);
                }
            }

            return result;
        }

        std::vector<Transaction> TransactionManager::get_pending_transactions() const
        {
            std::lock_guard<std::mutex> lock(pimpl_->transactions_mutex_);

            std::vector<Transaction> result;
            for (const auto &[tx_id, tx] : pimpl_->transactions_)
            {
                if (tx->status == TransactionStatus::PENDING || tx->status == TransactionStatus::SUBMITTED)
                {
                    result.push_back(*tx);
                }
            }

            return result;
        }

        void TransactionManager::set_confirmation_callback(ConfirmationCallback callback)
        {
            pimpl_->confirmation_callback_ = callback;
        }

        void TransactionManager::wait_for_confirmation(const std::string &tx_id, uint32_t timeout_seconds)
        {
            auto start_time = std::chrono::steady_clock::now();

            while (true)
            {
                auto status = get_transaction_status(tx_id);
                if (status == TransactionStatus::CONFIRMED)
                {
                    if (pimpl_->confirmation_callback_)
                    {
                        pimpl_->confirmation_callback_(tx_id, true);
                    }
                    break;
                }

                if (status == TransactionStatus::FAILED)
                {
                    if (pimpl_->confirmation_callback_)
                    {
                        pimpl_->confirmation_callback_(tx_id, false);
                    }
                    break;
                }

                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::steady_clock::now() - start_time)
                                   .count();

                if (elapsed >= timeout_seconds)
                {
                    utils::Logger::instance().log(utils::LogLevel::WARNING, "TransactionManager",
                                                  "Transaction confirmation timeout: " + tx_id);
                    break;
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        bool TransactionManager::is_transaction_confirmed(const std::string &tx_id) const
        {
            return get_transaction_status(tx_id) == TransactionStatus::CONFIRMED;
        }

        void TransactionManager::set_utxo_selection_strategy(UTXOSelectionStrategy strategy)
        {
            pimpl_->utxo_strategy_ = strategy;
        }

        std::vector<UTXO> TransactionManager::select_utxos(
            const std::vector<UTXO> &available_utxos,
            uint64_t target_amount,
            const std::map<std::string, uint64_t> &required_tokens) const
        {
            return pimpl_->select_utxos_impl(available_utxos, target_amount, required_tokens, pimpl_->utxo_strategy_);
        }

        bool TransactionManager::create_multisig_transaction(
            const std::vector<std::string> &signing_addresses,
            uint32_t required_signatures,
            const TransactionOutput &output,
            const std::string &device_id)
        {
            // Simplified multisig implementation for demo
            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Created multisig transaction requiring " +
                                              std::to_string(required_signatures) + " signatures");
            return true;
        }

        bool TransactionManager::add_multisig_signature(
            const std::string &tx_id,
            const std::string &signature,
            const std::string &public_key)
        {
            std::lock_guard<std::mutex> lock(pimpl_->transactions_mutex_);

            auto it = pimpl_->transactions_.find(tx_id);
            if (it != pimpl_->transactions_.end())
            {
                it->second->witnesses.push_back(signature + ":" + public_key);

                utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                              "Added multisig signature to: " + tx_id);
                return true;
            }

            return false;
        }

        void TransactionManager::update_protocol_parameters(const FeeParameters &params)
        {
            pimpl_->fee_params_ = params;

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Updated protocol parameters");
        }

        FeeParameters TransactionManager::get_protocol_parameters() const
        {
            return pimpl_->fee_params_;
        }

        TransactionManager::TransactionStats TransactionManager::get_statistics() const
        {
            std::lock_guard<std::mutex> lock(pimpl_->stats_mutex_);
            return pimpl_->stats_;
        }

        void TransactionManager::reset_statistics()
        {
            std::lock_guard<std::mutex> lock(pimpl_->stats_mutex_);
            pimpl_->stats_ = {};

            utils::Logger::instance().log(utils::LogLevel::INFO, "TransactionManager",
                                          "Statistics reset");
        }

        std::string TransactionManager::address_from_public_key(const std::string &public_key, const std::string &network)
        {
            return pimpl_->generate_mock_address(public_key, network);
        }

        bool TransactionManager::validate_address(const std::string &address) const
        {
            // Simple validation for demo
            return (address.length() > 20 &&
                    (address.find("addr1") == 0 || address.find("addr_test") == 0));
        }

        std::string TransactionManager::encode_transaction(const Transaction &transaction) const
        {
            // Simplified CBOR encoding for demo
            return "cbor_" + transaction.tx_id;
        }

        std::unique_ptr<Transaction> TransactionManager::decode_transaction(const std::string &cbor_hex) const
        {
            // Simplified CBOR decoding for demo
            auto transaction = std::make_unique<Transaction>();
            transaction->tx_id = "decoded_" + cbor_hex.substr(0, 8);
            transaction->status = TransactionStatus::PENDING;

            return transaction;
        }

    } // namespace core
} // namespace cardano_iot
