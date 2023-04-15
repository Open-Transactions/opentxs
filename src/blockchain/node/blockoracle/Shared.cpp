// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/blockoracle/Shared.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <future>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>

#include "TBB.hpp"
#include "blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Parser.hpp"
#include "internal/blockchain/database/Block.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/blockchain/node/headeroracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/storage/file/Reader.hpp"
#include "internal/util/storage/file/Types.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::internal
{
BlockOracle::Shared::Shared(
    const api::Session& api,
    const node::Manager& node,
    allocator_type alloc) noexcept
    : log_(LogTrace())
    , api_(api)
    , node_(node)
    , chain_(node_.Internal().Chain())
    , name_([&] {
        using namespace std::literals;
        auto out = CString{alloc};
        out.append(print(node_.Internal().Chain()));
        out.append(" block oracle"sv);

        return out;
    }())
    , genesis_(0, params::get(chain_).GenesisHash())
    , db_(node_.Internal().DB())
    , use_persistent_storage_(
          BlockchainProfile::mobile != node_.Internal().GetConfig().profile_)
    , cache_(alloc)
    , futures_(api_, name_, chain_, alloc)
    , queue_(
          log_,
          name_,
          node_.Internal().GetConfig().PeerTarget(chain_),
          alloc)
    , update_(api_, node_.Internal().Endpoints(), log_, name_, alloc)
    , to_blockchain_api_([&, this] {
        using enum network::zeromq::socket::Type;
        auto out = api_.Network().ZeroMQ().Internal().RawSocket(Push);
        const auto rc = out.Connect(
            api_.Endpoints().Internal().BlockchainMessageRouter().data());

        OT_ASSERT(rc);

        return out;
    }())
    , to_header_oracle_([&, this] {
        using enum network::zeromq::socket::Type;
        auto out = api_.Network().ZeroMQ().Internal().RawSocket(Push);
        const auto rc = out.Connect(
            node_.Internal().Endpoints().header_oracle_pull_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , publish_([&, this] {
        using enum network::zeromq::socket::Type;
        auto out = api_.Network().ZeroMQ().Internal().RawSocket(Publish);
        const auto rc = out.Bind(
            node_.Internal().Endpoints().block_oracle_publish_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
{
}

auto BlockOracle::Shared::bad_block(
    const block::Hash& id,
    const BlockLocation& block) const noexcept -> void
{
    struct Visitor {
        const block::Hash& id_;
        const Shared& this_;

        auto operator()(const MissingBlock&) noexcept {}
        auto operator()(const PersistentBlock& bytes) noexcept
        {
            this_.db_.BlockDelete(id_);
        }
        auto operator()(const CachedBlock& block) noexcept
        {
            this_.cache_.lock()->Clear();
        }
    };

    std::visit(Visitor{id, *this}, block);
}

auto BlockOracle::Shared::block_is_ready(
    const block::Hash& id,
    const BlockLocation& block,
    allocator_type monotonic) const noexcept -> void
{
    futures_.lock()->Receive(api_.Crypto(), chain_, id, block, monotonic);
    publish_queue(queue_.lock()->Receive(id));
    update_.lock()->Queue(id, block);
}

auto BlockOracle::Shared::check_block(BlockData& data) const noexcept -> void
{
    const auto& id = *std::get<0>(data);
    const auto& block = *std::get<1>(data);
    auto& result = *std::get<2>(data);
    const auto& crypto = api_.Crypto();
    using block::Parser;
    auto files = Vector<storage::file::Reader>{};
    files.clear();

    if (false == is_valid(block)) {
        result = 0;
    } else if (
        false ==
        Parser::Check(crypto, chain_, id, reader(block, files, {}), {})) {
        // TODO monotonic allocator
        result = 1;
    } else {
        result = 2;
    }
}

auto BlockOracle::Shared::check_blocks(std::span<BlockData> view) const noexcept
    -> void
{
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, view.size()},
        [&, this](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                check_block(view[i]);
            }
        });
}

auto BlockOracle::Shared::check_header(
    const blockchain::block::Header& header) const noexcept -> void
{
    const auto data = [&] {
        auto out = ByteArray{};
        header.Serialize(out.WriteInto());

        return out;
    }();
    check_header(header.Hash(), data.Bytes());
}

auto BlockOracle::Shared::check_header(
    const block::Hash& id,
    const ReadView header) const noexcept -> void
{
    if (false == node_.HeaderOracle().Exists(id)) {
        to_header_oracle_.lock()->SendDeferred(
            [&] {
                using enum headeroracle::Job;
                auto work = MakeWork(submit_block_header);
                work.AddFrame(header.data(), header.size());

                return work;
            }(),
            __FILE__,
            __LINE__);
    }
}

auto BlockOracle::Shared::DownloadQueue() const noexcept -> std::size_t
{
    return queue_.lock()->Items().second;
}

auto BlockOracle::Shared::FinishJob(download::JobID job) const noexcept -> void
{
    publish_queue(queue_.lock()->Finish(job));
    update_.lock()->FinishJob();
}

auto BlockOracle::Shared::FinishWork() noexcept -> void
{
    update_.lock()->FinishWork();
}

auto BlockOracle::Shared::GetBlocks(
    Hashes hashes,
    allocator_type monotonic,
    allocator_type alloc) const noexcept -> Vector<BlockLocation>
{
    if (hashes.empty()) { return Vector<BlockLocation>{alloc}; }

    const auto count = hashes.size();

    auto download = Vector<block::Hash>{monotonic};
    download.reserve(count);
    auto blocks = load_blocks(hashes, alloc, monotonic);
    auto results = Vector<int>{count, 0, monotonic};
    auto view = [&] {
        auto out = Vector<BlockData>{monotonic};
        out.reserve(count);

        for (auto n = 0_uz; n < count; ++n) {
            const auto& id = hashes[n];
            const auto& block = blocks[n];
            auto& result = results[n];
            out.emplace_back(
                std::addressof(id),
                std::addressof(block),
                std::addressof(result));
        }

        return out;
    }();

    OT_ASSERT(blocks.size() == count);
    OT_ASSERT(results.size() == count);
    OT_ASSERT(view.size() == count);

    check_blocks(view);

    for (auto n = 0_uz; n < count; ++n) {
        const auto& id = hashes[n];
        auto& block = blocks[n];
        const auto& result = results[n];

        switch (result) {
            case 1: {
                LogError()(OT_PRETTY_CLASS())(name_)(": block ")
                    .asHex(id)(" does not pass validation checks and must be "
                               "re-downloaded")
                    .Flush();
                bad_block(id, block);
                block = MissingBlock{};
                [[fallthrough]];
            }
            case 0: {
                download.emplace_back(id);
            } break;
            default: {
            }
        }
    }

    publish_queue(queue_.lock()->Add(download));

    return blocks;
}

auto BlockOracle::Shared::GetTip(allocator_type monotonic) noexcept
    -> block::Position
{
    static const auto blank = block::Position{};

    if (const auto pos = db_.BlockTip(); blank != pos) {
        constexpr auto verify = block::Height{100};
        constexpr auto getTarget = [](auto current) {
            return std::max<block::Height>(1, current - verify + 1);
        };
        constexpr auto getCount = [](auto current, auto target) {
            return static_cast<std::size_t>(
                std::max<block::Height>(0, current - target + 1));
        };
        static_assert(getTarget(0) == 1);
        static_assert(getTarget(1) == 1);
        static_assert(getTarget(2) == 1);
        static_assert(getTarget(99) == 1);
        static_assert(getTarget(100) == 1);
        static_assert(getTarget(101) == 2);
        static_assert(getTarget(102) == 3);
        static_assert(getCount(0, 1) == 0);
        static_assert(getCount(1, 1) == 1);
        static_assert(getCount(2, 1) == 2);
        static_assert(getCount(99, 1) == 99);
        static_assert(getCount(100, 1) == 100);
        static_assert(getCount(101, 2) == 100);
        static_assert(getCount(102, 3) == 100);

        const auto& oracle = node_.HeaderOracle();
        const auto& best = [&]() -> block::Position {
            auto target = getTarget(pos.height_);
            auto count = getCount(pos.height_, target);

            if (0_uz < count) {
                do {
                    LogConsole()("Verifying ")(count)(" ")(print(chain_))(
                        " blocks starting from height ")(target)
                        .Flush();
                    auto files = Vector<storage::file::Reader>{monotonic};
                    files.reserve(count);
                    files.clear();
                    const auto hashes = oracle.BestHashes(target, count);
                    const auto blocks =
                        load_blocks(hashes, monotonic, monotonic);
                    auto height{target};
                    auto h = hashes.cbegin();
                    auto b{blocks.cbegin()};
                    auto good = std::optional<block::Position>{std::nullopt};

                    for (auto end = blocks.cend(); b != end;
                         ++b, ++h, ++height) {
                        const auto& crypto = api_.Crypto();
                        const auto& id = *h;
                        const auto& block = *b;
                        using block::Parser;

                        if (false == is_valid(block)) {
                            LogError()(print(chain_))(" block ")
                                .asHex(id)(" at height ")(height)(" is missing")
                                .Flush();

                            break;
                        } else if (
                            false == Parser::Check(
                                         crypto,
                                         chain_,
                                         id,
                                         reader(block, files, monotonic),
                                         monotonic)) {
                            LogError()(print(chain_))(" block ")
                                .asHex(id)(" at height ")(
                                    height)(" is corrupted")
                                .Flush();

                            break;
                        } else {
                            log_(print(chain_))(" block ")
                                .asHex(id)(" at height ")(height)(" is valid")
                                .Flush();
                            good.reset();
                            good.emplace(height, id);
                        }
                    }

                    if (good.has_value()) {

                        return *good;
                    } else {
                        const auto current{target};
                        target = getTarget(current);
                        count = getCount(current, target);
                    }
                } while (block::Height{1} > target);
            }

            return genesis_;
        }();

        if (best != pos) {
            const auto rc = db_.SetBlockTip(best);

            OT_ASSERT(rc);
        }

        return best;
    } else {

        return genesis_;
    }
}

auto BlockOracle::Shared::GetWork(alloc::Default alloc) const noexcept
    -> BlockBatch
{
    const auto& log = log_;
    auto pmr = alloc::PMR<node::internal::BlockBatch::Imp>{alloc};
    auto work = queue_.lock()->GetWork(alloc);
    // TODO c++20
    auto post = ScopeGuard{[&] {
        auto& [id, hashes, jobs, downloading] = work;
        publish_queue(std::make_pair(jobs, downloading));
    }};
    auto& [id, hashes, jobs, downloading] = work;

    if (hashes.empty()) {
        OT_ASSERT(0 > id);

        log(OT_PRETTY_CLASS())(name_)(": no work").Flush();

        return {};
    } else {
        OT_ASSERT(0 <= id);

        log(OT_PRETTY_CLASS())(name_)(": issuing job ")(id)(" for ")(
            hashes.size())(" blocks")
            .Flush();
        auto me = boost::shared_from(this);

        OT_ASSERT(me);

        // TODO c++20
        auto* imp = pmr.allocate(1_uz);
        pmr.construct(
            imp,
            id,
            std::move(hashes),
            // TODO monotonic allocator
            [me](const auto bytes) { me->Receive(bytes, {}); },
            [me, job = id] { me->FinishJob(job); });
        update_.lock()->StartJob();

        return imp;
    }
}

auto BlockOracle::Shared::get_allocator() const noexcept -> allocator_type
{
    return futures_.lock()->get_allocator();
}

auto BlockOracle::Shared::Load(
    const block::Hash& block,
    allocator_type monotonic) const noexcept -> BlockResult
{
    auto output =
        Load(Hashes{std::addressof(block), 1_uz}, monotonic, monotonic);

    OT_ASSERT(false == output.empty());

    return std::move(output.front());
}

auto BlockOracle::Shared::Load(
    Hashes hashes,
    allocator_type alloc,
    allocator_type monotonic) const noexcept -> BlockResults
{
    using block::Parser;
    const auto count = hashes.size();
    auto out = BlockResults{alloc};
    auto download = Vector<block::Hash>{monotonic};
    out.reserve(count);
    download.reserve(count);
    auto files = Vector<storage::file::Reader>{monotonic};
    files.reserve(count);
    files.clear();
    {
        auto handle = futures_.lock();
        auto& futures = *handle;
        const auto& crypto = api_.Crypto();
        const auto blocks = load_blocks(hashes, monotonic, monotonic);

        OT_ASSERT(blocks.size() == hashes.size());

        auto h = hashes.begin();
        auto b = blocks.cbegin();

        for (auto end = blocks.cend(); b != end; ++b, ++h) {
            const auto& block = *b;
            const auto& hash = *h;
            auto& result = out.emplace_back();
            // NOTE we have no idea what allocator the holder of the future
            // prefers so use the default allocator
            auto system = alloc::Default{};
            auto p = block::Block{system};

            if (false == is_valid(block)) {
                futures.Queue(hash, result);
                download.emplace_back(hash);
            } else if (Parser::Construct(
                           crypto,
                           chain_,
                           hash,
                           reader(block, files, monotonic),
                           p,
                           system)) {
                auto promise = Promise{};
                result = promise.get_future();
                promise.set_value(std::move(p));
                to_blockchain_api_.lock()->SendDeferred(
                    [&] {
                        auto work = network::zeromq::tagged_message(
                            WorkType::BlockchainBlockAvailable, true);
                        work.AddFrame(chain_);
                        work.AddFrame(hash);

                        return work;
                    }(),
                    __FILE__,
                    __LINE__);
            } else {
                LogError()(OT_PRETTY_CLASS())(name_)(": block ")
                    .asHex(hash)(" does not pass validation checks and must be "
                                 "re-downloaded")
                    .Flush();
                bad_block(hash, block);
                futures.Queue(hash, result);
                download.emplace_back(hash);
            }
        }

        OT_ASSERT(out.size() == count);
    }

    publish_queue(queue_.lock()->Add(download));

    return out;
}

auto BlockOracle::Shared::load_blocks(
    const Hashes& blocks,
    allocator_type alloc,
    allocator_type monotonic) const noexcept -> Vector<BlockLocation>
{
    const auto count = blocks.size();
    auto out = Vector<BlockLocation>{alloc};
    out.reserve(count);

    if (use_persistent_storage_) {
        const auto result = db_.BlockLoad(blocks, monotonic, monotonic);
        std::transform(
            result.begin(),
            result.end(),
            std::back_inserter(out),
            [](const auto& position) -> BlockLocation {
                if (position) {

                    return position;
                } else {

                    return MissingBlock{};
                }
            });
    } else {
        auto handle = cache_.lock();
        auto& cache = *handle;
        std::transform(
            blocks.begin(),
            blocks.end(),
            std::back_inserter(out),
            [&](const auto& id) -> BlockLocation {
                auto block = cache.Load(id);

                if (block) {

                    return block;
                } else {

                    return MissingBlock{};
                }
            });
    }

    OT_ASSERT(out.size() == count);

    return out;
}

auto BlockOracle::Shared::publish_queue(QueueData queue) const noexcept -> void
{
    const auto& [jobs, downloading] = queue;
    // TODO c++20
    to_blockchain_api_.lock()->SendDeferred(
        [&](const auto& value) {
            auto work = network::zeromq::tagged_message(
                WorkType::BlockchainBlockDownloadQueue, true);
            work.AddFrame(chain_);
            work.AddFrame(value);

            return work;
        }(downloading),
        __FILE__,
        __LINE__);

    if (0_uz < jobs) { work_available(); }
}

auto BlockOracle::Shared::Receive(
    const ReadView block,
    allocator_type monotonic) const noexcept -> bool
{
    const auto& log = log_;
    using block::Parser;
    auto id = block::Hash{};
    auto header = ReadView{};
    const auto valid =
        Parser::Check(api_.Crypto(), chain_, block, id, header, {});

    if (valid) {
        log(OT_PRETTY_CLASS())(name_)(": validated block ").asHex(id).Flush();
        check_header(id, header);

        return receive(id, block, monotonic);
    } else {
        LogError()(OT_PRETTY_CLASS())(
            name_)(": received an invalid block with apparent hash ")
            .asHex(id)
            .Flush();

        return false;
    }
}

auto BlockOracle::Shared::receive(
    const block::Hash& id,
    const ReadView block,
    allocator_type monotonic) const noexcept -> bool
{
    const auto& log = log_;
    const auto saved = save_block(id, block, monotonic);

    if (is_valid(saved)) {
        log(OT_PRETTY_CLASS())("saved block ").asHex(id).Flush();
        block_is_ready(id, saved, monotonic);

        return true;
    } else {
        log(OT_PRETTY_CLASS())("failed to save block ").asHex(id).Flush();

        return false;
    }
}

auto BlockOracle::Shared::save_block(
    const block::Hash& id,
    const ReadView bytes,
    allocator_type monotonic) const noexcept -> BlockLocation
{
    if (use_persistent_storage_) {
        const auto location = save_to_database(id, bytes, monotonic);

        if (location) { return location; }
    } else {
        const auto saved = save_to_cache(id, bytes);

        if (saved.operator bool()) { return saved; }
    }

    return MissingBlock{};
}

auto BlockOracle::Shared::save_to_cache(
    const block::Hash& id,
    const ReadView bytes) const noexcept -> CachedBlock
{
    return cache_.lock()->Store(id, bytes);
}

auto BlockOracle::Shared::save_to_database(
    const block::Hash& id,
    const ReadView bytes,
    allocator_type monotonic) const noexcept -> PersistentBlock
{
    return db_.BlockStore(id, bytes, monotonic);
}

auto BlockOracle::Shared::SetTip(const block::Position& tip) noexcept -> bool
{
    return db_.SetBlockTip(tip);
}

auto BlockOracle::Shared::SubmitBlock(
    const blockchain::block::Block& in,
    allocator_type monotonic) const noexcept -> bool
{
    try {
        const auto& header = in.Header();
        check_header(header);
        const auto serialized = [&] {
            auto out = ByteArray{};

            if (false == in.Serialize(out.WriteInto())) {

                throw std::runtime_error{"serialization error"};
            }

            return out;
        }();

        return receive(header.Hash(), serialized.Bytes(), monotonic);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto BlockOracle::Shared::Tip() const noexcept -> block::Position
{
    return db_.BlockTip();
}

auto BlockOracle::Shared::work_available() const noexcept -> void
{
    publish_.lock()->SendDeferred(
        MakeWork(OT_ZMQ_BLOCK_ORACLE_JOB_AVAILABLE), __FILE__, __LINE__);
}

BlockOracle::Shared::~Shared() = default;
}  // namespace opentxs::blockchain::node::internal
