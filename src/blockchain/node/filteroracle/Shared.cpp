// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "blockchain/node/filteroracle/Shared.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <compare>
#include <cstddef>
#include <future>
#include <iterator>
#include <stdexcept>

#include "blockchain/node/filteroracle/CfheaderDownloader.hpp"
#include "blockchain/node/filteroracle/CfilterDownloader.hpp"
#include "blockchain/node/filteroracle/FilterOracle.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/database/Cfilter.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/filteroracle/BlockIndexer.hpp"
#include "internal/blockchain/node/filteroracle/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/AsyncConst.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/otdht/Block.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::filteroracle
{
Shared::Shared(
    const api::Session& api,
    const node::Manager& node,
    const blockchain::cfilter::Type type) noexcept
    : api_(api)
    , node_(node)
    , header_(node.HeaderOracle())
    , log_(LogTrace())
    , chain_(node_.Internal().Chain())
    , default_type_(type)
    , server_mode_([&] {
        switch (node_.Internal().GetConfig().profile_) {
            case BlockchainProfile::server: {

                return true;
            }
            case BlockchainProfile::mobile:
            case BlockchainProfile::desktop:
            case BlockchainProfile::desktop_native: {

                return false;
            }
            default: {
                LogAbort()(OT_PRETTY_CLASS())("invalid profile").Abort();
            }
        }
    }())
    , standalone_mode_([&] {
        switch (node_.Internal().GetConfig().profile_) {
            case BlockchainProfile::desktop_native: {

                return true;
            }
            case BlockchainProfile::mobile:
            case BlockchainProfile::desktop:
            case BlockchainProfile::server: {

                return false;
            }
            default: {
                LogAbort()(OT_PRETTY_CLASS())("invalid profile").Abort();
            }
        }
    }())
    , data_(api_, node_)
{
}

auto Shared::broadcast_cfilter_tip(
    const cfilter::Type type,
    const block::Position& tip,
    Data& data) const noexcept -> void
{
    log_(OT_PRETTY_CLASS())(print(chain_))(
        ": notifying peers of new filter tip ")(tip)
        .Flush();
    data.last_sync_progress_ = sClock::now();
    data.filter_notifier_internal_.SendDeferred(
        [&] {
            auto work = MakeWork(OT_ZMQ_NEW_FILTER_SIGNAL);
            work.AddFrame(type);
            work.AddFrame(tip.height_);
            work.AddFrame(tip.hash_);

            return work;
        }(),
        __FILE__,
        __LINE__);
    data.to_blockchain_api_.SendDeferred(
        [&] {
            auto work = MakeWork(WorkType::BlockchainNewFilter);
            work.AddFrame(chain_);
            work.AddFrame(type);
            work.AddFrame(tip.height_);
            work.AddFrame(tip.hash_);

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Shared::CfheaderTip() const noexcept -> block::Position
{
    return CfheaderTip(default_type_);
}

auto Shared::CfheaderTip(const cfilter::Type type) const noexcept
    -> block::Position
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return cfheader_tip(type, data);
}

auto Shared::CfilterTip() const noexcept -> block::Position
{
    return CfilterTip(default_type_);
}

auto Shared::CfilterTip(const cfilter::Type type) const noexcept
    -> block::Position
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return cfilter_tip(type, data);
}

auto Shared::Tips() const noexcept
    -> std::pair<block::Position, block::Position>
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;
    const auto& type = default_type_;

    return std::make_pair(cfheader_tip(type, data), cfilter_tip(type, data));
}

auto Shared::cfheader_tip(const cfilter::Type type, const Data& data)
    const noexcept -> block::Position
{
    return data.db_.get()->FilterHeaderTip(type);
}

auto Shared::cfilter_tip(const cfilter::Type type, const Data& data)
    const noexcept -> block::Position
{
    return data.db_.get()->FilterTip(type);
}

auto Shared::cfilter_tip_needs_broadcast(
    const cfilter::Type type,
    const block::Position& tip,
    Data& data) const noexcept -> bool
{
    auto& last = data.last_broadcast_
                     .try_emplace(type, 0, params::get(chain_).GenesisHash())
                     .first->second;

    if (tip == last) {

        return false;
    } else {
        last = tip;

        return true;
    }
}

auto Shared::compare_cfheader_tip_to_checkpoint(
    Data& data,
    block::Position& tip) const noexcept -> void
{
    const auto& cp =
        implementation::FilterOracle::filter_checkpoints_.at(chain_);
    auto changed{false};
    auto check = block::Position{};

    for (auto i{cp.crbegin()}; i != cp.crend(); ++i) {
        const auto& cpHeight = i->first;

        if (cpHeight > tip.height_) { continue; }

        check = {cpHeight, header_.BestHash(cpHeight)};
        const auto existingHeader =
            load_cfheader(default_type_, check.hash_.Bytes(), data);

        try {
            const auto& cpHeader = i->second.at(default_type_);
            const auto cpBytes = api_.Factory().DataFromHex(cpHeader);

            if (existingHeader == cpBytes) {

                break;
            } else {
                changed = true;
            }
        } catch (...) {
            break;
        }
    }

    if (changed) {
        LogConsole()(print(chain_))(
            " cfheader tip not consistent with checkpoint. Resetting to last "
            "known good position")
            .Flush();
        tip = std::move(check);
    } else {
        log_(print(chain_))(" cfheader tip consistent with checkpoint").Flush();
    }
}

auto Shared::find_acceptable_cfheader(
    const Data& data,
    block::Position& tip) noexcept -> void
{
    const auto& headerOracle = node_.HeaderOracle();
    const auto& db = *data.db_.get();

    if (1 > tip.height_) {
        tip = headerOracle.GetPosition(0);

        OT_ASSERT(db.HaveFilterHeader(default_type_, tip.hash_));
    } else {
        const auto have_data = [&](const auto& prev, const auto& cur) -> bool {
            return db.HaveFilterHeader(default_type_, cur.hash_) &&
                   db.HaveFilterHeader(default_type_, prev) &&
                   headerOracle.IsInBestChain(cur);
        };

        while (0 <= tip.height_) {
            if (0 == tip.height_) {

                return;
            } else {
                auto prior = tip.height_ - 1;
                auto previous = headerOracle.BestHash(prior);

                if (have_data(previous, tip)) {

                    return;
                } else {
                    tip = {std::move(prior), std::move(previous)};
                    LogError()(OT_PRETTY_CLASS())(" rewinding ")(print(chain_))(
                        " cfheader tip to ")(
                        tip)(" due to missing or invalid data")
                        .Flush();
                }
            }
        }

        OT_FAIL;
    }
}

auto Shared::find_acceptable_cfilter(
    const block::Position& cfheaderTip,
    const Data& data,
    block::Position& tip) noexcept -> void
{
    {
        const auto reset = (cfheaderTip.height_ < tip.height_) ||
                           ((cfheaderTip.height_ == tip.height_) &&
                            (cfheaderTip.hash_ != tip.hash_));

        if (reset) {
            LogError()(OT_PRETTY_CLASS())(" rewinding ")(print(chain_))(
                " cfilter tip to match cfheader tip")
                .Flush();
            tip = cfheaderTip;
        }
    }

    const auto& headerOracle = node_.HeaderOracle();
    const auto& db = *data.db_.get();

    if (1 > tip.height_) {
        tip = headerOracle.GetPosition(0);

        OT_ASSERT(db.HaveFilter(default_type_, tip.hash_));
    } else {
        const auto have_data = [&](const auto& cur) -> bool {
            return db.HaveFilter(default_type_, cur.hash_) &&
                   headerOracle.IsInBestChain(cur);
        };

        while (0 <= tip.height_) {
            if (0 == tip.height_) {

                return;
            } else {
                if (have_data(tip)) {

                    return;
                } else {
                    auto prior = tip.height_ - 1;
                    auto previous = headerOracle.BestHash(prior);
                    tip = {std::move(prior), std::move(previous)};
                    LogError()(OT_PRETTY_CLASS())(" rewinding ")(print(chain_))(
                        " cfilter tip to ")(
                        tip)(" due to missing or invalid data")
                        .Flush();
                }
            }
        }

        OT_FAIL;
    }
}

auto Shared::FindBestPosition(const block::Position& candidate) const noexcept
    -> BestPosition
{
    const auto handle = data_.lock_shared();
    const auto& data = *handle;

    return find_best_position({candidate}, data);
}

auto Shared::find_best_position(block::Position candidate, const Data& data)
    const noexcept -> BestPosition
{
    static const auto blank = block::Height{-1};
    const auto& headerOracle = node_.HeaderOracle();
    const auto& db = *data.db_.get();

    if (blank == candidate.height_) {
        candidate = headerOracle.GetPosition(0);

        OT_ASSERT(db.HaveFilterHeader(default_type_, candidate.hash_));
        OT_ASSERT(db.HaveFilter(default_type_, candidate.hash_));
    }

    if (0 == candidate.height_) {

        return BestPosition{
            db.LoadFilterHeader(default_type_, candidate.hash_.Bytes()),
            {},
            std::move(candidate)};
    } else {
        // NOTE this procedure allows for recovery from certain types of
        // database corruption. If the expected data are not present for the
        // cfilter tip and cfheader tip recorded in the database then the tips
        // will be rewound to the point at which consistent data is found, or
        // the genesis position is reached, whichever comes first.
        const auto have_data = [&](const auto& prev, const auto& cur) -> bool {
            return db.HaveFilterHeader(default_type_, cur) &&
                   db.HaveFilterHeader(default_type_, prev) &&
                   db.HaveFilter(default_type_, cur) &&
                   db.HaveFilter(default_type_, prev);
        };

        while (0 <= candidate.height_) {
            if (0 == candidate.height_) {

                return BestPosition{
                    db.LoadFilterHeader(default_type_, candidate.hash_.Bytes()),
                    {},
                    std::move(candidate)};
            } else {
                auto prior = candidate.height_ - 1;
                auto previous = headerOracle.BestHash(prior);

                if (have_data(previous, candidate.hash_)) {

                    return BestPosition{
                        db.LoadFilterHeader(
                            default_type_, candidate.hash_.Bytes()),
                        db.LoadFilterHeader(default_type_, previous.Bytes()),
                        std::move(candidate)};
                } else {
                    candidate = {std::move(prior), std::move(previous)};
                }
            }
        }

        LogAbort()(OT_PRETTY_CLASS())("genesis data not found").Abort();
    }
}

auto Shared::GetFilterJob() const noexcept -> CfilterJob
{
    auto handle = data_.lock_shared();
    const auto& worker = handle->filter_downloader_;

    if (worker) {

        return worker->NextBatch();
    } else {

        return {};
    }
}

auto Shared::GetHeaderJob() const noexcept -> CfheaderJob
{
    auto handle = data_.lock_shared();
    const auto& worker = handle->header_downloader_;

    if (worker) {

        return worker->NextBatch();
    } else {

        return {};
    }
}

auto Shared::Heartbeat() noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto& cfheader = data.header_downloader_;
    auto& cfilter = data.header_downloader_;

    if (cfheader) { cfheader->Heartbeat(); }
    if (cfilter) { cfilter->Heartbeat(); }

    constexpr auto limit = 5s;

    if ((sClock::now() - data.last_sync_progress_) > limit) {
        broadcast_cfilter_tip(
            default_type_, cfheader_tip(default_type_, data), data);
    }
}

