// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/stats/Shared.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <utility>

#include "blockchain/node/stats/Actor.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::stats
{
Shared::Shared() noexcept
    : endpoint_(network::zeromq::MakeArbitraryInproc({}))  // TODO allocator
    , data_()
{
}

auto Shared::BlockHeaderTip(Type chain) const noexcept -> block::Position
{
    const auto handle = data_.lock_shared();
    const auto& data = *handle;

    return get_position(data, data.header_tips_, chain);
}

auto Shared::BlockTip(Type chain) const noexcept -> block::Position
{
    const auto handle = data_.lock_shared();
    const auto& data = *handle;

    return get_position(data, data.block_tips_, chain);
}

auto Shared::CfilterTip(Type chain) const noexcept -> block::Position
{
    const auto handle = data_.lock_shared();
    const auto& data = *handle;

    return get_position(data, data.cfilter_tips_, chain);
}

auto Shared::get_position(
    const Data& data,
    const Data::PositionMap& map,
    Type chain) noexcept -> block::Position
{
    if (const auto i = map.find(chain); map.end() != i) {

        return i->second;
    } else {
        data.Trigger();

        return {};
    }
}

auto Shared::PeerCount(Type chain) const noexcept -> std::size_t
{
    const auto handle = data_.lock_shared();
    const auto& data = *handle;
    const auto& map = data.peer_count_;

    if (const auto i = map.find(chain); map.end() != i) {

        return i->second;
    } else {
        data.Trigger();

        return 0_uz;
    }
}

auto Shared::SetBlockHeaderTip(Type chain, block::Position tip) noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    set_position(data.header_tips_, chain, std::move(tip));
}

auto Shared::SetBlockTip(Type chain, block::Position tip) noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    set_position(data.block_tips_, chain, std::move(tip));
}

auto Shared::SetCfilterTip(Type chain, block::Position tip) noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    set_position(data.cfilter_tips_, chain, std::move(tip));
}

auto Shared::SetPeerCount(Type chain, std::size_t count) noexcept -> void
{
    data_.lock()->peer_count_[chain] = count;
}

auto Shared::SetSyncTip(Type chain, block::Position tip) noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    set_position(data.sync_tips_, chain, std::move(tip));
}

auto Shared::set_position(
    Data::PositionMap& map,
    Type chain,
    block::Position tip) noexcept -> void
{
    if (auto i = map.find(chain); map.end() != i) {
        i->second = std::move(tip);
    } else {
        map.try_emplace(chain, std::move(tip));
    }
}

auto Shared::Start(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<Shared> me) noexcept -> void
{
    OT_ASSERT(api);
    OT_ASSERT(me);

    data_.lock()->Init(*api, endpoint_);
    const auto& zmq = api->Network().ZeroMQ().Internal();
    const auto batchID = zmq.PreallocateBatch();
    auto* alloc = zmq.Alloc(batchID);
    // TODO the version of libc++ present in android ndk 23.0.7599858 has a
    // broken std::allocate_shared function so we're using boost::shared_ptr
    // instead of std::shared_ptr
    auto actor = boost::allocate_shared<Actor>(
        alloc::PMR<Actor>{alloc}, std::move(api), std::move(me), batchID);

    OT_ASSERT(actor);

    actor->Init(actor);
}

auto Shared::SyncTip(Type chain) const noexcept -> block::Position
{
    const auto handle = data_.lock_shared();
    const auto& data = *handle;

    return get_position(data, data.sync_tips_, chain);
}

Shared::~Shared() = default;
}  // namespace opentxs::blockchain::node::stats
