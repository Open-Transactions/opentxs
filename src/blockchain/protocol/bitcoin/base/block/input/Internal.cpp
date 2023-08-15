// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/base/block/Input.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Input.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
{
auto Input::AddMultisigSignatures(const Signatures&) noexcept -> bool
{
    return {};
}

auto Input::AddSignatures(const Signatures&) noexcept -> bool { return {}; }

auto Input::AssociatePreviousOutput(const block::Output&) noexcept -> bool
{
    return {};
}

auto Input::AssociatedLocalNyms(
    const api::crypto::Blockchain&,
    Set<identifier::Nym>&) const noexcept -> void
{
}

auto Input::AssociatedRemoteContacts(
    const api::session::Client&,
    Set<identifier::Generic>&) const noexcept -> void
{
}

auto Input::CalculateSize(const bool) const noexcept -> std::size_t
{
    return {};
}

auto Input::Coinbase() const noexcept -> ReadView { return {}; }

auto Input::ExtractElements(const cfilter::Type, Elements&) const noexcept
    -> void
{
}

auto Input::FindMatches(
    const api::Session&,
    const TransactionHash&,
    const cfilter::Type,
    const Patterns&,
    const ParsedPatterns&,
    const std::size_t,
    const Log&,
    Matches&,
    alloc::Default) const noexcept -> void
{
}

auto Input::GetBytes(std::size_t&, std::size_t&) const noexcept -> void {}

auto Input::IndexElements(const api::Session&, ElementHashes&) const noexcept
    -> void
{
}

auto Input::IsValid() const noexcept -> bool { return {}; }

auto Input::Keys(Set<crypto::Key>&) const noexcept -> void {}

auto Input::Keys(alloc::Default alloc) const noexcept -> Set<crypto::Key>
{
    return Set<crypto::Key>{alloc};
}

auto Input::MergeMetadata(
    const api::Crypto&,
    const Input&,
    const std::size_t,
    const Log&) noexcept -> void
{
}

auto Input::NetBalanceChange(
    const api::crypto::Blockchain&,
    const identifier::Nym&,
    const std::size_t,
    const Log&) const noexcept -> opentxs::Amount
{
    return {};
}

auto Input::PreviousOutput() const noexcept
    -> const blockchain::block::Outpoint&
{
    static const auto blank = blockchain::block::Outpoint{};

    return blank;
}

auto Input::Print(const api::Crypto&) const noexcept -> UnallocatedCString
{
    return {};
}

auto Input::Print(const api::Crypto&, alloc::Default alloc) const noexcept
    -> CString
{
    return CString{alloc};
}

auto Input::ReplaceScript() noexcept -> bool { return {}; }

auto Input::Script() const noexcept -> const block::Script&
{
    return block::Script::Blank();
}

auto Input::Sequence() const noexcept -> std::uint32_t { return {}; }

auto Input::Serialize(Writer&&) const noexcept -> std::optional<std::size_t>
{
    return {};
}

auto Input::Serialize(const api::Session&, const std::uint32_t, SerializeType&)
    const noexcept -> bool
{
    return {};
}

auto Input::SerializeNormalized(Writer&&) const noexcept
    -> std::optional<std::size_t>
{
    return {};
}

auto Input::SetKeyData(const KeyData&) noexcept -> void {}

auto Input::SignatureVersion(alloc::Default alloc) const noexcept
    -> block::Input
{
    return {alloc};
}

auto Input::SignatureVersion(block::Script, alloc::Default alloc) const noexcept
    -> block::Input
{
    return {alloc};
}

auto Input::Spends() const noexcept(false) -> const block::Output&
{
    return block::Output::Blank();
}

auto Input::Witness() const noexcept -> std::span<const WitnessItem>
{
    return {};
}
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
