// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory_resource>
#include <set>
#include <string>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Options.hpp"

namespace opentxs
{
class Options;
}  // namespace opentxs

namespace ottest
{
struct OptionsData {
    std::pmr::set<std::string> blockchain_bind_ipv4_;
    std::pmr::set<std::string> blockchain_bind_ipv6_;
    std::pmr::set<opentxs::blockchain::Type> blockchain_disabled_chains_;
    int blockchain_storage_level_;
    bool blockchain_sync_server_enabled_;
    std::pmr::set<std::string> blockchain_sync_servers_;
    bool blockchain_wallet_enabled_;
    std::string home_;
    opentxs::Options::ConnectionMode ipv4_connection_mode_;
    opentxs::Options::ConnectionMode ipv6_connection_mode_;
    std::string log_endpoint_;
    int log_level_;
    std::size_t mint_key_bytes_;
    bool notary_bind_inproc_;
    std::string notary_bind_ip_;
    std::uint16_t notary_bind_port_;
    std::string notary_name_;
    std::pmr::set<std::string> notary_public_eep_;
    std::pmr::set<std::string> notary_public_ipv4_;
    std::pmr::set<std::string> notary_public_ipv6_;
    std::pmr::set<std::string> notary_public_onion_;
    std::uint16_t notary_public_port_;
    std::string notary_terms_;
    std::string storage_primary_plugin_;
    bool test_mode_;
};

auto check_options(
    const opentxs::Options& options,
    const OptionsData& expected) noexcept -> bool;
}  // namespace ottest
