#pragma once

#include <string>
#include "cardano_iot/network/cardano_client.h"

namespace cardano_iot
{
    namespace network_utils
    {

        inline std::string network_to_string(cardano_iot::network::Network net)
        {
            using cardano_iot::network::Network;
            switch (net)
            {
            case Network::MAINNET:
                return "mainnet";
            case Network::TESTNET:
                return "testnet";
            case Network::PREPROD:
                return "preprod";
            case Network::PREVIEW:
                return "preview";
            case Network::LOCAL:
                return "local";
            }
            return "testnet";
        }

        inline cardano_iot::network::Network parse_network(const std::string &s)
        {
            if (s == "mainnet")
                return cardano_iot::network::Network::MAINNET;
            if (s == "preprod")
                return cardano_iot::network::Network::PREPROD;
            if (s == "preview")
                return cardano_iot::network::Network::PREVIEW;
            if (s == "local")
                return cardano_iot::network::Network::LOCAL;
            return cardano_iot::network::Network::TESTNET;
        }

        inline std::string address_prefix(cardano_iot::network::Network net, bool stake = false)
        {
            using cardano_iot::network::Network;
            switch (net)
            {
            case Network::MAINNET:
                return stake ? "stake1" : "addr1";
            case Network::TESTNET:
                return stake ? "stake_test1" : "addr_test1";
            case Network::PREPROD:
                return stake ? "stake_test1" : "addr_test1";
            case Network::PREVIEW:
                return stake ? "stake_test1" : "addr_test1";
            case Network::LOCAL:
                return stake ? "stake_test1" : "addr_test1";
            }
            return stake ? "stake_test1" : "addr_test1";
        }

    } // namespace network_utils
} // namespace cardano_iot
