// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "blockchain/node/wallet/Account.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <chrono>
#include <exception>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/subchain/NotificationStateData.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Notification.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(AccountJobs job) noexcept -> std::string_view
{
    try {
        using Job = AccountJobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::subaccount, "subaccount"},
            {Job::prepare_reorg, "prepare_reorg"},
            {Job::rescan, "rescan"},
            {Job::finish_reorg, "finish_reorg"},
            {Job::init, "init"},
            {Job::key, "key"},
            {Job::prepare_shutdown, "prepare_shutdown"},
            {Job::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogAbort()(__FUNCTION__)("invalid AccountJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Account::Imp::Imp(
    Reorg& reorg,
    const crypto::Account& account,
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    CString&& fromParent,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Actor(
          *api,
          LogTrace(),
          [&] {
              using namespace std::literals;

              return CString{alloc}
                  .append(print(node->Internal().Chain()))
                  .append(" account for "sv)
                  .append(account.NymID().asBase58(api->Crypto()));
          }(),
          0ms,
          batch,
          alloc,
          {
              {fromParent, Direction::Connect},
              {CString{
                   api->Crypto().Blockchain().Internal().KeyEndpoint(),
                   alloc},
               Direction::Connect},
              {CString{api->Endpoints().BlockchainAccountCreated(), alloc},
               Direction::Connect},
          })
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , api_(*api_p_)
    , account_(account)
    , node_(*node_p_)
    , db_(node_.Internal().DB())
    , mempool_(node_.Internal().Mempool())
    , chain_(node_.Internal().Chain())
    , filter_type_(node_.FilterOracle().DefaultType())
    , from_parent_(std::move(fromParent))
    , state_(State::normal)
    , reorgs_(alloc)
    , notification_(alloc)
    , internal_(alloc)
    , external_(alloc)
    , outgoing_(alloc)
    , incoming_(alloc)
    , reorg_(reorg.GetSlave(pipeline_, name_, alloc))
{
}

Account::Imp::Imp(
    Reorg& reorg,
    const crypto::Account& account,
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    std::string_view fromParent,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Imp(reorg,
          account,
          std::move(api),
          std::move(node),
          CString{fromParent, alloc},
          std::move(batch),
          alloc)
{
}

auto Account::Imp::check(
    const crypto::Deterministic& subaccount,
    const crypto::Subchain subchain,
    SubchainsIDs& set) noexcept -> void
{
    const auto [it, added] = set.emplace(subaccount.ID());

    if (added) {
        log_("Instantiating ")(name_)(" subaccount ")(subaccount.ID())(" ")(
            print(subchain))(" subchain for ")(subaccount.Parent().NymID())
            .Flush();
        const auto& asio = api_.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        auto ptr = boost::allocate_shared<DeterministicStateData>(
            alloc::PMR<DeterministicStateData>{asio.Alloc(batchID)},
            reorg_,
            subaccount,
            api_p_,
            node_p_,
            subchain,
            from_parent_,
            batchID);

        OT_ASSERT(ptr);

        ptr->Init(ptr);
    }
}

auto Account::Imp::check_hd(const identifier::Generic& id) noexcept -> void
{
    check_hd(account_.GetHD().at(id));
}

auto Account::Imp::check_hd(const crypto::HD& subaccount) noexcept -> void
{
    check(subaccount, crypto::Subchain::Internal, internal_);
    check(subaccount, crypto::Subchain::External, external_);
}

auto Account::Imp::check_notification(const identifier::Generic& id) noexcept
    -> void
{
    check_notification(account_.GetNotification().at(id));
}

auto Account::Imp::check_notification(
    const crypto::Notification& subaccount) noexcept -> void
{
    const auto [it, added] = notification_.emplace(subaccount.ID());

    if (added) {
        const auto& code = subaccount.LocalPaymentCode();
        log_("Initializing payment code ")(code.asBase58())(" on ")(name_)
            .Flush();
        const auto& asio = api_.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        auto ptr = boost::allocate_shared<NotificationStateData>(
            alloc::PMR<NotificationStateData>{asio.Alloc(batchID)},
            reorg_,
            subaccount,
            code,
            api_p_,
            node_p_,
            crypto::Subchain::NotificationV3,
            from_parent_,
            batchID);

        OT_ASSERT(ptr);

        ptr->Init(ptr);
    }
}

auto Account::Imp::check_pc(const identifier::Generic& id) noexcept -> void
{
    check_pc(account_.GetPaymentCode().at(id));
}

auto Account::Imp::check_pc(const crypto::PaymentCode& subaccount) noexcept
    -> void
{
    check(subaccount, crypto::Subchain::Outgoing, outgoing_);
    check(subaccount, crypto::Subchain::Incoming, incoming_);
}

auto Account::Imp::do_reorg(
    const node::HeaderOracle& oracle,
    const node::internal::HeaderOraclePrivate& data,
    Reorg::Params& params) noexcept -> bool
{
    // NOTE no action necessary

    return true;
}

auto Account::Imp::do_shutdown() noexcept -> void
{
    state_ = State::shutdown;
    reorg_.Stop();
    node_p_.reset();
    api_p_.reset();
}

auto Account::Imp::do_startup(allocator_type) noexcept -> bool
{
    if (Reorg::State::shutdown == reorg_.Start()) { return true; }

    api_.Wallet().Internal().PublishNym(account_.NymID());
    scan_subchains();
    index_nym(account_.NymID());

    return false;
}

auto Account::Imp::index_nym(const identifier::Nym& id) noexcept -> void
{
    for (const auto& subaccount : account_.GetNotification()) {
        check_notification(subaccount);
    }
}

auto Account::Imp::Init(boost::shared_ptr<Imp> me) noexcept -> void
{
    signal_startup(me);
}

auto Account::Imp::pipeline(
    const Work work,
    Message&& msg,
    allocator_type) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        case State::pre_shutdown: {
            state_pre_shutdown(work, std::move(msg));
        } break;
        case State::shutdown: {
            // NOTE do nothing
        } break;
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid state").Abort();
        }
    }
}

auto Account::Imp::process_key(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(5u < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto owner = api_.Factory().NymIDFromHash(body.at(2).Bytes());

    if (owner != account_.NymID()) { return; }

    const auto id = api_.Factory().IdentifierFromHash(body.at(3).Bytes());
    const auto type = body.at(5).as<crypto::SubaccountType>();
    process_subaccount(id, type);
}

auto Account::Imp::process_prepare_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1u < body.size());

    transition_state_reorg(body.at(1).as<StateSequence>());
}