auto Shared::Init() noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    data.db_.set_value(
        std::addressof(static_cast<database::Cfilter&>(node_.Internal().DB())));
    auto cfheaderTip = cfheader_tip(default_type_, data);
    auto cfilterTip = cfilter_tip(default_type_, data);
    const auto originalCfheader{cfheaderTip};
    const auto originalCfilter{cfilterTip};
    LogConsole()(print(chain_))(" cfheader tip is ")(cfheaderTip).Flush();
    LogConsole()(print(chain_))(" cfilter tip is ")(cfilterTip).Flush();
    find_acceptable_cfheader(data, cfheaderTip);
    compare_cfheader_tip_to_checkpoint(data, cfheaderTip);
    find_acceptable_cfilter(cfheaderTip, data, cfilterTip);

    if (originalCfheader != cfheaderTip) {
        const auto rc = set_cfheader_tip(default_type_, cfheaderTip, data);

        OT_ASSERT(rc);
    }

    if (originalCfilter != cfilterTip) {
        const auto rc = set_cfilter_tip(default_type_, cfilterTip, data);

        OT_ASSERT(rc);
    }

    update_cfilter_tip(default_type_, cfilterTip, data);
}

auto Shared::Init(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    std::shared_ptr<Shared> shared) noexcept -> void
{
    auto handle = data_.lock();
    auto& workers = *handle;

    if (standalone_mode_) {
        workers.filter_downloader_ =
            std::make_unique<filteroracle::CfilterDownloader>(shared);
        workers.header_downloader_ =
            std::make_unique<filteroracle::CfheaderDownloader>(shared);
    } else if (server_mode_) {
        filteroracle::BlockIndexer{api, node, shared}.Start();
    }
}

