#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <cstdint>

namespace cardano_iot
{
    namespace security
    {
        enum class AuthMethod
        {
            PASSWORD,
            PUBLIC_KEY,
            BIOMETRIC,
            MULTI_FACTOR,
            CERTIFICATE,
            TOKEN,
            CHALLENGE_RESPONSE
        };

        enum class AuthStatus
        {
            SUCCESS,
            FAILED,
            EXPIRED,
            LOCKED,
            PENDING,
            REVOKED
        };

        struct AuthCredentials
        {
            AuthMethod method;
            std::string identifier;
            std::vector<uint8_t> credential_data;
            std::string public_key;
            std::string certificate;
            uint64_t expiry_timestamp;
            std::map<std::string, std::string> metadata;
        };

        struct AuthSession
        {
            std::string session_id;
            std::string device_id;
            std::string user_id;
            AuthStatus status;
            uint64_t created_timestamp;
            uint64_t last_activity_timestamp;
            uint64_t expiry_timestamp;
            std::vector<std::string> permissions;
            std::string token;
        };

        struct AuthResult
        {
            AuthStatus status;
            std::string session_id;
            std::string token;
            std::string error_message;
            uint64_t timestamp;
            std::vector<std::string> granted_permissions;
        };

        using AuthCallback = std::function<void(const AuthResult&)>;

        class Authentication
        {
        public:
            Authentication();
            ~Authentication();

            bool initialize();
            void shutdown();

            // Credential management
            bool register_device_credentials(const std::string& device_id, const AuthCredentials& credentials);
            bool update_credentials(const std::string& device_id, const AuthCredentials& credentials);
            bool revoke_credentials(const std::string& device_id);
            std::vector<std::string> list_registered_devices() const;

            // Authentication operations
            AuthResult authenticate_device(const std::string& device_id, const AuthCredentials& provided_credentials);
            AuthResult authenticate_with_challenge(const std::string& device_id, const std::string& challenge, const std::string& response);
            bool verify_signature(const std::string& device_id, const std::string& message, const std::string& signature);

            // Session management
            AuthSession create_session(const std::string& device_id, const std::vector<std::string>& permissions = {});
            bool validate_session(const std::string& session_id);
            bool refresh_session(const std::string& session_id);
            bool terminate_session(const std::string& session_id);
            std::vector<AuthSession> get_active_sessions() const;

            // Token operations
            std::string generate_access_token(const std::string& device_id, uint32_t validity_seconds = 3600);
            bool validate_token(const std::string& token);
            bool revoke_token(const std::string& token);
            std::string refresh_token(const std::string& token);

            // Multi-factor authentication
            bool enable_mfa(const std::string& device_id, const std::vector<AuthMethod>& methods);
            bool verify_mfa_factor(const std::string& device_id, AuthMethod method, const std::vector<uint8_t>& factor_data);
            std::vector<AuthMethod> get_mfa_methods(const std::string& device_id) const;

            // Biometric authentication
            bool register_biometric_template(const std::string& device_id, const std::vector<uint8_t>& template_data);
            bool verify_biometric(const std::string& device_id, const std::vector<uint8_t>& biometric_data);
            bool update_biometric_template(const std::string& device_id, const std::vector<uint8_t>& new_template);

            // Certificate-based authentication
            bool register_certificate(const std::string& device_id, const std::string& certificate_pem);
            bool verify_certificate_chain(const std::string& certificate_pem);
            bool revoke_certificate(const std::string& device_id);

            // Security policies
            struct SecurityPolicy
            {
                uint32_t max_failed_attempts = 3;
                uint32_t lockout_duration_seconds = 300;
                uint32_t session_timeout_seconds = 3600;
                uint32_t token_validity_seconds = 3600;
                bool require_mfa = false;
                bool enable_biometric = false;
                std::vector<AuthMethod> allowed_methods;
            };

            void set_security_policy(const SecurityPolicy& policy);
            SecurityPolicy get_security_policy() const;

            // Audit and monitoring
            struct AuthEvent
            {
                std::string event_id;
                std::string device_id;
                AuthMethod method;
                AuthStatus result;
                std::string ip_address;
                uint64_t timestamp;
                std::string user_agent;
                std::string error_details;
            };

            void set_audit_callback(std::function<void(const AuthEvent&)> callback);
            std::vector<AuthEvent> get_auth_history(const std::string& device_id = "") const;
            void clear_auth_history();

            // Statistics
            struct AuthStats
            {
                uint64_t total_attempts;
                uint64_t successful_authentications;
                uint64_t failed_authentications;
                uint64_t locked_devices;
                uint64_t active_sessions;
                uint64_t issued_tokens;
                uint64_t revoked_tokens;
                std::map<AuthMethod, uint64_t> method_usage;
            };

            AuthStats get_statistics() const;
            void reset_statistics();

        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl_;
        };

    } // namespace security
} // namespace cardano_iot