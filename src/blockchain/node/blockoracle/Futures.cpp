// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/blockoracle/Futures.hpp"  // IWYU pragma: associated

#include <future>
#include <utility>

#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/block/Parser.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/file/Reader.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::blockoracle
{
Futures::Futures(
    const api::Session& api,
    const std::string_view name,
    blockchain::Type chain,
    allocator_type alloc) noexcept
    : log_(LogTrace())
    , name_(name)
    , chain_(chain)
    , requests_(alloc)
    , pending_(alloc)
    , to_blockchain_api_([&] {
        using enum network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Push);
        const auto rc = out.Connect(
            api.Endpoints().Internal().BlockchainMessageRouter().data());

        OT_ASSERT(rc);

        return out;
    }())
{
}

auto Futures::get_allocator() const noexcept -> allocator_type
{
    return requests_.get_allocator();
}

auto Futures::Queue(const block::Hash& hash, BlockResult& out) noexcept -> void
{
    if (auto i = requests_.find(hash); requests_.end() != i) {
        log_(OT_PRETTY_CLASS())(name_)(": promise for block ")
            .asHex(hash)(" already exists")
            .Flush();
        out = i->second.second;
    } else {
        auto& [promise, future] = requests_[hash];
        future = promise.get_future();
        pending_.emplace(hash);
        log_(OT_PRETTY_CLASS())(name_)(": promise for block ")
            .asHex(hash)(" created")
            .Flush();
        out = future;
    }
}

auto Futures::Receive(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const block::Hash& hash,
    const BlockLocation& location,
    allocator_type monotonic) noexcept -> void
{
    auto files = Vector<storage::file::Reader>{monotonic};
    files.clear();

    if (auto i = requests_.find(hash); requests_.end() != i) {
        auto& [promise, future] = i->second;
        auto alloc = get_allocator();
        auto block = block::Block{alloc};
        const auto bytes = reader(location, files, monotonic);
        const auto rc =
            block::Parser::Construct(crypto, chain, hash, bytes, block, alloc);

        OT_ASSERT(rc && block.IsValid());

        promise.set_value(block);
        requests_.erase(i);
        log_(OT_PRETTY_CLASS())(name_)(": promise for block ")
            .asHex(hash)(" satisfied")
            .Flush();
        to_blockchain_api_.SendDeferred(
            [&] {
                auto work = network::zeromq::tagged_message(
                    WorkType::BlockchainBlockAvailable, true);
                work.AddFrame(chain_);
                work.AddFrame(hash);

                return work;
            }(),
            __FILE__,
            __LINE__);
    }
}

Futures::~Futures() = default;
}  // namespace opentxs::blockchain::node::blockoracle
