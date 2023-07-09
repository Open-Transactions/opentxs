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
    alloc::Strategy alloc) const noexcept -> Set<identifier::Nym>
{
    return {alloc.result_};
}

auto Transaction::AssociatedRemoteContacts(
    const api::session::Client&,
    const identifier::Nym&,
    alloc::Strategy alloc) const noexcept -> Set<identifier::Generic>
{
    return {alloc.result_};
}

auto Transaction::BlockPosition() const noexcept -> std::optional<std::size_t>
{
    return {};
}

auto Transaction::Chains(alloc::Strategy alloc) const noexcept
    -> Set<blockchain::Type>
{
    return Set<blockchain::Type>{alloc.result_};
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

auto Transaction::Keys(alloc::Strategy alloc) const noexcept -> Set<crypto::Key>
{
    return Set<crypto::Key>{alloc.result_};
}

auto Transaction::Memo(const api::crypto::Blockchain&) const noexcept
    -> UnallocatedCString
{
    return {};
}

auto Transaction::Memo(const api::crypto::Blockchain&, alloc::Strategy alloc)
    const noexcept -> CString
{
    return CString{alloc.result_};
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

auto Transaction::Print(const api::Crypto&, alloc::Strategy alloc)
    const noexcept -> CString
{
    return CString{alloc.result_};
}
}  // namespace opentxs::blockchain::block::internal
