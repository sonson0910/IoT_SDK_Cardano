#include "cardano_iot/core/smart_contract_interface.h"
#include "cardano_iot/utils/logger.h"

#include <chrono>
#include <sstream>
#include <random>
#include <algorithm>
#include <mutex>
#include <thread>
#include <iomanip>
#include <variant>

namespace cardano_iot
{
    namespace core
    {
        // PIMPL implementation
        struct SmartContractInterface::Impl
        {
            bool initialized_ = false;
            std::string network_ = "testnet";
            mutable std::mutex contracts_mutex_;
            mutable std::mutex deployments_mutex_;
            mutable std::mutex executions_mutex_;
            mutable std::mutex events_mutex_;
            mutable std::mutex stats_mutex_;

            // Contract storage
            std::map<std::string, std::shared_ptr<SmartContract>> contracts_;
            std::map<std::string, std::shared_ptr<ContractDeployment>> deployments_;
            std::map<std::string, std::shared_ptr<ExecutionLog>> execution_logs_;
            std::map<std::string, std::vector<ContractEvent>> contract_events_;
            std::map<std::string, std::shared_ptr<ContractState>> contract_states_;

            // Templates
            std::map<std::string, ContractTemplate> templates_;

            // Event subscriptions
            std::map<std::string, std::map<std::string, EventCallback>> event_subscriptions_;

            // State watchers
            std::map<std::string, std::map<std::string, std::function<void(const PlutusData &)>>> state_watchers_;

            // Configuration
            ContractConfig config_;

            // Statistics
            ContractStats stats_ = {};

            // Generate unique IDs
            std::string generate_id(const std::string &prefix)
            {
                auto now = std::chrono::system_clock::now();
                auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1000, 9999);

                std::stringstream ss;
                ss << prefix << "_" << timestamp << "_" << dis(gen);
                return ss.str();
            }

            // Generate mock contract address
            std::string generate_contract_address()
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, 15);

