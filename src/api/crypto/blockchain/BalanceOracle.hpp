// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <memory>
#include <string_view>
#include <utility>

#include "internal/api/crypto/blockchain/BalanceOracle.hpp"
#include "internal/api/crypto/blockchain/Types.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

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

namespace opentxs::api::crypto::blockchain
{
class BalanceOracle::Imp final : public opentxs::Actor<Imp, BalanceOracleJobs>
{
public:
    auto Init(std::shared_ptr<Imp> me) noexcept -> void { signal_startup(me); }

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Imp(std::shared_ptr<const api::Session> api,
        std::string_view endpoint,
        const opentxs::network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;

    ~Imp() final;

private:
    friend opentxs::Actor<Imp, BalanceOracleJobs>;

    using Balance = opentxs::blockchain::Balance;
    using Chain = opentxs::blockchain::Type;
    using Subscribers = Set<opentxs::network::zeromq::Envelope>;
    using Data = std::pair<Balance, Subscribers>;
    using NymData = Map<identifier::Nym, Data>;
    using ChainData = std::pair<Data, NymData>;

    std::shared_ptr<const api::Session> api_;
    opentxs::network::zeromq::socket::Raw& router_;
    opentxs::network::zeromq::socket::Raw& publish_;
    Map<Chain, ChainData> data_;

    auto make_message(
        const opentxs::network::zeromq::Envelope& connectionID,
        const identifier::Nym* owner,
        const Chain chain,
        const Balance& balance,
        const WorkType type) const noexcept -> Message;

    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto notify_subscribers(
        const Subscribers& recipients,
        const Balance& balance,
        const Chain chain) noexcept -> void;
    auto notify_subscribers(
        const Subscribers& recipients,
        const identifier::Nym& owner,
        const Balance& balance,
        const Chain chain) noexcept -> void;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto process_registration(Message&& in) noexcept -> void;
    auto process_update_balance(Message&& in) noexcept -> void;
    auto process_update_balance(const Chain chain, Balance balance) noexcept
        -> void;
    auto process_update_balance(
        const identifier::Nym& owner,
        const Chain chain,
        Balance balance) noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::api::crypto::blockchain
