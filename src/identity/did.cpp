#include "cardano_iot/identity/did.h"
#include "cardano_iot/network/network_utils.h"
#include "cardano_iot/utils/logger.h"

#include <sstream>
#include <functional>

namespace cardano_iot
{
    namespace identity
    {

        std::string DIDRegistry::create_did_from_public_key(const std::string &network,
                                                            const std::string &public_key_hex)
        {
            // did:cardano:<network>:<hash(public_key_hex)>
            auto net = network_utils::parse_network(network);
            std::string net_str = network_utils::network_to_string(net);
            std::hash<std::string> hasher;
            auto h = hasher(public_key_hex);
            std::stringstream ss;
            ss << "did:cardano:" << net_str << ":" << std::hex << h;
            return ss.str();
        }

        DIDDocument DIDRegistry::build_document(const std::string &did,
                                                const std::map<std::string, std::string> &public_keys,
                                                const std::map<std::string, std::string> &services,
                                                const std::map<std::string, std::string> &metadata)
        {
            DIDDocument doc;
            doc.id = did;
            doc.public_keys = public_keys;
            doc.services = services;
            doc.metadata = metadata;
            cardano_iot::utils::Logger::instance().log(cardano_iot::utils::LogLevel::INFO,
                                                       "DIDRegistry", "Built DID document: " + did);
            return doc;
        }

    } // namespace identity
} // namespace cardano_iot
