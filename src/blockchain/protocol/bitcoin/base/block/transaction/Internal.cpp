// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <utility>

#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
{
auto Transaction::Blank() noexcept -> Transaction&
{
    static auto blank = Transaction{};

    return blank;
}

auto Transaction::AssociatePreviousOutput(
    const std::size_t,
    const block::Output&) noexcept -> bool
{
    return {};
}

auto Transaction::CalculateSize() const noexcept -> std::size_t { return {}; }

auto Transaction::ConfirmationHeight() const noexcept -> block::Height
{
    return {};
}

auto Transaction::ExtractElements(const cfilter::Type, Elements&) const noexcept
    -> void
{
}

auto Transaction::FindMatches(
    const api::Session&,
    const cfilter::Type,
    const Patterns&,
    const ParsedPatterns&,
    const Log&,
    Matches&,
    alloc::Default) const noexcept -> void
{
}

auto Transaction::FindMatches(
    const api::Session&,
    const cfilter::Type,
    const Patterns&,
    const ParsedPatterns&,
    const Log&,
    alloc::Default alloc,
    alloc::Default) const noexcept -> Matches
{
    return std::make_pair(InputMatches{alloc}, OutputMatches{alloc});
}

auto Transaction::ForTestingOnlyAddKey(
    const std::size_t,
    const blockchain::crypto::Key&) noexcept -> bool
{
    return {};
}

auto Transaction::GetPreimageBTC(
    const std::size_t,
    const blockchain::protocol::bitcoin::base::SigHash&) const noexcept -> Space
{
    return {};
}

auto Transaction::IDNormalized(const api::Factory&) const noexcept
    -> const identifier::Generic&
{
    static const auto blank = identifier::Generic{};

    return blank;
}

auto Transaction::IndexElements(const api::Session&, alloc::Default alloc)
    const noexcept -> ElementHashes
{
    return ElementHashes{alloc};
}

auto Transaction::Inputs() const noexcept -> std::span<const block::Input>
{
    return {};
}

auto Transaction::IsGeneration() const noexcept -> bool { return {}; }

auto Transaction::Locktime() const noexcept -> std::uint32_t { return {}; }

auto Transaction::MergeMetadata(
    const api::crypto::Blockchain&,
    const blockchain::Type,
    const Transaction&,
    const Log&) noexcept -> void
{
}

auto Transaction::MinedPosition() const noexcept -> const block::Position&
{
    static const auto blank = block::Position{};

    return blank;
}

auto Transaction::Outputs() const noexcept -> std::span<const block::Output>
{
    return {};
}

auto Transaction::SegwitFlag() const noexcept -> std::byte { return {}; }

auto Transaction::Serialize(EncodedTransaction&) const noexcept -> bool
{
    return {};
}

auto Transaction::Serialize(Writer&&) const noexcept
    -> std::optional<std::size_t>
{
    return std::nullopt;
}

auto Transaction::Serialize(const api::Session&) const noexcept
    -> std::optional<SerializeType>
{
    return std::nullopt;
}

auto Transaction::SetKeyData(const KeyData&) noexcept -> void {}

auto Transaction::SetMemo(const std::string_view) noexcept -> void {}

auto Transaction::SetMinedPosition(const block::Position&) noexcept -> void {}

auto Transaction::SetPosition(std::size_t) noexcept -> void {}

auto Transaction::Timestamp() const noexcept -> Time { return {}; }

auto Transaction::Version() const noexcept -> std::int32_t { return {}; }

auto Transaction::vBytes(blockchain::Type) const noexcept -> std::size_t
{
    return {};
}
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
