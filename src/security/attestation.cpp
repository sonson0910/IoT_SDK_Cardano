#include "cardano_iot/security/attestation.h"
#include "cardano_iot/utils/logger.h"

#include <algorithm>

namespace cardano_iot
{
    namespace security
    {

        AttestationResult AttestationVerifier::verify_quote(const AttestationEvidence &evidence,
                                                            const std::string &expected_nonce,
                                                            const std::vector<std::string> & /*trusted_roots*/)
        {
            AttestationResult res{};
            // Minimal checks (mock). In a real implementation, parse the quote and verify signature
            if (evidence.nonce != expected_nonce)
            {
                res.valid = false;
                res.error = "Nonce mismatch";
                return res;
            }
            if (evidence.quote.empty())
            {
                res.valid = false;
                res.error = "Empty quote";
                return res;
            }
            // Accept with claims for now
            res.valid = true;
            res.verified_claims = evidence.claims;
            cardano_iot::utils::Logger::instance().log(cardano_iot::utils::LogLevel::INFO,
                                                       "Attestation", "Attestation verified for device: " + evidence.device_id);
            return res;
        }

    } // namespace security
} // namespace cardano_iot
