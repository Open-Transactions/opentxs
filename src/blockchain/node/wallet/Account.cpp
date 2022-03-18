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
#include <memory>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/subchain/NotificationStateData.hpp"
#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/subchain/Subchain.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
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
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/HDPath.pb.h"

namespace opentxs::blockchain::node::wallet
{
auto print(AccountJobs job) noexcept -> std::string_view
{
    try {
        using Job = AccountJobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::subaccount, "subaccount"},
            {Job::prepare_shutdown, "prepare_shutdown"},
            {Job::init, "init"},
            {Job::key, "key"},
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
    node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const network::zeromq::BatchID batch,
    const Type chain,
    const cfilter::Type filter,
    CString&& shutdown,
    allocator_type alloc) noexcept
    : Actor(
          api,
          LogTrace(),
          0ms,
          batch,
          alloc,
          {
              {shutdown, Direction::Connect},
              {CString{
                   api.Crypto().Blockchain().Internal().KeyEndpoint(),
                   alloc},
               Direction::Connect},
              {CString{api.Endpoints().BlockchainAccountCreated(), alloc},
               Direction::Connect},
          })
    , api_(api)
    , account_(account)
    , node_(node)
    , db_(db)
    , mempool_(mempool)
    , chain_(chain)
    , name_([&] {
        using namespace std::literals;

        return CString{alloc}
            .append(print(chain_))
            .append(" account for "sv)
            .append(account_.NymID().str());
    }())
    , filter_type_(node_.FilterOracleInternal().DefaultType())
    , shutdown_endpoint_(std::move(shutdown))
    , state_(State::normal)
    , notification_(alloc)
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
    node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const network::zeromq::BatchID batch,
    const Type chain,
    const cfilter::Type filter,
    const std::string_view shutdown,
    allocator_type alloc) noexcept
    : Imp(api,
          account,
          node,
          db,
          mempool,
          batch,
          chain,
          filter,
          CString{shutdown, alloc},
          alloc)
{
}

auto Account::Imp::ChangeState(const State state) noexcept -> bool
{
    auto lock = lock_for_reorg(reorg_lock_);

    switch (state) {
        case State::normal: {
            OT_ASSERT(State::reorg == state_);

            transition_state_normal();
        } break;
        case State::reorg: {
            OT_ASSERT(State::normal == state_);

            transition_state_reorg();
        } break;
        case State::shutdown: {
            OT_ASSERT(State::reorg != state_);

            if (State::shutdown != state_) { transition_state_shutdown(); }
        } break;
        default: {
            OT_FAIL;
        }
    }

    return true;
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
    notification_.clear();
    internal_.clear();
    external_.clear();
    outgoing_.clear();
    incoming_.clear();
}

auto Account::Imp::do_startup() noexcept -> void
{
    api_.Wallet().Internal().PublishNym(account_.NymID());
    scan_subchains();
    index_nym(account_.NymID());
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

auto Account::Imp::index_nym(const identifier::Nym& id) noexcept -> void
{
    const auto pNym = api_.Wallet().Nym(id);

    OT_ASSERT(pNym);

    const auto& nym = *pNym;
    auto code = api_.Factory().PaymentCode(nym.PaymentCode());

    if (3 > code.Version()) { return; }

    log_("Initializing payment code ")(code.asBase58())(" on ")(name_).Flush();
    auto accountID = NotificationStateData::calculate_id(api_, chain_, code);
    const auto& asio = api_.Network().ZeroMQ().Internal();
    const auto batchID = asio.PreallocateBatch();
    auto& map = notification_;

    if (auto i = map.find(accountID); map.end() != i) { return; }

    auto [it, added] = map.try_emplace(
        std::move(accountID),
        boost::allocate_shared<NotificationStateData>(
            alloc::PMR<NotificationStateData>{asio.Alloc(batchID)},
            api_,
            node_,
            db_,
            mempool_,
            id,
            filter_type_,
            batchID,
            chain_,
            shutdown_endpoint_,
            std::move(code),
            [&] {
                auto out = proto::HDPath{};
                nym.PaymentCodePath(out);

                return out;
            }()));
    auto& ptr = it->second;

    OT_ASSERT(ptr);

    auto& subchain = *ptr;

    if (added) { subchain.Init(ptr); }
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
    log_("Instantiating ")(name_)(" subaccount ")(subaccount.ID())(" ")(
        opentxs::print(subchain))(" subchain for ")(subaccount.Parent().NymID())
        .Flush();
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
            shutdown_endpoint_));
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
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        case State::shutdown: {
            shutdown_actor();
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
    for_each([&](auto& item) {
        item.second->ProcessReorg(headerOracleLock, tx, errors, parent);
    });
}

auto Account::Imp::scan_subchains() noexcept -> void
{
    for (const auto& subaccount : account_.GetHD()) { check_hd(subaccount); }

    for (const auto& subaccount : account_.GetPaymentCode()) {
        check_pc(subaccount);
    }
}

auto Account::Imp::state_normal(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::shutdown: {
            shutdown_actor();
        } break;
        case Work::subaccount: {
            process_subaccount(std::move(msg));
        } break;
        case Work::prepare_shutdown: {
            transition_state_shutdown();
        } break;
        case Work::init: {
            do_init();
        } break;
        case Work::key: {
            process_key(std::move(msg));
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::subaccount:
        case Work::key:
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::prepare_shutdown:
        case Work::shutdown:
        case Work::init: {
            LogError()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Flush();

            OT_FAIL;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Account::Imp::transition_state_normal() noexcept -> void
{
    disable_automatic_processing_ = false;
    const auto cb = [](auto& value) {
        auto rc = value.second->ChangeState(Subchain::State::normal);

        OT_ASSERT(rc);
    };
    for_each(cb);
    state_ = State::normal;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to normal state ").Flush();
    trigger();
}

auto Account::Imp::transition_state_reorg() noexcept -> void
{
    disable_automatic_processing_ = true;
    const auto cb = [](auto& value) {
        auto rc = value.second->ChangeState(Subchain::State::reorg);

        OT_ASSERT(rc);
    };
    for_each(cb);
    state_ = State::reorg;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to reorg state ").Flush();
}

auto Account::Imp::transition_state_shutdown() noexcept -> void
{
    const auto cb = [](auto& value) {
        auto rc = value.second->ChangeState(Subchain::State::shutdown);

        OT_ASSERT(rc);
    };
    for_each(cb);
    state_ = State::shutdown;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to shutdown state ").Flush();
    signal_shutdown();
}

auto Account::Imp::VerifyState(const State state) const noexcept -> void
{
    OT_ASSERT(state == state_);
}

auto Account::Imp::work() noexcept -> bool { return false; }
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Account::Account(
    const api::Session& api,
    const crypto::Account& account,
    const node::internal::Network& node,
    node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const Type chain,
    const cfilter::Type filter,
    const std::string_view shutdown) noexcept
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
            shutdown);
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

auto Account::ChangeState(const State state) noexcept -> bool
{
    return imp_->ChangeState(state);
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
