// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
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
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::internal
{
using blockoracle::CachedBlock;
using blockoracle::MissingBlock;
using blockoracle::PersistentBlock;

struct BlockOracle::Shared::GetBytes {
    auto operator()(const MissingBlock&) noexcept -> ReadView { return {}; }
    auto operator()(const PersistentBlock& bytes) noexcept -> ReadView
    {
        return bytes;
    }
    auto operator()(const CachedBlock& block) noexcept -> ReadView
    {
        return block->Bytes();
    }
};
}  // namespace opentxs::blockchain::node::internal

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
    const blockoracle::BlockLocation& block) const noexcept -> void
{
    using blockoracle::CachedBlock;
    using blockoracle::MissingBlock;
    using blockoracle::PersistentBlock;

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
    const ReadView bytes) const noexcept -> void
{
    futures_.lock()->Receive(api_.Crypto(), chain_, id, bytes);
    publish_queue(queue_.lock()->Receive(id));
}

auto BlockOracle::Shared::block_is_ready_cached(
    const block::Hash& id,
    const ReadView bytes) const noexcept -> void
{
    block_is_ready(id, bytes);
    update_.lock()->Queue(id, bytes, false);
}

auto BlockOracle::Shared::block_is_ready_db(
    const block::Hash& id,
    const ReadView bytes) const noexcept -> void
{
    block_is_ready(id, bytes);
    update_.lock()->Queue(id, bytes, true);
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
    allocator_type alloc) const noexcept -> Vector<blockoracle::BlockLocation>
{
    using block::Parser;
    const auto count = hashes.size();
    auto download = Vector<block::Hash>{monotonic};
    download.reserve(count);
    const auto& crypto = api_.Crypto();
    auto out = load_blocks(hashes, alloc, monotonic);

    OT_ASSERT(out.size() == hashes.size());

    auto h = hashes.begin();
    auto b = out.begin();

    for (auto stop = out.end(); b != stop; ++b, ++h) {
        const auto& id = *h;
        auto& block = *b;
        const auto bytes = std::visit(GetBytes{}, block);

        if (false == valid(bytes)) {
            download.emplace_back(id);
            block = std::monostate{};
        } else if (false == Parser::Check(crypto, chain_, id, bytes)) {
            LogError()(OT_PRETTY_CLASS())(name_)(": block ")
                .asHex(id)(" does not pass validation checks and must be "
                           "re-downloaded")
                .Flush();
            bad_block(id, block);
            download.emplace_back(id);
            block = std::monostate{};
        }
    }

    publish_queue(queue_.lock()->Add(download));

    OT_ASSERT(out.size() == hashes.size());

    return out;
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
                        const auto bytes = std::visit(GetBytes{}, block);
                        using block::Parser;

                        if (false == valid(bytes)) {
                            LogError()(print(chain_))(" block ")
                                .asHex(id)(" at height ")(height)(" is missing")
                                .Flush();

                            break;
                        } else if (!Parser::Check(crypto, chain_, id, bytes)) {
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
    auto& [id, hashes, jobs, downloading] = work;
    // TODO c++20
    auto post = ScopeGuard{[&] {
        auto& [id, hashes, jobs, downloading] = work;
        publish_queue(std::make_pair(jobs, downloading));
    }};

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
            [me](const auto bytes) { me->Receive(bytes); },
            [me, job = id] { me->FinishJob(job); });
        update_.lock()->StartJob();

        return imp;
    }
}

auto BlockOracle::Shared::get_allocator() const noexcept -> allocator_type
{
    return futures_.lock()->get_allocator();
}

auto BlockOracle::Shared::Load(const block::Hash& block) const noexcept
    -> BitcoinBlockResult
{
    // TODO monotonic allocator
    auto output = Load(Hashes{std::addressof(block), 1_uz});

    OT_ASSERT(false == output.empty());

    return std::move(output.front());
}

