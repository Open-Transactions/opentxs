// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
{
auto Output::AddTag(node::TxoTag) noexcept -> void {}

auto Output::AssociatedLocalNyms(
    const api::crypto::Blockchain&,
    Set<identifier::Nym>&) const noexcept -> void
{
}

auto Output::AssociatedRemoteContacts(
    const api::session::Client&,
    Set<identifier::Generic>&) const noexcept -> void
{
}

auto Output::CalculateSize() const noexcept -> std::size_t { return {}; }

auto Output::Cashtoken() const noexcept -> const token::cashtoken::View*
{
    return nullptr;
}

auto Output::ExtractElements(const cfilter::Type, Elements&) const noexcept
    -> void
{
}

auto Output::FindMatches(
    const api::Session&,
    const TransactionHash&,
    const cfilter::Type,
    const ParsedPatterns&,
    const Log&,
    Matches&,
    alloc::Default) const noexcept -> void
{
}

auto Output::ForTestingOnlyAddKey(const crypto::Key&) noexcept -> void {}

auto Output::IndexElements(const api::Session&, ElementHashes&) const noexcept
    -> void
{
}

auto Output::IsValid() const noexcept -> bool { return {}; }

auto Output::Keys(Set<crypto::Key>&) const noexcept -> void {}

auto Output::Keys(alloc::Default alloc) const noexcept -> Set<crypto::Key>
{
    return Set<crypto::Key>{alloc};
}

auto Output::MergeMetadata(
    const api::Crypto&,
    const Output&,
    const Log&) noexcept -> void
{
}

auto Output::MinedPosition() const noexcept -> const block::Position&
{
    static const auto blank = block::Position{};

    return blank;
}

auto Output::NetBalanceChange(
    const api::crypto::Blockchain&,
    const identifier::Nym&,
    const Log&) const noexcept -> opentxs::Amount
{
    return Amount{};
}

auto Output::Note(const api::crypto::Blockchain& crypto) const noexcept
    -> UnallocatedCString
{
    return {};
}

auto Output::Note(const api::crypto::Blockchain& crypto, alloc::Default alloc)
    const noexcept -> CString
{
    return {};
}

auto Output::Payee() const noexcept -> ContactID { return {}; }

auto Output::Payer() const noexcept -> ContactID { return {}; }

auto Output::Print(const api::Crypto&) const noexcept -> UnallocatedCString
{
    return {};
}

auto Output::Print(const api::Crypto&, alloc::Default alloc) const noexcept
    -> CString
{
    return CString{alloc};
}

auto Output::Script() const noexcept -> const block::Script&
{
    return block::Script::Blank();
}

auto Output::Serialize(Writer&&) const noexcept -> std::optional<std::size_t>
{
    return {};
}

auto Output::Serialize(const api::Session&, SerializeType&) const noexcept
    -> bool
{
    return {};
}

auto Output::SetIndex(const std::uint32_t) noexcept -> void {}

auto Output::SetKeyData(const KeyData&) noexcept -> void {}

auto Output::SetMinedPosition(const block::Position&) noexcept -> void {}

auto Output::SetPayee(const identifier::Generic&) noexcept -> void {}

auto Output::SetPayer(const identifier::Generic&) noexcept -> void {}

auto Output::SetState(node::TxoState) noexcept -> void {}

auto Output::SetValue(const Amount&) noexcept -> void {}

auto Output::SigningSubscript(alloc::Default alloc) const noexcept
    -> block::Script
{
    return block::Script{alloc};
}

auto Output::State() const noexcept -> node::TxoState { return {}; }

auto Output::Tags() const noexcept -> const UnallocatedSet<node::TxoTag>
{
    return {};
}

auto Output::Value() const noexcept -> Amount { return {}; }
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
