// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <variant>

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <typeindex>  // IWYU pragma: keep
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Block;
class Hash;
class Position;
}  // namespace block

namespace cfilter
{
class GCS;
class Header;
}  // namespace cfilter

namespace params
{
class ChainDataPrivate;
struct Data;
}  // namespace params
}  // namespace blockchain

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::params
{
class ChainData
{
public:
    ChainDataPrivate* imp_;

    using ZMQParams = std::pair<Bip44Type, network::blockchain::Subchain>;

    auto AssociatedMainnet() const noexcept -> blockchain::Type;
    auto Bip44Code() const noexcept -> Bip44Type;
    auto BlockDownloadBatch() const noexcept -> std::size_t;
    auto BlockHeaderAt(block::Height) const noexcept
        -> std::optional<block::Hash>;
    auto Category() const noexcept -> blockchain::Category;
    auto CfheaderAfter(cfilter::Type, block::Height) const noexcept
        -> std::optional<block::Height>;
    auto CfheaderAt(cfilter::Type, block::Height) const noexcept
        -> std::optional<cfilter::Header>;
    auto CfheaderBefore(cfilter::Type, block::Height) const noexcept
        -> block::Height;
    auto CfilterBatchEstimate() const noexcept -> std::size_t;
    auto CheckpointCfheader() const noexcept -> const cfilter::Header&;
    auto CheckpointPosition() const noexcept -> const block::Position&;
    auto CheckpointPrevious() const noexcept -> const block::Position&;
    auto CurrencyType() const noexcept -> UnitType;
    auto DefaultAddressStyle() const noexcept
        -> std::optional<blockchain::crypto::AddressStyle>;
    auto DefaultCfilterType() const noexcept -> cfilter::Type;
    auto Difficulty() const noexcept -> std::uint32_t;
    auto FallbackTxFeeRate() const noexcept -> const Amount&;
    auto ForkedFrom() const noexcept -> const std::optional<blockchain::Type>&;
    auto GenesisBlock(const api::Crypto& crypto) const noexcept
        -> const block::Block&;
    auto GenesisBlockSerialized() const noexcept -> ReadView;
    auto GenesisCfilter(const api::Session& api, cfilter::Type) const noexcept
        -> const cfilter::GCS&;
    auto GenesisCfheader(cfilter::Type) const noexcept
        -> const cfilter::Header&;
    auto GenesisHash() const noexcept -> const block::Hash&;
    auto HighestCfheaderCheckpoint(cfilter::Type) const noexcept
        -> block::Height;
    auto IsAllowed(blockchain::crypto::AddressStyle) const noexcept -> bool;
    auto IsSupported() const noexcept -> bool;
    auto IsTestnet() const noexcept -> bool;
    auto KnownCfilterTypes() const noexcept -> Set<cfilter::Type>;
    auto MaturationInterval() const noexcept -> block::Height;
    auto P2PDefaultPort() const noexcept -> std::uint16_t;
    auto P2PDefaultProtocol() const noexcept -> network::blockchain::Protocol;
    auto P2PMagicBits() const noexcept -> std::uint32_t;
    auto P2PSeeds() const noexcept -> const Vector<std::string_view>&;
    auto P2PVersion() const noexcept
        -> network::blockchain::bitcoin::message::ProtocolVersion;
    auto SegwitScaleFactor() const noexcept -> unsigned int;
    auto SupportsSegwit() const noexcept -> bool;
    auto TranslateBip158(cfilter::Type) const noexcept
        -> std::optional<std::uint8_t>;
    auto TranslateBip158(std::uint8_t) const noexcept
        -> std::optional<cfilter::Type>;
    auto TranslateService(network::blockchain::bitcoin::Service) const noexcept
        -> std::optional<network::blockchain::bitcoin::message::Service>;
    auto TranslateService(network::blockchain::bitcoin::message::Service)
        const noexcept -> std::optional<network::blockchain::bitcoin::Service>;
    auto ZMQ() const noexcept -> ZMQParams;

    ChainData(blockchain::Type chain) noexcept;
    ChainData() = delete;
    ChainData(const ChainData&) = delete;
    ChainData(ChainData&&) = delete;
    auto operator=(const ChainData&) -> ChainData& = delete;
    auto operator=(ChainData&&) -> ChainData& = delete;

    ~ChainData();

private:
    ChainData(const Data& data) noexcept;
};

auto chains() noexcept -> const Set<blockchain::Type>&;
auto get(blockchain::Type chain) noexcept(false) -> const ChainData&;
auto supported() noexcept -> const Set<blockchain::Type>&;
}  // namespace opentxs::blockchain::params

// NOLINTBEGIN(cert-dcl58-cpp)
namespace std
{
template <>
struct hash<opentxs::blockchain::params::ChainData::ZMQParams> {
    using is_avalanching = void;

    auto operator()(const opentxs::blockchain::params::ChainData::ZMQParams&
                        data) const noexcept -> std::size_t;
};
}  // namespace std
// NOLINTEND(cert-dcl58-cpp)
