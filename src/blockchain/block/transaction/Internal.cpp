// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/block/Transaction.hpp"  // IWYU pragma: associated

#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::block::internal
{
auto Transaction::asBitcoin() const noexcept
    -> const bitcoin::block::internal::Transaction&
{
    return bitcoin::block::internal::Transaction::Blank();
}

auto Transaction::asBitcoin() noexcept -> bitcoin::block::internal::Transaction&
{
    return bitcoin::block::internal::Transaction::Blank();
}

auto Transaction::AssociatedLocalNyms(
    const api::crypto::Blockchain&,
    alloc::Default alloc) const noexcept -> Set<identifier::Nym>
{
    return {alloc};
}

auto Transaction::AssociatedRemoteContacts(
    const api::session::Client&,
    const identifier::Nym&,
    alloc::Default alloc) const noexcept -> Set<identifier::Generic>
{
    return {alloc};
}

auto Transaction::BlockPosition() const noexcept -> std::optional<std::size_t>
{
    return {};
}

auto Transaction::Chains(alloc::Default alloc) const noexcept
    -> Set<blockchain::Type>
{
    return Set<blockchain::Type>{alloc};
}

auto Transaction::Hash() const noexcept -> const TransactionHash&
{
    static const auto blank = TransactionHash{};

    return blank;
}

auto Transaction::ID() const noexcept -> const TransactionHash&
{
    return Hash();
}

auto Transaction::IsValid() const noexcept -> bool { return {}; }

auto Transaction::Keys(alloc::Default alloc) const noexcept -> Set<crypto::Key>
{
    return Set<crypto::Key>{alloc};
}

auto Transaction::Memo(const api::crypto::Blockchain&) const noexcept
    -> UnallocatedCString
{
    return {};
}

auto Transaction::Memo(const api::crypto::Blockchain&, alloc::Default alloc)
    const noexcept -> CString
{
    return CString{alloc};
}

auto Transaction::NetBalanceChange(
    const api::crypto::Blockchain&,
    const identifier::Nym&) const noexcept -> opentxs::Amount
{
    return {};
}

auto Transaction::Print(const api::Crypto&) const noexcept -> UnallocatedCString
{
    return {};
}

auto Transaction::Print(const api::Crypto&, alloc::Default alloc) const noexcept
    -> CString
{
    return CString{alloc};
}
}  // namespace opentxs::blockchain::block::internal
