// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cstddef>
#include <cstdint>

#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Options.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Options;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
struct OptionsData {
    ot::UnallocatedSet<ot::UnallocatedCString> blockchain_bind_ipv4_;
    ot::UnallocatedSet<ot::UnallocatedCString> blockchain_bind_ipv6_;
    ot::UnallocatedSet<opentxs::blockchain::Type> blockchain_disabled_chains_;
    int blockchain_storage_level_;
    bool blockchain_sync_server_enabled_;
    ot::UnallocatedSet<ot::UnallocatedCString> blockchain_sync_servers_;
    bool blockchain_wallet_enabled_;
    ot::UnallocatedCString home_;
    opentxs::Options::ConnectionMode ipv4_connection_mode_;
    opentxs::Options::ConnectionMode ipv6_connection_mode_;
    ot::UnallocatedCString log_endpoint_;
    int log_level_;
    std::size_t mint_key_bytes_;
    bool notary_bind_inproc_;
    ot::UnallocatedCString notary_bind_ip_;
    std::uint16_t notary_bind_port_;
    ot::UnallocatedCString notary_name_;
    ot::UnallocatedSet<ot::UnallocatedCString> notary_public_eep_;
    ot::UnallocatedSet<ot::UnallocatedCString> notary_public_ipv4_;
    ot::UnallocatedSet<ot::UnallocatedCString> notary_public_ipv6_;
    ot::UnallocatedSet<ot::UnallocatedCString> notary_public_onion_;
    std::uint16_t notary_public_port_;
    ot::UnallocatedCString notary_terms_;
    ot::UnallocatedCString storage_primary_plugin_;
    bool test_mode_;
};

auto check_options(
    const opentxs::Options& options,
    const OptionsData& expected) noexcept -> bool;
}  // namespace ottest
