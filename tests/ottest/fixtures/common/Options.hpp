// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <cstdint>
#include <filesystem>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
// NOLINTBEGIN(clang-analyzer-optin.performance.Padding)
struct OptionsData {
    ot::Set<ot::CString> blockchain_bind_ipv4_;
    ot::Set<ot::CString> blockchain_bind_ipv6_;
    ot::Set<opentxs::blockchain::Type> blockchain_disabled_chains_;
    ot::BlockchainProfile blockchain_profile_;
    bool blockchain_sync_server_enabled_;
    ot::Set<ot::CString> blockchain_sync_servers_;
    bool blockchain_wallet_enabled_;
    bool experimental_;
    std::filesystem::path home_;
    opentxs::ConnectionMode ipv4_connection_mode_;
    opentxs::ConnectionMode ipv6_connection_mode_;
    ot::CString log_endpoint_;
    int log_level_;
    unsigned int max_jobs_;
    std::size_t mint_key_bytes_;
    bool notary_bind_inproc_;
    ot::CString notary_bind_ip_;
    std::uint16_t notary_bind_port_;
    ot::CString notary_name_;
    ot::Set<ot::CString> notary_public_eep_;
    ot::Set<ot::CString> notary_public_ipv4_;
    ot::Set<ot::CString> notary_public_ipv6_;
    ot::Set<ot::CString> notary_public_onion_;
    std::uint16_t notary_public_port_;
    ot::CString notary_terms_;
    ot::CString storage_primary_plugin_;
    bool test_mode_;
};
// NOLINTEND(clang-analyzer-optin.performance.Padding)

auto check_options(
    const opentxs::Options& options,
    const OptionsData& expected) noexcept -> bool;
}  // namespace ottest
