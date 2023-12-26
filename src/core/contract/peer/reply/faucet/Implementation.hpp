// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <BlockchainTransaction.pb.h>
#include <cs_plain_guarded.h>
#include <optional>

#include "core/contract/peer/reply/base/Implementation.hpp"
#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "core/contract/peer/reply/faucet/FaucetPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/reply/Faucet.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply::faucet
{
class Implementation final : public FaucetPrivate, public base::Implementation
{
public:
    [[nodiscard]] auto Accepted() const noexcept -> bool final
    {
        return accepted_;
    }
    [[nodiscard]] auto asFaucetPublic() const& noexcept
        -> const reply::Faucet& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::ReplyPrivate* final
    {
        return pmr::clone(this, alloc::PMR<Implementation>{alloc});
    }
    [[nodiscard]] auto Transaction() const noexcept
        -> const blockchain::block::Transaction& final
    {
        return transaction_;
    }

    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Implementation(
        const api::Session& api,
        Nym_p signer,
        identifier::Nym initiator,
        identifier::Nym responder,
        peer::Request::identifier_type ref,
        blockchain::block::Transaction transaction,
        allocator_type alloc) noexcept(false);
    Implementation(
        const api::Session& api,
        Nym_p signer,
        const serialized_type& proto,
        allocator_type alloc) noexcept(false);
    Implementation() = delete;
    Implementation(const Implementation& rhs, allocator_type alloc) noexcept;
    Implementation(const Implementation&) = delete;
    Implementation(Implementation&&) = delete;
    auto operator=(const Implementation&) -> Implementation& = delete;
    auto operator=(Implementation&&) -> Implementation& = delete;

    ~Implementation() final;

private:
    using OptionalTransaction = std::optional<proto::BlockchainTransaction>;
    using GuardedTransaction = libguarded::plain_guarded<OptionalTransaction>;

    static constexpr auto default_version_ = VersionNumber{4};

    const blockchain::block::TransactionHash txid_;
    const blockchain::block::Transaction transaction_;
    mutable GuardedTransaction tx_serialized_;
    const bool accepted_;
    reply::Faucet self_;

    auto id_form() const noexcept -> serialized_type final;
};
}  // namespace opentxs::contract::peer::reply::faucet
