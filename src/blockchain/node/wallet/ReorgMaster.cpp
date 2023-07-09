// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/ReorgMaster.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/wallet/ReorgMaster.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/ReorgSlave.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/blockchain/node/wallet/ReorgSlave.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
using namespace std::literals;

ReorgMasterPrivate::ReorgMasterPrivate(
    const network::zeromq::Pipeline& parent,
    allocator_type alloc) noexcept
    : log_(LogInsane())
    , alloc_(std::move(alloc))
    , data_(parent, alloc_)
{
}

auto ReorgMasterPrivate::AcknowledgePrepareReorg(
    SlaveID id,
    Reorg::Job&& job) noexcept -> void
{
    try {
        if (false == job.operator bool()) {
            const auto error =
                CString{"invalid job received from ", get_allocator()}.append(
                    std::to_string(id));

            throw std::runtime_error{error.c_str()};
        }

        auto handle = data_.lock();
        auto& data = *handle;
        auto& map = data.actions_;
        auto [it, added] = map.try_emplace(id, std::move(job));

        if (false == added) {
            const auto error =
                CString{std::to_string(id), get_allocator()}.append(
                    " already provided a reorg job");

            throw std::runtime_error{error.c_str()};
        }

        acknowledge(
            data,
            true,
            Reorg::State::pre_reorg,
            AccountsJobs::reorg_ready,
            "prepare reorg"sv,
            id);
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(e.what()).Abort();
    }
}

auto ReorgMasterPrivate::AcknowledgeShutdown(SlaveID id) noexcept -> void
{
    acknowledge(
        false,
        Reorg::State::shutdown,
        AccountsJobs::shutdown_ready,
        "prepare shutdown"sv,
        id);
}

auto ReorgMasterPrivate::acknowledge(
    bool reorg,
    Reorg::State expected,
    AccountsJobs work,
    std::string_view action,
    SlaveID id) noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    acknowledge(
        data,
        reorg,
        std::move(expected),
        std::move(work),
        std::move(action),
        std::move(id));
}

auto ReorgMasterPrivate::acknowledge(
    Data& data,
    bool reorg,
    Reorg::State expected,
    AccountsJobs work,
    std::string_view action,
    SlaveID id) noexcept -> void
{
    try {
        if (expected != data.state_) {
            throw std::runtime_error{"invalid state"};
        }

        const auto& slaves = reorg ? data.reorg_slaves_ : data.shutdown_slaves_;

        if (false == slaves.contains(id)) {
            const auto error = CString{"invalid id: ", get_allocator()}.append(
                std::to_string(id));

            throw std::runtime_error{error.c_str()};
        }

        auto& acks = reorg ? data.reorg_acks_ : data.shutdown_acks_;
        const auto [i, added] = acks.emplace(id);

        if (false == added) {
            const auto error = CString{"slave ", get_allocator()}
                                   .append(std::to_string(id))
                                   .append(" already acknowledged ")
                                   .append(action);

            throw std::runtime_error{error.c_str()};
        }

        log_(OT_PRETTY_CLASS())(acks.size())(" of ")(slaves.size())(
            " have acknowledged ")(action)
            .Flush();

        check_condition(data, reorg, work, action);
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(e.what()).Abort();
    }
}

auto ReorgMasterPrivate::check_condition(
    const Data& data,
    bool reorg,
    AccountsJobs work,
    std::string_view action) noexcept -> bool
{
    const auto& slaves = reorg ? data.reorg_slaves_ : data.shutdown_slaves_;
    const auto& acks = reorg ? data.reorg_acks_ : data.shutdown_acks_;
    const auto count = slaves.size();
    const auto required = acks.size();

    if ((0_uz == count) || (required == count)) {
        log_(OT_PRETTY_CLASS())("finished ")(action).Flush();
        data.parent_.Push(MakeWork(work));

        return true;
    }

    return false;
}

auto ReorgMasterPrivate::check_prepare_reorg(const Data& data) noexcept -> bool
{
    return check_condition(
        data, true, AccountsJobs::reorg_ready, "prepare reorg"sv);
}

auto ReorgMasterPrivate::check_shutdown(const Data& data) noexcept -> bool
{
    return check_condition(
        data, false, AccountsJobs::shutdown_ready, "prepare shutdown"sv);
}

