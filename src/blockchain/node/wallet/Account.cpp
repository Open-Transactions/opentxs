// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/node/wallet/Account.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <chrono>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
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
        static const auto map = Map<AccountJobs, CString>{
            {AccountJobs::shutdown, "shutdown"},
            {AccountJobs::reorg_begin, "reorg_begin"},
            {AccountJobs::reorg_begin_ack, "reorg_begin_ack"},
            {AccountJobs::reorg_end, "reorg_end"},
            {AccountJobs::reorg_end_ack, "reorg_end_ack"},
            {AccountJobs::init, "init"},
            {AccountJobs::key, "key"},
            {AccountJobs::statemachine, "statemachine"},
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
    const filter::Type filter,
    const std::string_view shutdown,
    const std::string_view fromParent,
    const std::string_view toParent,
    CString&& fromChildren,
    CString&& toChildren,
    allocator_type alloc) noexcept
    : Actor(
          api,
          0ms,
          batch,
          alloc,
          {
              {CString{shutdown, alloc}, Direction::Connect},
              {CString{fromParent, alloc}, Direction::Connect},
              {CString{
                   api.Crypto().Blockchain().Internal().KeyEndpoint(),
                   alloc},
               Direction::Connect},
          },
          {
              {fromChildren, Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {CString{toParent}, Direction::Connect},
               }},
              {SocketType::Publish,
               {
                   {toChildren, Direction::Bind},
               }},
          })
    , api_(api)
    , account_(account)
    , node_(node)
    , db_(db)
    , mempool_(mempool)
    , chain_(chain)
    , filter_type_(node_.FilterOracleInternal().DefaultType())
    , shutdown_endpoint_(shutdown, alloc)
    , to_children_endpoint_(std::move(toChildren))
    , from_children_endpoint_(std::move(fromChildren))
    , to_parent_(pipeline_.Internal().ExtraSocket(0))
    , to_children_(pipeline_.Internal().ExtraSocket(1))
    , state_(State::normal)
    , reorg_(std::nullopt)
    , internal_(alloc)
    , external_(alloc)
    , outgoing_(alloc)
    , incoming_(alloc)
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
    const filter::Type filter,
    const std::string_view shutdown,
    const std::string_view fromParent,
    const std::string_view toParent,
    allocator_type alloc) noexcept
    : Imp(api,
          account,
          node,
          db,
          mempool,
          batch,
          chain,
          filter,
          shutdown,
          fromParent,
          toParent,
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
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
    to_children_.Send(MakeWork(WorkType::Shutdown));
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

    if (map.end() != it) { return *(it->second); }

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
    const auto& asio = api_.Network().ZeroMQ().Internal();
    const auto batchID = asio.PreallocateBatch();
    auto [it, added] = map.try_emplace(
        subaccount.ID(),
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
            shutdown_endpoint_,
            to_children_endpoint_,
            from_children_endpoint_));
    auto& ptr = it->second;

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

    const auto subaccount = api_.Factory().Identifier(body.at(3));
    const auto type = body.at(5).as<crypto::SubaccountType>();

    switch (type) {
        case crypto::SubaccountType::HD: {
            check_hd(subaccount);
        } break;
        case crypto::SubaccountType::PaymentCode: {
            check_pc(subaccount);
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
    for (auto& [id, account] : internal_) {
        account->ProcessReorg(headerOracleLock, tx, errors, parent);
    }

    for (auto& [id, account] : external_) {
        account->ProcessReorg(headerOracleLock, tx, errors, parent);
    }

    for (auto& [id, account] : outgoing_) {
        account->ProcessReorg(headerOracleLock, tx, errors, parent);
    }

    for (auto& [id, account] : incoming_) {
        account->ProcessReorg(headerOracleLock, tx, errors, parent);
    }
}

auto Account::Imp::ready_for_normal() noexcept -> void
{
    disable_automatic_processing_ = false;
    reorg_ = std::nullopt;
    state_ = State::normal;
    to_parent_.Send(MakeWork(AccountsJobs::reorg_end_ack));
}

auto Account::Imp::ready_for_reorg() noexcept -> void
{
    state_ = State::reorg;
    to_parent_.Send(MakeWork(AccountsJobs::reorg_begin_ack));
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

auto Account::Imp::startup() noexcept -> void
{
    api_.Wallet().Internal().PublishNym(account_.NymID());
    scan_subchains();
}

auto Account::Imp::state_normal(const Work work, Message&& msg) noexcept -> void
{
    OT_ASSERT(false == reorg_.has_value());

    switch (work) {
        case Work::reorg_begin: {
            transition_state_pre_reorg(std::move(msg));
        } break;
        case Work::reorg_begin_ack:
        case Work::reorg_end:
        case Work::reorg_end_ack: {
            LogError()(OT_PRETTY_CLASS())(": wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        case Work::key: {
            process_key(std::move(msg));
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::state_post_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(reorg_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::key:
        case Work::statemachine: {
            // NOTE defer processing of non-reorg messages until after reorg is
            // complete
            pipeline_.Push(std::move(msg));
        } break;
        case Work::reorg_end_ack: {
            transition_state_normal(std::move(msg));
        } break;
        case Work::reorg_begin:
        case Work::reorg_begin_ack:
        case Work::reorg_end: {
            LogError()(OT_PRETTY_CLASS())(": wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::state_pre_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(reorg_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::key:
        case Work::statemachine: {
            // NOTE defer processing of non-reorg messages until after reorg is
            // complete
            pipeline_.Push(std::move(msg));
        } break;
        case Work::reorg_begin_ack: {
            transition_state_reorg(std::move(msg));
        } break;
        case Work::reorg_begin:
        case Work::reorg_end:
        case Work::reorg_end_ack: {
            LogError()(OT_PRETTY_CLASS())(": wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    OT_ASSERT(reorg_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::key:
        case Work::statemachine: {
            // NOTE defer processing of non-reorg messages until after reorg is
            // complete
            pipeline_.Push(std::move(msg));
        } break;
        case Work::reorg_end: {
            transition_state_post_reorg(std::move(msg));
        } break;
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack: {
            LogError()(OT_PRETTY_CLASS())(": wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::transition_state_normal(Message&& in) noexcept -> void
{
    OT_ASSERT(0u < reorg_children());

    auto& reorg = reorg_.value();
    const auto& target = reorg.target_;
    auto& counter = reorg.done_;
    ++counter;

    if (counter < target) {

        return;
    } else if (counter == target) {
        ready_for_normal();
    } else {

        OT_FAIL;
    }
}

auto Account::Imp::transition_state_post_reorg(Message&& in) noexcept -> void
{
    const auto& reorg = reorg_.value();

    if (0u < reorg.target_) {
        to_children_.Send(MakeWork(SubchainJobs::reorg_end));
        state_ = State::post_reorg;
    } else {
        ready_for_normal();
    }
}

auto Account::Imp::transition_state_pre_reorg(Message&& in) noexcept -> void
{
    disable_automatic_processing_ = true;
    reorg_.emplace(reorg_children());
    const auto& reorg = reorg_.value();

    if (0u < reorg.target_) {
        to_children_.Send(MakeWork(SubchainJobs::reorg_begin));
        state_ = State::pre_reorg;
    } else {
        ready_for_reorg();
    }
}

auto Account::Imp::transition_state_reorg(Message&& in) noexcept -> void
{
    OT_ASSERT(0u < reorg_children());

    auto& reorg = reorg_.value();
    const auto& target = reorg.target_;
    auto& counter = reorg.ready_;
    ++counter;

    if (counter < target) {

        return;
    } else if (counter == target) {
        ready_for_reorg();
    } else {

        OT_FAIL;
    }
}

auto Account::Imp::work() noexcept -> bool
{
    to_children_.Send(MakeWork(OT_ZMQ_STATE_MACHINE_SIGNAL));

    return false;
}

Account::Imp::~Imp() { signal_shutdown(); }
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
    const filter::Type filter,
    const std::string_view shutdown,
    const std::string_view fromParent,
    const std::string_view toParent) noexcept
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
            shutdown,
            fromParent,
            toParent);
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

Account::~Account() { imp_->Shutdown(); }
}  // namespace opentxs::blockchain::node::wallet
