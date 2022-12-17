// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ContactItemType

#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/bitcoin/block/script/ScriptPrivate.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"

namespace opentxs::factory
{
auto BitcoinScript(
    const blockchain::Type,
    ReadView,
    const blockchain::bitcoin::block::script::Position,
    const bool,
    const bool,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script
{
    return blockchain::bitcoin::block::ScriptPrivate::Blank(alloc);
}

auto BitcoinScript(
    const blockchain::Type,
    std::span<blockchain::bitcoin::block::script::Element>,
    const blockchain::bitcoin::block::script::Position,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script
{
    return blockchain::bitcoin::block::ScriptPrivate::Blank(alloc);
}

auto BitcoinScriptNullData(
    const blockchain::Type,
    std::span<const ReadView>,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script
{
    return blockchain::bitcoin::block::ScriptPrivate::Blank(alloc);
}

auto BitcoinScriptP2MS(
    const blockchain::Type,
    const std::uint8_t,
    const std::uint8_t,
    std::span<const opentxs::crypto::asymmetric::key::EllipticCurve*> keys,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script
{
    return blockchain::bitcoin::block::ScriptPrivate::Blank(alloc);
}

auto BitcoinScriptP2PK(
    const blockchain::Type,
    const opentxs::crypto::asymmetric::key::EllipticCurve&,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script
{
    return blockchain::bitcoin::block::ScriptPrivate::Blank(alloc);
}

auto BitcoinScriptP2PKH(
    const api::Crypto& crypto,
    const blockchain::Type,
    const opentxs::crypto::asymmetric::key::EllipticCurve&,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script
{
    return blockchain::bitcoin::block::ScriptPrivate::Blank(alloc);
}

auto BitcoinScriptP2SH(
    const api::Crypto& crypto,
    const blockchain::Type,
    const blockchain::bitcoin::block::Script&,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script
{
    return blockchain::bitcoin::block::ScriptPrivate::Blank(alloc);
}

auto BitcoinScriptP2WPKH(
    const api::Crypto& crypto,
    const blockchain::Type,
    const opentxs::crypto::asymmetric::key::EllipticCurve&,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script
{
    return blockchain::bitcoin::block::ScriptPrivate::Blank(alloc);
}

auto BitcoinScriptP2WSH(
    const api::Crypto& crypto,
    const blockchain::Type,
    const blockchain::bitcoin::block::Script&,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script
{
    return blockchain::bitcoin::block::ScriptPrivate::Blank(alloc);
}
}  // namespace opentxs::factory