auto BlockOracle::Shared::Load(Hashes hashes, allocator_type alloc)
    const noexcept -> BitcoinBlockResults
{
    using block::Parser;
    const auto count = hashes.size();
    auto out = BitcoinBlockResults{alloc};
    auto download = Vector<block::Hash>{};  // TODO monotonic allocator
    out.reserve(count);
    download.reserve(count);
    {
        auto handle = futures_.lock();
        auto& futures = *handle;
        const auto& crypto = api_.Crypto();
        const auto blocks = load_blocks(hashes, {}, {});  // TODO monotonic

        OT_ASSERT(blocks.size() == hashes.size());

        auto h = hashes.begin();
        auto b = blocks.cbegin();

        for (auto end = blocks.cend(); b != end; ++b, ++h) {
            const auto& block = *b;
            const auto& hash = *h;
            auto& result = out.emplace_back();
            const auto bytes = std::visit(GetBytes{}, block);
            auto p = std::shared_ptr<bitcoin::block::Block>{};

            if (false == valid(bytes)) {
                futures.Queue(hash, result);
                download.emplace_back(hash);
            } else if (Parser::Construct(crypto, chain_, hash, bytes, p)) {
                OT_ASSERT(p);

                auto promise = blockoracle::Promise{};
                result = promise.get_future();
                promise.set_value(std::move(p));
                to_blockchain_api_.lock()->SendDeferred(
                    [&] {
                        auto work = network::zeromq::tagged_message(
                            WorkType::BlockchainBlockAvailable);
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
    allocator_type monotonic) const noexcept
    -> Vector<blockoracle::BlockLocation>
{
    const auto count = blocks.size();
    auto out = Vector<blockoracle::BlockLocation>{alloc};
    out.reserve(count);

    if (use_persistent_storage_) {
        const auto result = db_.BlockLoad(blocks, monotonic);
        std::transform(
            result.begin(),
            result.end(),
            std::back_inserter(out),
            [](const auto& bytes) -> blockoracle::BlockLocation {
                if (valid(bytes)) {

                    return bytes;
                } else {

                    return blockoracle::MissingBlock{};
                }
            });
    } else {
        auto handle = cache_.lock();
        auto& cache = *handle;
        std::transform(
            blocks.begin(),
            blocks.end(),
            std::back_inserter(out),
            [&](const auto& id) -> blockoracle::BlockLocation {
                auto block = cache.Load(id);

                if (block) {

                    return block;
                } else {

                    return blockoracle::MissingBlock{};
                }
            });
    }

    OT_ASSERT(out.size() == count);

    return out;
}

auto BlockOracle::Shared::publish_queue(
    blockoracle::QueueData queue) const noexcept -> void
{
    const auto& [jobs, downloading] = queue;
    // TODO c++20
    to_blockchain_api_.lock()->SendDeferred(
        [&](const auto& value) {
            auto work = network::zeromq::tagged_message(
                WorkType::BlockchainBlockDownloadQueue);
            work.AddFrame(chain_);
            work.AddFrame(value);

            return work;
        }(downloading),
        __FILE__,
        __LINE__);

    if (0_uz < jobs) { work_available(); }
}

auto BlockOracle::Shared::Receive(const ReadView block) const noexcept -> bool
{
    const auto& log = log_;
    using block::Parser;
    auto id = block::Hash{};
    auto header = ReadView{};
    const auto valid = Parser::Check(api_.Crypto(), chain_, block, id, header);

    if (valid) {
        log(OT_PRETTY_CLASS())(name_)(": validated block ").asHex(id).Flush();
        check_header(id, header);

        return receive(id, block);
    } else {
        LogError()(OT_PRETTY_CLASS())(
            name_)(": received an invalid block with apparent hash ")
            .asHex(id)
            .Flush();

        return false;
    }
}

auto BlockOracle::Shared::receive(const block::Hash& id, const ReadView block)
    const noexcept -> bool
{
    const auto& log = log_;
    const auto saved = save_block(id, block);
    using blockoracle::CachedBlock;
    using blockoracle::MissingBlock;
    using blockoracle::PersistentBlock;
    struct Visitor {
        const Log& log_;
        const block::Hash& id_;
        const Shared& this_;

        auto operator()(const MissingBlock&) noexcept -> bool
        {
            log_(": failed to save block ").asHex(id_).Flush();

            return false;
        }
        auto operator()(const PersistentBlock& bytes) noexcept -> bool
        {
            log_(": block ").asHex(id_)(" saved to database").Flush();
            this_.block_is_ready_db(id_, bytes);

            return true;
        }
        auto operator()(const CachedBlock& block) noexcept -> bool
        {
            log_(": block ").asHex(id_)(" saved to cache").Flush();
            this_.block_is_ready_cached(id_, block->Bytes());

            return true;
        }
    };

    log(OT_PRETTY_CLASS())(name_);

    return std::visit(Visitor{log, id, *this}, saved);
}

auto BlockOracle::Shared::save_block(
    const block::Hash& id,
    const ReadView bytes) const noexcept -> blockoracle::BlockLocation
{
    if (use_persistent_storage_) {
        const auto saved = save_to_database(id, bytes);

        if (valid(saved)) { return saved; }
    } else {
        const auto saved = save_to_cache(id, bytes);

        if (saved.operator bool()) { return saved; }
    }

    return std::monostate{};
}

auto BlockOracle::Shared::save_to_cache(
    const block::Hash& id,
    const ReadView bytes) const noexcept -> std::shared_ptr<const ByteArray>
{
    return cache_.lock()->Store(id, bytes);
}

auto BlockOracle::Shared::save_to_database(
    const block::Hash& id,
    const ReadView bytes) const noexcept -> ReadView
{
    return db_.BlockStore(id, bytes);
}

auto BlockOracle::Shared::SetTip(const block::Position& tip) noexcept -> bool
{
    return db_.SetBlockTip(tip);
}

auto BlockOracle::Shared::SubmitBlock(
    const blockchain::block::Block& in) const noexcept -> bool
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

        return receive(header.Hash(), serialized.Bytes());
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
