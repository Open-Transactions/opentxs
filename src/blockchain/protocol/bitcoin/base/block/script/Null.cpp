// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::ContactItemType

#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/protocol/bitcoin/base/block/script/ScriptPrivate.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"

namespace opentxs::factory
{
auto BitcoinScript(
    const blockchain::Type,
    ReadView,
    const blockchain::protocol::bitcoin::base::block::script::Position,
    const bool,
    const bool,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return blockchain::protocol::bitcoin::base::block::ScriptPrivate::Blank(
        alloc.result_);
}

auto BitcoinScript(
    const blockchain::Type,
    Vector<blockchain::protocol::bitcoin::base::block::script::Element>,
    const blockchain::protocol::bitcoin::base::block::script::Position,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return blockchain::protocol::bitcoin::base::block::ScriptPrivate::Blank(
        alloc.result_);
}

auto BitcoinScriptNullData(
    const blockchain::Type,
    std::span<const ReadView>,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return blockchain::protocol::bitcoin::base::block::ScriptPrivate::Blank(
        alloc.result_);
}

auto BitcoinScriptP2MS(
    const blockchain::Type,
    const std::uint8_t,
    const std::uint8_t,
    std::span<const opentxs::crypto::asymmetric::key::EllipticCurve*> keys,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return blockchain::protocol::bitcoin::base::block::ScriptPrivate::Blank(
        alloc.result_);
}

auto BitcoinScriptP2PK(
    const blockchain::Type,
    const opentxs::crypto::asymmetric::key::EllipticCurve&,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return blockchain::protocol::bitcoin::base::block::ScriptPrivate::Blank(
        alloc.result_);
}

auto BitcoinScriptP2PKH(
    const api::Crypto& crypto,
    const blockchain::Type,
    const opentxs::crypto::asymmetric::key::EllipticCurve&,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return blockchain::protocol::bitcoin::base::block::ScriptPrivate::Blank(
        alloc.result_);
}

auto BitcoinScriptP2SH(
    const api::Crypto& crypto,
    const blockchain::Type,
    const blockchain::protocol::bitcoin::base::block::Script&,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return blockchain::protocol::bitcoin::base::block::ScriptPrivate::Blank(
        alloc.result_);
}

auto BitcoinScriptP2WPKH(
    const api::Crypto& crypto,
    const blockchain::Type,
    const opentxs::crypto::asymmetric::key::EllipticCurve&,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return blockchain::protocol::bitcoin::base::block::ScriptPrivate::Blank(
        alloc.result_);
}

auto BitcoinScriptP2WSH(
    const api::Crypto& crypto,
    const blockchain::Type,
    const blockchain::protocol::bitcoin::base::block::Script&,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return blockchain::protocol::bitcoin::base::block::ScriptPrivate::Blank(
        alloc.result_);
}
}  // namespace opentxs::factory
