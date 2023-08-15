// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/move/algo/move.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::params
{
struct Data {
    using Style = blockchain::crypto::AddressStyle;
    using ScriptMap = boost::container::flat_map<Style, bool>;
    using ServiceMap = boost::container::flat_map<
        network::blockchain::bitcoin::message::Service,
        network::blockchain::bitcoin::Service>;
    using ServiceMapReverse = boost::container::flat_map<
        network::blockchain::bitcoin::Service,
        network::blockchain::bitcoin::message::Service>;
    using Bip158 = boost::container::flat_map<cfilter::Type, std::uint8_t>;
    using Bip158Reverse =
        boost::container::flat_map<std::uint8_t, cfilter::Type>;
    using GenesisBip158 = boost::container::
        flat_map<cfilter::Type, std::pair<std::string_view, std::string_view>>;

    bool supported_{};
    bool testnet_{};
    bool segwit_{};
    unsigned segwit_scale_factor_{};
    blockchain::Type associated_mainnet_{};
    blockchain::Category category_{};
    std::optional<blockchain::Type> forked_from_{};
    UnitType itemtype_{};
    Bip44Type bip44_{};
    Bip44Type parent_bip44_{};
    network::blockchain::Subchain subchain_{};
    std::int32_t n_bits_{};
    std::string_view genesis_hash_hex_{};
    std::string_view genesis_block_hex_{};
    cfilter::Type default_filter_type_{};
    network::blockchain::Protocol p2p_protocol_{};
    network::blockchain::bitcoin::message::ProtocolVersion
        p2p_protocol_version_{};
    std::uint32_t p2p_magic_bits_{};
    std::uint16_t default_port_{};
    UnallocatedVector<std::string_view> dns_seeds_{};
    Amount default_fee_rate_{};
    std::size_t block_download_batch_{};
    ScriptMap scripts_{};
    Style default_script_{};
    block::Height maturation_interval_{};
    std::size_t cfilter_element_count_estimate_{};
    ServiceMap services_{};
    Bip158 bip158_{};
    GenesisBip158 genesis_bip158_{};
};
using ChainMap = boost::container::flat_map<blockchain::Type, Data>;

auto Chains() noexcept -> const ChainMap&;
}  // namespace opentxs::blockchain::params