                std::stringstream ss;
                ss << "contract_";
                for (int i = 0; i < 32; ++i)
                {
                    ss << std::hex << dis(gen);
                }
                return ss.str();
            }

            // Create mock execution context
            ExecutionContext create_mock_context(const std::string &tx_hash)
            {
                ExecutionContext context;
                context.tx_hash = tx_hash;
                context.tx_index = 0;
                context.purpose = "spend";

                // Add some mock script context
                context.script_context["tx_info"] = PlutusData(static_cast<int64_t>(42));
                context.script_context["purpose"] = PlutusData(std::vector<uint8_t>{'s', 'p', 'e', 'n', 'd'});

                return context;
            }

            // Simulate contract execution
            ExecutionResult simulate_execution(
                const std::string &contract_address,
                const std::string &function_name,
                const std::vector<PlutusData> &parameters)
            {

                // Simulate execution time
                std::this_thread::sleep_for(std::chrono::milliseconds(50 + (parameters.size() * 10)));

                // Simulate 95% success rate
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1, 100);

                if (dis(gen) <= 95)
                {
                    return ExecutionResult::SUCCESS;
                }
                else
                {
                    return ExecutionResult::EXECUTION_ERROR;
                }
            }

            // Estimate gas usage
            uint64_t estimate_gas_usage(
                const std::string &function_name,
                const std::vector<PlutusData> &parameters)
            {

                uint64_t base_gas = 100000;                     // Base execution cost
                uint64_t param_gas = parameters.size() * 50000; // Per parameter cost

                // Function-specific costs
                if (function_name.find("complex") != std::string::npos)
                {
                    base_gas *= 3;
                }
                else if (function_name.find("simple") != std::string::npos)
                {
                    base_gas /= 2;
                }

                return base_gas + param_gas;
            }

            // Encode Plutus data to CBOR hex (simplified)
            std::string encode_plutus_data_impl(const PlutusData &data)
            {
                std::stringstream ss;
                ss << "cbor_";

                switch (data.type)
                {
                case PlutusDataType::INTEGER:
                    ss << "int_" << std::get<int64_t>(data.value);
                    break;
                case PlutusDataType::BYTESTRING:
                    ss << "bytes_" << std::get<std::vector<uint8_t>>(data.value).size();
                    break;
                case PlutusDataType::BOOLEAN:
                    ss << "bool_" << (std::get<bool>(data.value) ? "true" : "false");
                    break;
                case PlutusDataType::LIST:
                    ss << "list_" << std::get<std::vector<std::shared_ptr<PlutusData>>>(data.value).size();
                    break;
                case PlutusDataType::MAP:
                    ss << "map_" << std::get<std::map<std::string, std::shared_ptr<PlutusData>>>(data.value).size();
                    break;
                case PlutusDataType::CONSTRUCTOR:
                {
                    auto constructor = std::get<std::pair<uint64_t, std::vector<std::shared_ptr<PlutusData>>>>(data.value);
                    ss << "constr_" << constructor.first << "_" << constructor.second.size();
                }
                break;
                case PlutusDataType::UNIT:
                    ss << "unit";
                    break;
                }

                return ss.str();
            }

            // Create built-in contract templates
            void initialize_templates()
            {
                // Device Identity Contract Template
                ContractTemplate device_identity;
                device_identity.template_id = "device_identity_v1";
                device_identity.name = "Device Identity Contract";
                device_identity.script_template = "device_identity_template_cbor";
                device_identity.parameter_placeholders["DEVICE_ID"] = "device_id";
                device_identity.parameter_placeholders["PUBLIC_KEY"] = "public_key";
                device_identity.required_parameters = {"device_id", "public_key"};
                templates_[device_identity.template_id] = device_identity;

                // Data Oracle Contract Template
                ContractTemplate data_oracle;
                data_oracle.template_id = "data_oracle_v1";
                data_oracle.name = "Data Oracle Contract";
                data_oracle.script_template = "data_oracle_template_cbor";
                data_oracle.parameter_placeholders["DATA_SOURCE"] = "data_source";
                data_oracle.parameter_placeholders["UPDATE_INTERVAL"] = "update_interval";
                data_oracle.required_parameters = {"data_source", "authorized_updaters"};
                templates_[data_oracle.template_id] = data_oracle;

                // Payment Channel Contract Template
                ContractTemplate payment_channel;
                payment_channel.template_id = "payment_channel_v1";
                payment_channel.name = "Payment Channel Contract";
                payment_channel.script_template = "payment_channel_template_cbor";
                payment_channel.parameter_placeholders["SENDER"] = "sender_address";
                payment_channel.parameter_placeholders["RECEIVER"] = "receiver_address";
                payment_channel.required_parameters = {"sender_address", "receiver_address", "deposit_amount"};
                templates_[payment_channel.template_id] = payment_channel;
            }

            // Process contract events
            void process_event(const ContractEvent &event)
            {
                // Store event
                contract_events_[event.contract_address].push_back(event);

                // Notify subscribers
                auto contract_it = event_subscriptions_.find(event.contract_address);
                if (contract_it != event_subscriptions_.end())
                {
                    auto event_it = contract_it->second.find(event.event_name);
                    if (event_it != contract_it->second.end() && event_it->second)
                    {
                        event_it->second(event);
                    }

                    // Also check for wildcard subscriptions
                    auto wildcard_it = contract_it->second.find("*");
                    if (wildcard_it != contract_it->second.end() && wildcard_it->second)
                    {
                        wildcard_it->second(event);
                    }
                }

                // Update statistics
                std::lock_guard<std::mutex> stats_lock(stats_mutex_);
                stats_.events_emitted++;
            }
        };

        // Constructor/Destructor
        SmartContractInterface::SmartContractInterface() : pimpl_(std::make_unique<Impl>()) {}
        SmartContractInterface::~SmartContractInterface() = default;

        bool SmartContractInterface::initialize(const std::string &network)
        {
            std::lock_guard<std::mutex> lock(pimpl_->contracts_mutex_);

            if (pimpl_->initialized_)
            {
                return true;
            }

            pimpl_->network_ = network;
            pimpl_->config_ = ContractConfig{}; // Default config
            pimpl_->initialize_templates();

            pimpl_->initialized_ = true;

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Smart contract interface initialized for network: " + network);
            return true;
        }

        void SmartContractInterface::shutdown()
        {
            std::lock_guard<std::mutex> lock(pimpl_->contracts_mutex_);

            if (!pimpl_->initialized_)
            {
                return;
            }

            // Clear all data
            pimpl_->contracts_.clear();
            pimpl_->deployments_.clear();
            pimpl_->execution_logs_.clear();
            pimpl_->contract_events_.clear();
            pimpl_->contract_states_.clear();
            pimpl_->event_subscriptions_.clear();
            pimpl_->state_watchers_.clear();
            pimpl_->initialized_ = false;

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Smart contract interface shut down");
        }

        std::string SmartContractInterface::deploy_contract(
            const std::string &script_cbor,
            const std::map<std::string, PlutusData> &init_params,
            const std::string &deployer_address,
            const std::string &device_id)
        {
            if (!pimpl_->initialized_)
            {
                return "";
            }

            std::string deployment_id = pimpl_->generate_id("deploy");
            std::string contract_address = pimpl_->generate_contract_address();

            auto deployment = std::make_shared<ContractDeployment>();
            deployment->deployment_id = deployment_id;
            deployment->contract_id = pimpl_->generate_id("contract");
            deployment->address = contract_address;
            deployment->network = pimpl_->network_;
            deployment->deployer_address = deployer_address;
            deployment->deployment_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                                   std::chrono::system_clock::now().time_since_epoch())
                                                   .count();
            deployment->deployment_tx_hash = pimpl_->generate_id("tx");
            deployment->initialization_parameters = init_params;

            {
                std::lock_guard<std::mutex> lock(pimpl_->deployments_mutex_);
                pimpl_->deployments_[deployment_id] = deployment;
            }

            // Initialize contract state
            auto state = std::make_shared<ContractState>();
            state->contract_address = contract_address;
            state->last_updated_timestamp = deployment->deployment_timestamp;
            state->last_update_tx_hash = deployment->deployment_tx_hash;
            state->state_version = 1;

            // Copy initialization parameters to state
            for (const auto &[key, value] : init_params)
            {
                state->state_variables[key] = value;
            }

            {
                std::lock_guard<std::mutex> lock(pimpl_->contracts_mutex_);
                pimpl_->contract_states_[contract_address] = state;
            }

            // Update statistics
            {
                std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                pimpl_->stats_.active_deployments++;
            }

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Contract deployed: " + deployment_id + " at " + contract_address);

            return deployment_id;
        }

        bool SmartContractInterface::register_contract(const SmartContract &contract)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->contracts_mutex_);

            auto contract_ptr = std::make_shared<SmartContract>(contract);
            pimpl_->contracts_[contract.contract_id] = contract_ptr;

            // Update statistics
            {
                std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                pimpl_->stats_.total_contracts++;
            }

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Contract registered: " + contract.contract_id);
            return true;
        }

        std::shared_ptr<SmartContract> SmartContractInterface::get_contract(const std::string &contract_id) const
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            std::lock_guard<std::mutex> lock(pimpl_->contracts_mutex_);

            auto it = pimpl_->contracts_.find(contract_id);
            if (it != pimpl_->contracts_.end())
            {
                return it->second;
            }

            return nullptr;
        }

        std::vector<SmartContract> SmartContractInterface::list_contracts() const
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->contracts_mutex_);

            std::vector<SmartContract> contracts;
            for (const auto &[contract_id, contract_ptr] : pimpl_->contracts_)
            {
                contracts.push_back(*contract_ptr);
            }

            return contracts;
        }

        bool SmartContractInterface::remove_contract(const std::string &contract_id)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->contracts_mutex_);

            auto it = pimpl_->contracts_.find(contract_id);
            if (it != pimpl_->contracts_.end())
            {
                pimpl_->contracts_.erase(it);

                utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                              "Contract removed: " + contract_id);
                return true;
            }

            return false;
        }

        std::shared_ptr<ContractDeployment> SmartContractInterface::get_deployment(const std::string &deployment_id) const
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            std::lock_guard<std::mutex> lock(pimpl_->deployments_mutex_);

            auto it = pimpl_->deployments_.find(deployment_id);
            if (it != pimpl_->deployments_.end())
            {
                return it->second;
            }

            return nullptr;
        }

        std::vector<ContractDeployment> SmartContractInterface::list_deployments(const std::string &contract_id) const
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->deployments_mutex_);

            std::vector<ContractDeployment> deployments;
            for (const auto &[deployment_id, deployment_ptr] : pimpl_->deployments_)
            {
                if (contract_id.empty() || deployment_ptr->contract_id == contract_id)
                {
                    deployments.push_back(*deployment_ptr);
                }
            }

            return deployments;
        }

        std::string SmartContractInterface::call_contract_function(
            const std::string &contract_address,
            const std::string &function_name,
            const std::vector<PlutusData> &parameters,
            const std::string &caller_address,
            uint64_t amount_lovelace,
            const std::string &device_id)
        {
            if (!pimpl_->initialized_)
            {
                return "";
            }

            std::string execution_id = pimpl_->generate_id("exec");
            auto start_time = std::chrono::high_resolution_clock::now();

            // Simulate execution
            ExecutionResult result = pimpl_->simulate_execution(contract_address, function_name, parameters);

            auto end_time = std::chrono::high_resolution_clock::now();
            auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

            // Create execution log
            auto log = std::make_shared<ExecutionLog>();
            log->execution_id = execution_id;
            log->contract_address = contract_address;
            log->function_name = function_name;
            log->parameters = parameters;
            log->result = result;
            log->gas_used = pimpl_->estimate_gas_usage(function_name, parameters);
            log->execution_time_ms = execution_time;
            log->timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::system_clock::now().time_since_epoch())
                                 .count();
            log->transaction_hash = pimpl_->generate_id("tx");

            if (result != ExecutionResult::SUCCESS)
            {
                log->error_message = "Simulated execution error";
            }

            {
                std::lock_guard<std::mutex> lock(pimpl_->executions_mutex_);
                pimpl_->execution_logs_[execution_id] = log;
            }

            // Update statistics
            {
                std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                pimpl_->stats_.total_executions++;
                if (result == ExecutionResult::SUCCESS)
                {
                    pimpl_->stats_.successful_executions++;
                }
                else
                {
                    pimpl_->stats_.failed_executions++;
                }
                pimpl_->stats_.total_gas_used += log->gas_used;
                pimpl_->stats_.avg_execution_time_ms =
                    (pimpl_->stats_.avg_execution_time_ms + execution_time) / 2.0;
            }

            // Emit event if execution was successful
            if (result == ExecutionResult::SUCCESS)
            {
                ContractEvent event;
                event.event_id = pimpl_->generate_id("event");
                event.contract_address = contract_address;
                event.event_name = "FunctionCalled";
                event.event_data["function"] = PlutusData(std::vector<uint8_t>(function_name.begin(), function_name.end()));
                event.event_data["caller"] = PlutusData(std::vector<uint8_t>(caller_address.begin(), caller_address.end()));
                event.event_data["amount"] = PlutusData(static_cast<int64_t>(amount_lovelace));
                event.timestamp = log->timestamp;
                event.transaction_hash = log->transaction_hash;
                event.block_number = 12345; // Mock block number

                pimpl_->process_event(event);
            }

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Function called: " + function_name + " on " + contract_address +
                                              " (result: " + (result == ExecutionResult::SUCCESS ? "SUCCESS" : "ERROR") + ")");

            return execution_id;
        }

        std::string SmartContractInterface::call_readonly_function(
            const std::string &contract_address,
            const std::string &function_name,
            const std::vector<PlutusData> &parameters) const
        {
            if (!pimpl_->initialized_)
            {
                return "";
            }

            // Simplified readonly call - just return encoded result
            std::stringstream ss;
            ss << "readonly_result_" << function_name << "_" << parameters.size();

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Readonly function called: " + function_name + " on " + contract_address);

            return ss.str();
        }

        std::shared_ptr<ContractState> SmartContractInterface::get_contract_state(const std::string &contract_address) const
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            std::lock_guard<std::mutex> lock(pimpl_->contracts_mutex_);

            auto it = pimpl_->contract_states_.find(contract_address);
            if (it != pimpl_->contract_states_.end())
            {
                return it->second;
            }

            return nullptr;
        }

        PlutusData SmartContractInterface::query_state_variable(
            const std::string &contract_address,
            const std::string &variable_name) const
        {
            auto state = get_contract_state(contract_address);
            if (state)
            {
                auto it = state->state_variables.find(variable_name);
                if (it != state->state_variables.end())
                {
                    return it->second;
                }
            }

            return PlutusData(); // Return unit if not found
        }

        bool SmartContractInterface::watch_state_changes(
            const std::string &contract_address,
            const std::string &variable_name,
            std::function<void(const PlutusData &)> callback)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->contracts_mutex_);
            pimpl_->state_watchers_[contract_address][variable_name] = callback;

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "State watcher added for " + contract_address + "." + variable_name);
            return true;
        }

        bool SmartContractInterface::subscribe_to_events(
            const std::string &contract_address,
            const std::string &event_name,
            EventCallback callback)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->events_mutex_);
            pimpl_->event_subscriptions_[contract_address][event_name] = callback;

            // Update statistics
            {
                std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                pimpl_->stats_.active_subscriptions++;
            }

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Event subscription added: " + contract_address + "." + event_name);
            return true;
        }

        bool SmartContractInterface::unsubscribe_from_events(
            const std::string &contract_address,
            const std::string &event_name)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->events_mutex_);

            auto contract_it = pimpl_->event_subscriptions_.find(contract_address);
            if (contract_it != pimpl_->event_subscriptions_.end())
            {
                auto event_it = contract_it->second.find(event_name);
                if (event_it != contract_it->second.end())
                {
                    contract_it->second.erase(event_it);

                    // Update statistics
                    {
                        std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                        if (pimpl_->stats_.active_subscriptions > 0)
                        {
                            pimpl_->stats_.active_subscriptions--;
                        }
                    }

                    utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                                  "Event subscription removed: " + contract_address + "." + event_name);
                    return true;
                }
            }

            return false;
        }

        std::vector<ContractEvent> SmartContractInterface::get_events(
            const std::string &contract_address,
            const std::string &event_name,
            uint64_t from_timestamp,
            uint64_t to_timestamp) const
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->events_mutex_);

            std::vector<ContractEvent> result;
            auto it = pimpl_->contract_events_.find(contract_address);
            if (it != pimpl_->contract_events_.end())
            {
                for (const auto &event : it->second)
                {
                    if (!event_name.empty() && event.event_name != event_name)
                    {
                        continue;
                    }
                    if (from_timestamp > 0 && event.timestamp < from_timestamp)
                    {
                        continue;
                    }
                    if (to_timestamp > 0 && event.timestamp > to_timestamp)
                    {
                        continue;
                    }
                    result.push_back(event);
                }
            }

            return result;
        }

        std::vector<ExecutionLog> SmartContractInterface::get_execution_logs(
            const std::string &contract_address,
            const std::string &function_name,
            uint64_t from_timestamp,
            uint64_t to_timestamp) const
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->executions_mutex_);

            std::vector<ExecutionLog> result;
            for (const auto &[execution_id, log_ptr] : pimpl_->execution_logs_)
            {
                if (log_ptr->contract_address != contract_address)
                {
                    continue;
                }
                if (!function_name.empty() && log_ptr->function_name != function_name)
                {
                    continue;
                }
                if (from_timestamp > 0 && log_ptr->timestamp < from_timestamp)
                {
                    continue;
                }
                if (to_timestamp > 0 && log_ptr->timestamp > to_timestamp)
                {
                    continue;
                }
                result.push_back(*log_ptr);
            }

            return result;
        }

        std::shared_ptr<ExecutionLog> SmartContractInterface::get_execution_log(const std::string &execution_id) const
        {
            if (!pimpl_->initialized_)
            {
                return nullptr;
            }

            std::lock_guard<std::mutex> lock(pimpl_->executions_mutex_);

            auto it = pimpl_->execution_logs_.find(execution_id);
            if (it != pimpl_->execution_logs_.end())
            {
                return it->second;
            }

            return nullptr;
        }

        uint64_t SmartContractInterface::estimate_gas(
            const std::string &contract_address,
            const std::string &function_name,
            const std::vector<PlutusData> &parameters) const
        {
            return pimpl_->estimate_gas_usage(function_name, parameters);
        }

        uint64_t SmartContractInterface::estimate_execution_units(
            const std::string &script_cbor,
            const ExecutionContext &context,
            const std::vector<PlutusData> &parameters) const
        {
            // Simplified estimation based on script size and parameters
            uint64_t base_units = script_cbor.length() * 10;
            uint64_t param_units = parameters.size() * 50000;

            return base_units + param_units;
        }

        std::string SmartContractInterface::encode_plutus_data(const PlutusData &data) const
        {
            return pimpl_->encode_plutus_data_impl(data);
        }

        PlutusData SmartContractInterface::decode_plutus_data(const std::string &cbor_hex) const
        {
            // Simplified decoding for demo
            if (cbor_hex.find("int_") != std::string::npos)
            {
                return PlutusData(static_cast<int64_t>(42)); // Mock integer
            }
            else if (cbor_hex.find("bool_") != std::string::npos)
            {
                return PlutusData(true); // Mock boolean
            }
            else
            {
                return PlutusData(); // Return unit for unknown types
            }
        }

        PlutusData SmartContractInterface::create_integer(int64_t value)
        {
            return PlutusData(value);
        }

        PlutusData SmartContractInterface::create_bytestring(const std::vector<uint8_t> &bytes)
        {
            return PlutusData(bytes);
        }

        PlutusData SmartContractInterface::create_list(const std::vector<PlutusData> &items)
        {
            PlutusData data;
            data.type = PlutusDataType::LIST;

            std::vector<std::shared_ptr<PlutusData>> shared_items;
            for (const auto &item : items)
            {
                shared_items.push_back(std::make_shared<PlutusData>(item));
            }

            data.value = shared_items;
            return data;
        }

        PlutusData SmartContractInterface::create_map(const std::map<std::string, PlutusData> &items)
        {
            PlutusData data;
            data.type = PlutusDataType::MAP;

            std::map<std::string, std::shared_ptr<PlutusData>> shared_items;
            for (const auto &[key, value] : items)
            {
                shared_items[key] = std::make_shared<PlutusData>(value);
            }

            data.value = shared_items;
            return data;
        }

        PlutusData SmartContractInterface::create_constructor(uint64_t tag, const std::vector<PlutusData> &fields)
        {
            PlutusData data;
            data.type = PlutusDataType::CONSTRUCTOR;

            std::vector<std::shared_ptr<PlutusData>> shared_fields;
            for (const auto &field : fields)
            {
                shared_fields.push_back(std::make_shared<PlutusData>(field));
            }

            data.value = std::make_pair(tag, shared_fields);
            return data;
        }

        bool SmartContractInterface::validate_script(const std::string &script_cbor) const
        {
            // Simplified validation - check if it's a valid hex string and not empty
            if (script_cbor.empty())
            {
                return false;
            }

            // Check if it contains valid hex characters
            for (char c : script_cbor)
            {
                if (!std::isxdigit(c))
                {
                    return false;
                }
            }

            return true;
        }

        bool SmartContractInterface::verify_execution(
            const std::string &script_cbor,
            const ExecutionContext &context,
            const std::vector<PlutusData> &parameters) const
        {
            // Simplified verification for demo
            return validate_script(script_cbor) && !parameters.empty();
        }

        std::string SmartContractInterface::execute_batch_calls(
            const std::vector<std::tuple<std::string, std::string, std::vector<PlutusData>>> &calls,
            const std::string &caller_address,
            const std::string &device_id)
        {
            if (!pimpl_->initialized_)
            {
                return "";
            }

            std::string batch_id = pimpl_->generate_id("batch");

            for (const auto &[contract_address, function_name, parameters] : calls)
            {
                call_contract_function(contract_address, function_name, parameters, caller_address, 0, device_id);
            }

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Batch execution completed: " + batch_id + " (" +
                                              std::to_string(calls.size()) + " calls)");

            return batch_id;
        }

        bool SmartContractInterface::create_contract_composition(
            const std::string &composition_name,
            const std::vector<std::string> &contract_addresses,
            const std::map<std::string, std::string> &data_flow)
        {
            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Contract composition created: " + composition_name + " with " +
                                              std::to_string(contract_addresses.size()) + " contracts");
            return true;
        }

        bool SmartContractInterface::register_template(const ContractTemplate &template_spec)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->contracts_mutex_);
            pimpl_->templates_[template_spec.template_id] = template_spec;

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Contract template registered: " + template_spec.template_id);
            return true;
        }

        std::string SmartContractInterface::deploy_from_template(
            const std::string &template_id,
            const std::map<std::string, PlutusData> &template_parameters,
            const std::string &deployer_address,
            const std::string &device_id)
        {
            if (!pimpl_->initialized_)
            {
                return "";
            }

            std::lock_guard<std::mutex> lock(pimpl_->contracts_mutex_);

            auto it = pimpl_->templates_.find(template_id);
            if (it == pimpl_->templates_.end())
            {
                utils::Logger::instance().log(utils::LogLevel::ERROR, "SmartContractInterface",
                                              "Template not found: " + template_id);
                return "";
            }

            // Create deployment with template
            std::string deployment_id = deploy_contract(
                it->second.script_template + "_instantiated",
                template_parameters,
                deployer_address,
                device_id);

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Contract deployed from template " + template_id + ": " + deployment_id);

            return deployment_id;
        }

        std::string SmartContractInterface::create_device_identity_contract(
            const std::string &device_id,
            const std::string &public_key,
            const std::map<std::string, std::string> &attributes)
        {
            std::map<std::string, PlutusData> params;
            params["device_id"] = PlutusData(std::vector<uint8_t>(device_id.begin(), device_id.end()));
            params["public_key"] = PlutusData(std::vector<uint8_t>(public_key.begin(), public_key.end()));

            for (const auto &[key, value] : attributes)
            {
                params[key] = PlutusData(std::vector<uint8_t>(value.begin(), value.end()));
            }

            return deploy_from_template("device_identity_v1", params, "system", device_id);
        }

        std::string SmartContractInterface::create_data_oracle_contract(
            const std::string &data_source,
            const std::vector<std::string> &authorized_updaters,
            uint64_t update_interval_seconds)
        {
            std::map<std::string, PlutusData> params;
            params["data_source"] = PlutusData(std::vector<uint8_t>(data_source.begin(), data_source.end()));
            params["update_interval"] = PlutusData(static_cast<int64_t>(update_interval_seconds));

            // Create list of authorized updaters
            std::vector<PlutusData> updaters;
            for (const auto &updater : authorized_updaters)
            {
                updaters.push_back(PlutusData(std::vector<uint8_t>(updater.begin(), updater.end())));
            }
            params["authorized_updaters"] = create_list(updaters);

            return deploy_from_template("data_oracle_v1", params, "system", "");
        }

        std::string SmartContractInterface::create_payment_channel_contract(
            const std::string &sender_address,
            const std::string &receiver_address,
            uint64_t deposit_amount,
            uint64_t timeout_slots)
        {
            std::map<std::string, PlutusData> params;
            params["sender_address"] = PlutusData(std::vector<uint8_t>(sender_address.begin(), sender_address.end()));
            params["receiver_address"] = PlutusData(std::vector<uint8_t>(receiver_address.begin(), receiver_address.end()));
            params["deposit_amount"] = PlutusData(static_cast<int64_t>(deposit_amount));
            params["timeout_slots"] = PlutusData(static_cast<int64_t>(timeout_slots));

            return deploy_from_template("payment_channel_v1", params, sender_address, "");
        }

        SmartContractInterface::ContractStats SmartContractInterface::get_statistics() const
        {
            std::lock_guard<std::mutex> lock(pimpl_->stats_mutex_);
            return pimpl_->stats_;
        }

        void SmartContractInterface::reset_statistics()
        {
            std::lock_guard<std::mutex> lock(pimpl_->stats_mutex_);
            pimpl_->stats_ = {};

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Statistics reset");
        }

        void SmartContractInterface::update_config(const ContractConfig &config)
        {
            pimpl_->config_ = config;

            utils::Logger::instance().log(utils::LogLevel::INFO, "SmartContractInterface",
                                          "Configuration updated");
        }

        SmartContractInterface::ContractConfig SmartContractInterface::get_config() const
        {
            return pimpl_->config_;
        }

    } // namespace core
} // namespace cardano_iot
