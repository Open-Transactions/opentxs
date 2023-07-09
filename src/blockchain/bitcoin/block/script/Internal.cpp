// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/bitcoin/block/Script.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::bitcoin::block::internal
{
auto Script::blank_signature(const blockchain::Type chain) noexcept
    -> const Space&
{
    static const auto output = space(72);

    return output;
}

auto Script::blank_pubkey(
    const blockchain::Type chain,
    const bool mode) noexcept -> const Space&
{
    static const auto compressed = space(33);
    static const auto uncompressed = space(65);

    return mode ? compressed : uncompressed;
}

auto Script::CalculateHash160(const api::Crypto&, Writer&&) const noexcept
    -> bool
{
    return {};
}

auto Script::CalculateSize() const noexcept -> std::size_t { return {}; }

auto Script::ExtractElements(const cfilter::Type, Elements&) const noexcept
    -> void
{
}

auto Script::get() const noexcept -> std::span<const script::Element>
{
    return {};
}

auto Script::IndexElements(const api::Session&, ElementHashes&) const noexcept
    -> void
{
}

auto Script::IsNotification(const std::uint8_t, const PaymentCode&)
    const noexcept -> bool
{
    return {};
}

auto Script::IsValid() const noexcept -> bool { return {}; }

auto Script::LikelyPubkeyHashes(const api::Crypto&) const noexcept
    -> UnallocatedVector<ByteArray>
{
    return {};
}

auto Script::M() const noexcept -> std::optional<std::uint8_t> { return {}; }

auto Script::MultisigPubkey(const std::size_t) const noexcept
    -> std::optional<ReadView>
{
    return {};
}

auto Script::N() const noexcept -> std::optional<std::uint8_t> { return {}; }

auto Script::Print() const noexcept -> UnallocatedCString { return {}; }

auto Script::Print(alloc::Strategy alloc) const noexcept -> CString
{
    return CString{alloc.result_};
}

auto Script::Pubkey() const noexcept -> std::optional<ReadView> { return {}; }

auto Script::PubkeyHash() const noexcept -> std::optional<ReadView>
{
    return {};
}

auto Script::RedeemScript(alloc::Strategy) const noexcept -> block::Script
{
    return {};
}

auto Script::Role() const noexcept -> script::Position { return {}; }

auto Script::ScriptHash() const noexcept -> std::optional<ReadView>
{
    return {};
}

auto Script::Serialize(Writer&&) const noexcept -> bool { return {}; }

auto Script::SigningSubscript(const blockchain::Type, alloc::Strategy alloc)
    const noexcept -> block::Script
{
    return {alloc.result_};
}

auto Script::Type() const noexcept -> script::Pattern { return {}; }

auto Script::Value(const std::size_t) const noexcept -> std::optional<ReadView>
{
    return {};
}
}  // namespace opentxs::blockchain::bitcoin::block::internal
