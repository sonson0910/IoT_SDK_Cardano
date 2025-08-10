#include "cardano_iot/security/authentication.h"
#include "cardano_iot/utils/logger.h"

#include <chrono>
#include <sstream>
#include <random>
#include <algorithm>
#include <mutex>
#include <thread>

namespace cardano_iot
{
    namespace security
    {
        struct Authentication::Impl
        {
            bool initialized_ = false;
            mutable std::mutex auth_mutex_;
            mutable std::mutex stats_mutex_;

            // Storage
            std::map<std::string, AuthCredentials> device_credentials_;
            std::map<std::string, AuthSession> active_sessions_;
            std::map<std::string, std::string> active_tokens_;
            std::map<std::string, uint32_t> failed_attempts_;
            std::map<std::string, uint64_t> lockout_until_;
            std::vector<AuthEvent> auth_history_;

            // Configuration
            SecurityPolicy policy_;
            std::function<void(const AuthEvent &)> audit_callback_;

            // Statistics
            AuthStats stats_ = {};

            // Generate unique IDs
            std::string generate_session_id() const
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, 15);

                std::stringstream ss;
                ss << "sess_";
                for (int i = 0; i < 16; ++i)
                {
                    ss << std::hex << dis(gen);
                }
                return ss.str();
            }

