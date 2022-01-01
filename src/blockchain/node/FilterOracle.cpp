// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "blockchain/node/FilterOracle.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <exception>
#include <future>
#include <iterator>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "blockchain/DownloadTask.hpp"
#include "blockchain/node/filteroracle/BlockIndexer.hpp"
#include "blockchain/node/filteroracle/FilterCheckpoints.hpp"
#include "blockchain/node/filteroracle/FilterDownloader.hpp"
#include "blockchain/node/filteroracle/HeaderDownloader.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::factory
{
using ReturnType = opentxs::blockchain::node::implementation::FilterOracle;

auto BlockchainFilterOracle(
    const api::Session& api,
    const blockchain::node::internal::Config& config,
    const blockchain::node::internal::Network& node,
    const blockchain::node::HeaderOracle& header,
    const blockchain::node::internal::BlockOracle& block,
    const blockchain::node::internal::FilterDatabase& database,
    const blockchain::Type chain,
    const blockchain::filter::Type filter,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::node::internal::FilterOracle>
{
    return std::make_unique<ReturnType>(
        api, config, node, header, block, database, chain, filter, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::implementation
{
struct FilterOracle::SyncClientFilterData {
    using Future = std::future<filter::pHeader>;
    using Promise = std::promise<filter::pHeader>;

    const block::Hash& block_hash_;
    const network::p2p::Block& incoming_data_;
    filter::pHash filter_hash_;
    internal::FilterDatabase::Filter& filter_data_;
    internal::FilterDatabase::Header& header_data_;
    Outstanding& job_counter_;
    Future previous_header_;
    Promise calculated_header_;

    SyncClientFilterData(
        OTData blank,
        const block::Hash& block,
        const network::p2p::Block& data,
        internal::FilterDatabase::Filter& filter,
        internal::FilterDatabase::Header& header,
        Outstanding& jobCounter,
        Future&& previous) noexcept
        : block_hash_(block)
        , incoming_data_(data)
        , filter_hash_(std::move(blank))
        , filter_data_(filter)
        , header_data_(header)
        , job_counter_(jobCounter)
        , previous_header_(std::move(previous))
        , calculated_header_()
    {
    }
};

FilterOracle::FilterOracle(
    const api::Session& api,
    const internal::Config& config,
    const internal::Network& node,
    const HeaderOracle& header,
    const internal::BlockOracle& block,
    const internal::FilterDatabase& database,
    const blockchain::Type chain,
    const blockchain::filter::Type filter,
    const std::string& shutdown) noexcept
    : internal::FilterOracle()
    , api_(api)
    , node_(node)
    , header_(header)
    , database_(database)
    , filter_notifier_(api_.Network().Blockchain().Internal().FilterUpdate())
    , chain_(chain)
    , default_type_(filter)
    , lock_()
    , new_filters_([&] {
        auto socket = api_.Network().ZeroMQ().PublishSocket();
        auto started = socket->Start(
            api_.Endpoints().Internal().BlockchainFilterUpdated(chain_));

        OT_ASSERT(started);

        return socket;
    }())
    , cb_([this](const auto type, const auto& pos) {
        auto lock = rLock{lock_};
        new_tip(lock, type, pos);
    })
    , filter_downloader_([&]() -> std::unique_ptr<FilterDownloader> {
        if (config.download_cfilters_) {
            return std::make_unique<FilterDownloader>(
                api,
                database_,
                header_,
                node_,
                chain,
                default_type_,
                shutdown,
                cb_);
        } else {
            return {};
        }
    }())
    , header_downloader_([&]() -> std::unique_ptr<HeaderDownloader> {
        if (config.download_cfilters_) {
            return std::make_unique<HeaderDownloader>(
                api,
                database_,
                header_,
                node_,
                *filter_downloader_,
                chain,
                default_type_,
                shutdown,
                [&](const auto& position, const auto& header) {
                    return compare_header_to_checkpoint(position, header);
                });
        } else {
            return {};
        }
    }())
    , block_indexer_([&]() -> std::unique_ptr<BlockIndexer> {
        if (config.generate_cfilters_) {
            return std::make_unique<BlockIndexer>(
                api,
                database_,
                header_,
                block,
                node_,
                *this,
                chain,
                default_type_,
                shutdown,
                cb_);
        } else {
            return {};
        }
    }())
    , last_sync_progress_()
    , last_broadcast_()
    , outstanding_jobs_()
    , running_(true)
{
    OT_ASSERT(cb_);

    compare_tips_to_header_chain();
    compare_tips_to_checkpoint();
}

auto FilterOracle::compare_header_to_checkpoint(
    const block::Position& block,
    const filter::Header& receivedHeader) noexcept -> block::Position
{
    const auto& cp = filter_checkpoints_.at(chain_);
    const auto height = block.first;

    if (auto it = cp.find(height); cp.end() != it) {
        const auto& bytes = it->second.at(default_type_);
        const auto expectedHeader =
            api_.Factory().Data(ReadView{bytes.data(), bytes.size()});

        if (expectedHeader == receivedHeader) {
            LogConsole()(DisplayString(chain_))(" filter header at height ")(
                height)(" verified against checkpoint")
                .Flush();

            return block;
        } else {
            OT_ASSERT(cp.begin() != it);

            std::advance(it, -1);
            const auto rollback =
                block::Position{it->first, header_.BestHash(it->first)};
            LogConsole()(DisplayString(chain_))(" filter header at height ")(
                height)(" does not match checkpoint. Resetting to previous "
                        "checkpoint at height ")(rollback.first)
                .Flush();

            return rollback;
        }
    }

    return block;
}

auto FilterOracle::compare_tips_to_checkpoint() noexcept -> void
{
    const auto& cp = filter_checkpoints_.at(chain_);
    const auto headerTip = database_.FilterHeaderTip(default_type_);
    auto checkPosition{headerTip};
    auto changed{false};

    for (auto i{cp.crbegin()}; i != cp.crend(); ++i) {
        const auto& cpHeight = i->first;

        if (cpHeight > checkPosition.first) { continue; }

        checkPosition = block::Position{cpHeight, header_.BestHash(cpHeight)};
        const auto existingHeader = database_.LoadFilterHeader(
            default_type_, checkPosition.second->Bytes());

        try {
            const auto& cpHeader = i->second.at(default_type_);
            const auto cpBytes =
                api_.Factory().Data(cpHeader, StringStyle::Hex);

            if (existingHeader == cpBytes) { break; }

            changed = true;
        } catch (...) {
            break;
        }
    }

    if (changed) {
        LogConsole()(DisplayString(chain_))(
            " filter header chain did not match checkpoint. Resetting to last "
            "known good position")
            .Flush();
        reset_tips_to(default_type_, headerTip, checkPosition, changed);
    } else {
        LogVerbose()(DisplayString(chain_))(
            " filter header chain matched checkpoint")
            .Flush();
    }
}

auto FilterOracle::compare_tips_to_header_chain() noexcept -> bool
{
    const auto current = database_.FilterHeaderTip(default_type_);
    const auto [parent, best] = header_.CommonParent(current);

    if ((parent.first == current.first) && (parent.second == current.second)) {
        LogVerbose()(DisplayString(chain_))(
            " filter header chain is following the best chain")
            .Flush();

        return false;
    }

    LogConsole()(DisplayString(chain_))(
        " filter header chain is following a sibling chain. Resetting to "
        "common ancestor at height ")(parent.first)
        .Flush();
    reset_tips_to(default_type_, current, parent);

    return true;
}

auto FilterOracle::GetFilterJob() const noexcept -> CfilterJob
{
    auto lock = rLock{lock_};

    if (filter_downloader_) {

        return filter_downloader_->NextBatch();
    } else {

        return {};
    }
}

auto FilterOracle::GetHeaderJob() const noexcept -> CfheaderJob
{
    auto lock = rLock{lock_};

    if (header_downloader_) {

        return header_downloader_->NextBatch();
    } else {

        return {};
    }
}

auto FilterOracle::Heartbeat() const noexcept -> void
{
    auto lock = rLock{lock_};

    if (filter_downloader_) { filter_downloader_->Heartbeat(); }
    if (header_downloader_) { header_downloader_->Heartbeat(); }
    if (block_indexer_) { block_indexer_->Heartbeat(); }

    constexpr auto limit = std::chrono::seconds{5};

    if ((Clock::now() - last_sync_progress_) > limit) {
        new_tip(lock, default_type_, database_.FilterTip(default_type_));
    }
}

auto FilterOracle::LoadFilterOrResetTip(
    const filter::Type type,
    const block::Position& position) const noexcept
    -> std::unique_ptr<const GCS>
{
    auto output = LoadFilter(type, position.second);

    if (output) { return output; }

    const auto height = position.first;

    OT_ASSERT(0 < height);

    const auto parent = height - 1;
    const auto hash = header_.BestHash(parent);

    OT_ASSERT(false == hash->empty());

    reset_tips_to(type, block::Position{parent, hash}, false, true);

    return {};
}

auto FilterOracle::new_tip(
    const rLock&,
    const filter::Type type,
    const block::Position& tip) const noexcept -> void
{
    auto last = last_broadcast_.find(type);

    if (last_broadcast_.end() != last) {
        if (tip == last->second) {
            // NOTE: new tip is same as previously broadcast

            return;
        } else {
            // NOTE: new tip is different from previously broadcast
            last->second = tip;
        }
    } else {
        // NOTE: first tip broadcast
        last_broadcast_.emplace(type, tip);
    }

    last_sync_progress_ = Clock::now();

    {
        auto work = MakeWork(OT_ZMQ_NEW_FILTER_SIGNAL);
        work.AddFrame(type);
        work.AddFrame(tip.first);
        work.AddFrame(tip.second);
        new_filters_->Send(std::move(work));
    }
    {
        auto work = MakeWork(WorkType::BlockchainNewFilter);
        work.AddFrame(chain_);
        work.AddFrame(type);
        work.AddFrame(tip.first);
        work.AddFrame(tip.second);
        filter_notifier_.Send(std::move(work));
    }
}

auto FilterOracle::ProcessBlock(
    const block::bitcoin::Block& block) const noexcept -> bool
{
    const auto& id = block.ID();
    const auto& header = block.Header();
    auto filters = std::pmr::vector<internal::FilterDatabase::Filter>{};
    auto headers = std::pmr::vector<internal::FilterDatabase::Header>{};
    const auto& pGCS =
        filters.emplace_back(id.Bytes(), process_block(default_type_, block))
            .second;

    if (false == bool(pGCS)) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate ")(
            DisplayString(chain_))(" cfilter")
            .Flush();

        return false;
    }

    const auto& gcs = *pGCS;
    const auto previousHeader =
        LoadFilterHeader(default_type_, header.ParentHash());

    if (previousHeader->empty()) {
        LogError()(OT_PRETTY_CLASS())("failed to load previous")(
            DisplayString(chain_))(" cfheader")
            .Flush();

        return false;
    }

    const auto filterHash = gcs.Hash();
    const auto& cfheader = std::get<1>(headers.emplace_back(
        id, gcs.Header(previousHeader->Bytes()), filterHash->Bytes()));

    if (cfheader->empty()) {
        LogError()(OT_PRETTY_CLASS())("failed to calculate ")(
            DisplayString(chain_))(" cfheader")
            .Flush();

        return false;
    }

    const auto position = make_blank<block::Position>::value(api_);
    const auto stored =
        database_.StoreFilters(default_type_, headers, filters, position);

    if (stored) {

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Database error ").Flush();

        return false;
    }
}

auto FilterOracle::ProcessBlock(BlockIndexerData& data) const noexcept -> void
{
    auto post = ScopeGuard{[&] { --data.job_counter_; }};
    auto& task = data.incoming_data_;
    const auto& [height, block] = task.position_;

    try {
        LogTrace()(OT_PRETTY_CLASS())("Calculating cfilter for ")(
            DisplayString(chain_))(" block at height ")(height)
            .Flush();
        auto& [blockHashView, pGCS] = data.filter_data_;
        auto& [blockHash, filterHeader, filterHashView] = data.header_data_;
        blockHash = block;
        blockHashView = blockHash->Bytes();
        const auto pBlock = task.data_.get();

        if (false == bool(pBlock)) {
            LogError()(OT_PRETTY_CLASS())("Failed to load ")(
                DisplayString(chain_))(" block #")(height)
                .Flush();

            throw std::runtime_error(
                std::string{"failed to load block "} + blockHash->asHex());
        }

        pGCS = process_block(data.type_, *pBlock);

        if (false == bool(pGCS)) {
            LogError()(OT_PRETTY_CLASS())("Failed to instantiate ")(
                DisplayString(chain_))(" cfilter #")(height)
                .Flush();

            throw std::runtime_error("Failed to instantiate gcs");
        }

        const auto& gcs = *pGCS;
        data.filter_hash_ = gcs.Hash();
        LogTrace()(OT_PRETTY_CLASS())("Finished calculating cfilter for ")(
            DisplayString(chain_))(" block at height ")(height)
            .Flush();
        filterHashView = data.filter_hash_->Bytes();
    } catch (...) {
        task.process(std::current_exception());
    }
}

auto FilterOracle::ProcessSyncData(
    const block::Hash& prior,
    const std::pmr::vector<block::pHash>& hashes,
    const network::p2p::Data& data) const noexcept -> void
{
    auto filters = std::pmr::vector<internal::FilterDatabase::Filter>{};
    auto headers = std::pmr::vector<internal::FilterDatabase::Header>{};
    auto cache = std::pmr::vector<SyncClientFilterData>{};
    const auto& blocks = data.Blocks();
    const auto incoming = blocks.front().Height();
    const auto filterType = blocks.front().FilterType();
    const auto current = database_.FilterTip(filterType);

    if ((1 == incoming) && (1000 < current.first)) {
        const auto height = current.first - 1000;
        reset_tips_to(
            filterType, block::Position{height, header_.BestHash(height)});

        return;
    }

    if (incoming > (current.first + 1)) { return; }

    const auto count{std::min<std::size_t>(hashes.size(), blocks.size())};

    try {
        auto jobCounter = outstanding_jobs_.Allocate();

        OT_ASSERT(0 < count);

        filters.reserve(count);
        headers.reserve(count);
        cache.reserve(count);
        auto previous = [&] {
            if (prior.empty()) {

                return block::BlankHash();
            } else {

                auto output = LoadFilterHeader(filterType, prior);

                if (output->empty()) {
                    LogError()(OT_PRETTY_CLASS())("cfheader for ")(
                        DisplayString(chain_))(" block ")(prior.asHex())(
                        " not found")
                        .Flush();

                    throw std::runtime_error(
                        "Failed to load previous cfheader");
                }

                return output;
            }
        }();
        static const auto blank = api_.Factory().Data();
        static const auto blankView = ReadView{};
        auto first{true};

        for (auto i = std::size_t{0u}; i < count; ++i) {
            auto& filter = filters.emplace_back(blankView, nullptr);
            auto& header = headers.emplace_back(blank, blank, blankView);
            auto& job = cache.emplace_back(
                blank,
                hashes.at(i),
                blocks.at(i),
                filter,
                header,
                jobCounter,
                [&] {
                    if (first) {
                        first = false;
                        auto promise = std::promise<filter::pHeader>{};
                        auto post = ScopeGuard{
                            [&] { promise.set_value(std::move(previous)); }};

                        return promise.get_future();
                    } else {
                        return cache.at(i - 1).calculated_header_.get_future();
                    }
                }());
            ++jobCounter;
            const auto queued = api_.Network().Asio().Internal().Post(
                ThreadPool::General, [&] { ProcessSyncData(job); });

            if (false == queued) {
                --jobCounter;

                return;
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return;
    }

    if (false == ProcessSyncData(cache)) { return; }

    try {
        auto future = cache.back().calculated_header_.get_future();
        future.get();
        const auto tip = [&] {
            const auto back = count - 1u;

            return block::Position{blocks.at(back).Height(), hashes.at(back)};
        }();
        make_blank<block::Position>::value(api_);
        const auto stored =
            database_.StoreFilters(filterType, headers, filters, tip);

        if (stored) {
            LogDetail()(DisplayString(chain_))(
                " cfheader and cfilter chain updated to height ")(tip.first)
                .Flush();
            cb_(filterType, tip);
        } else {
            LogError()(OT_PRETTY_CLASS())("Database error ").Flush();
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return;
    }
}

auto FilterOracle::ProcessSyncData(SyncClientFilterData& data) const noexcept
    -> void
{
    auto post = ScopeGuard{[&] { --data.job_counter_; }};

    try {
        const auto& block = data.block_hash_;
        const auto height = data.incoming_data_.Height();
        const auto type = data.incoming_data_.FilterType();
        const auto count = data.incoming_data_.FilterElements();
        const auto bytes = data.incoming_data_.Filter();
        LogTrace()(OT_PRETTY_CLASS())("Received filter for ")(
            DisplayString(chain_))(" block at height ")(height)
            .Flush();
        auto& [blockHashView, pGCS] = data.filter_data_;
        auto& [blockHash, filterHeader, filterHashView] = data.header_data_;
        blockHash = block;
        blockHashView = blockHash->Bytes();
        const auto params = blockchain::internal::GetFilterParams(type);
        pGCS = factory::GCS(
            api_,
            params.first,
            params.second,
            blockchain::internal::BlockHashToFilterKey(block.Bytes()),
            count,
            bytes);

        if (false == bool(pGCS)) {
            LogError()(OT_PRETTY_CLASS())("Failed to instantiate ")(
                DisplayString(chain_))(" cfilter #")(height)
                .Flush();

            throw std::runtime_error("Failed to instantiate gcs");
        }

        const auto& gcs = *pGCS;
        data.filter_hash_ = gcs.Hash();
        filterHashView = data.filter_hash_->Bytes();
        LogTrace()(OT_PRETTY_CLASS())("Finished calculating cfilter for ")(
            DisplayString(chain_))(" block at height ")(height)
            .Flush();
    } catch (...) {
        data.calculated_header_.set_exception(std::current_exception());
    }
}

auto FilterOracle::ProcessSyncData(
    std::pmr::vector<SyncClientFilterData>& cache) const noexcept -> bool
{
    auto failures{0};

    for (auto& data : cache) {
        try {
            const auto height = data.incoming_data_.Height();
            auto& previous = data.previous_header_;
            auto& [blockHashView, pGCS] = data.filter_data_;
            auto& [blockHash, filterHeader, filterHashView] = data.header_data_;
            static constexpr auto zero = std::chrono::seconds{0};
            using State = std::future_status;

            if (auto status = previous.wait_for(zero); State::ready != status) {
                LogError()(OT_PRETTY_CLASS())("Timeout waiting for previous ")(
                    DisplayString(chain_))(" cfheader #")(height - 1)
                    .Flush();

                throw std::runtime_error("timeout");
            }

            OT_ASSERT(pGCS);

            const auto& gcs = *pGCS;
            filterHeader = gcs.Header(previous.get()->Bytes());

            if (filterHeader->empty()) {
                LogError()(OT_PRETTY_CLASS())("failed to calculate ")(
                    DisplayString(chain_))(" cfheader #")(height)
                    .Flush();

                throw std::runtime_error("Failed to calculate cfheader");
            }

            data.calculated_header_.set_value(filterHeader);
            LogTrace()(OT_PRETTY_CLASS())("Finished calculating cfheader for ")(
                DisplayString(chain_))(" block at height ")(height)
                .Flush();
        } catch (...) {
            data.calculated_header_.set_exception(std::current_exception());
            ++failures;
        }
    }

    return (0 == failures);
}

auto FilterOracle::process_block(
    const filter::Type filterType,
    const block::bitcoin::Block& block) const noexcept
    -> std::unique_ptr<const GCS>
{
    const auto& id = block.ID();
    const auto params = blockchain::internal::GetFilterParams(filterType);
    const auto elements = [&] {
        const auto input = block.Internal().ExtractElements(filterType);
        auto output = std::pmr::vector<OTData>{};
        std::transform(
            input.begin(),
            input.end(),
            std::back_inserter(output),
            [&](const auto& element) -> OTData {
                return api_.Factory().Data(reader(element));
            });

        return output;
    }();

    return factory::GCS(
        api_,
        params.first,
        params.second,
        blockchain::internal::BlockHashToFilterKey(id.Bytes()),
        elements);
}

auto FilterOracle::reset_tips_to(
    const filter::Type type,
    const block::Position& position,
    const std::optional<bool> resetHeader,
    const std::optional<bool> resetfilter) const noexcept -> bool
{
    return reset_tips_to(
        type,
        database_.FilterHeaderTip(default_type_),
        database_.FilterTip(default_type_),
        position,
        resetHeader,
        resetfilter);
}

auto FilterOracle::reset_tips_to(
    const filter::Type type,
    const block::Position& headerTip,
    const block::Position& position,
    const std::optional<bool> resetHeader) const noexcept -> bool
{
    return reset_tips_to(
        type,
        headerTip,
        database_.FilterTip(default_type_),
        position,
        resetHeader);
}

auto FilterOracle::reset_tips_to(
    const filter::Type type,
    const block::Position& headerTip,
    const block::Position& filterTip,
    const block::Position& position,
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

    auto lock = rLock{lock_};
    using Future = std::shared_future<filter::pHeader>;
    auto previous = [&]() -> Future {
        const auto& block = header_.LoadHeader(position.second);

        OT_ASSERT(block);

        auto promise = std::promise<filter::pHeader>{};
        promise.set_value(database_.LoadFilterHeader(
            default_type_, block->ParentHash().Bytes()));

        return promise.get_future();
    }();
    auto resetBlock{false};
    auto headerTipHasBeenReset{false};
    auto filterTipHasBeenReset{false};

    if (resetHeader.value()) {
        if (header_downloader_) {
            header_downloader_->Reset(position, Future{previous});
            headerTipHasBeenReset = true;
        }

        resetBlock = true;
        ++counter;
    }

    if (resetfilter.value()) {
        if (filter_downloader_) {
            filter_downloader_->Reset(position, Future{previous});
            filterTipHasBeenReset = true;
        }

        resetBlock = true;
        ++counter;
    }

    if (resetBlock && block_indexer_) {
        block_indexer_->Reset(position, std::move(previous));
        headerTipHasBeenReset = true;
        filterTipHasBeenReset = true;
    }

    if (resetHeader.value() && (false == headerTipHasBeenReset)) {
        database_.SetFilterHeaderTip(default_type_, position);
    }

    if (resetfilter.value() && (false == filterTipHasBeenReset)) {
        database_.SetFilterTip(default_type_, position);
    }

    return 0 < counter;
}

auto FilterOracle::Shutdown() noexcept -> void
{
    running_ = false;

    auto lock = rLock{lock_};

    if (header_downloader_) { header_downloader_.reset(); }

    if (filter_downloader_) { filter_downloader_.reset(); }

    if (block_indexer_) { block_indexer_.reset(); }
}

auto FilterOracle::Start() noexcept -> void
{
    auto lock = rLock{lock_};

    if (header_downloader_) { header_downloader_->Start(); }

    if (filter_downloader_) { filter_downloader_->Start(); }

    if (block_indexer_) { block_indexer_->Start(); }
}

FilterOracle::~FilterOracle() { Shutdown(); }
}  // namespace opentxs::blockchain::node::implementation
