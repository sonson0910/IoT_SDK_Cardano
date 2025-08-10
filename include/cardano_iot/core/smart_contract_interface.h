#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <variant>

namespace cardano_iot
{
    namespace core
    {
        // Plutus data types
        enum class PlutusDataType
        {
            INTEGER,
            BYTESTRING,
            LIST,
            MAP,
            CONSTRUCTOR,
            BOOLEAN,
            UNIT
        };

        // Plutus data value
        struct PlutusData
        {
            PlutusDataType type;
            std::variant<int64_t, std::vector<uint8_t>, std::vector<std::shared_ptr<PlutusData>>,
                         std::map<std::string, std::shared_ptr<PlutusData>>,
                         std::pair<uint64_t, std::vector<std::shared_ptr<PlutusData>>>, bool>
                value;

            PlutusData() : type(PlutusDataType::UNIT) {}
            explicit PlutusData(int64_t val) : type(PlutusDataType::INTEGER), value(val) {}
            explicit PlutusData(const std::vector<uint8_t> &val) : type(PlutusDataType::BYTESTRING), value(val) {}
            explicit PlutusData(bool val) : type(PlutusDataType::BOOLEAN), value(val) {}
        };

        // Contract execution result
        enum class ExecutionResult
        {
            SUCCESS,
            VALIDATION_FAILED,
            EXECUTION_ERROR,
            INSUFFICIENT_FUNDS,
            TIMEOUT,
            SCRIPT_ERROR
        };

        // Contract execution context
        struct ExecutionContext
        {
            std::string tx_hash;
            uint32_t tx_index;
            std::vector<uint8_t> tx_body;
            std::string purpose; // "spend", "mint", "cert", "reward"
            std::map<std::string, PlutusData> script_context;
        };

        // Smart contract information
        struct SmartContract
        {
            std::string contract_id;
            std::string name;
            std::string description;
            std::string script_hash;
            std::string script_cbor;
            std::string language_version; // "PlutusV1", "PlutusV2", "PlutusV3"

            // Contract metadata
            std::string author;
            std::string version;
            uint64_t creation_timestamp;
            std::map<std::string, std::string> tags;

            // ABI information
            std::vector<std::string> endpoints;
            std::map<std::string, std::string> parameter_types;
            std::map<std::string, std::string> return_types;
        };

        // Contract deployment info
        struct ContractDeployment
        {
            std::string deployment_id;
            std::string contract_id;
            std::string address;
            std::string network;
            std::string deployer_address;
            uint64_t deployment_timestamp;
            std::string deployment_tx_hash;
            std::map<std::string, PlutusData> initialization_parameters;
        };

        // Contract execution log
        struct ExecutionLog
        {
            std::string execution_id;
            std::string contract_address;
            std::string function_name;
            std::vector<PlutusData> parameters;
            ExecutionResult result;
            std::string error_message;
            uint64_t gas_used;
            uint64_t execution_time_ms;
            uint64_t timestamp;
            std::string transaction_hash;
        };

        // Contract event
        struct ContractEvent
        {
            std::string event_id;
            std::string contract_address;
            std::string event_name;
            std::map<std::string, PlutusData> event_data;
            uint64_t timestamp;
            std::string transaction_hash;
            uint32_t block_number;
        };

        // Contract state
        struct ContractState
        {
            std::string contract_address;
            std::map<std::string, PlutusData> state_variables;
            uint64_t last_updated_timestamp;
            std::string last_update_tx_hash;
            uint64_t state_version;
        };

        // Event callback type
        using EventCallback = std::function<void(const ContractEvent &)>;

        /**
         * @brief Smart Contract Interface for Plutus contract interaction
         *
         * Provides comprehensive smart contract operations including:
         * - Contract deployment and management
         * - Function calling and parameter encoding
         * - Event monitoring and callbacks
         * - State querying and watching
         * - Multi-contract orchestration
         * - Gas optimization and estimation
         */
        class SmartContractInterface
        {
        public:
            SmartContractInterface();
            ~SmartContractInterface();

            // Initialize/shutdown
            bool initialize(const std::string &network = "testnet");
            void shutdown();

            // Contract management
            std::string deploy_contract(
                const std::string &script_cbor,
                const std::map<std::string, PlutusData> &init_params,
                const std::string &deployer_address,
                const std::string &device_id = "");

            bool register_contract(const SmartContract &contract);
            std::shared_ptr<SmartContract> get_contract(const std::string &contract_id) const;
            std::vector<SmartContract> list_contracts() const;
            bool remove_contract(const std::string &contract_id);

            // Contract deployment tracking
            std::shared_ptr<ContractDeployment> get_deployment(const std::string &deployment_id) const;
            std::vector<ContractDeployment> list_deployments(const std::string &contract_id = "") const;

