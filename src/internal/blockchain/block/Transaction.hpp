// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <optional>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
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
namespace bitcoin
{
namespace block
{
namespace internal
{
class Transaction;
}  // namespace internal
}  // namespace block
}  // namespace bitcoin

namespace block
{
class TransactionHash;
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::internal
{
class Transaction
{
public:
    virtual auto asBitcoin() const noexcept
        -> const bitcoin::block::internal::Transaction&;
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

    virtual auto asBitcoin() noexcept -> bitcoin::block::internal::Transaction&;

    virtual ~Transaction() = default;
};
}  // namespace opentxs::blockchain::block::internal
