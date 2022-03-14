// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/node/wallet/Accounts.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/system/error_code.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
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
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/HDPath.pb.h"
#include "util/LMDB.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(AccountsJobs job) noexcept -> std::string_view
{
    try {
        using Job = AccountsJobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::nym, "nym"},
            {Job::header, "header"},
            {Job::reorg, "reorg"},
            {Job::reorg_begin_ack, "reorg_begin_ack"},
            {Job::reorg_end_ack, "reorg_end_ack"},
            {Job::init, "init"},
            {Job::shutdown_begin, "shutdown_begin"},
            {Job::shutdown_ready, "shutdown_ready"},
            {Job::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid AccountsJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Accounts::Imp::Imp(
    const api::Session& api,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const network::zeromq::BatchID batch,
    const Type chain,
    const std::string_view toParent,
    CString&& fromChildren,
    allocator_type alloc) noexcept
    : Actor(
          api,
          LogTrace(),
          0ms,
          batch,
          alloc,
          {
              {CString{api.Endpoints().BlockchainReorg(), alloc},
               Direction::Connect},
              {CString{api.Endpoints().NymCreated(), alloc},
               Direction::Connect},
          },
          {
              {fromChildren, Direction::Bind},
          },
          {},
          {
              {SocketType::Pair,
               {
                   {CString{toParent}, Direction::Connect},
               }},
          })
    , api_(api)
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
    , accounts_(alloc)
    , codes_(alloc)
    , reorg_timer_(api_.Network().Asio().Internal().GetTimer())
{
}

Accounts::Imp::Imp(
    const api::Session& api,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const network::zeromq::BatchID batch,
    const Type chain,
    const std::string_view toParent,
    allocator_type alloc) noexcept
    : Imp(api,
          node,
          db,
          mempool,
          batch,
          chain,
          toParent,
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          std::move(alloc))
{
}

auto Accounts::Imp::do_reorg() noexcept -> void
{
    const auto& reorg = reorg_.value();
    auto errors = std::atomic_int{0};

    {
        auto headerOracleLock =
            Lock{node_.HeaderOracle().Internal().GetMutex()};
        auto tx = db_.StartReorg();

        for (auto& [nym, data] : accounts_) {
            auto& [socket, account] = data;
            account.ProcessReorg(headerOracleLock, tx, errors, reorg.ancestor_);
        }

        for (auto& [id, data] : codes_) {
            auto& [socket, code] = data;
            code->ProcessReorg(headerOracleLock, tx, errors, reorg.ancestor_);
        }

        if (false == db_.FinalizeReorg(tx, reorg.ancestor_)) { ++errors; }

        try {
            if (0 < errors) { throw std::runtime_error{"Prepare step failed"}; }

            if (false == tx.Finalize(true)) {

                throw std::runtime_error{"Finalize transaction failed"};
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(": ")(e.what())
                .Flush();

            OT_FAIL;
        }
    }

    try {
        if (false == db_.AdvanceTo(reorg.tip_)) {

            throw std::runtime_error{"Advance chain failed"};
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(print(chain_))(" ")(e.what()).Flush();

        OT_FAIL;
    }

    // TODO ensure filter oracle has processed the reorg before proceeding
    state_ = State::post_reorg;

    if (0u < reorg_->target_) {
        log_(OT_PRETTY_CLASS())(print(chain_))(
            " waiting for acknowledgements from ")(reorg_->target_)(
            " children prior to returning to normal state")
            .Flush();
        send_to_children(MakeWork(AccountJobs::reorg_end));
    } else {
        log_(OT_PRETTY_CLASS())(print(chain_))(
            " no children instantiated therefore reorg can be completed "
            "immediately")
            .Flush();
        finish_reorg();
    }
}

auto Accounts::Imp::do_shutdown() noexcept -> void
{
    send_to_children(MakeWork(WorkType::Shutdown));
    codes_.clear();
    accounts_.clear();
}

auto Accounts::Imp::finish_shutdown() noexcept -> void
{
    state_ = State::shutdown;
    log_(OT_PRETTY_CLASS())("transitioned to shutdown state").Flush();
    to_parent_.SendDeferred(MakeWork(WalletJobs::shutdown_ready));
}

auto Accounts::Imp::finish_reorg() noexcept -> void
{
    const auto& tip = reorg_->tip_;
    LogConsole()(print(chain_))(": reorg to ")(tip.second->asHex())(
        " at height ")(tip.first)(" finished")
        .Flush();
    reorg_ = std::nullopt;
    disable_automatic_processing_ = false;
    state_ = State::normal;
    log_(OT_PRETTY_CLASS())("transitioned to normal state").Flush();
    flush_cache();
}

auto Accounts::Imp::index_nym(const identifier::Nym& id) noexcept -> void
{
    const auto pNym = api_.Wallet().Nym(id);

    OT_ASSERT(pNym);

    const auto& nym = *pNym;
    auto code = api_.Factory().PaymentCode(nym.PaymentCode());

    if (3 > code.Version()) { return; }

    LogConsole()("Initializing payment code ")(code.asBase58())(" on ")(
        print(chain_))
        .Flush();
    auto accountID = NotificationStateData::calculate_id(api_, chain_, code);
    const auto& asio = api_.Network().ZeroMQ().Internal();
    const auto batchID = asio.PreallocateBatch();
    const auto endpoint =
        network::zeromq::MakeArbitraryInproc(get_allocator().resource());
    auto [it, added] = codes_.try_emplace(
        std::move(accountID),
        std::make_pair(
            [&] {
                auto out = api_.Network().ZeroMQ().Internal().RawSocket(
                    SocketType::Pair);
                auto rc = out.Bind(endpoint.c_str());

                OT_ASSERT(rc);

                return out;
            }(),
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
                endpoint,
                std::move(code),
                [&] {
                    auto out = proto::HDPath{};
                    nym.PaymentCodePath(out);

                    return out;
                }())));
    auto& ptr = it->second.second;

    OT_ASSERT(ptr);

    auto& subchain = *ptr;

    if (added) { subchain.Init(ptr); }
}

auto Accounts::Imp::Init(boost::shared_ptr<Imp> me) noexcept -> void
{
    signal_startup(me);
}

auto Accounts::Imp::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
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

auto Accounts::Imp::process_block_header(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    if (3 >= body.size()) {
        LogError()(OT_PRETTY_CLASS())(print(chain_))(": invalid message")
            .Flush();

        OT_FAIL;
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto position = block::Position{
        body.at(3).as<block::Height>(),
        api_.Factory().Data(body.at(2).Bytes())};
    db_.AdvanceTo(position);
}

auto Accounts::Imp::process_nym(const identifier::Nym& nym) noexcept -> bool
{
    const auto endpoint =
        network::zeromq::MakeArbitraryInproc(get_allocator().resource());
    auto [it, added] = accounts_.try_emplace(
        nym,
        std::make_pair(
            [&] {
                auto out = api_.Network().ZeroMQ().Internal().RawSocket(
                    SocketType::Pair);
                auto rc = out.Bind(endpoint.c_str());

                OT_ASSERT(rc);

                return out;
            }(),
            wallet::Account{
                api_,
                api_.Crypto().Blockchain().Account(nym, chain_),
                node_,
                db_,
                mempool_,
                chain_,
                filter_type_,
                endpoint}));

    if (added) {
        LogConsole()("Initializing ")(print(chain_))(" wallet for ")(nym)
            .Flush();
    }

    index_nym(nym);

    return added;
}

auto Accounts::Imp::process_nym(Message&& in) noexcept -> bool
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto id = api_.Factory().NymID(body.at(1));

    if (0 == id->size()) { return false; }

    return process_nym(id);
}

auto Accounts::Imp::reorg_children() const noexcept -> std::size_t
{
    return accounts_.size() + codes_.size();
}

auto Accounts::Imp::send_to_children(const Message& msg) noexcept -> void
{
    for (auto& [key, value] : accounts_) {
        value.first.SendDeferred(Message{msg});
    }

    for (auto& [key, value] : codes_) {
        value.first.SendDeferred(Message{msg});
    }
}

auto Accounts::Imp::startup() noexcept -> void
{
    for (const auto& id : api_.Wallet().LocalNyms()) { process_nym(id); }
}

auto Accounts::Imp::state_normal(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(false == reorg_.has_value());
    OT_ASSERT(false == shutdown_.has_value());

    switch (work) {
        case Work::nym: {
            process_nym(std::move(msg));
        } break;
        case Work::header: {
            process_block_header(std::move(msg));
        } break;
        case Work::reorg: {
            transition_state_reorg(std::move(msg));
        } break;
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        case Work::shutdown_ready: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(" wrong state for ")(
                print(work))(" message")
                .Flush();

            OT_FAIL;
        }
        case Work::shutdown_begin: {
            transition_state_pre_shutdown(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine:
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " unhandled message type ")(static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Accounts::Imp::state_post_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(reorg_.has_value());
    OT_ASSERT(false == shutdown_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::nym:
        case Work::header:
        case Work::reorg:
        case Work::shutdown_begin:  // TODO move to "wrong state" section
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::reorg_end_ack: {
            transition_state_normal(std::move(msg));
        } break;
        case Work::reorg_begin_ack:
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

auto Accounts::Imp::state_pre_shutdown(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(false == reorg_.has_value());
    OT_ASSERT(shutdown_.has_value());

    switch (work) {
        case Work::nym:
        case Work::header: {
            // NOTE drop non-shutdown messages
        } break;
        case Work::reorg:
        case Work::reorg_begin_ack:
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
        case Work::shutdown:
        case Work::init:
        case Work::statemachine:
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " unhandled message type ")(static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Accounts::Imp::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    OT_ASSERT(reorg_.has_value());
    OT_ASSERT(false == shutdown_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::nym:
        case Work::header:
        case Work::reorg:
        case Work::shutdown_begin:  // TODO move to "wrong state" section
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::reorg_begin_ack: {
            transition_state_post_reorg(std::move(msg));
        } break;
        case Work::reorg_end_ack:
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

auto Accounts::Imp::transition_state_normal(Message&& in) noexcept -> void
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
        verify_child_state(Subchain::State::normal, Account::State::normal);
        log_(OT_PRETTY_CLASS())(print(chain_))(" all ")(
            target)(" children finished with reorg")
            .Flush();
        finish_reorg();
    } else {

        OT_FAIL;
    }
}

auto Accounts::Imp::transition_state_post_reorg(Message&& in) noexcept -> void
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
        verify_child_state(Subchain::State::reorg, Account::State::reorg);
        log_(OT_PRETTY_CLASS())(print(chain_))(" all ")(
            target)(" children ready for reorg")
            .Flush();
        do_reorg();
    } else {

        OT_FAIL;
    }
}

auto Accounts::Imp::transition_state_pre_shutdown(Message&& in) noexcept -> void
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
        finish_shutdown();
    }
}

auto Accounts::Imp::transition_state_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    if (6 > body.size()) {
        LogError()(OT_PRETTY_CLASS())(print(chain_))(": invalid message")
            .Flush();

        OT_FAIL;
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    reorg_.emplace(
        block::Position{
            body.at(3).as<block::Height>(),
            api_.Factory().Data(body.at(2).Bytes())},
        block::Position{
            body.at(5).as<block::Height>(),
            api_.Factory().Data(body.at(4).Bytes())},
        reorg_children());
    LogConsole()("Processing ")(print(chain_))(" reorg to ")(
        reorg_->tip_.second->asHex())(" at height ")(reorg_->tip_.first)
        .Flush();
    disable_automatic_processing_ = true;
    state_ = State::reorg;

    if (0u < reorg_->target_) {
        log_(OT_PRETTY_CLASS())(print(chain_))(
            " waiting for acknowledgements from ")(reorg_->target_)(
            " children prior to executing reorg")
            .Flush();
        send_to_children(MakeWork(AccountJobs::reorg_begin));
        reorg_timer_.SetRelative(10 * 60s);
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
            " no children instantiated therefore reorg can be processed "
            "immediately")
            .Flush();
        do_reorg();
    }
}

auto Accounts::Imp::transition_state_shutdown(Message&& in) noexcept -> void
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
        verify_child_state(Subchain::State::shutdown, Account::State::shutdown);
        log_(OT_PRETTY_CLASS())(print(chain_))(" all ")(
            target)(" children ready for shutdown")
            .Flush();
        finish_shutdown();
    } else {

        OT_FAIL;
    }
}

auto Accounts::Imp::verify_child_state(
    const Subchain::State sub,
    const Account::State accountState) const noexcept -> void
{
    const auto a = [&](const auto& data) {
        auto& [socket, account] = data.second;
        account.VerifyState(accountState);
    };
    const auto s = [&](const auto& data) {
        auto& [socket, code] = data.second;
        code->VerifyState(sub);
    };
    std::for_each(accounts_.begin(), accounts_.end(), a);
    std::for_each(codes_.begin(), codes_.end(), s);
}

auto Accounts::Imp::work() noexcept -> bool { OT_FAIL; }

Accounts::Imp::~Imp() { reorg_timer_.Cancel(); }
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Accounts::Accounts(
    const api::Session& api,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const Type chain,
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
            node,
            db,
            mempool,
            batchID,
            chain,
            toParent);
    }())
{
    OT_ASSERT(imp_);
}

auto Accounts::Init() noexcept -> void { imp_->Init(imp_); }

Accounts::~Accounts() { imp_->Shutdown(); }
}  // namespace opentxs::blockchain::node::wallet
