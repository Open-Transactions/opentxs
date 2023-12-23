// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/Actor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <memory>
#include <utility>

#include "blockchain/node/wallet/Shared.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Accounts.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Client.internal.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::internal
{
using enum opentxs::network::zeromq::socket::Direction;
using opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

Wallet::Actor::Actor(
    std::shared_ptr<const api::session::internal::Client> api,
    std::shared_ptr<const node::Manager> node,
    std::shared_ptr<internal::Wallet::Shared> shared,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : opentxs::Actor<Wallet::Actor, wallet::WalletJobs>(
          api->Self(),
          LogTrace(),
          [&] {
              auto out = CString{print(node->Internal().Chain()), alloc};
              out.append(" wallet");

              return out;
          }(),
          10ms,
          batch,
          alloc,
          {
              {api->Endpoints().Shutdown(), Connect},
              {node->Internal().Endpoints().shutdown_publish_, Connect},
          },
          {
              {node->Internal().Endpoints().wallet_pull_, Bind},
          },
          {},
          {
              {Push,
               Policy::Internal,
               {
                   {node->Internal().Endpoints().wallet_to_accounts_push_,
                    Bind},
               }},
          })
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , shared_p_(std::move(shared))
    , api_(api_p_->asClientPublic())
    , node_(*node_p_)
    , shared_(*shared_p_)
    , to_accounts_(pipeline_.Internal().ExtraSocket(0))
    , running_(false)
{
}

auto Wallet::Actor::do_startup(allocator_type) noexcept -> bool
{
    if (api_.Internal().ShuttingDown() || node_.Internal().ShuttingDown()) {

        return true;
    }

    trigger();

    return false;
}

auto Wallet::Actor::do_shutdown() noexcept -> void
{
    shared_p_.reset();
    node_p_.reset();
    api_p_.reset();
}

auto Wallet::Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type) noexcept -> void
{
    switch (work) {
        case Work::start_wallet: {
            wallet::Accounts{api_p_, node_p_}.Init();
            running_ = true;
        } break;
        case Work::rescan: {
            if (running_) {
                to_accounts_.SendDeferred(
                    MakeWork(wallet::AccountsJobs::rescan));
            }
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine:
        default: {
            LogAbort()()(name_)(": unhandled type").Abort();
        }
    }
}

auto Wallet::Actor::work(allocator_type monotonic) noexcept -> bool
{
    if (running_) {

        return shared_.Run();
    } else {

        return false;
    }
}

Wallet::Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::internal