            // Function calling
            std::string call_contract_function(
                const std::string &contract_address,
                const std::string &function_name,
                const std::vector<PlutusData> &parameters,
                const std::string &caller_address,
                uint64_t amount_lovelace = 0,
                const std::string &device_id = "");

            std::string call_readonly_function(
                const std::string &contract_address,
                const std::string &function_name,
                const std::vector<PlutusData> &parameters) const;

            // State querying
            std::shared_ptr<ContractState> get_contract_state(const std::string &contract_address) const;
            PlutusData query_state_variable(
                const std::string &contract_address,
                const std::string &variable_name) const;

            bool watch_state_changes(
                const std::string &contract_address,
                const std::string &variable_name,
                std::function<void(const PlutusData &)> callback);

            // Event handling
            bool subscribe_to_events(
                const std::string &contract_address,
                const std::string &event_name,
                EventCallback callback);

            bool unsubscribe_from_events(
                const std::string &contract_address,
                const std::string &event_name);

            std::vector<ContractEvent> get_events(
                const std::string &contract_address,
                const std::string &event_name = "",
                uint64_t from_timestamp = 0,
                uint64_t to_timestamp = 0) const;

            // Execution logs and history
            std::vector<ExecutionLog> get_execution_logs(
                const std::string &contract_address,
                const std::string &function_name = "",
                uint64_t from_timestamp = 0,
                uint64_t to_timestamp = 0) const;

            std::shared_ptr<ExecutionLog> get_execution_log(const std::string &execution_id) const;

            // Gas estimation and optimization
            uint64_t estimate_gas(
                const std::string &contract_address,
                const std::string &function_name,
                const std::vector<PlutusData> &parameters) const;

            uint64_t estimate_execution_units(
                const std::string &script_cbor,
                const ExecutionContext &context,
                const std::vector<PlutusData> &parameters) const;

            // Plutus data utilities
            std::string encode_plutus_data(const PlutusData &data) const;
            PlutusData decode_plutus_data(const std::string &cbor_hex) const;

            PlutusData create_integer(int64_t value);
            PlutusData create_bytestring(const std::vector<uint8_t> &bytes);
            PlutusData create_list(const std::vector<PlutusData> &items);
            PlutusData create_map(const std::map<std::string, PlutusData> &items);
            PlutusData create_constructor(uint64_t tag, const std::vector<PlutusData> &fields);

            // Validation and verification
            bool validate_script(const std::string &script_cbor) const;
            bool verify_execution(
                const std::string &script_cbor,
                const ExecutionContext &context,
                const std::vector<PlutusData> &parameters) const;

            // Multi-contract operations
            std::string execute_batch_calls(
                const std::vector<std::tuple<std::string, std::string, std::vector<PlutusData>>> &calls,
                const std::string &caller_address,
                const std::string &device_id = "");

            bool create_contract_composition(
                const std::string &composition_name,
                const std::vector<std::string> &contract_addresses,
                const std::map<std::string, std::string> &data_flow);

            // Contract templates and factories
            struct ContractTemplate
            {
                std::string template_id;
                std::string name;
                std::string script_template;
                std::map<std::string, std::string> parameter_placeholders;
                std::vector<std::string> required_parameters;
            };

            bool register_template(const ContractTemplate &template_spec);
            std::string deploy_from_template(
                const std::string &template_id,
                const std::map<std::string, PlutusData> &template_parameters,
                const std::string &deployer_address,
                const std::string &device_id = "");

            // IoT-specific contract utilities
            std::string create_device_identity_contract(
                const std::string &device_id,
                const std::string &public_key,
                const std::map<std::string, std::string> &attributes);

            std::string create_data_oracle_contract(
                const std::string &data_source,
                const std::vector<std::string> &authorized_updaters,
                uint64_t update_interval_seconds);

            std::string create_payment_channel_contract(
                const std::string &sender_address,
                const std::string &receiver_address,
                uint64_t deposit_amount,
                uint64_t timeout_slots);

            // Statistics and monitoring
            struct ContractStats
            {
                uint64_t total_contracts;
                uint64_t active_deployments;
                uint64_t total_executions;
                uint64_t successful_executions;
                uint64_t failed_executions;
                uint64_t total_gas_used;
                double avg_execution_time_ms;
                uint64_t events_emitted;
                uint64_t active_subscriptions;
            };

            ContractStats get_statistics() const;
            void reset_statistics();

            // Configuration
            struct ContractConfig
            {
                uint64_t default_gas_limit = 10000000;
                uint64_t execution_timeout_ms = 30000;
                bool enable_event_logging = true;
                bool enable_state_caching = true;
                uint32_t max_concurrent_executions = 10;
                std::string preferred_language_version = "PlutusV2";
            };

            void update_config(const ContractConfig &config);
            ContractConfig get_config() const;

        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl_;
        };

    } // namespace core
} // namespace cardano_iot