auto Shared::LoadCfheader(const cfilter::Type type, const block::Hash& block)
    const noexcept -> cfilter::Header
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return load_cfheader(type, block, data);
}

auto Shared::load_cfheader(
    const cfilter::Type type,
    const block::Hash& block,
    const Data& data) const noexcept -> cfilter::Header
{
    return data.db_.get()->LoadFilterHeader(type, block.Bytes());
}

auto Shared::LoadCfilter(
    const cfilter::Type type,
    const block::Hash& block,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> GCS
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return load_cfilter(type, block, data, alloc, monotonic);
}

auto Shared::load_cfilter(
    const cfilter::Type type,
    const block::Hash& block,
    const Data& data,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> GCS
{
    return data.db_.get()->LoadFilter(type, block.Bytes(), alloc, monotonic);
}

auto Shared::LoadCfilters(
    const cfilter::Type type,
    const Vector<block::Hash>& blocks,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> Vector<GCS>
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return load_cfilters(type, blocks, data, alloc, monotonic);
}

auto Shared::load_cfilters(
    const cfilter::Type type,
    const Vector<block::Hash>& blocks,
    const Data& data,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> Vector<GCS>
{
    return data.db_.get()->LoadFilters(type, blocks, monotonic);
}

auto Shared::LoadCfilterHash(const block::Hash& block, const Data& data)
    const noexcept -> cfilter::Hash
{
    return load_cfilter_hash(default_type_, block, data);
}

auto Shared::load_cfilter_hash(
    const cfilter::Type type,
    const block::Hash& block,
    const Data& data) const noexcept -> cfilter::Hash
{
    return data.db_.get()->LoadFilterHash(type, block.Bytes());
}

auto Shared::Lock() noexcept -> GuardedData::handle { return data_.lock(); }

auto Shared::ProcessBlock(
    const bitcoin::block::Block& block,
    alloc::Default monotonic) noexcept -> bool
{
    const auto& id = block.ID();
    const auto& header = block.Header();
    auto filters = Vector<database::Cfilter::CFilterParams>{monotonic};
    auto headers = Vector<database::Cfilter::CFHeaderParams>{monotonic};
    filters.clear();
    headers.clear();
    const auto& cfilter =
        filters
            .emplace_back(
                id,
                process_block(api_, default_type_, block, monotonic, monotonic))
            .second;

    if (false == cfilter.IsValid()) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate ")(print(chain_))(
            " cfilter")
            .Flush();

        return false;
    }

    auto handle = data_.lock();
    auto& data = *handle;
    const auto previousCfheader =
        load_cfheader(default_type_, header.ParentHash(), data);

    if (previousCfheader.IsNull()) {
        LogError()(OT_PRETTY_CLASS())("failed to load previous")(print(chain_))(
            " cfheader")
            .Flush();

        return false;
    }

    const auto filterHash = cfilter.Hash();
    const auto& cfheader = std::get<1>(headers.emplace_back(
        id, cfilter.Header(previousCfheader.Bytes()), filterHash.Bytes()));

    if (cfheader.IsNull()) {
        LogError()(OT_PRETTY_CLASS())("failed to calculate ")(print(chain_))(
            " cfheader")
            .Flush();

        return false;
    }

    const auto position = block::Position{};
    const auto stored = store_cfilters(
        default_type_,
        position,
        std::move(headers),
        std::move(filters),
        data,
        monotonic);

    if (stored) {

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Database error ").Flush();

        return false;
    }
}

