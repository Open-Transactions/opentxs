// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "blockchain/node/wallet/Actor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <memory>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/Shared.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Accounts.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::internal
{
Wallet::Actor::Actor(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    boost::shared_ptr<internal::Wallet::Shared> shared,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : opentxs::Actor<Wallet::Actor, wallet::WalletJobs>(
          *api,
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
              {CString{api->Endpoints().Shutdown(), alloc}, Direction::Connect},
              {CString{node->Internal().Endpoints().shutdown_publish_, alloc},
               Direction::Connect},
          },
          {
              {CString{node->Internal().Endpoints().wallet_pull_, alloc},
               Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {node->Internal().Endpoints().wallet_to_accounts_push_,
                    Direction::Bind},
               }},
          })
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , shared_p_(std::move(shared))
    , api_(*api_p_)
    , node_(*node_p_)
    , shared_(*shared_p_)
    , to_accounts_(pipeline_.Internal().ExtraSocket(0))
    , running_(false)
{
}

auto Wallet::Actor::do_startup() noexcept -> bool
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

auto Wallet::Actor::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::start_wallet: {
            wallet::Accounts{api_p_, node_p_}.Init();
            running_ = true;
        } break;
        case Work::rescan: {
            if (running_) {
                to_accounts_.SendDeferred(
                    MakeWork(wallet::AccountsJobs::rescan), __FILE__, __LINE__);
            }
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine:
        default: {
            LogError()(OT_PRETTY_CLASS())(name_)(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Wallet::Actor::work() noexcept -> bool
{
    if (running_) {

        return shared_.Run();
    } else {

        return false;
    }
}

Wallet::Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::internal
