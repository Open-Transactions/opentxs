// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/crypto/Imported.hpp"
#include "opentxs/blockchain/protocol/ethereum/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
class Ethereum : virtual public Imported
{
public:
    static auto Blank() noexcept -> Ethereum&;

    virtual auto Balance() const noexcept -> Amount;
    virtual auto KnownTransactions(alloc::Strategy alloc) const noexcept
        -> Set<protocol::ethereum::AccountNonce>;
    virtual auto MissingTransactions(alloc::Strategy alloc) const noexcept
        -> Set<protocol::ethereum::AccountNonce>;
    virtual auto NextNonce() const noexcept -> protocol::ethereum::AccountNonce;
    virtual auto UpdateBalance(
        const Amount& balance,
        protocol::ethereum::AccountNonce nonce) const noexcept -> bool;

    Ethereum() = default;
    Ethereum(const Ethereum&) = delete;
    Ethereum(Ethereum&&) = delete;
    auto operator=(const Ethereum&) -> Ethereum& = delete;
    auto operator=(Ethereum&&) -> Ethereum& = delete;

    ~Ethereum() override = default;
};
}  // namespace opentxs::blockchain::crypto::internal