auto Shared::ProcessBlock(
    const cfilter::Type filterType,
    const bitcoin::block::Block& block,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> GCS
{
    return process_block(api_, filterType, block, alloc, monotonic);
}

auto Shared::process_block(
    const api::Session& api,
    const cfilter::Type filterType,
    const bitcoin::block::Block& block,
    alloc::Default alloc,
    alloc::Default monotonic) noexcept -> GCS
{
    const auto& id = block.ID();
    const auto params = blockchain::internal::GetFilterParams(filterType);
    const auto elements = [&] {
        const auto input = block.Internal().ExtractElements(filterType, alloc);
        auto output = Vector<ByteArray>{monotonic};
        output.reserve(input.size());
        output.clear();
        std::transform(
            input.begin(),
            input.end(),
            std::back_inserter(output),
            [&](const auto& element) -> ByteArray {
                return api.Factory().DataFromBytes(reader(element));
            });

        return output;
    }();

    return factory::GCS(
        api,
        params.first,
        params.second,
        blockchain::internal::BlockHashToFilterKey(id.Bytes()),
        elements,
        alloc);
}

auto Shared::ProcessSyncData(
    const block::Hash& prior,
    const Vector<block::Hash>& hashes,
    const network::otdht::Data& in,
    alloc::Default monotonic) noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    process_sync_data(prior, hashes, in, data, monotonic);
}

