// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_map.hpp>
#include <cs_plain_guarded.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <utility>

#include "blockchain/params/Data.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/GCS.hpp"
#include "opentxs/blockchain/cfilter/Header.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace json
{
class object;
}  // namespace json
}  // namespace boost

namespace opentxs
{
namespace api
{
class Crypto;
class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::params
{
class ChainDataPrivate
{
public:
    using GenesisCfheader =
        boost::container::flat_map<cfilter::Type, cfilter::Header>;
    using Cfheaders =
        boost::container::flat_map<cfilter::Type, cfilter::Header>;
    using CfheaderCheckpoints =
        Map<block::Height, std::pair<block::Hash, Cfheaders>>;
    using GuardedCheckpoints = libguarded::plain_guarded<CfheaderCheckpoints>;

    const blockchain::Type chain_;
    const Data& data_;
    const blockchain::Type associated_mainnet_;
    const blockchain::Category category_;
    const std::optional<blockchain::Type> forked_from_;
    const bool supported_;
    const bool testnet_;
    const bool segwit_;
    const bool cashtoken_;
    const bool spend_unconfirmed_;
    const unsigned segwit_scale_factor_;
    const UnitType currency_type_;
    const crypto::Bip44Type bip44_;
    const std::uint32_t difficulty_;
    const block::Hash genesis_hash_;
    const ByteArray serialized_genesis_block_;
    const GenesisCfheader genesis_cfheader_;
    const Set<cfilter::Type> known_cfilter_types_;
    const block::Position checkpoint_;
    const block::Position checkpoint_prior_;
    const cfilter::Header checkpoint_cfheader_;
    const cfilter::Type default_filter_type_;
    const network::blockchain::Protocol p2p_protocol_;
    const network::blockchain::bitcoin::message::ProtocolVersion
        p2p_protocol_version_;
    const std::uint32_t p2p_magic_bits_;
    const std::uint16_t default_port_;
    const Vector<std::string_view> dns_seeds_;
    const Amount default_fee_rate_;
    const std::size_t block_download_batch_;
    const block::Height maturation_interval_;
    const std::size_t cfilter_element_count_estimate_;
    const Data::ScriptMap scripts_;
    const Data::Style default_script_;
    const Data::ServiceMap services_;
    const Data::ServiceMapReverse services_reverse_;
    const Data::Bip158 bip158_;
    const Data::Bip158Reverse bip158_reverse_;
    const std::pair<crypto::Bip44Type, network::blockchain::Subchain> zmq_;
    const std::size_t max_notifications_;
    mutable GuardedCheckpoints cfheaders_;

    auto GenesisBlock(const api::Crypto& crypto) const noexcept
        -> const block::Block&;
    auto GenesisCfilter(const api::Session& api, cfilter::Type type)
        const noexcept -> const cfilter::GCS&;

    ChainDataPrivate(
        const boost::json::object& json,
        const Data& data,
        blockchain::Type chain) noexcept;
    ChainDataPrivate(blockchain::Type chain) noexcept;
    ChainDataPrivate() = delete;
    ChainDataPrivate(const ChainDataPrivate&) = delete;
    ChainDataPrivate(ChainDataPrivate&&) = delete;
    auto operator=(const ChainDataPrivate&) -> ChainDataPrivate& = delete;
    auto operator=(ChainDataPrivate&&) -> ChainDataPrivate& = delete;

    ~ChainDataPrivate() = default;

private:
    using Cfilters = boost::container::flat_map<cfilter::Type, cfilter::GCS>;
    using GuardedBlock = libguarded::plain_guarded<block::Block>;
    using GuardedCfilters = libguarded::plain_guarded<Cfilters>;

    mutable GuardedBlock genesis_block_;
    mutable GuardedCfilters genesis_cfilters_;

    static auto add_to_json(
        std::string_view in,
        boost::json::object& out) noexcept -> void;
    static auto json() noexcept -> const boost::json::object&;
};
}  // namespace opentxs::blockchain::params
