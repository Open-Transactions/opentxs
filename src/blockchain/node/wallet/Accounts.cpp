// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/node/wallet/Accounts.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <atomic>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
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
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
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
            {Job::init, "init"},
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
    node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const network::zeromq::BatchID batch,
    const Type chain,
    CString&& shutdown,
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
          {},
          {},
          {
              {SocketType::Publish,
               {
                   {shutdown, Direction::Bind},
               }},
          })
    , api_(api)
    , node_(node)
    , db_(db)
    , mempool_(mempool)
    , chain_(chain)
    , filter_type_(node_.FilterOracleInternal().DefaultType())
    , shutdown_endpoint_(std::move(shutdown))
    , shutdown_socket_(pipeline_.Internal().ExtraSocket(0))
    , state_(State::normal)
    , accounts_(alloc)
{
}

Accounts::Imp::Imp(
    const api::Session& api,
    const node::internal::Network& node,
    node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const network::zeromq::BatchID batch,
    const Type chain,
    allocator_type alloc) noexcept
    : Imp(api,
          node,
          db,
          mempool,
          batch,
          chain,
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          std::move(alloc))
{
}

auto Accounts::Imp::do_shutdown() noexcept -> void { accounts_.clear(); }

auto Accounts::Imp::do_startup() noexcept -> void
{
    for (const auto& id : api_.Wallet().LocalNyms()) { process_nym(id); }
}

auto Accounts::Imp::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::shutdown: {
            shutdown_actor();
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
        api_,
        api_.Crypto().Blockchain().Account(nym, chain_),
        node_,
        db_,
        mempool_,
        chain_,
        filter_type_,
        shutdown_endpoint_);

    if (added) {
        LogConsole()("Initializing ")(print(chain_))(" account for ")(nym)
            .Flush();
    }

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

auto Accounts::Imp::process_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    if (6 > body.size()) {
        LogError()(OT_PRETTY_CLASS())(print(chain_))(": invalid message")
            .Flush();

        OT_FAIL;
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    process_reorg(
        std::make_pair(
            body.at(3).as<block::Height>(),
            api_.Factory().Data(body.at(2).Bytes())),
        std::make_pair(
            body.at(5).as<block::Height>(),
            api_.Factory().Data(body.at(4).Bytes())));
}

auto Accounts::Imp::process_reorg(
    const block::Position& ancestor,
    const block::Position& tip) noexcept -> void
{
    transition_state_reorg();
    auto errors = std::atomic_int{0};

    {
        auto lock = Lock{node_.HeaderOracle().Internal().GetMutex()};
        auto tx = db_.StartReorg();
        for_each([&](auto& a) {
            a.second.ProcessReorg(lock, tx, errors, ancestor);
        });

        if (false == db_.FinalizeReorg(tx, ancestor)) { ++errors; }

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
        if (false == db_.AdvanceTo(tip)) {

            throw std::runtime_error{"Advance chain failed"};
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(print(chain_))(" ")(e.what()).Flush();

        OT_FAIL;
    }

    // TODO ensure filter oracle has processed the reorg before proceeding
    for_each([&](auto& a) {
        const auto rc = a.second.ChangeState(Account::State::normal);

        OT_ASSERT(rc);
    });
    LogConsole()(print(chain_))(": reorg to ")(tip.second->asHex())(
        " at height ")(tip.first)(" finished")
        .Flush();
}

auto Accounts::Imp::Shutdown() noexcept -> void
{
    auto lock = std::unique_lock<std::recursive_timed_mutex>{reorg_lock_};
    transition_state_shutdown();
}

auto Accounts::Imp::state_normal(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::shutdown: {
            shutdown_actor();
        } break;
        case Work::nym: {
            process_nym(std::move(msg));
        } break;
        case Work::header: {
            process_block_header(std::move(msg));
        } break;
        case Work::reorg: {
            process_reorg(std::move(msg));
        } break;
        case Work::init: {
            do_init();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " unhandled message type ")(static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Accounts::Imp::transition_state_reorg() noexcept -> void
{
    for_each([](auto& a) {
        auto rc = a.second.ChangeState(Account::State::reorg);

        OT_ASSERT(rc);
    });
}

auto Accounts::Imp::transition_state_shutdown() noexcept -> void
{
    shutdown_socket_.SendDeferred(MakeWork(OT_ZMQ_PREPARE_SHUTDOWN));
    for_each([](auto& a) {
        auto rc = a.second.ChangeState(Account::State::shutdown);

        OT_ASSERT(rc);
    });
    state_ = State::shutdown;
    log_(OT_PRETTY_CLASS())("transitioned to shutdown state").Flush();
    signal_shutdown();
}

auto Accounts::Imp::work() noexcept -> bool { return false; }

Accounts::Imp::~Imp() = default;
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Accounts::Accounts(
    const api::Session& api,
    const node::internal::Network& node,
    node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const Type chain) noexcept
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
            chain);
    }())
{
    OT_ASSERT(imp_);
}

auto Accounts::Init() noexcept -> void { imp_->Init(imp_); }

auto Accounts::Shutdown() noexcept -> void { imp_->Shutdown(); }

Accounts::~Accounts() { imp_->Shutdown(); }
}  // namespace opentxs::blockchain::node::wallet
