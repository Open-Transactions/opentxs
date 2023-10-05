// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/params/ChainData.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>

#include "blockchain/params/ChainDataPrivate.hpp"
#include "blockchain/params/Data.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/cfilter/Header.hpp"
#include "opentxs/core/ByteArray.hpp"

namespace opentxs::blockchain::params
{
ChainData::ChainData(blockchain::Type chain) noexcept
    : imp_(std::make_unique<ChainDataPrivate>(chain).release())
{
}

auto ChainData::AssociatedMainnet() const noexcept -> blockchain::Type
{
    return imp_->associated_mainnet_;
}

auto ChainData::Bip44Code() const noexcept -> Bip44Type { return imp_->bip44_; }

auto ChainData::BlockDownloadBatch() const noexcept -> std::size_t
{
    return imp_->block_download_batch_;
}

auto ChainData::BlockHeaderAt(block::Height height) const noexcept
    -> std::optional<block::Hash>
{
    auto handle = imp_->cfheaders_.lock();
    const auto& outer = *handle;

    if (const auto o = outer.find(height); outer.end() != o) {

        return o->second.first;
    }

    return std::nullopt;
}

auto ChainData::Category() const noexcept -> blockchain::Category
{
    return imp_->category_;
}

auto ChainData::CfheaderAfter(cfilter::Type type, block::Height tip)
    const noexcept -> std::optional<block::Height>
{
    auto handle = imp_->cfheaders_.lock();
    const auto& outer = *handle;

    for (auto o = outer.lower_bound(tip + 1); outer.end() != o; ++o) {
        const auto& [height, map] = *o;
        const auto& [_, inner] = map;

        if (inner.contains(type)) { return height; }
    }

    return std::nullopt;
}

auto ChainData::CfheaderAt(cfilter::Type type, block::Height height)
    const noexcept -> std::optional<cfilter::Header>
{
    auto handle = imp_->cfheaders_.lock();
    const auto& outer = *handle;

    if (const auto o = outer.find(height); outer.end() != o) {
        const auto& [block, inner] = o->second;

        if (const auto i = inner.find(type); inner.end() != i) {

            return i->second;
        }
    }

    return std::nullopt;
}

auto ChainData::CfheaderBefore(cfilter::Type type, block::Height height)
    const noexcept -> block::Height
{
    auto handle = imp_->cfheaders_.lock();
    const auto& outer = *handle;

    // NOLINTBEGIN(modernize-loop-convert)
    for (auto o = outer.crbegin(); o != outer.crend(); ++o) {
        if (o->first >= height) { continue; }

        const auto& [block, inner] = o->second;

        if (inner.contains(type)) { return o->first; }
    }
    // NOLINTEND(modernize-loop-convert)

    return 0;
}

auto ChainData::CfilterBatchEstimate() const noexcept -> std::size_t
{
    return imp_->cfilter_element_count_estimate_;
}

auto ChainData::CheckpointCfheader() const noexcept -> const cfilter::Header&
{
    return imp_->checkpoint_cfheader_;
}

auto ChainData::CheckpointPosition() const noexcept -> const block::Position&
{
    return imp_->checkpoint_;
}

auto ChainData::CheckpointPrevious() const noexcept -> const block::Position&
{
    return imp_->checkpoint_prior_;
}

auto ChainData::CurrencyType() const noexcept -> UnitType
{
    return imp_->currency_type_;
}

auto ChainData::DefaultAddressStyle() const noexcept
    -> std::optional<blockchain::crypto::AddressStyle>
{
    const auto& style = imp_->default_script_;

    try {
        if (imp_->scripts_.at(style)) {

            return style;
        } else {

            return std::nullopt;
        }
    } catch (...) {

        return std::nullopt;
    }
}

auto ChainData::DefaultCfilterType() const noexcept -> cfilter::Type
{
    return imp_->default_filter_type_;
}

auto ChainData::Difficulty() const noexcept -> std::uint32_t
{
    return imp_->difficulty_;
}

auto ChainData::FallbackTxFeeRate() const noexcept -> const Amount&
{
    return imp_->default_fee_rate_;
}

auto ChainData::ForkedFrom() const noexcept
    -> const std::optional<blockchain::Type>&
{
    return imp_->forked_from_;
}

auto ChainData::GenesisBlock(const api::Crypto& crypto) const noexcept
    -> const block::Block&
{
    return imp_->GenesisBlock(crypto);
}

auto ChainData::GenesisBlockSerialized() const noexcept -> ReadView
{
    return imp_->serialized_genesis_block_.Bytes();
}

auto ChainData::GenesisCfilter(const api::Session& api, cfilter::Type type)
    const noexcept -> const cfilter::GCS&
{
    return imp_->GenesisCfilter(api, type);
}

auto ChainData::GenesisCfheader(cfilter::Type type) const noexcept
    -> const cfilter::Header&
{
    const auto& map = imp_->genesis_cfheader_;

    if (auto i = map.find(type); map.end() != i) {

        return i->second;
    } else {
        static const auto blank = cfilter::Header{};

        return blank;
    }
}

auto ChainData::GenesisHash() const noexcept -> const block::Hash&
{
    return imp_->genesis_hash_;
}

auto ChainData::HighestCfheaderCheckpoint(cfilter::Type type) const noexcept
    -> block::Height
{
    auto handle = imp_->cfheaders_.lock();
    const auto& outer = *handle;

    // NOLINTBEGIN(modernize-loop-convert)
    for (auto o = outer.crbegin(); o != outer.crend(); ++o) {
        const auto& [block, inner] = o->second;

        if (inner.contains(type)) { return o->first; }
    }
    // NOLINTEND(modernize-loop-convert)

    return 0;
}

auto ChainData::IsAllowed(blockchain::crypto::AddressStyle style) const noexcept
    -> bool
{
    const auto& map = imp_->scripts_;

    if (auto i = map.find(style); map.end() != i) {

        return i->second;
    } else {

        return false;
    }
}

auto ChainData::IsSupported() const noexcept -> bool
{
    return imp_->supported_;
}

auto ChainData::IsTestnet() const noexcept -> bool { return imp_->testnet_; }

auto ChainData::KnownCfilterTypes() const noexcept -> Set<cfilter::Type>
{
    return imp_->known_cfilter_types_;
}

auto ChainData::MaturationInterval() const noexcept -> block::Height
{
    return imp_->maturation_interval_;
}

auto ChainData::MaxNotifications() const noexcept -> std::size_t
{
    return imp_->max_notifications_;
}

auto ChainData::P2PDefaultPort() const noexcept -> std::uint16_t
{
    return imp_->default_port_;
}

auto ChainData::P2PDefaultProtocol() const noexcept
    -> network::blockchain::Protocol
{
    return imp_->p2p_protocol_;
}

auto ChainData::P2PMagicBits() const noexcept -> std::uint32_t
{
    return imp_->p2p_magic_bits_;
}

auto ChainData::P2PSeeds() const noexcept -> const Vector<std::string_view>&
{
    return imp_->dns_seeds_;
}

auto ChainData::P2PVersion() const noexcept
    -> network::blockchain::bitcoin::message::ProtocolVersion
{
    return imp_->p2p_protocol_version_;
}

auto ChainData::SegwitScaleFactor() const noexcept -> unsigned int
{
    return imp_->segwit_scale_factor_;
}

auto ChainData::SpendUnconfirmed() const noexcept -> bool
{
    return imp_->spend_unconfirmed_;
}

auto ChainData::SupportsCashtoken() const noexcept -> bool
{
    return imp_->cashtoken_;
}

auto ChainData::SupportsSegwit() const noexcept -> bool
{
    return imp_->segwit_;
}

auto ChainData::TranslateBip158(cfilter::Type in) const noexcept
    -> std::optional<std::uint8_t>
{
    const auto& map = imp_->bip158_;

    if (auto i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return std::nullopt;
    }
}

auto ChainData::TranslateBip158(std::uint8_t in) const noexcept
    -> std::optional<cfilter::Type>
{
    const auto& map = imp_->bip158_reverse_;

    if (auto i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return std::nullopt;
    }
}

auto ChainData::TranslateService(
    network::blockchain::bitcoin::message::Service in) const noexcept
    -> std::optional<network::blockchain::bitcoin::Service>
{
    const auto& map = imp_->services_;

    if (auto i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return std::nullopt;
    }
}

auto ChainData::TranslateService(
    network::blockchain::bitcoin::Service in) const noexcept
    -> std::optional<network::blockchain::bitcoin::message::Service>
{
    const auto& map = imp_->services_reverse_;

    if (auto i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return std::nullopt;
    }
}

auto ChainData::ZMQ() const noexcept
    -> std::pair<Bip44Type, network::blockchain::Subchain>
{
    return imp_->zmq_;
}

ChainData::~ChainData()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::params

namespace opentxs::blockchain::params
{
using namespace std::literals;

auto chains() noexcept -> const Set<blockchain::Type>&
{
    static const auto output = [] {
        auto out = Set<Type>{};

        for (const auto& [chain, _] : params::Chains()) { out.emplace(chain); }

        return out;
    }();

    return output;
}

auto get(blockchain::Type chain) noexcept(false) -> const ChainData&
{
    static const auto data = [] {
        auto out = Map<blockchain::Type, ChainData>{};

        for (const auto type : chains()) { out.emplace(type, type); }

        return out;
    }();

    if (const auto i = data.find(chain); data.end() != i) {

        return i->second;
    } else {

        throw std::out_of_range{"undefined chain "s.append(
            std::to_string(static_cast<blockchain::TypeEnum>(chain)))};
    }
}

auto supported() noexcept -> const Set<blockchain::Type>&
{
    static const auto output = [] {
        auto out = Set<Type>{};

        for (const auto& [chain, data] : params::Chains()) {
            if (data.supported_) { out.emplace(chain); }
        }

        return out;
    }();

    return output;
}
}  // namespace opentxs::blockchain::params