auto ReorgMasterPrivate::CheckShutdown() noexcept -> bool
{
    const auto handle = data_.lock();
    const auto& data = *handle;

    if (check_shutdown(data)) { return true; }

    log_(OT_PRETTY_CLASS())(": waiting for shutdown acknowledgement from:\n");

    for (const auto& [id, slave] : data.shutdown_slaves_) {
        log_("  * ID: ")(id)(": ")(slave->name_)("\n");
        slave->BroadcastShutdown();
    }

    log_.Flush();

    return false;
}

auto ReorgMasterPrivate::ClearReorg() noexcept -> void
{
    try {
        auto handle = data_.lock();
        auto& data = *handle;

        if (Reorg::State::reorg != data.state_) {
            throw std::runtime_error{"invalid state"};
        }

        if (false == data.params_.has_value()) {
            throw std::runtime_error{"reorg missing"};
        }

        data.params_.reset();
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(e.what()).Abort();
    }
}

auto ReorgMasterPrivate::FinishReorg() noexcept -> void
{
    try {
        auto handle = data_.lock();
        auto& data = *handle;

        if (Reorg::State::reorg != data.state_) {
            throw std::runtime_error{"invalid state"};
        }

        if (data.params_.has_value()) {
            throw std::runtime_error{"reorg data not cleared"};
        }

        if (false == data.reorg_acks_.empty()) {
            throw std::runtime_error{"acks not cleared"};
        }

        if (false == data.actions_.empty()) {
            throw std::runtime_error{"actions not cleared"};
        }

        data.state_ = Reorg::State::normal;

        log_(OT_PRETTY_CLASS())("instructing ")(data.reorg_slaves_.size())(
            " slaves to resume normal operation")
            .Flush();

        for (auto& [id, slave] : data.reorg_slaves_) {
            slave->BroadcastFinishReorg();
        }
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(e.what()).Abort();
    }
}

auto ReorgMasterPrivate::GetReorg(
    const block::Position& position,
    storage::lmdb::Transaction&& tx) noexcept -> Reorg::Params&
{
    try {
        auto handle = data_.lock();
        auto& data = *handle;

        if (Reorg::State::pre_reorg != data.state_) {
            throw std::runtime_error{"invalid state"};
        }

        if (data.params_.has_value()) {
            throw std::runtime_error{"reorg already exists"};
        }

        return data.params_.emplace(position, std::move(tx));
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(e.what()).Abort();
    }
}

auto ReorgMasterPrivate::GetSlave(
    const network::zeromq::Pipeline& parent,
    std::string_view name,
    alloc::Strategy alloc) noexcept -> ReorgSlave
{
    auto handle = data_.lock();
    auto& data = *handle;
    const auto id = ++data.counter_;
    const auto [it, added] = data.shutdown_slaves_.try_emplace(
        id,
        boost::allocate_shared<ReorgSlavePrivate>(
            alloc::PMR<ReorgMasterPrivate>{alloc.result_},
            parent,
            boost::shared_from(this),
            id,
            std::move(name)));

    OT_ASSERT(added);
    OT_ASSERT(it->second);

    return it->second;
}

auto ReorgMasterPrivate::get_allocator() const noexcept -> allocator_type
{
    return alloc_;
}

auto ReorgMasterPrivate::PerformReorg(const node::HeaderOracle& oracle) noexcept
    -> bool
{
    auto jobs = [&] {
        auto handle = data_.lock();
        auto& data = *handle;
        auto out = Vector<ReorgTask>{data.actions_.get_allocator()};

        if (Reorg::State::pre_reorg != data.state_) {
            throw std::runtime_error{"invalid state"};
        }

        if (false == data.params_.has_value()) {
            throw std::runtime_error{"missing params"};
        }

        if (data.actions_.size() != data.reorg_slaves_.size()) {
            throw std::runtime_error{"not all slaves acknowledged reorg"};
        }

        auto& params = data.params_.value();
        data.state_ = Reorg::State::reorg;
        data.reorg_acks_.clear();
        out.reserve(data.actions_.size());

        for (auto& [id, action] : data.actions_) {
            out.emplace_back([&params, job = std::move(action)](
                                 const auto& headeroracle, const auto& lock) {
                return std::invoke(job, headeroracle, lock, params);
            });
        }

        data.actions_.clear();

        return out;
    }();

    return oracle.Internal().Execute(std::move(jobs));
}