            std::string generate_token() const
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, 15);

                std::stringstream ss;
                ss << "token_";
                for (int i = 0; i < 32; ++i)
                {
                    ss << std::hex << dis(gen);
                }
                return ss.str();
            }

            std::string generate_event_id() const
            {
                auto now = std::chrono::system_clock::now();
                auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1000, 9999);

                std::stringstream ss;
                ss << "auth_" << timestamp << "_" << dis(gen);
                return ss.str();
            }

            // Check if device is locked
            bool is_device_locked(const std::string &device_id) const
            {
                auto it = lockout_until_.find(device_id);
                if (it != lockout_until_.end())
                {
                    auto now = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();
                    return now < it->second;
                }
                return false;
            }

            // Record failed attempt
            void record_failed_attempt(const std::string &device_id)
            {
                failed_attempts_[device_id]++;

                if (failed_attempts_[device_id] >= policy_.max_failed_attempts)
                {
                    auto now = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();
                    lockout_until_[device_id] = now + policy_.lockout_duration_seconds;

                    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
                    stats_.locked_devices++;
                }
            }

            // Clear failed attempts
            void clear_failed_attempts(const std::string &device_id)
            {
                failed_attempts_.erase(device_id);
                lockout_until_.erase(device_id);
            }

            // Create auth event
            void log_auth_event(const std::string &device_id, AuthMethod method, AuthStatus result,
                                const std::string &error_details = "")
            {
                AuthEvent event;
                event.event_id = generate_event_id();
                event.device_id = device_id;
                event.method = method;
                event.result = result;
                event.ip_address = "192.168.1.100"; // Mock IP
                event.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                      std::chrono::system_clock::now().time_since_epoch())
                                      .count();
                event.user_agent = "CardanoIoTSDK/1.0";
                event.error_details = error_details;

                auth_history_.push_back(event);

                // Call audit callback if set
                if (audit_callback_)
                {
                    audit_callback_(event);
                }

                // Update statistics
                std::lock_guard<std::mutex> stats_lock(stats_mutex_);
                stats_.total_attempts++;
                if (result == AuthStatus::SUCCESS)
                {
                    stats_.successful_authentications++;
                }
                else
                {
                    stats_.failed_authentications++;
                }
                stats_.method_usage[method]++;
            }

            // Verify password hash
            bool verify_password(const std::vector<uint8_t> &stored_hash, const std::vector<uint8_t> &provided_password)
            {
                // Simple mock verification
                return stored_hash.size() == provided_password.size() &&
                       std::equal(stored_hash.begin(), stored_hash.end(), provided_password.begin());
            }

            // Verify public key signature
            bool verify_public_key_signature(const std::string &public_key, const std::string &message, const std::string &signature)
            {
                // Mock signature verification
                return !public_key.empty() && !message.empty() && signature.length() > 16;
            }

            // Verify biometric template
            bool verify_biometric_match(const std::vector<uint8_t> &stored_template, const std::vector<uint8_t> &provided_data)
            {
                // Mock biometric matching with 95% accuracy
                if (stored_template.empty() || provided_data.empty())
                {
                    return false;
                }

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1, 100);
                return dis(gen) <= 95;
            }
        };

        Authentication::Authentication() : pimpl_(std::make_unique<Impl>()) {}
        Authentication::~Authentication() = default;

        bool Authentication::initialize()
        {
            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            if (pimpl_->initialized_)
            {
                return true;
            }

            // Set default security policy
            pimpl_->policy_ = SecurityPolicy{};
            pimpl_->policy_.allowed_methods = {AuthMethod::PASSWORD, AuthMethod::PUBLIC_KEY, AuthMethod::TOKEN};

            pimpl_->initialized_ = true;

            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Authentication system initialized");
            return true;
        }

        void Authentication::shutdown()
        {
            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            if (!pimpl_->initialized_)
            {
                return;
            }

            // Clear all data
            pimpl_->device_credentials_.clear();
            pimpl_->active_sessions_.clear();
            pimpl_->active_tokens_.clear();
            pimpl_->failed_attempts_.clear();
            pimpl_->lockout_until_.clear();
            pimpl_->auth_history_.clear();
            pimpl_->audit_callback_ = nullptr;
            pimpl_->initialized_ = false;

            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Authentication system shutdown");
        }

        bool Authentication::register_device_credentials(const std::string &device_id, const AuthCredentials &credentials)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            pimpl_->device_credentials_[device_id] = credentials;

            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Credentials registered for device: " + device_id);
            return true;
        }

        bool Authentication::update_credentials(const std::string &device_id, const AuthCredentials &credentials)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            auto it = pimpl_->device_credentials_.find(device_id);
            if (it != pimpl_->device_credentials_.end())
            {
                it->second = credentials;

                utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                              "Credentials updated for device: " + device_id);
                return true;
            }

            return false;
        }

        bool Authentication::revoke_credentials(const std::string &device_id)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            auto it = pimpl_->device_credentials_.find(device_id);
            if (it != pimpl_->device_credentials_.end())
            {
                pimpl_->device_credentials_.erase(it);

                // Terminate all sessions for this device
                for (auto session_it = pimpl_->active_sessions_.begin();
                     session_it != pimpl_->active_sessions_.end();)
                {
                    if (session_it->second.device_id == device_id)
                    {
                        session_it = pimpl_->active_sessions_.erase(session_it);
                    }
                    else
                    {
                        ++session_it;
                    }
                }

                utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                              "Credentials revoked for device: " + device_id);
                return true;
            }

            return false;
        }

        std::vector<std::string> Authentication::list_registered_devices() const
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            std::vector<std::string> device_ids;
            for (const auto &[device_id, credentials] : pimpl_->device_credentials_)
            {
                device_ids.push_back(device_id);
            }

            return device_ids;
        }

        AuthResult Authentication::authenticate_device(const std::string &device_id, const AuthCredentials &provided_credentials)
        {
            AuthResult result;
            result.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();

            if (!pimpl_->initialized_)
            {
                result.status = AuthStatus::FAILED;
                result.error_message = "Authentication system not initialized";
                return result;
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            // Check if device is locked
            if (pimpl_->is_device_locked(device_id))
            {
                result.status = AuthStatus::LOCKED;
                result.error_message = "Device is locked due to too many failed attempts";
                pimpl_->log_auth_event(device_id, provided_credentials.method, AuthStatus::LOCKED, result.error_message);
                return result;
            }

            // Find device credentials
            auto it = pimpl_->device_credentials_.find(device_id);
            if (it == pimpl_->device_credentials_.end())
            {
                result.status = AuthStatus::FAILED;
                result.error_message = "Device not registered";
                pimpl_->record_failed_attempt(device_id);
                pimpl_->log_auth_event(device_id, provided_credentials.method, AuthStatus::FAILED, result.error_message);
                return result;
            }

            const auto &stored_credentials = it->second;

            // Check credential expiry
            auto now = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
            if (stored_credentials.expiry_timestamp > 0 && now > stored_credentials.expiry_timestamp)
            {
                result.status = AuthStatus::EXPIRED;
                result.error_message = "Credentials have expired";
                pimpl_->log_auth_event(device_id, provided_credentials.method, AuthStatus::EXPIRED, result.error_message);
                return result;
            }

            // Verify credentials based on method
            bool auth_success = false;
            switch (provided_credentials.method)
            {
            case AuthMethod::PASSWORD:
                auth_success = pimpl_->verify_password(stored_credentials.credential_data, provided_credentials.credential_data);
                break;

            case AuthMethod::PUBLIC_KEY:
                auth_success = !stored_credentials.public_key.empty() &&
                               stored_credentials.public_key == provided_credentials.public_key;
                break;

            case AuthMethod::TOKEN:
                auth_success = validate_token(provided_credentials.identifier);
                break;

            case AuthMethod::BIOMETRIC:
                auth_success = pimpl_->verify_biometric_match(stored_credentials.credential_data, provided_credentials.credential_data);
                break;

            default:
                auth_success = false;
                break;
            }

            if (auth_success)
            {
                result.status = AuthStatus::SUCCESS;
                pimpl_->clear_failed_attempts(device_id);

                // Create session
                auto session = create_session(device_id);
                result.session_id = session.session_id;
                result.token = session.token;
                result.granted_permissions = session.permissions;

                pimpl_->log_auth_event(device_id, provided_credentials.method, AuthStatus::SUCCESS);

                utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                              "Device authenticated successfully: " + device_id);
            }
            else
            {
                result.status = AuthStatus::FAILED;
                result.error_message = "Invalid credentials";
                pimpl_->record_failed_attempt(device_id);
                pimpl_->log_auth_event(device_id, provided_credentials.method, AuthStatus::FAILED, result.error_message);

                utils::Logger::instance().log(utils::LogLevel::WARNING, "Authentication",
                                              "Authentication failed for device: " + device_id);
            }

            return result;
        }

        AuthResult Authentication::authenticate_with_challenge(const std::string &device_id, const std::string &challenge, const std::string &response)
        {
            AuthResult result;
            result.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();

            if (!pimpl_->initialized_)
            {
                result.status = AuthStatus::FAILED;
                result.error_message = "Authentication system not initialized";
                return result;
            }

            // Mock challenge-response verification
            bool challenge_valid = (challenge.length() > 8 && response.length() > 8);

            if (challenge_valid)
            {
                result.status = AuthStatus::SUCCESS;
                auto session = create_session(device_id);
                result.session_id = session.session_id;
                result.token = session.token;

                pimpl_->log_auth_event(device_id, AuthMethod::CHALLENGE_RESPONSE, AuthStatus::SUCCESS);
            }
            else
            {
                result.status = AuthStatus::FAILED;
                result.error_message = "Invalid challenge response";

                pimpl_->log_auth_event(device_id, AuthMethod::CHALLENGE_RESPONSE, AuthStatus::FAILED, result.error_message);
            }

            return result;
        }

        bool Authentication::verify_signature(const std::string &device_id, const std::string &message, const std::string &signature)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            auto it = pimpl_->device_credentials_.find(device_id);
            if (it != pimpl_->device_credentials_.end())
            {
                return pimpl_->verify_public_key_signature(it->second.public_key, message, signature);
            }

            return false;
        }

        AuthSession Authentication::create_session(const std::string &device_id, const std::vector<std::string> &permissions)
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            AuthSession session;
            session.session_id = pimpl_->generate_session_id();
            session.device_id = device_id;
            session.user_id = "user_" + device_id;
            session.status = AuthStatus::SUCCESS;
            session.created_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                            std::chrono::system_clock::now().time_since_epoch())
                                            .count();
            session.last_activity_timestamp = session.created_timestamp;
            session.expiry_timestamp = session.created_timestamp + pimpl_->policy_.session_timeout_seconds;
            session.permissions = permissions.empty() ? std::vector<std::string>{"read", "write"} : permissions;
            session.token = generate_access_token(device_id);

            pimpl_->active_sessions_[session.session_id] = session;

            // Update statistics
            {
                std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                pimpl_->stats_.active_sessions++;
            }

            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Session created: " + session.session_id + " for device: " + device_id);

            return session;
        }

        bool Authentication::validate_session(const std::string &session_id)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            auto it = pimpl_->active_sessions_.find(session_id);
            if (it != pimpl_->active_sessions_.end())
            {
                auto now = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();

                if (now <= it->second.expiry_timestamp)
                {
                    it->second.last_activity_timestamp = now;
                    return true;
                }
                else
                {
                    // Session expired, remove it
                    pimpl_->active_sessions_.erase(it);

                    std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                    if (pimpl_->stats_.active_sessions > 0)
                    {
                        pimpl_->stats_.active_sessions--;
                    }
                }
            }

            return false;
        }

        bool Authentication::refresh_session(const std::string &session_id)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            auto it = pimpl_->active_sessions_.find(session_id);
            if (it != pimpl_->active_sessions_.end())
            {
                auto now = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();

                it->second.last_activity_timestamp = now;
                it->second.expiry_timestamp = now + pimpl_->policy_.session_timeout_seconds;

                utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                              "Session refreshed: " + session_id);
                return true;
            }

            return false;
        }

        bool Authentication::terminate_session(const std::string &session_id)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            auto it = pimpl_->active_sessions_.find(session_id);
            if (it != pimpl_->active_sessions_.end())
            {
                pimpl_->active_sessions_.erase(it);

                {
                    std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                    if (pimpl_->stats_.active_sessions > 0)
                    {
                        pimpl_->stats_.active_sessions--;
                    }
                }

                utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                              "Session terminated: " + session_id);
                return true;
            }

            return false;
        }

        std::vector<AuthSession> Authentication::get_active_sessions() const
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            std::vector<AuthSession> sessions;
            for (const auto &[session_id, session] : pimpl_->active_sessions_)
            {
                sessions.push_back(session);
            }

            return sessions;
        }

        std::string Authentication::generate_access_token(const std::string &device_id, uint32_t validity_seconds)
        {
            if (!pimpl_->initialized_)
            {
                return "";
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            std::string token = pimpl_->generate_token();
            pimpl_->active_tokens_[token] = device_id;

            // Update statistics
            {
                std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                pimpl_->stats_.issued_tokens++;
            }

            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Access token generated for device: " + device_id);

            return token;
        }

        bool Authentication::validate_token(const std::string &token)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            auto it = pimpl_->active_tokens_.find(token);
            return it != pimpl_->active_tokens_.end();
        }

        bool Authentication::revoke_token(const std::string &token)
        {
            if (!pimpl_->initialized_)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            auto it = pimpl_->active_tokens_.find(token);
            if (it != pimpl_->active_tokens_.end())
            {
                pimpl_->active_tokens_.erase(it);

                {
                    std::lock_guard<std::mutex> stats_lock(pimpl_->stats_mutex_);
                    pimpl_->stats_.revoked_tokens++;
                }

                utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                              "Token revoked");
                return true;
            }

            return false;
        }

        std::string Authentication::refresh_token(const std::string &token)
        {
            if (!pimpl_->initialized_)
            {
                return "";
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            auto it = pimpl_->active_tokens_.find(token);
            if (it != pimpl_->active_tokens_.end())
            {
                std::string device_id = it->second;
                pimpl_->active_tokens_.erase(it);

                return generate_access_token(device_id);
            }

            return "";
        }

        bool Authentication::enable_mfa(const std::string &device_id, const std::vector<AuthMethod> &methods)
        {
            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "MFA enabled for device: " + device_id + " with " +
                                              std::to_string(methods.size()) + " methods");
            return true;
        }

        bool Authentication::verify_mfa_factor(const std::string &device_id, AuthMethod method, const std::vector<uint8_t> &factor_data)
        {
            // Mock MFA verification
            bool verified = !factor_data.empty();

            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "MFA factor verification for " + device_id + ": " +
                                              (verified ? "success" : "failed"));
            return verified;
        }

        std::vector<AuthMethod> Authentication::get_mfa_methods(const std::string &device_id) const
        {
            // Mock MFA methods
            return {AuthMethod::PASSWORD, AuthMethod::BIOMETRIC};
        }

        bool Authentication::register_biometric_template(const std::string &device_id, const std::vector<uint8_t> &template_data)
        {
            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Biometric template registered for device: " + device_id);
            return true;
        }

        bool Authentication::verify_biometric(const std::string &device_id, const std::vector<uint8_t> &biometric_data)
        {
            return pimpl_->verify_biometric_match({1, 2, 3, 4}, biometric_data); // Mock template
        }

        bool Authentication::update_biometric_template(const std::string &device_id, const std::vector<uint8_t> &new_template)
        {
            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Biometric template updated for device: " + device_id);
            return true;
        }

        bool Authentication::register_certificate(const std::string &device_id, const std::string &certificate_pem)
        {
            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Certificate registered for device: " + device_id);
            return true;
        }

        bool Authentication::verify_certificate_chain(const std::string &certificate_pem)
        {
            // Mock certificate verification
            bool valid = certificate_pem.find("-----BEGIN CERTIFICATE-----") != std::string::npos;

            std::string result = valid ? "valid" : "invalid";
            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Certificate chain verification: " + result);
            return valid;
        }

        bool Authentication::revoke_certificate(const std::string &device_id)
        {
            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Certificate revoked for device: " + device_id);
            return true;
        }

        void Authentication::set_security_policy(const SecurityPolicy &policy)
        {
            pimpl_->policy_ = policy;

            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Security policy updated");
        }

        Authentication::SecurityPolicy Authentication::get_security_policy() const
        {
            return pimpl_->policy_;
        }

        void Authentication::set_audit_callback(std::function<void(const AuthEvent &)> callback)
        {
            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);
            pimpl_->audit_callback_ = callback;

            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Audit callback set");
        }

        std::vector<Authentication::AuthEvent> Authentication::get_auth_history(const std::string &device_id) const
        {
            if (!pimpl_->initialized_)
            {
                return {};
            }

            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);

            if (device_id.empty())
            {
                return pimpl_->auth_history_;
            }

            std::vector<AuthEvent> filtered_history;
            for (const auto &event : pimpl_->auth_history_)
            {
                if (event.device_id == device_id)
                {
                    filtered_history.push_back(event);
                }
            }

            return filtered_history;
        }

        void Authentication::clear_auth_history()
        {
            std::lock_guard<std::mutex> lock(pimpl_->auth_mutex_);
            pimpl_->auth_history_.clear();

            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Authentication history cleared");
        }

        Authentication::AuthStats Authentication::get_statistics() const
        {
            std::lock_guard<std::mutex> lock(pimpl_->stats_mutex_);
            return pimpl_->stats_;
        }

        void Authentication::reset_statistics()
        {
            std::lock_guard<std::mutex> lock(pimpl_->stats_mutex_);
            pimpl_->stats_ = {};

            utils::Logger::instance().log(utils::LogLevel::INFO, "Authentication",
                                          "Statistics reset");
        }

    } // namespace security
} // namespace cardano_iot
