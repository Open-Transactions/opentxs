// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Imported.hpp"
#include "opentxs/blockchain/protocol/ethereum/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
class Subaccount;
}  // namespace internal
}  // namespace crypto
}  // namespace blockchain

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT Ethereum : public Imported
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Ethereum&;

    auto Balance() const noexcept -> Amount;
    auto KnownIncoming(alloc::Strategy alloc) const noexcept
        -> Set<block::TransactionHash>;
    auto KnownOutgoing(alloc::Strategy alloc) const noexcept
        -> Set<block::TransactionHash>;
    auto MissingOutgoing(alloc::Strategy alloc) const noexcept
        -> Set<protocol::ethereum::AccountNonce>;
    auto NextOutgoing() const noexcept -> protocol::ethereum::AccountNonce;

    auto AddIncoming(
        const Amount& balance,
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool;
    auto AddIncoming(
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool;
    auto AddOutgoing(
        const Amount& balance,
        protocol::ethereum::AccountNonce nonce,
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool;
    auto AddOutgoing(
        protocol::ethereum::AccountNonce nonce,
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool;
    auto UpdateBalance(const Amount& balance) noexcept -> bool;

    OPENTXS_NO_EXPORT Ethereum(
        std::shared_ptr<internal::Subaccount> imp) noexcept;
    Ethereum() = delete;
    Ethereum(const Ethereum& rhs) noexcept;
    Ethereum(Ethereum&& rhs) noexcept;
    auto operator=(const Ethereum&) -> Ethereum& = delete;
    auto operator=(Ethereum&&) -> Ethereum& = delete;

    ~Ethereum() override;
};
}  // namespace opentxs::blockchain::crypto
