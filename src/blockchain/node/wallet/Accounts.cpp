// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/Accounts.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <chrono>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/api/session/Session.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(AccountsJobs job) noexcept -> std::string_view
{
    try {
        using enum AccountsJobs;
        static const auto map = Map<AccountsJobs, CString>{
            {shutdown, "shutdown"},
            {nym, "nym"},
            {header, "header"},
            {reorg, "reorg"},
            {rescan, "rescan"},
            {reorg_ready, "reorg_ready"},
            {shutdown_ready, "shutdown_ready"},
            {init, "init"},
            {prepare_shutdown, "prepare_shutdown"},
            {statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogAbort()(__FUNCTION__)("invalid AccountsJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Accounts::Imp::Imp(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    network::zeromq::BatchID batch,
    CString&& toChildren,
    allocator_type alloc) noexcept
    : Actor(
          *api,
          LogTrace(),
          [&] {
              auto out = CString{print(node->Internal().Chain()), alloc};
              out.append(" wallet account manager");

              return out;
          }(),
          0ms,
          batch,
          alloc,
          {
              {CString{api->Endpoints().BlockchainReorg(), alloc},
               Direction::Connect},
              {CString{api->Endpoints().NymCreated(), alloc},
               Direction::Connect},
              {CString{api->Endpoints().Shutdown(), alloc}, Direction::Connect},
              {CString{node->Internal().Endpoints().shutdown_publish_, alloc},
               Direction::Connect},
          },
          {
              {CString{
                   node->Internal().Endpoints().wallet_to_accounts_push_,
                   alloc},
               Direction::Connect},
          },
          {},
          {
              {SocketType::Publish,
               {
                   {toChildren, Direction::Bind},
               }},
          })
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , api_(*api_p_)
    , node_(*node_p_)
    , db_(node_.Internal().DB())
    , mempool_(node_.Internal().Mempool())
    , chain_(node_.Internal().Chain())
    , to_children_endpoint_(std::move(toChildren))
    , to_children_(pipeline_.Internal().ExtraSocket(0))
    , state_(State::normal)
    , reorg_counter_(0)
    , accounts_(alloc)
    , startup_reorg_(std::nullopt)
    , reorg_data_(std::nullopt)
    , reorg_(pipeline_, alloc)
{
}

Accounts::Imp::Imp(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Imp(std::move(api),
          std::move(node),
          std::move(batch),
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          std::move(alloc))
{
}

auto Accounts::Imp::do_reorg() noexcept -> void
{
    try {
        if (false == reorg_data_.has_value()) {
            throw std::runtime_error{"missing reorg data"};
        }

        const auto& [ancestor, tip] = *reorg_data_;

        {
            auto& [position, tx] = reorg_.GetReorg(ancestor, db_.StartReorg());

            if (false == reorg_.PerformReorg(node_.HeaderOracle())) {
                throw std::runtime_error{"perform reorg error"};
            }

            if (false == db_.FinalizeReorg(tx, ancestor)) {
                throw std::runtime_error{"finalize reorg error"};
            }

            reorg_.ClearReorg();
        }

        if (false == db_.AdvanceTo(tip)) {
            throw std::runtime_error{"Advance chain failed"};
        }

        // TODO ensure filter oracle has processed the reorg before proceeding
        reorg_.FinishReorg();
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Abort();
    }

    transition_state_normal();
}

auto Accounts::Imp::do_shutdown() noexcept -> void
{
    state_ = State::shutdown;
    reorg_.Stop();
    node_p_.reset();
    api_p_.reset();
}

auto Accounts::Imp::do_startup(allocator_type) noexcept -> bool
{
    if (api_.Internal().ShuttingDown() || node_.Internal().ShuttingDown()) {

        return true;
    }

    for (const auto& id : api_.Wallet().LocalNyms()) { process_nym(id); }

    const auto oldPosition = db_.GetPosition();
    log_(OT_PRETTY_CLASS())(name_)(" last wallet position is ")(oldPosition)
        .Flush();

    if (0 > oldPosition.height_) { return false; }

    const auto [parent, best] = node_.HeaderOracle().CommonParent(oldPosition);

    if (parent == oldPosition) {
        log_(OT_PRETTY_CLASS())(name_)(" last wallet position is in best chain")
            .Flush();
    } else {
        log_(OT_PRETTY_CLASS())(name_)(" last wallet position is stale")
            .Flush();
        startup_reorg_.emplace(++reorg_counter_);
        pipeline_.Push([&](const auto& ancestor, const auto& tip) {
            auto out = MakeWork(Work::reorg);
            out.AddFrame(chain_);
            out.AddFrame(ancestor.hash_);
            out.AddFrame(ancestor.height_);
            out.AddFrame(tip.hash_);
            out.AddFrame(tip.height_);

            return out;
        }(parent, best));
    }

    return false;
}

auto Accounts::Imp::pipeline(
    const Work work,
    Message&& msg,
    allocator_type) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::pre_reorg: {
            state_pre_reorg(work, std::move(msg));
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

auto Accounts::Imp::process_block_header(Message&& in) noexcept -> void
{
    if (startup_reorg_.has_value()) {
        defer(std::move(in));

        return;
    }

    const auto body = in.Body();

    if (3 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto position =
        block::Position{body.at(3).as<block::Height>(), body.at(2).Bytes()};
    log_(OT_PRETTY_CLASS())("processing block header for ")(position).Flush();
    db_.AdvanceTo(position);
}

auto Accounts::Imp::process_nym(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto id = api_.Factory().NymIDFromHash(body.at(1).Bytes());

    if (0_uz == id.size()) { return; }

    process_nym(id);
}

auto Accounts::Imp::process_nym(const identifier::Nym& nym) noexcept -> void
{
    const auto [i, added] = accounts_.emplace(nym);

    if (added) {
        const auto endpoint =
            network::zeromq::MakeArbitraryInproc(get_allocator().resource());
        LogConsole()("Initializing ")(name_)(" for ")(nym).Flush();
        const auto& account = api_.Crypto().Blockchain().Account(nym, chain_);
        account.Internal().Startup();
        wallet::Account{reorg_, account, api_p_, node_p_, to_children_endpoint_}
            .Init();
    }
}

auto Accounts::Imp::process_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    if (6 > body.size()) {
        LogError()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    process_reorg(
        std::move(in),
        {body.at(3).as<block::Height>(), body.at(2).Bytes()},
        {body.at(5).as<block::Height>(), body.at(4).Bytes()});
}

auto Accounts::Imp::process_reorg(
    Message&& in,
    block::Position&& ancestor,
    block::Position&& tip) noexcept -> void
{
    OT_ASSERT(false == reorg_data_.has_value());

    reorg_data_.emplace(std::move(ancestor), std::move(tip));
    transition_state_pre_reorg();
}

auto Accounts::Imp::process_rescan(Message&& in) noexcept -> void
{
    to_children_.Send(MakeWork(AccountJobs::rescan), __FILE__, __LINE__);
}

auto Accounts::Imp::state_normal(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::nym: {
            process_nym(std::move(msg));
        } break;
        case Work::header: {
            process_block_header(std::move(msg));
        } break;
        case Work::reorg: {
            process_reorg(std::move(msg));
        } break;
        case Work::rescan: {
            process_rescan(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::prepare_shutdown: {
            transition_state_pre_shutdown();
        } break;
        case Work::reorg_ready: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Abort();
        }
        case Work::shutdown_ready:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Accounts::Imp::state_pre_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::shutdown:
        case Work::nym:
        case Work::header:
        case Work::reorg:
        case Work::rescan:
        case Work::prepare_shutdown: {
            defer(std::move(msg));
        } break;
        case Work::reorg_ready: {
            do_reorg();
        } break;
        case Work::shutdown_ready:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Accounts::Imp::state_pre_shutdown(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::shutdown:
        case Work::prepare_shutdown: {
            if (reorg_.CheckShutdown()) { shutdown_actor(); }
        } break;
        case Work::nym:
        case Work::header:
        case Work::reorg:
        case Work::rescan: {
            // NOTE ignore message
        } break;
        case Work::reorg_ready: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Abort();
        }
        case Work::shutdown_ready:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Accounts::Imp::transition_state_normal() noexcept -> void
{
    OT_ASSERT(reorg_data_.has_value());

    const auto& [ancestor, tip] = *reorg_data_;
    auto post = ScopeGuard{[&] { reorg_data_.reset(); }};
    LogConsole()(name_)(": reorg to ")(tip)(" finished").Flush();
    state_ = State::normal;
    trigger();
}

auto Accounts::Imp::transition_state_pre_reorg() noexcept -> void
{
    const auto id = [this] {
        if (startup_reorg_.has_value()) {

            return startup_reorg_.value();
        } else {

            return ++reorg_counter_;
        }
    }();
    log_(OT_PRETTY_CLASS())(name_)(": processing reorg ")(id).Flush();
    state_ = State::pre_reorg;
    startup_reorg_.reset();
    log_(OT_PRETTY_CLASS())(name_)(": transitioned to pre_reorg state").Flush();

    if (reorg_.PrepareReorg(id)) {
        log_(OT_PRETTY_CLASS())(name_)(": zero children").Flush();
    }
}

auto Accounts::Imp::transition_state_pre_shutdown() noexcept -> void
{
    state_ = State::pre_shutdown;
    log_(OT_PRETTY_CLASS())(name_)(": transitioned to pre_shutdown state")
        .Flush();

    if (reorg_.PrepareShutdown()) { shutdown_actor(); }
}

auto Accounts::Imp::work(allocator_type monotonic) noexcept -> bool
{
    return false;
}

Accounts::Imp::~Imp() = default;
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Accounts::Accounts(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node) noexcept
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
            std::move(api),
            std::move(node),
            batchID);
    }())
{
    OT_ASSERT(imp_);
}

auto Accounts::Init() noexcept -> void { imp_->Init(imp_); }

Accounts::~Accounts() = default;
}  // namespace opentxs::blockchain::node::wallet
