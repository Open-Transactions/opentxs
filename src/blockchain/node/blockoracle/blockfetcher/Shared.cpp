// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/node/blockoracle/blockfetcher/Shared.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/shared_ptr.hpp>
#include <functional>
#include <memory>
#include <utility>

#include "blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/database/Database.hpp"  // IWYU pragma: keep
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Allocator.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::blockoracle
{
BlockFetcher::Shared::Shared(
    const api::Session& api,
    const node::Manager& node,
    network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : batch_id_(std::move(batchID))
    , peer_target_(
          node.Internal().GetConfig().PeerTarget(node.Internal().Chain()))
    , data_(
          api,
          node.Internal().DB(),
          node.Internal().Chain(),
          std::move(alloc))
    , to_actor_([&] {
        using Type = network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto rc = out.Connect(
            node.Internal().Endpoints().block_fetcher_pull_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
{
}

auto BlockFetcher::Shared::GetJob(
    boost::shared_ptr<Shared> self,
    allocator_type pmr) const noexcept -> internal::BlockBatch
{
    OT_ASSERT(self);

    auto alloc = alloc::PMR<internal::BlockBatch::Imp>{pmr};
    auto* imp = alloc.allocate(1_uz);
    auto [job, hashes] = data_.lock()->GetBatch(peer_target_, alloc);
    // TODO c++20 lambda capture structured binding
    auto id{job};
    const auto trigger = [self] {
        self->to_actor_.lock()->SendDeferred(
            MakeWork(BlockFetcherJob::statemachine), __FILE__, __LINE__);
    };

    if (0 > job) {
        OT_ASSERT(0_uz == hashes.size());
        std::invoke(trigger);

        return {};
    } else {
        alloc.construct(
            imp,
            id,
            std::move(hashes),
            [self, id, trigger](const auto bytes) {
                self->data_.lock()->ReceiveBlock(id, bytes);
                std::invoke(trigger);
            },
            [self, id, trigger] {
                self->data_.lock()->FinishJob(id);
                std::invoke(trigger);
            });

        return imp;
    }
}

auto BlockFetcher::Shared::get_allocator() const noexcept -> allocator_type
{
    return data_.lock()->get_allocator();
}

BlockFetcher::Shared::~Shared() = default;
}  // namespace opentxs::blockchain::node::blockoracle