auto Account::Imp::process_rescan(Message&& in) noexcept -> void
{
    // NOTE no action necessary
}

auto Account::Imp::process_subaccount(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto owner = api_.Factory().NymIDFromHash(body.at(2).Bytes());

    if (owner != account_.NymID()) { return; }

    const auto type = body.at(3).as<crypto::SubaccountType>();
    const auto id = api_.Factory().IdentifierFromHash(body.at(4).Bytes());
    process_subaccount(id, type);
}

auto Account::Imp::process_subaccount(
    const identifier::Generic& id,
    const crypto::SubaccountType type) noexcept -> void
{
    switch (type) {
        case crypto::SubaccountType::HD: {
            check_hd(id);
        } break;
        case crypto::SubaccountType::PaymentCode: {
            check_pc(id);
        } break;
        case crypto::SubaccountType::Error:
        case crypto::SubaccountType::Imported:
        case crypto::SubaccountType::Notification:
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid subaccount type")
                .Abort();
        }
    }
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
        case Work::subaccount: {
            process_subaccount(std::move(msg));
        } break;
        case Work::prepare_reorg: {
            process_prepare_reorg(std::move(msg));
        } break;
        case Work::rescan: {
            process_rescan(std::move(msg));
        } break;
        case Work::key: {
            process_key(std::move(msg));
        } break;
        case Work::prepare_shutdown: {
            transition_state_pre_shutdown();
        } break;
        case Work::finish_reorg: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Abort();
        }
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Account::Imp::state_pre_shutdown(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::subaccount:
        case Work::rescan:
        case Work::key: {
            // NOTE ignore message
        } break;
        case Work::prepare_reorg:
        case Work::finish_reorg:
        case Work::prepare_shutdown: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Abort();
        }
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Account::Imp::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::subaccount:
        case Work::prepare_reorg:
        case Work::rescan:
        case Work::key: {
            defer(std::move(msg));
        } break;
        case Work::finish_reorg: {
            transition_state_normal();
        } break;
        case Work::prepare_shutdown: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Abort();
        }
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Account::Imp::transition_state_normal() noexcept -> void
{
    state_ = State::normal;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to normal state ").Flush();
    trigger();
}

auto Account::Imp::transition_state_pre_shutdown() noexcept -> void
{
    reorg_.AcknowledgeShutdown();
    state_ = State::pre_shutdown;
    log_(OT_PRETTY_CLASS())(name_)(": transitioned to pre_shutdown state")
        .Flush();
}

auto Account::Imp::transition_state_reorg(StateSequence id) noexcept -> void
{
    OT_ASSERT(0_uz < id);

    if (0_uz == reorgs_.count(id)) {
        reorgs_.emplace(id);
        state_ = State::reorg;
        log_(OT_PRETTY_CLASS())(name_)(" ready to process reorg ")(id).Flush();
        reorg_.AcknowledgePrepareReorg(
            [this](const auto& header, const auto& lock, auto& params) {
                return do_reorg(header, lock, params);
            });
    } else {
        LogAbort()(OT_PRETTY_CLASS())(name_)(" reorg ")(id)(" already handled")
            .Abort();
    }
}

auto Account::Imp::work(allocator_type monotonic) noexcept -> bool
{
    return false;
}

Account::Imp::~Imp() = default;
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Account::Account(
    Reorg& reorg,
    const crypto::Account& account,
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    std::string_view fromParent) noexcept
    : imp_([&] {
        OT_ASSERT(api);
        OT_ASSERT(node);

        const auto& asio = api->Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)},
            reorg,
            account,
            std::move(api),
            std::move(node),
            fromParent,
            batchID);
    }())
{
}

auto Account::Init() noexcept -> void
{
    OT_ASSERT(imp_);

    imp_->Init(imp_);
    imp_.reset();
}

Account::~Account() = default;
}  // namespace opentxs::blockchain::node::wallet
