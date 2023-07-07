// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <BlockchainTransaction.pb.h>
#include <cs_plain_guarded.h>
#include <optional>

#include "core/contract/peer/reply/Base.hpp"
#include "internal/core/contract/peer/reply/Base.hpp"
#include "internal/core/contract/peer/reply/Faucet.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

class Factory;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply::implementation
{
class Faucet final : public reply::internal::Faucet,
                     public implementation::Reply
{
public:
    auto asFaucet() const noexcept -> const internal::Faucet& final
    {
        return *this;
    }

    Faucet(
        const api::Session& api,
        const Nym_p& nym,
        const SerializedType& serialized);
    Faucet(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const blockchain::block::Transaction& transaction);
    Faucet() = delete;
    Faucet(const Faucet&);
    Faucet(Faucet&&) = delete;
    auto operator=(const Faucet&) -> Faucet& = delete;
    auto operator=(Faucet&&) -> Faucet& = delete;

    ~Faucet() final = default;

private:
    friend opentxs::Factory;

    using OptionalTransaction = std::optional<proto::BlockchainTransaction>;
    using GuardedTransaction = libguarded::plain_guarded<OptionalTransaction>;

    static constexpr auto current_version_ = VersionNumber{4};

    const blockchain::block::TransactionHash txid_;
    const blockchain::block::Transaction transaction_;
    mutable GuardedTransaction tx_serialized_;

    auto IDVersion() const -> SerializedType final;
};
}  // namespace opentxs::contract::peer::reply::implementation
