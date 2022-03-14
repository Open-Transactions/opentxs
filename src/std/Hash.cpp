// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "blockchain/database/wallet/Pattern.hpp"
#include "internal/crypto/Parameters.hpp"
#include "internal/network/zeromq/message/FrameIterator.hpp"
#include "internal/serialization/protobuf/Contact.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/Seed.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/Sodium.hpp"

namespace std
{
#if OT_BLOCKCHAIN
auto hash<opentxs::blockchain::block::Outpoint>::operator()(
    const opentxs::blockchain::block::Outpoint& data) const noexcept
    -> std::size_t
{
    const auto key = data.Index();

    return opentxs::crypto::sodium::Siphash(
        opentxs::crypto::sodium::MakeSiphashKey(
            {reinterpret_cast<const char*>(&key), sizeof(key)}),
        data.Txid());
}
#endif  // OT_BLOCKCHAIN

auto hash<opentxs::blockchain::block::Position>::operator()(
    const opentxs::blockchain::block::Position& data) const noexcept
    -> std::size_t
{
    const auto& [height, hash] = data;
    const auto bytes = hash->Bytes();

    return opentxs::crypto::sodium::Siphash(
        opentxs::crypto::sodium::MakeSiphashKey(
            {reinterpret_cast<const char*>(&height), sizeof(height)}),
        {bytes.data(), std::min(bytes.size(), sizeof(std::size_t))});
}

#if OT_BLOCKCHAIN
auto hash<opentxs::blockchain::crypto::Key>::operator()(
    const opentxs::blockchain::crypto::Key& data) const noexcept -> std::size_t
{
    const auto preimage = opentxs::preimage(data);
    static const auto key = opentxs::crypto::sodium::SiphashKey{};

    return opentxs::crypto::sodium::Siphash(key, opentxs::reader(preimage));
}
#endif  // OT_BLOCKCHAIN

auto hash<opentxs::blockchain::database::wallet::db::Pattern>::operator()(
    const opentxs::blockchain::database::wallet::db::Pattern& data)
    const noexcept -> std::size_t
{
    static const auto key = opentxs::crypto::sodium::SiphashKey{};

    return opentxs::crypto::sodium::Siphash(key, opentxs::reader(data.data_));
}

auto hash<opentxs::crypto::Parameters>::operator()(
    const opentxs::crypto::Parameters& rhs) const noexcept -> std::size_t
{
    static const auto key = opentxs::crypto::sodium::SiphashKey{};
    const auto preimage = rhs.Internal().Hash();

    return opentxs::crypto::sodium::Siphash(key, preimage->Bytes());
}

auto hash<opentxs::crypto::Seed>::operator()(
    const opentxs::crypto::Seed& rhs) const noexcept -> std::size_t
{
    static const auto hasher = hash<opentxs::OTIdentifier>{};

    return hasher(rhs.ID());
}

auto hash<opentxs::network::zeromq::Frame>::operator()(
    const opentxs::network::zeromq::Frame& data) const noexcept -> std::size_t
{
    static const auto key = opentxs::crypto::sodium::SiphashKey{};

    return opentxs::crypto::sodium::Siphash(key, data.Bytes());
}

auto hash<opentxs::network::zeromq::FrameIterator>::operator()(
    const opentxs::network::zeromq::FrameIterator& rhs) const noexcept
    -> std::size_t
{
    return rhs.Internal().hash();
}

auto hash<opentxs::OTData>::operator()(const opentxs::Data& data) const noexcept
    -> std::size_t
{
    static const auto key = opentxs::crypto::sodium::SiphashKey{};

    return opentxs::crypto::sodium::Siphash(key, data.Bytes());
}

auto hash<opentxs::proto::ContactSectionVersion>::operator()(
    const opentxs::proto::ContactSectionVersion& data) const noexcept
    -> std::size_t
{
    const auto& [key, value] = data;

    return opentxs::crypto::sodium::Siphash(
        opentxs::crypto::sodium::MakeSiphashKey(
            {reinterpret_cast<const char*>(&key), sizeof(key)}),
        {reinterpret_cast<const char*>(&value), sizeof(value)});
}

auto hash<opentxs::proto::EnumLang>::operator()(
    const opentxs::proto::EnumLang& data) const noexcept -> std::size_t
{
    const auto& [key, text] = data;

    return opentxs::crypto::sodium::Siphash(
        opentxs::crypto::sodium::MakeSiphashKey(
            {reinterpret_cast<const char*>(&key), sizeof(key)}),
        text);
}

auto hash<opentxs::Amount>::operator()(
    const opentxs::Amount& data) const noexcept -> std::size_t
{
    static const auto key = opentxs::crypto::sodium::SiphashKey{};
    const auto buffer = [&] {
        auto out = opentxs::Space{};
        out.reserve(100);
        data.Serialize(opentxs::writer(out));

        return out;
    }();

    return opentxs::crypto::sodium::Siphash(key, opentxs::reader(buffer));
}

auto hash<opentxs::OTIdentifier>::operator()(
    const opentxs::Identifier& data) const noexcept -> std::size_t
{
    auto out = std::size_t{};
    std::memcpy(&out, data.data(), std::min(sizeof(out), data.size()));

    return out;
}

auto hash<opentxs::OTNotaryID>::operator()(
    const opentxs::identifier::Notary& data) const noexcept -> std::size_t
{
    static const auto hasher = hash<opentxs::OTIdentifier>{};

    return hasher(data);
}

auto hash<opentxs::OTNymID>::operator()(
    const opentxs::identifier::Nym& data) const noexcept -> std::size_t
{
    static const auto hasher = hash<opentxs::OTIdentifier>{};

    return hasher(data);
}

auto hash<opentxs::OTUnitID>::operator()(
    const opentxs::identifier::UnitDefinition& data) const noexcept
    -> std::size_t
{
    static const auto hasher = hash<opentxs::OTIdentifier>{};

    return hasher(data);
}

auto hash<opentxs::PaymentCode>::operator()(
    const opentxs::PaymentCode& rhs) const noexcept -> std::size_t
{
    static const auto hasher = hash<opentxs::OTIdentifier>{};

    return hasher(rhs.ID());
}
}  // namespace std
