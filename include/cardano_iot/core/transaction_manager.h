#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

namespace cardano_iot
{
    namespace core
    {
        // Transaction types
        enum class TransactionType
        {
            PAYMENT,          // Simple ADA payment
            TOKEN_TRANSFER,   // Native token transfer
            SMART_CONTRACT,   // Smart contract interaction
            METADATA,         // Transaction with metadata only
            MULTI_ASSET,      // Multiple asset transfer
            STAKE_DELEGATION, // Staking delegation
            WITHDRAWAL,       // Rewards withdrawal
            CERTIFICATE       // Certificate transaction
        };

        // Transaction status
        enum class TransactionStatus
        {
            PENDING,   // Created but not submitted
            SUBMITTED, // Submitted to blockchain
            CONFIRMED, // Confirmed on blockchain
            FAILED,    // Transaction failed
            CANCELLED  // Transaction cancelled
        };

        // UTXO (Unspent Transaction Output) structure
        struct UTXO
        {
            std::string tx_hash;
            uint32_t output_index;
            uint64_t amount_lovelace;
            std::string address;
            std::map<std::string, uint64_t> native_tokens; // policy_id.asset_name -> amount
            std::string datum_hash;
            std::string script_ref;
        };

        // Transaction input
        struct TransactionInput
        {
            std::string tx_hash;
            uint32_t output_index;
            UTXO utxo_info;
        };

        // Transaction output
        struct TransactionOutput
        {
            std::string address;
            uint64_t amount_lovelace;
            std::map<std::string, uint64_t> native_tokens;
            std::string datum;
            std::string script_ref;
        };

        // Transaction metadata
        struct TransactionMetadata
        {
            std::map<std::string, std::string> labels;
            std::string json_metadata;
            std::vector<uint8_t> binary_metadata;
        };

        // Fee calculation parameters
        struct FeeParameters
        {
            uint64_t min_fee_a = 155381;       // Linear fee parameter (constant term)
            uint64_t min_fee_b = 44;           // Linear fee parameter (per-byte term)
            uint64_t max_tx_size = 16384;      // Maximum transaction size
            uint64_t key_deposit = 2000000;    // Key deposit (2 ADA)
            uint64_t pool_deposit = 500000000; // Pool deposit (500 ADA)
            uint64_t min_utxo = 1000000;       // Minimum UTXO value (1 ADA)
        };

        // Transaction structure
        struct Transaction
        {
            std::string tx_id;
            TransactionType type;
            TransactionStatus status;

            std::vector<TransactionInput> inputs;
            std::vector<TransactionOutput> outputs;
            uint64_t fee;
            uint64_t ttl; // Time to live (slot number)

            std::unique_ptr<TransactionMetadata> metadata;
            std::vector<std::string> certificates;
            std::vector<std::string> withdrawals;

            std::string raw_cbor;
            std::string signed_cbor;
            std::vector<std::string> witnesses;

            uint64_t created_timestamp;
            uint64_t submitted_timestamp;
            uint64_t confirmed_timestamp;

            std::string device_id; // IoT device that created this transaction
            std::string error_message;

            // Copy constructor
            Transaction(const Transaction &other)
                : tx_id(other.tx_id), type(other.type), status(other.status),
                  inputs(other.inputs), outputs(other.outputs), fee(other.fee), ttl(other.ttl),
                  certificates(other.certificates), withdrawals(other.withdrawals),
                  raw_cbor(other.raw_cbor), signed_cbor(other.signed_cbor), witnesses(other.witnesses),
                  created_timestamp(other.created_timestamp), submitted_timestamp(other.submitted_timestamp),
                  confirmed_timestamp(other.confirmed_timestamp), device_id(other.device_id),
                  error_message(other.error_message)
            {
                if (other.metadata)
                {
                    metadata = std::make_unique<TransactionMetadata>(*other.metadata);
                }
            }

            // Copy assignment operator
            Transaction &operator=(const Transaction &other)
            {
                if (this != &other)
                {
                    tx_id = other.tx_id;
                    type = other.type;
                    status = other.status;
                    inputs = other.inputs;
                    outputs = other.outputs;
                    fee = other.fee;
                    ttl = other.ttl;
                    certificates = other.certificates;
                    withdrawals = other.withdrawals;
                    raw_cbor = other.raw_cbor;
                    signed_cbor = other.signed_cbor;
                    witnesses = other.witnesses;
                    created_timestamp = other.created_timestamp;
                    submitted_timestamp = other.submitted_timestamp;
                    confirmed_timestamp = other.confirmed_timestamp;
                    device_id = other.device_id;
                    error_message = other.error_message;

                    if (other.metadata)
                    {
                        metadata = std::make_unique<TransactionMetadata>(*other.metadata);
                    }
                    else
                    {
                        metadata.reset();
                    }
                }
                return *this;
            }

            // Default constructor
            Transaction() = default;

            // Move constructor and assignment (default)
            Transaction(Transaction &&) = default;
            Transaction &operator=(Transaction &&) = default;
        };