auto Shared::process_sync_data(
    const block::Hash& prior,
    const Vector<block::Hash>& hashes,
    const network::otdht::Data& in,
    Data& data,
    alloc::Default monotonic) const noexcept -> void
{
    auto filters = Vector<database::Cfilter::CFilterParams>{monotonic};
    auto headers = Vector<database::Cfilter::CFHeaderParams>{monotonic};
    const auto& blocks = in.Blocks();
    filters.reserve(blocks.size());
    filters.clear();
    headers.reserve(blocks.size());
    headers.clear();
    const auto incoming = blocks.front().Height();
    const auto finalFilter = in.LastPosition(api_);
    const auto filterType = blocks.front().FilterType();
    const auto current = cfilter_tip(filterType, data);
    const auto params = blockchain::internal::GetFilterParams(filterType);

    if ((1 == incoming) && (1000 < current.height_)) {
        const auto height = current.height_ - 1000;
        reset_tips_to(
            filterType,
            block::Position{height, header_.BestHash(height)},
            data);

        return;
    }

    log_(OT_PRETTY_CLASS())("current ")(print(chain_))(
        " filter tip height is ")(current.height_)
        .Flush();
    log_(OT_PRETTY_CLASS())("incoming ")(print(chain_))(
        " sync data provides heights ")(incoming)(" to ")(finalFilter.height_)
        .Flush();

    if (incoming > (current.height_ + 1)) {
        log_(OT_PRETTY_CLASS())("cannot connect ")(print(chain_))(
            " sync data to current tip")
            .Flush();

        return;
    }

    const auto redundant = (finalFilter.height_ < current.height_) ||
                           (finalFilter.hash_ == current.hash_);

    if (redundant) {
        log_(OT_PRETTY_CLASS())("ignoring redundant ")(print(chain_))(
            " sync data")
            .Flush();

        return;
    }

    const auto count{std::min<std::size_t>(hashes.size(), blocks.size())};
    filters.reserve(count);
    headers.reserve(count);

    try {
        OT_ASSERT(0 < count);

        const auto previous = [&] {
            if (prior.empty()) {

                return cfilter::Header{};
            } else {

                auto output = load_cfheader(filterType, prior, data);

                if (output.IsNull()) {
                    LogError()(OT_PRETTY_CLASS())("cfheader for ")(
                        print(chain_))(" block ")(prior.asHex())(" not found")
                        .Flush();

                    throw std::runtime_error(
                        "Failed to load previous cfheader");
                }

                return output;
            }
        }();
        const auto* parent = &previous;
        auto b = hashes.cbegin();
        auto d = blocks.cbegin();

        for (auto i = 0_uz; i < count; ++i, ++b, ++d) {
            const auto& blockHash = *b;
            const auto& syncData = *d;
            const auto height = syncData.Height();
            auto& [fBlockHash, cfilter] = filters.emplace_back(
                blockHash,
                factory::GCS(
                    api_,
                    params.first,
                    params.second,
                    blockchain::internal::BlockHashToFilterKey(
                        blockHash.Bytes()),
                    syncData.FilterElements(),
                    syncData.Filter(),
                    monotonic));

            if (false == cfilter.IsValid()) {
                LogError()(OT_PRETTY_CLASS())("Failed to instantiate ")(
                    print(chain_))(" cfilter #")(height)
                    .Flush();

                throw std::runtime_error("Failed to instantiate gcs");
            }

            auto& [hBlockHash, cfheader, cfhash] = headers.emplace_back(
                blockHash, cfilter.Header(*parent), cfilter.Hash());
            parent = &cfheader;
        }

        const auto tip = [&] {
            const auto last = count - 1u;

            return block::Position{blocks.at(last).Height(), hashes.at(last)};
        }();
        const auto stored = store_cfilters(
            default_type_,
            tip,
            std::move(headers),
            std::move(filters),
            data,
            monotonic);

        if (stored) {
            log_(print(chain_))(
                " cfheader and cfilter chain updated to height ")(tip.height_)
                .Flush();
            update_cfilter_tip(filterType, tip, data);
        } else {
            throw std::runtime_error{"database error"};
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto Shared::Report() noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    broadcast_cfilter_tip(
        default_type_, data.db_.get()->FilterTip(default_type_), data);
}

auto Shared::reset_tips_to(
    const cfilter::Type type,
    const block::Position& position,
    Data& data,
    const std::optional<bool> resetHeader,
    const std::optional<bool> resetfilter) const noexcept -> bool
{
    return reset_tips_to(
        type,
        cfheader_tip(default_type_, data),
        cfilter_tip(default_type_, data),
        position,
        data,
        resetHeader,
        resetfilter);
}

auto Shared::reset_tips_to(
    const cfilter::Type type,
    const block::Position& headerTip,
    const block::Position& position,
    Data& data,
    const std::optional<bool> resetHeader) const noexcept -> bool
{
    return reset_tips_to(
        type,
        headerTip,
        cfilter_tip(default_type_, data),
        position,
        data,
        resetHeader);
}

auto Shared::reset_tips_to(
    const cfilter::Type type,
    const block::Position& headerTip,
    const block::Position& filterTip,
    const block::Position& position,
    Data& data,
    std::optional<bool> resetHeader,
    std::optional<bool> resetfilter) const noexcept -> bool
{
    auto counter{0};

    if (false == resetHeader.has_value()) {
        resetHeader = headerTip > position;
    }

    if (false == resetfilter.has_value()) {
        resetfilter = filterTip > position;
    }

    OT_ASSERT(resetHeader.has_value());
    OT_ASSERT(resetfilter.has_value());

    using Future = std::shared_future<cfilter::Header>;
    auto previous = [&]() -> Future {
        const auto& block = header_.LoadHeader(position.hash_);

        OT_ASSERT(block);

        auto promise = std::promise<cfilter::Header>{};
        promise.set_value(
            load_cfheader(default_type_, block->ParentHash().Bytes(), data));

        return promise.get_future();
    }();
    auto resetBlock{false};
    auto headerTipHasBeenReset{false};
    auto filterTipHasBeenReset{false};

    if (resetHeader.value()) {
        auto& worker = data.header_downloader_;

        if (worker) {
            worker->Reset(position, Future{previous});
            headerTipHasBeenReset = true;
        }

        resetBlock = true;
        ++counter;
    }

    if (resetfilter.value()) {
        auto& worker = data.filter_downloader_;

        if (worker) {
            worker->Reset(position, Future{previous});
            filterTipHasBeenReset = true;
        }

        resetBlock = true;
        ++counter;
    }

    if (resetBlock && server_mode_) {
        data.reindex_blocks_.SendDeferred(
            MakeWork(BlockIndexerJob::reindex), __FILE__, __LINE__);
        headerTipHasBeenReset = true;
        filterTipHasBeenReset = true;
    }

    if (resetHeader.value() && (false == headerTipHasBeenReset)) {
        set_cfheader_tip(default_type_, position, data);
    }

    if (resetfilter.value() && (false == filterTipHasBeenReset)) {
        set_cfilter_tip(default_type_, position, data);
    }

    return 0 < counter;
}

auto Shared::SetCfheaderTip(
    const cfilter::Type type,
    const block::Position& tip) noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;

    return set_cfheader_tip(type, tip, data);
}

auto Shared::set_cfheader_tip(
    const cfilter::Type type,
    const block::Position& tip,
    Data& data) const noexcept -> bool
{
    return data.db_.get()->SetFilterHeaderTip(default_type_, tip);
}

auto Shared::SetCfilterTip(
    const cfilter::Type type,
    const block::Position& tip) noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;

    return set_cfilter_tip(type, tip, data);
}

