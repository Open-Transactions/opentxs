// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <tuple>
#include <utility>

#include "internal/api/crypto/blockchain/BalanceOracle.hpp"
#include "internal/api/crypto/blockchain/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::blockchain
{
class BalanceOracle::Imp final : public opentxs::Actor<Imp, BalanceOracleJobs>
{
public:
    auto UpdateBalance(const Chain chain, const Balance balance) const noexcept
        -> void;
    auto UpdateBalance(
        const identifier::Nym& owner,
        const Chain chain,
        const Balance balance) const noexcept -> void;

    auto Init(boost::shared_ptr<Imp> me) noexcept -> void
    {
        signal_startup(me);
    }
    auto Shutdown() noexcept -> void { signal_shutdown(); }

    Imp(const api::Session& api,
        const opentxs::network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;

    ~Imp() final;

private:
    friend opentxs::Actor<Imp, BalanceOracleJobs>;

    using Subscribers = Set<OTData>;
    using Data = std::pair<Balance, Subscribers>;
    using NymData = Map<OTNymID, Data>;
    using ChainData = std::pair<Data, NymData>;

    const api::Session& api_;
    opentxs::network::zeromq::socket::Raw& router_;
    opentxs::network::zeromq::socket::Raw& publish_;
    Map<Chain, ChainData> data_;

    auto make_message(
        const ReadView connectionID,
        const identifier::Nym* owner,
        const Chain chain,
        const Balance& balance,
        const WorkType type) const noexcept -> Message;

    auto do_shutdown() noexcept -> void {}
    auto notify_subscribers(
        const Subscribers& recipients,
        const Balance& balance,
        const Chain chain) noexcept -> void;
    auto notify_subscribers(
        const Subscribers& recipients,
        const identifier::Nym& owner,
        const Balance& balance,
        const Chain chain) noexcept -> void;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_registration(Message&& in) noexcept -> void;
    auto process_update_balance(const Chain chain, Balance balance) noexcept
        -> void;
    auto process_update_balance(
        const identifier::Nym& owner,
        const Chain chain,
        Balance balance) noexcept -> void;
    auto process_update_chain_balance(Message&& in) noexcept -> void;
    auto process_update_nym_balance(Message&& in) noexcept -> void;
    auto startup() noexcept -> void {}
    auto work() noexcept -> bool { return false; }
};
}  // namespace opentxs::api::crypto::blockchain