auto ReorgMasterPrivate::PrepareReorg(StateSequence id) noexcept -> bool
{
    try {
        auto handle = data_.lock();
        auto& data = *handle;

        if (Reorg::State::normal != data.state_) {

            throw std::runtime_error{"invalid state"};
        }

        data.state_ = Reorg::State::pre_reorg;
        data.reorg_acks_.clear();

        if (false == data.actions_.empty()) {
            throw std::runtime_error{"reorg actions already populated"};
        }

        if (check_prepare_reorg(data)) { return true; }

        log_(OT_PRETTY_CLASS())("preparing ")(data.reorg_slaves_.size())(
            " slaves for reorg ")(id)
            .Flush();

        for (auto& [_, slave] : data.reorg_slaves_) {
            slave->BroadcastPrepareReorg(id);
        }

        return true;
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(e.what()).Abort();
    }
}

auto ReorgMasterPrivate::PrepareShutdown() noexcept -> bool
{
    try {
        auto handle = data_.lock();
        auto& data = *handle;

        if (Reorg::State::normal != data.state_) {

            throw std::runtime_error{"invalid state"};
        }

        data.state_ = Reorg::State::shutdown;
        data.shutdown_acks_.clear();

        if (check_shutdown(data)) { return true; }

        const auto count = data.shutdown_slaves_.size();
        log_(OT_PRETTY_CLASS())("preparing ")(count)(" slaves for shutdown")
            .Flush();

        for (auto& [id, slave] : data.shutdown_slaves_) {
            slave->BroadcastPrepareShutdown();
        }

        return false;
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(e.what()).Abort();
    }
}

auto ReorgMasterPrivate::Register(
    boost::shared_ptr<ReorgSlavePrivate> slave) noexcept -> Reorg::State
{
    OT_ASSERT(slave);

    const auto id = slave->id_;
    auto handle = data_.lock();
    auto& data = *handle;
    auto [i, rc] = data.reorg_slaves_.try_emplace(id, std::move(slave));

    OT_ASSERT(rc);

    return data.state_;
}

auto ReorgMasterPrivate::Stop() noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;

    for (auto& [id, slave] : data.shutdown_slaves_) {
        slave->BroadcastShutdown();
    }
}

auto ReorgMasterPrivate::Unregister(SlaveID id) noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    data.shutdown_slaves_.erase(id);
    data.reorg_slaves_.erase(id);
    data.actions_.erase(id);
    data.shutdown_acks_.erase(id);
    data.reorg_acks_.erase(id);
}

ReorgMasterPrivate::~ReorgMasterPrivate() = default;
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
ReorgMaster::ReorgMaster(
    const network::zeromq::Pipeline& parent,
    allocator_type alloc) noexcept
    : imp_(boost::allocate_shared<ReorgMasterPrivate>(
          alloc::PMR<ReorgMasterPrivate>{alloc},
          parent))
{
}

auto ReorgMaster::CheckShutdown() noexcept -> bool
{
    return imp_->CheckShutdown();
}

auto ReorgMaster::ClearReorg() noexcept -> void { imp_->ClearReorg(); }

auto ReorgMaster::FinishReorg() noexcept -> void { imp_->FinishReorg(); }

auto ReorgMaster::GetReorg(
    const block::Position& position,
    storage::lmdb::Transaction&& tx) noexcept -> Params&
{
    return imp_->GetReorg(position, std::move(tx));
}

auto ReorgMaster::GetSlave(
    const network::zeromq::Pipeline& parent,
    std::string_view name,
    alloc::Strategy alloc) noexcept -> ReorgSlave
{
    return imp_->GetSlave(parent, std::move(name), std::move(alloc));
}

auto ReorgMaster::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto ReorgMaster::PerformReorg(const node::HeaderOracle& oracle) noexcept
    -> bool
{
    return imp_->PerformReorg(oracle);
}

auto ReorgMaster::PrepareReorg(StateSequence id) noexcept -> bool
{
    return imp_->PrepareReorg(id);
}

auto ReorgMaster::PrepareShutdown() noexcept -> bool
{
    return imp_->PrepareShutdown();
}

auto ReorgMaster::Stop() noexcept -> void { imp_->Stop(); }

ReorgMaster::~ReorgMaster() = default;
}  // namespace opentxs::blockchain::node::wallet