auto Shared::SetTips(const block::Position& tip) noexcept -> bool
{
    return SetTips(default_type_, tip);
}

auto Shared::SetTips(
    const cfilter::Type type,
    const block::Position& tip) noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;
    const auto out =
        set_cfheader_tip(type, tip, data) && set_cfilter_tip(type, tip, data);
    update_cfilter_tip(type, tip, data);

    return out;
}

auto Shared::set_cfilter_tip(
    const cfilter::Type type,
    const block::Position& tip,
    Data& data) const noexcept -> bool
{
    return data.db_.get()->SetFilterTip(default_type_, tip);
}

auto Shared::Shutdown() noexcept -> void
{
    auto handle = data_.lock();
    auto& cfheader = handle->header_downloader_;
    auto& cfilter = handle->header_downloader_;

    cfheader.reset();
    cfilter.reset();
}

auto Shared::Start() noexcept -> void
{
    auto handle = data_.lock();
    auto& cfheader = handle->header_downloader_;
    auto& cfilter = handle->header_downloader_;

    if (cfheader) { cfheader->Start(); }
    if (cfilter) { cfilter->Start(); }
}

auto Shared::StoreCfheaders(
    const cfilter::Type type,
    const cfilter::Header& previous,
    Vector<database::Cfilter::CFHeaderParams>&& headers) noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;

    return store_cfheaders(type, previous, std::move(headers), data);
}

