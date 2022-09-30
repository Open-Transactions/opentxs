// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/AddressStyle.hpp"
// IWYU pragma: no_include "opentxs/identity/wot/claim/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"
// IWYU pragma: no_include "opentxs/crypto/Bip44Type.hpp"
// IWYU pragma: no_include <boost/intrusive/detail/iterator.hpp>

#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/cstdint.hpp>
#include <boost/move/algo/move.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Block;
}  // namespace block

namespace params
{
class ChainDataPrivate;
struct Data;
}  // namespace params
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::params
{
class ChainData
{
public:
    struct Checkpoint {
        const block::Height height_;
        const block::Hash block_;
        const block::Hash previous_block_;
        const cfilter::Header cfheader_;

        Checkpoint(const Data& data) noexcept;
        Checkpoint() = delete;
        Checkpoint(const Checkpoint&) = delete;
        Checkpoint(Checkpoint&&) = delete;
        auto operator=(const Checkpoint&) -> Checkpoint& = delete;
        auto operator=(Checkpoint&&) -> Checkpoint& = delete;
    };

    auto Bip44Code() const noexcept -> Bip44Type;
    auto BlockDownloadBatch() const noexcept -> std::size_t;
    auto CfilterBatchEstimate() const noexcept -> std::size_t;
    auto Checkpoints() const noexcept -> const Checkpoint&;
    auto CurrencyType() const noexcept -> UnitType;
    auto DefaultAddressStyle() const noexcept
        -> std::optional<blockchain::crypto::AddressStyle>;
    auto DefaultCfilterType() const noexcept -> cfilter::Type;
    auto Difficulty() const noexcept -> std::uint32_t;
    auto FallbackTxFeeRate() const noexcept -> const Amount&;
    auto GenesisBlock() const noexcept -> const block::Block&;
    auto GenesisHash() const noexcept -> const block::Hash&;
    auto IsAllowed(blockchain::crypto::AddressStyle) const noexcept -> bool;
    auto IsSupported() const noexcept -> bool;
    auto IsTestnet() const noexcept -> bool;
    auto MaturationInterval() const noexcept -> block::Height;
    auto P2PDefaultPort() const noexcept -> std::uint16_t;
    auto P2PDefaultProtocol() const noexcept -> p2p::Protocol;
    auto P2PMagicBits() const noexcept -> std::uint32_t;
    auto P2PSeeds() const noexcept -> const Vector<std::string_view>&;
    auto P2PVersion() const noexcept -> p2p::bitcoin::ProtocolVersion;
    auto SegwitScaleFactor() const noexcept -> unsigned int;
    auto SupportsSegwit() const noexcept -> bool;

    ChainData(blockchain::Type chain) noexcept;
    ChainData() = delete;
    ChainData(const ChainData&) = delete;
    ChainData(ChainData&&) = delete;
    auto operator=(const ChainData&) -> ChainData& = delete;
    auto operator=(ChainData&&) -> ChainData& = delete;

    ~ChainData();

private:
    ChainDataPrivate* imp_;

    ChainData(const Data& data) noexcept;
};

auto chains() noexcept -> const Set<blockchain::Type>;
auto get(blockchain::Type chain) noexcept(false) -> const ChainData&;

using FilterData = boost::container::flat_map<
    blockchain::Type,
    boost::container::
        flat_map<cfilter::Type, std::pair<std::string_view, std::string_view>>>;
using FilterTypes = boost::container::
    flat_map<Type, boost::container::flat_map<cfilter::Type, std::uint8_t>>;
using ServiceBits = std::map<
    blockchain::Type,
    boost::container::flat_map<p2p::bitcoin::Service, p2p::Service>>;

auto Bip158() noexcept -> const FilterTypes&;
auto Filters() noexcept -> const FilterData&;
auto Services() noexcept -> const ServiceBits&;
}  // namespace opentxs::blockchain::params