        // Balance information
        struct WalletBalance
        {
            uint64_t total_lovelace;
            uint64_t available_lovelace;
            uint64_t rewards;
            std::map<std::string, uint64_t> native_tokens;
            std::vector<UTXO> utxos;
        };

        // Transaction callback types
        using TransactionCallback = std::function<void(const Transaction &)>;
        using ConfirmationCallback = std::function<void(const std::string &tx_id, bool confirmed)>;

        /**
         * @brief Transaction Manager for Cardano blockchain operations
         *
         * Handles all blockchain transaction operations including:
         * - UTXO management and selection
         * - Transaction building and signing
         * - Fee calculation and optimization
         * - Multi-signature support
         * - Native token transfers
         * - Smart contract interactions
         * - Transaction monitoring and confirmation
         */
        class TransactionManager
        {
        public:
            TransactionManager();
            ~TransactionManager();

            // Initialize/shutdown
            bool initialize(const std::string &network = "testnet");
            void shutdown();

            // Wallet and UTXO management
            bool import_wallet(const std::string &mnemonic, const std::string &passphrase = "");
            bool load_wallet_from_keys(const std::string &signing_key, const std::string &verification_key);
            WalletBalance get_wallet_balance(const std::string &address) const;
            std::vector<UTXO> get_utxos(const std::string &address) const;
            bool refresh_utxos(const std::string &address);

            // Transaction building
            std::unique_ptr<Transaction> create_payment_transaction(
                const std::string &from_address,
                const std::string &to_address,
                uint64_t amount_lovelace,
                const std::string &device_id = "");

            std::unique_ptr<Transaction> create_token_transfer(
                const std::string &from_address,
                const std::string &to_address,
                const std::map<std::string, uint64_t> &tokens,
                const std::string &device_id = "");

            std::unique_ptr<Transaction> create_metadata_transaction(
                const std::string &from_address,
                const TransactionMetadata &metadata,
                const std::string &device_id = "");

            std::unique_ptr<Transaction> create_smart_contract_transaction(
                const std::string &from_address,
                const std::string &contract_address,
                const std::string &function_name,
                const std::vector<std::string> &parameters,
                uint64_t amount_lovelace,
                const std::string &device_id = "");

            // Fee calculation
            uint64_t calculate_fee(const Transaction &transaction) const;
            uint64_t estimate_fee(
                size_t num_inputs,
                size_t num_outputs,
                size_t metadata_size = 0) const;

            // Transaction signing and submission
            bool sign_transaction(Transaction &transaction, const std::string &signing_key);
            bool add_witness(Transaction &transaction, const std::string &witness);
            std::string submit_transaction(const Transaction &transaction);
            bool cancel_transaction(const std::string &tx_id);

            // Transaction monitoring
            TransactionStatus get_transaction_status(const std::string &tx_id) const;
            std::unique_ptr<Transaction> get_transaction(const std::string &tx_id) const;
            std::vector<Transaction> get_transactions_by_device(const std::string &device_id) const;
            std::vector<Transaction> get_pending_transactions() const;

            // Confirmation tracking
            void set_confirmation_callback(ConfirmationCallback callback);
            void wait_for_confirmation(const std::string &tx_id, uint32_t timeout_seconds = 300);
            bool is_transaction_confirmed(const std::string &tx_id) const;

            // UTXO selection strategies
            enum class UTXOSelectionStrategy
            {
                LARGEST_FIRST,  // Select largest UTXOs first
                SMALLEST_FIRST, // Select smallest UTXOs first
                RANDOM,         // Random selection
                OPTIMAL_FEE     // Optimize for lowest fees
            };

            void set_utxo_selection_strategy(UTXOSelectionStrategy strategy);
            std::vector<UTXO> select_utxos(
                const std::vector<UTXO> &available_utxos,
                uint64_t target_amount,
                const std::map<std::string, uint64_t> &required_tokens = {}) const;

            // Multi-signature support
            bool create_multisig_transaction(
                const std::vector<std::string> &signing_addresses,
                uint32_t required_signatures,
                const TransactionOutput &output,
                const std::string &device_id = "");

            bool add_multisig_signature(
                const std::string &tx_id,
                const std::string &signature,
                const std::string &public_key);

            // Network parameters
            void update_protocol_parameters(const FeeParameters &params);
            FeeParameters get_protocol_parameters() const;

            // Statistics and monitoring
            struct TransactionStats
            {
                uint64_t total_transactions;
                uint64_t confirmed_transactions;
                uint64_t failed_transactions;
                uint64_t total_fees_paid;
                uint64_t total_volume_lovelace;
                double avg_confirmation_time_seconds;
                double avg_fee_per_transaction;
                uint64_t pending_transactions;
            };

            TransactionStats get_statistics() const;
            void reset_statistics();

            // Utility functions
            std::string address_from_public_key(const std::string &public_key, const std::string &network = "testnet");
            bool validate_address(const std::string &address) const;
            std::string encode_transaction(const Transaction &transaction) const;
            std::unique_ptr<Transaction> decode_transaction(const std::string &cbor_hex) const;

        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl_;
        };

    } // namespace core
} // namespace cardano_iot