auto Shared::store_cfheaders(
    const cfilter::Type type,
    const cfilter::Header& previous,
    Vector<database::Cfilter::CFHeaderParams>&& headers,
    Data& data) const noexcept -> bool
{
    return data.db_.get()->StoreFilterHeaders(
        type, previous.Bytes(), std::move(headers));
}

auto Shared::StoreCfilters(
    Vector<database::Cfilter::CFilterParams>&& filters,
    Data& data,
    alloc::Default monotonic) noexcept -> bool
{
    return store_cfilters(default_type_, std::move(filters), data, monotonic);
}

auto Shared::StoreCfilters(
    const cfilter::Type type,
    const block::Position& tip,
    Vector<database::Cfilter::CFHeaderParams>&& headers,
    Vector<database::Cfilter::CFilterParams>&& filters,
    alloc::Default monotonic) noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;

    return store_cfilters(
        type, tip, std::move(headers), std::move(filters), data, monotonic);
}

auto Shared::store_cfilters(
    const cfilter::Type type,
    const block::Position& tip,
    Vector<database::Cfilter::CFHeaderParams>&& headers,
    Vector<database::Cfilter::CFilterParams>&& filters,
    Data& data,
    alloc::Default monotonic) const noexcept -> bool
{
    return data.db_.get()->StoreFilters(
        type, std::move(headers), std::move(filters), tip, monotonic);
}

auto Shared::store_cfilters(
    const cfilter::Type type,
    Vector<database::Cfilter::CFilterParams>&& filters,
    Data& data,
    alloc::Default monotonic) const noexcept -> bool
{
    return data.db_.get()->StoreFilters(type, std::move(filters), monotonic);
}

auto Shared::UpdateCfilterTip(const block::Position& tip) noexcept -> void
{
    UpdateCfilterTip(default_type_, tip);
}

auto Shared::UpdateCfilterTip(
    const cfilter::Type type,
    const block::Position& tip) noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    update_cfilter_tip(type, tip, data);
}

auto Shared::update_cfilter_tip(
    const cfilter::Type type,
    const block::Position& tip,
    Data& data) const noexcept -> void
{
    if (cfilter_tip_needs_broadcast(type, tip, data)) {
        broadcast_cfilter_tip(type, tip, data);
    }
}

auto Shared::ValidateAgainstCheckpoint(
    const block::Position& block,
    const cfilter::Header& receivedHeader) noexcept -> block::Position
{
    const auto& cp =
        implementation::FilterOracle::filter_checkpoints_.at(chain_);
    const auto height = block.height_;

    if (auto it = cp.find(height); cp.end() != it) {
        const auto& bytes = it->second.at(default_type_);
        const auto expectedHeader =
            api_.Factory().DataFromBytes(ReadView{bytes.data(), bytes.size()});

        if (expectedHeader == receivedHeader) {
            LogConsole()(print(chain_))(" filter header at height ")(
                height)(" verified against checkpoint")
                .Flush();

            return block;
        } else {
            OT_ASSERT(cp.begin() != it);

            std::advance(it, -1);
            const auto rollback =
                block::Position{it->first, header_.BestHash(it->first)};
            LogConsole()(print(chain_))(" filter header at height ")(
                height)(" does not match checkpoint. Resetting to previous "
                        "checkpoint at height ")(rollback.height_)
                .Flush();

            return rollback;
        }
    }

    return block;
}

Shared::~Shared() = default;
}  // namespace opentxs::blockchain::node::filteroracle
