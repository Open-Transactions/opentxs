// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/FilterType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/AddressStyle.hpp"
// IWYU pragma: no_include "opentxs/identity/wot/claim/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"
// IWYU pragma: no_include "opentxs/crypto/Bip44Type.hpp"
// IWYU pragma: no_include <boost/intrusive/detail/iterator.hpp>

#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/move/algo/move.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <tuple>
#include <utility>

#if OT_BLOCKCHAIN
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::params
{
struct Data {
    using ChainData = boost::container::flat_map<blockchain::Type, Data>;
#if OT_BLOCKCHAIN
    using FilterData = boost::container::flat_map<
        blockchain::Type,
        boost::container::flat_map<
            filter::Type,
            std::pair<UnallocatedCString, UnallocatedCString>>>;
    using FilterTypes =
        UnallocatedMap<Type, UnallocatedMap<filter::Type, std::uint8_t>>;
    using ServiceBits = std::map<
        blockchain::Type,
        UnallocatedMap<p2p::bitcoin::Service, p2p::Service>>;
#endif  // OT_BLOCKCHAIN
    using Style = blockchain::crypto::AddressStyle;
    using ScriptMap = boost::container::flat_map<Style, bool>;
    using StylePref = UnallocatedVector<std::pair<Style, UnallocatedCString>>;

    struct Checkpoint {
        block::Height height_{};
        UnallocatedCString block_hash_{};
        UnallocatedCString previous_block_hash_{};
        UnallocatedCString filter_header_{};
    };

    bool supported_{};
    bool testnet_{};
    bool segwit_{};
    unsigned segwit_scale_factor_{};
    UnitType itemtype_{};
    Bip44Type bip44_{};
    std::int32_t nBits_{};
    UnallocatedCString genesis_header_hex_{};
    UnallocatedCString genesis_hash_hex_{};
    UnallocatedCString genesis_block_hex_{};
    Checkpoint checkpoint_{};
    filter::Type default_filter_type_{};
    p2p::Protocol p2p_protocol_{};
    std::uint32_t p2p_magic_bits_{};
    std::uint16_t default_port_{};
    UnallocatedVector<UnallocatedCString> dns_seeds_{};
    Amount default_fee_rate_{};  // satoshis per 1000 bytes
    std::size_t block_download_batch_{};
    ScriptMap scripts_{};
    StylePref styles_{};
    block::Height maturation_interval_{};

#if OT_BLOCKCHAIN
    static auto Bip158() noexcept -> const FilterTypes&;
#endif  // OT_BLOCKCHAIN
    static auto Chains() noexcept -> const ChainData&;
#if OT_BLOCKCHAIN
    static auto Filters() noexcept -> const FilterData&;
    static auto Services() noexcept -> const ServiceBits&;
#endif  // OT_BLOCKCHAIN
};
}  // namespace opentxs::blockchain::params
