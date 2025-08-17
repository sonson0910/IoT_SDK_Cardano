#pragma once

#include <string>
#include <vector>
#include <map>

namespace cardano_iot
{
    namespace security
    {

        // Evidence provided by trusted hardware (TPM/TEE)
        struct AttestationEvidence
        {
            std::string device_id;
            std::string nonce;                         // server-provided
            std::string quote;                         // attestation quote blob (base64)
            std::string certificate;                   // device cert chain (PEM) if any
            std::map<std::string, std::string> claims; // extra claims (fw version, measurements)
        };

        // Result of attestation verification
        struct AttestationResult
        {
            bool valid;
            std::string error;
            std::map<std::string, std::string> verified_claims;
        };

        class AttestationVerifier
        {
        public:
            // Verify TEE/TPM quote (mock; hook for real verifier)
            static AttestationResult verify_quote(const AttestationEvidence &evidence,
                                                  const std::string &expected_nonce,
                                                  const std::vector<std::string> &trusted_roots = {});
        };

    } // namespace security
} // namespace cardano_iot
