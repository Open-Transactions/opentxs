// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <optional>

#include "internal/blockchain/database/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.internal.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class Client;
}  // namespace session

class Crypto;
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
namespace internal
{
class Transaction;
}  // namespace internal
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::internal
{
class Transaction
{
public:
    virtual auto asBitcoin() const noexcept
        -> const protocol::bitcoin::base::block::internal::Transaction&;
    virtual auto AssociatedLocalNyms(
        const api::crypto::Blockchain& crypto,
        alloc::Default alloc) const noexcept -> Set<identifier::Nym>;
    virtual auto AssociatedRemoteContacts(
        const api::session::Client& api,
        const identifier::Nym& nym,
        alloc::Default alloc) const noexcept -> Set<identifier::Generic>;
    virtual auto BlockPosition() const noexcept -> std::optional<std::size_t>;
    virtual auto Chains(alloc::Default alloc) const noexcept
        -> Set<blockchain::Type>;
    virtual auto Hash() const noexcept -> const TransactionHash&;
    virtual auto ID() const noexcept -> const TransactionHash&;
    virtual auto IsValid() const noexcept -> bool;
    virtual auto Keys(alloc::Default alloc) const noexcept -> Set<crypto::Key>;
    virtual auto Memo(const api::crypto::Blockchain& crypto) const noexcept
        -> UnallocatedCString;
    virtual auto Memo(
        const api::crypto::Blockchain& crypto,
        alloc::Default alloc) const noexcept -> CString;
    virtual auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount;
    virtual auto Print(const api::Crypto& crypto) const noexcept
        -> UnallocatedCString;
    virtual auto Print(const api::Crypto& crypto, alloc::Default alloc)
        const noexcept -> CString;

    virtual auto asBitcoin() noexcept
        -> protocol::bitcoin::base::block::internal::Transaction&;
    virtual auto ConfirmMatches(
        const Log& log,
        const api::crypto::Blockchain& api,
        const Matches& candiates,
        database::BlockMatches& out,
        alloc::Strategy alloc) noexcept -> void;
    virtual auto SetMinedPosition(const block::Position& pos) noexcept -> void;

    virtual ~Transaction() = default;
};
}  // namespace opentxs::blockchain::block::internal
