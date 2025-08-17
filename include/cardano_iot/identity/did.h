#pragma once

#include <string>
#include <map>
#include <vector>

namespace cardano_iot
{
    namespace identity
    {

        // Lightweight DID document structure for IoT devices
        struct DIDDocument
        {
            std::string id;                                 // did:cardano:<hash>
            std::map<std::string, std::string> public_keys; // keyId -> base58/hex
            std::map<std::string, std::string> services;    // svcId -> endpoint
            std::map<std::string, std::string> metadata;    // arbitrary metadata
        };

        class DIDRegistry
        {
        public:
            // Create a DID from public key; returns DID string
            static std::string create_did_from_public_key(const std::string &network,
                                                          const std::string &public_key_hex);

            // Build a DID document for device
            static DIDDocument build_document(const std::string &did,
                                              const std::map<std::string, std::string> &public_keys,
                                              const std::map<std::string, std::string> &services = {},
                                              const std::map<std::string, std::string> &metadata = {});
        };

    } // namespace identity
} // namespace cardano_iot
