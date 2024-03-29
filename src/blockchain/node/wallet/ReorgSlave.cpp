// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/ReorgSlave.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/wallet/ReorgSlave.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/node/wallet/ReorgMaster.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::wallet
{
ReorgSlavePrivate::ReorgSlavePrivate(
    const network::zeromq::Pipeline& parent,
    std::shared_ptr<ReorgMasterPrivate> master,
    const int id,
    std::string_view name,
    allocator_type alloc) noexcept
    : log_(LogInsane())
    , name_(name, alloc)
    , id_(id)
    , parent_(parent)
    , master_(std::move(master))
    , alloc_(std::move(alloc))
{
    assert_false(nullptr == master_);

    log_()("instantiated ")(name_)(" as id ")(id_).Flush();
}

auto ReorgSlavePrivate::AcknowledgePrepareReorg(Reorg::Job&& job) noexcept
    -> void
{
    log_()(name_).Flush();

    assert_false(nullptr == master_);

    master_->AcknowledgePrepareReorg(id_, std::move(job));
}

auto ReorgSlavePrivate::AcknowledgeShutdown() noexcept -> void
{
    log_()(name_).Flush();

    assert_false(nullptr == master_);

    master_->AcknowledgeShutdown(id_);
}

auto ReorgSlavePrivate::BroadcastFinishReorg() noexcept -> void
{
    log_()(name_).Flush();
    parent_.Push(MakeWork(SubchainJobs::finish_reorg));
}

auto ReorgSlavePrivate::BroadcastPrepareReorg(StateSequence id) noexcept -> void
{
    log_()(id)(" to ")(name_)().Flush();
    parent_.Push([&] {
        auto out = MakeWork(SubchainJobs::prepare_reorg);
        out.AddFrame(id);

        return out;
    }());
}

auto ReorgSlavePrivate::BroadcastPrepareShutdown() noexcept -> void
{
    log_()(name_).Flush();
    parent_.Push(MakeWork(SubchainJobs::prepare_shutdown));
}

auto ReorgSlavePrivate::BroadcastShutdown() noexcept -> void
{
    log_()(name_).Flush();
    parent_.Push(MakeWork(SubchainJobs::shutdown));
}

auto ReorgSlavePrivate::GetSlave(
    const network::zeromq::Pipeline& parent,
    std::string_view name,
    allocator_type alloc) noexcept -> ReorgSlave
{
    assert_false(nullptr == master_);

    return master_->GetSlave(parent, std::move(name), std::move(alloc));
}

auto ReorgSlavePrivate::get_allocator() const noexcept -> allocator_type
{
    return alloc_;
}

auto ReorgSlavePrivate::Start() noexcept -> bool
{
    assert_false(nullptr == master_);

    const auto state = master_->Register(shared_from_this());
    log_()("registered ")(name_).Flush();

    if (Reorg::State::shutdown == state) {
        Stop();

        return true;
    } else {

        return false;
    }
}

auto ReorgSlavePrivate::Stop() noexcept -> void
{
    if (master_) {
        log_()(name_).Flush();
        master_->Unregister(id_);
        master_.reset();
    }
}

ReorgSlavePrivate::~ReorgSlavePrivate() { Stop(); }
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
ReorgSlave::ReorgSlave(std::shared_ptr<ReorgSlavePrivate> imp) noexcept
    : imp_(std::move(imp))
{
}

ReorgSlave::ReorgSlave(ReorgSlave&& rhs) noexcept
    : ReorgSlave(std::exchange(rhs.imp_, nullptr))
{
}

auto ReorgSlave::AcknowledgePrepareReorg(Reorg::Job&& job) noexcept -> void
{
    imp_->AcknowledgePrepareReorg(std::move(job));
}

auto ReorgSlave::AcknowledgeShutdown() noexcept -> void
{
    imp_->AcknowledgeShutdown();
}

auto ReorgSlave::GetSlave(
    const network::zeromq::Pipeline& parent,
    std::string_view name,
    allocator_type alloc) noexcept -> ReorgSlave
{
    return imp_->GetSlave(parent, std::move(name), std::move(alloc));
}

auto ReorgSlave::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto ReorgSlave::Start() noexcept -> bool { return imp_->Start(); }

auto ReorgSlave::Stop() noexcept -> void { imp_->Stop(); }

ReorgSlave::~ReorgSlave() = default;
}  // namespace opentxs::blockchain::node::wallet
