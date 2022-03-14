// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/node/wallet/Account.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(AccountJobs job) noexcept -> std::string_view
{
    try {
        using Job = AccountJobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::subaccount, "subaccount"},
            {Job::reorg_begin, "reorg_begin"},
            {Job::reorg_begin_ack, "reorg_begin_ack"},
            {Job::reorg_end, "reorg_end"},
            {Job::reorg_end_ack, "reorg_end_ack"},
            {Job::init, "init"},
            {Job::key, "key"},
            {Job::shutdown_begin, "shutdown_begin"},
            {Job::shutdown_ready, "shutdown_ready"},
            {Job::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid AccountJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Account::Imp::Imp(
    const api::Session& api,
    const crypto::Account& account,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const network::zeromq::BatchID batch,
    const Type chain,
    const cfilter::Type filter,
    const std::string_view parent,
    CString&& fromChildren,
    allocator_type alloc) noexcept
    : Actor(
          api,
          LogTrace(),
          0ms,
          batch,
          alloc,
          {
              {CString{
                   api.Crypto().Blockchain().Internal().KeyEndpoint(),
                   alloc},
               Direction::Connect},
              {CString{api.Endpoints().BlockchainAccountCreated(), alloc},
               Direction::Connect},
          },
          {
              {fromChildren, Direction::Bind},
          },
          {},
          {
              {SocketType::Pair,
               {
                   {CString{parent, alloc}, Direction::Connect},
               }},
          })
    , api_(api)
    , account_(account)
    , node_(node)
    , db_(db)
    , mempool_(mempool)
    , chain_(chain)
    , filter_type_(node_.FilterOracleInternal().DefaultType())
    , from_children_endpoint_(std::move(fromChildren))
    , to_parent_(pipeline_.Internal().ExtraSocket(0))
    , state_(State::normal)
    , reorg_(std::nullopt)
    , shutdown_(std::nullopt)
    , internal_(alloc)
    , external_(alloc)
    , outgoing_(alloc)
    , incoming_(alloc)
    , reorg_timer_(api_.Network().Asio().Internal().GetTimer())
{
}

Account::Imp::Imp(
    const api::Session& api,
    const crypto::Account& account,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const network::zeromq::BatchID batch,
    const Type chain,
    const cfilter::Type filter,
    const std::string_view parent,
    allocator_type alloc) noexcept
    : Imp(api,
          account,
          node,
          db,
          mempool,
          batch,
          chain,
          filter,
          parent,
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          alloc)
{
}

auto Account::Imp::check_hd(const Identifier& id) noexcept -> void
{
    check_hd(account_.GetHD().at(id));
}

auto Account::Imp::check_hd(const crypto::HD& subaccount) noexcept -> void
{
    get(subaccount, Subtype::Internal, internal_);
    get(subaccount, Subtype::External, external_);
}

auto Account::Imp::check_pc(const Identifier& id) noexcept -> void
{
    check_pc(account_.GetPaymentCode().at(id));
}

auto Account::Imp::check_pc(const crypto::PaymentCode& subaccount) noexcept
    -> void
{
    get(subaccount, Subtype::Outgoing, outgoing_);
    get(subaccount, Subtype::Incoming, incoming_);
}

auto Account::Imp::do_shutdown() noexcept -> void
{
    send_to_children(MakeWork(WorkType::Shutdown));
    internal_.clear();
    external_.clear();
    outgoing_.clear();
    incoming_.clear();
}

auto Account::Imp::get(
    const crypto::Deterministic& subaccount,
    const Subtype subchain,
    Subchains& map) noexcept -> Subchain&
{
    auto it = map.find(subaccount.ID());

    if (map.end() != it) { return *(it->second.second); }

    return instantiate(subaccount, subchain, map);
}

auto Account::Imp::Init(boost::shared_ptr<Imp> me) noexcept -> void
{
    signal_startup(me);
}

auto Account::Imp::instantiate(
    const crypto::Deterministic& subaccount,
    const Subtype subchain,
    Subchains& map) noexcept -> Subchain&
{
    LogTrace()("Instantiating ")(print(chain_))(" subaccount ")(
        subaccount.ID())(" ")(opentxs::print(subchain))(" subchain for ")(
        subaccount.Parent().NymID())
        .Flush();
    const auto& asio = api_.Network().ZeroMQ().Internal();
    const auto batchID = asio.PreallocateBatch();
    const auto endpoint =
        network::zeromq::MakeArbitraryInproc(get_allocator().resource());
    auto [it, added] = map.try_emplace(
        subaccount.ID(),
        std::make_pair(
            [&] {
                auto out = api_.Network().ZeroMQ().Internal().RawSocket(
                    SocketType::Pair);
                auto rc = out.Bind(endpoint.c_str());

                OT_ASSERT(rc);

                return out;
            }(),
            boost::allocate_shared<DeterministicStateData>(
                alloc::PMR<DeterministicStateData>{asio.Alloc(batchID)},
                api_,
                node_,
                db_,
                mempool_,
                subaccount,
                filter_type_,
                subchain,
                batchID,
                endpoint)));
    auto& ptr = it->second.second;

    OT_ASSERT(ptr);

    auto& output = *ptr;

    if (added) { output.Init(ptr); }

    return output;
}

auto Account::Imp::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::pre_reorg: {
            state_pre_reorg(work, std::move(msg));
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        case State::post_reorg: {
            state_post_reorg(work, std::move(msg));
        } break;
        case State::pre_shutdown: {
            state_pre_shutdown(work, std::move(msg));
        } break;
        case State::shutdown: {
            // NOTE do not process any messages
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Account::Imp::process_key(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(5u < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto owner = api_.Factory().NymID(body.at(2));

    if (owner != account_.NymID()) { return; }

    const auto id = api_.Factory().Identifier(body.at(3));
    const auto type = body.at(5).as<crypto::SubaccountType>();
    process_subaccount(id, type);
}

auto Account::Imp::process_subaccount(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto owner = api_.Factory().NymID(body.at(2));

    if (owner != account_.NymID()) { return; }

    const auto type = body.at(3).as<crypto::SubaccountType>();
    const auto id = api_.Factory().Identifier(body.at(4));
    process_subaccount(id, type);
}

auto Account::Imp::process_subaccount(
    const Identifier& id,
    const crypto::SubaccountType type) noexcept -> void
{
    switch (type) {
        case crypto::SubaccountType::HD: {
            check_hd(id);
        } break;
        case crypto::SubaccountType::PaymentCode: {
            check_pc(id);
        } break;
        default: {

            OT_FAIL;
        }
    }
}

auto Account::Imp::ProcessReorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& parent) noexcept -> void
{
    for (auto& [id, data] : internal_) {
        auto& [socket, account] = data;
        account->ProcessReorg(headerOracleLock, tx, errors, parent);
    }

    for (auto& [id, data] : external_) {
        auto& [socket, account] = data;
        account->ProcessReorg(headerOracleLock, tx, errors, parent);
    }

    for (auto& [id, data] : outgoing_) {
        auto& [socket, account] = data;
        account->ProcessReorg(headerOracleLock, tx, errors, parent);
    }

    for (auto& [id, data] : incoming_) {
        auto& [socket, account] = data;
        account->ProcessReorg(headerOracleLock, tx, errors, parent);
    }
}

auto Account::Imp::ready_for_normal() noexcept -> void
{
    disable_automatic_processing_ = false;
    reorg_ = std::nullopt;
    state_ = State::normal;
    log_(OT_PRETTY_CLASS())("transitioned to normal state").Flush();
    to_parent_.SendDeferred(MakeWork(AccountsJobs::reorg_end_ack));
    flush_cache();
}

auto Account::Imp::ready_for_reorg() noexcept -> void
{
    state_ = State::reorg;
    log_(OT_PRETTY_CLASS())("transitioned to reorg state").Flush();
    to_parent_.SendDeferred(MakeWork(AccountsJobs::reorg_begin_ack));
}

auto Account::Imp::ready_for_shutdown() noexcept -> void
{
    state_ = State::shutdown;
    log_(OT_PRETTY_CLASS())("transitioned to shutdown state").Flush();
    to_parent_.SendDeferred(MakeWork(AccountsJobs::shutdown_ready));
}

auto Account::Imp::reorg_children() const noexcept -> std::size_t
{
    return internal_.size() + external_.size() + outgoing_.size() +
           incoming_.size();
}

auto Account::Imp::scan_subchains() noexcept -> void
{
    for (const auto& subaccount : account_.GetHD()) { check_hd(subaccount); }

    for (const auto& subaccount : account_.GetPaymentCode()) {
        check_pc(subaccount);
    }
}

auto Account::Imp::send_to_children(const Message& msg) noexcept -> void
{
    for_each([&](auto& a) { a.second.first.SendDeferred(Message{msg}); });
}

auto Account::Imp::startup() noexcept -> void
{
    api_.Wallet().Internal().PublishNym(account_.NymID());
    scan_subchains();
}

auto Account::Imp::state_normal(const Work work, Message&& msg) noexcept -> void
{
    OT_ASSERT(false == reorg_.has_value());
    OT_ASSERT(false == shutdown_.has_value());

    switch (work) {
        case Work::subaccount: {
            process_subaccount(std::move(msg));
        } break;
        case Work::reorg_begin: {
            transition_state_pre_reorg(std::move(msg));
        } break;
        case Work::reorg_begin_ack:
        case Work::reorg_end:
        case Work::reorg_end_ack:
        case Work::shutdown_ready: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(" wrong state for ")(
                print(work))(" message")
                .Flush();

            OT_FAIL;
        }
        case Work::key: {
            process_key(std::move(msg));
        } break;
        case Work::shutdown_begin: {
            transition_state_pre_shutdown(std::move(msg));
        } break;
        case Work::init:
        case Work::shutdown:
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " unhandled message type ")(static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::state_post_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(reorg_.has_value());
    OT_ASSERT(false == shutdown_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::subaccount:
        case Work::key:
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::reorg_end_ack: {
            transition_state_normal(std::move(msg));
        } break;
        case Work::reorg_begin:
        case Work::reorg_begin_ack:
        case Work::reorg_end:
        case Work::shutdown_begin:
        case Work::shutdown_ready: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(" wrong state for ")(
                print(work))(" message")
                .Flush();

            OT_FAIL;
        }
        case Work::init:
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " unhandled message type ")(static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::state_pre_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(reorg_.has_value());
    OT_ASSERT(false == shutdown_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::subaccount:
        case Work::key:
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::reorg_begin_ack: {
            transition_state_reorg(std::move(msg));
        } break;
        case Work::reorg_begin:
        case Work::reorg_end:
        case Work::reorg_end_ack:
        case Work::shutdown_begin:
        case Work::shutdown_ready: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(" wrong state for ")(
                print(work))(" message")
                .Flush();

            OT_FAIL;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " unhandled message type ")(static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::state_pre_shutdown(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(false == reorg_.has_value());
    OT_ASSERT(shutdown_.has_value());

    switch (work) {
        case Work::subaccount:
        case Work::key: {
            // NOTE drop non-shutdown messages
        } break;
        case Work::reorg_begin:
        case Work::reorg_begin_ack:
        case Work::reorg_end:
        case Work::reorg_end_ack:
        case Work::shutdown_begin: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(" wrong state for ")(
                print(work))(" message")
                .Flush();

            OT_FAIL;
        }
        case Work::shutdown_ready: {
            transition_state_shutdown(std::move(msg));
        } break;
        case Work::init:
        case Work::shutdown:
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " unhandled message type ")(static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    OT_ASSERT(reorg_.has_value());
    OT_ASSERT(false == shutdown_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::key:
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::reorg_end: {
            transition_state_post_reorg(std::move(msg));
        } break;
        case Work::reorg_begin:
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        case Work::shutdown_begin:
        case Work::shutdown_ready: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(" wrong state for ")(
                print(work))(" message")
                .Flush();

            OT_FAIL;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " unhandled message type ")(static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::transition_state_normal(Message&& in) noexcept -> void
{
    OT_ASSERT(0u < reorg_->target_);

    auto& reorg = reorg_.value();
    const auto& target = reorg.target_;
    auto& counter = reorg.done_;
    ++counter;

    if (counter < target) {
        log_(OT_PRETTY_CLASS())(print(chain_))(" ")(counter)(" of ")(
            target)(" children finished with reorg")
            .Flush();

        return;
    } else if (counter == target) {
        verify_child_state(Subchain::State::normal);
        log_(OT_PRETTY_CLASS())(print(chain_))(" all ")(
            target)(" children finished with reorg")
            .Flush();
        ready_for_normal();
    } else {

        OT_FAIL;
    }
}

auto Account::Imp::transition_state_post_reorg(Message&& in) noexcept -> void
{
    const auto& reorg = reorg_.value();

    if (0u < reorg.target_) {
        log_(OT_PRETTY_CLASS())(print(chain_))(
            " waiting for acknowledgements from ")(reorg_->target_)(
            " children prior to acknowledging reorg completion")
            .Flush();
        send_to_children(MakeWork(SubchainJobs::reorg_end));
        state_ = State::post_reorg;
    } else {
        log_(OT_PRETTY_CLASS())(print(chain_))(
            " no children instantiated therefore reorg may be completed "
            "immediately")
            .Flush();
        ready_for_normal();
    }
}

auto Account::Imp::transition_state_pre_reorg(Message&& in) noexcept -> void
{
    disable_automatic_processing_ = true;
    reorg_.emplace(reorg_children());
    const auto& reorg = reorg_.value();

    if (0u < reorg.target_) {
        log_(OT_PRETTY_CLASS())(print(chain_))(
            " waiting for acknowledgements from ")(reorg_->target_)(
            " children prior to acknowledging reorg")
            .Flush();
        send_to_children(MakeWork(SubchainJobs::reorg_begin));
        state_ = State::pre_reorg;
        reorg_timer_.SetRelative(9 * 60s);
        reorg_timer_.Wait([this](const auto& ec) {
            if (ec) {
                if (boost::system::errc::operation_canceled != ec.value()) {
                    LogError()(OT_PRETTY_CLASS())(ec).Flush();
                }
            } else {
                LogError()(OT_PRETTY_CLASS())(
                    "timeout while waiting for children to acknowledge reorg")
                    .Flush();

                OT_FAIL;
            }
        });
    } else {
        log_(OT_PRETTY_CLASS())(print(chain_))(
            " no children instantiated therefore reorg may be acknowledged "
            "immediately")
            .Flush();
        ready_for_reorg();
    }
}

auto Account::Imp::transition_state_pre_shutdown(Message&& in) noexcept -> void
{
    shutdown_.emplace(reorg_children());
    const auto& reorg = shutdown_.value();

    if (0u < reorg.target_) {
        log_(OT_PRETTY_CLASS())(print(chain_))(
            " waiting for acknowledgements from ")(reorg_->target_)(
            " children prior to acknowledging shutdown")
            .Flush();
        send_to_children(std::move(in));
        state_ = State::pre_shutdown;
    } else {
        log_(OT_PRETTY_CLASS())(print(chain_))(
            " no children instantiated therefore shutdown may be acknowledged "
            "immediately")
            .Flush();
        ready_for_shutdown();
    }
}

auto Account::Imp::transition_state_reorg(Message&& in) noexcept -> void
{
    OT_ASSERT(0u < reorg_->target_);

    auto& reorg = reorg_.value();
    const auto& target = reorg.target_;
    auto& counter = reorg.ready_;
    ++counter;

    if (counter < target) {
        log_(OT_PRETTY_CLASS())(print(chain_))(" ")(counter)(" of ")(
            target)(" children ready for reorg")
            .Flush();

        return;
    } else if (counter == target) {
        reorg_timer_.Cancel();
        verify_child_state(Subchain::State::reorg);
        log_(OT_PRETTY_CLASS())(print(chain_))(" all ")(
            target)(" children ready for reorg")
            .Flush();
        ready_for_reorg();
    } else {

        OT_FAIL;
    }
}

auto Account::Imp::transition_state_shutdown(Message&& in) noexcept -> void
{
    OT_ASSERT(0u < shutdown_->target_);

    auto& reorg = shutdown_.value();
    const auto& target = reorg.target_;
    auto& counter = reorg.ready_;
    ++counter;

    if (counter < target) {
        log_(OT_PRETTY_CLASS())(print(chain_))(" ")(counter)(" of ")(
            target)(" children ready for shutdown")
            .Flush();

        return;
    } else if (counter == target) {
        verify_child_state(Subchain::State::shutdown);
        log_(OT_PRETTY_CLASS())(print(chain_))(" all ")(
            target)(" children ready for shutdown")
            .Flush();
        ready_for_shutdown();
    } else {

        OT_FAIL;
    }
}

auto Account::Imp::VerifyState(const State state) const noexcept -> void
{
    OT_ASSERT(state == state_);
}

auto Account::Imp::verify_child_state(const Subchain::State s) noexcept -> void
{
    for_each([&](const auto& a) { a.second.second->VerifyState(s); });
}

auto Account::Imp::work() noexcept -> bool { OT_FAIL; }

Account::Imp::~Imp()
{
    reorg_timer_.Cancel();
    signal_shutdown();
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Account::Account(
    const api::Session& api,
    const crypto::Account& account,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const Type chain,
    const cfilter::Type filter,
    const std::string_view parent) noexcept
    : imp_([&] {
        const auto& asio = api.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)},
            api,
            account,
            node,
            db,
            mempool,
            batchID,
            chain,
            filter,
            parent);
    }())
{
    OT_ASSERT(imp_);

    imp_->Init(imp_);
}

Account::Account(Account&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
    OT_ASSERT(imp_);
}

auto Account::ProcessReorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& parent) noexcept -> void
{
    imp_->ProcessReorg(headerOracleLock, tx, errors, parent);
}

auto Account::VerifyState(const State state) const noexcept -> void
{
    imp_->VerifyState(state);
}

Account::~Account()
{
    if (imp_) { imp_->Shutdown(); }
}
}  // namespace opentxs::blockchain::node::wallet
