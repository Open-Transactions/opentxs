// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/blockchain/node/Wallet.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Client;
}  // namespace internal

class Client;
}  // namespace session
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class Wallet::Actor final
    : public opentxs::Actor<Wallet::Actor, wallet::WalletJobs>
{
public:
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto Init(std::shared_ptr<Wallet::Actor> me) noexcept -> void
    {
        signal_startup(me);
    }

    Actor(
        std::shared_ptr<const api::session::internal::Client> api,
        std::shared_ptr<const node::Manager> node,
        std::shared_ptr<internal::Wallet::Shared> shared,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend opentxs::Actor<Wallet::Actor, wallet::WalletJobs>;

    std::shared_ptr<const api::session::internal::Client> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    std::shared_ptr<internal::Wallet::Shared> shared_p_;
    const api::session::Client& api_;
    const node::Manager& node_;
    internal::Wallet::Shared& shared_;
    network::zeromq::socket::Raw& to_accounts_;
    bool running_;

    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::internal
