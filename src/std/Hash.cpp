// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated

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
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/FixedByteArray.hpp"
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
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Sodium.hpp"

namespace std
{
using namespace opentxs::literals;  // NOLINT(cert-dcl58-cpp)

auto hash<opentxs::blockchain::block::Hash>::operator()(
    const opentxs::blockchain::block::Hash& data) const noexcept -> std::size_t
{
    // NOTE block hashes are cryptographic so no further hashing is required

    auto out = 0_uz;

    static_assert(
        sizeof(out) <= opentxs::blockchain::block::Hash::payload_size_);

    std::memcpy(&out, data.data(), sizeof(out));

    return out;
}

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

auto hash<opentxs::blockchain::block::Position>::operator()(
    const opentxs::blockchain::block::Position& data) const noexcept
    -> std::size_t
{
    const auto& [height, hash] = data;
    const auto bytes = hash.Bytes();

    return opentxs::crypto::sodium::Siphash(
        opentxs::crypto::sodium::MakeSiphashKey(
            {reinterpret_cast<const char*>(&height), sizeof(height)}),
        {bytes.data(), std::min(bytes.size(), sizeof(std::size_t))});
}

auto hash<opentxs::blockchain::cfilter::Hash>::operator()(
    const opentxs::blockchain::cfilter::Hash& data) const noexcept
    -> std::size_t
{
    // NOTE cfilter hashes are cryptographic so no further hashing is required

    auto out = 0_uz;

    static_assert(
        sizeof(out) <= opentxs::blockchain::cfilter::Hash::payload_size_);

    std::memcpy(&out, data.data(), sizeof(out));

    return out;
}

auto hash<opentxs::blockchain::cfilter::Header>::operator()(
    const opentxs::blockchain::cfilter::Header& data) const noexcept
    -> std::size_t
{
    // NOTE cfheaders are cryptographic so no further hashing is required

    auto out = 0_uz;

    static_assert(
        sizeof(out) <= opentxs::blockchain::cfilter::Header::payload_size_);

    std::memcpy(&out, data.data(), sizeof(out));

    return out;
}

auto hash<opentxs::blockchain::crypto::Key>::operator()(
    const opentxs::blockchain::crypto::Key& data) const noexcept -> std::size_t
{
    const auto preimage = opentxs::serialize(data);
    static const auto key = opentxs::crypto::sodium::SiphashKey{};

    return opentxs::crypto::sodium::Siphash(key, opentxs::reader(preimage));
}

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

    return opentxs::crypto::sodium::Siphash(key, preimage.Bytes());
}

auto hash<opentxs::crypto::Seed>::operator()(
    const opentxs::crypto::Seed& rhs) const noexcept -> std::size_t
{
    static const auto hasher = hash<opentxs::identifier::Generic>{};

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

auto hash<opentxs::ByteArray>::operator()(
    const opentxs::ByteArray& data) const noexcept -> std::size_t
{
    static const auto key = opentxs::crypto::sodium::SiphashKey{};

    return opentxs::crypto::sodium::Siphash(key, data.Bytes());
}

auto hash<opentxs::FixedByteArray<16>>::operator()(
    const opentxs::FixedByteArray<16>& data) const noexcept -> std::size_t
{
    static const auto hasher = hash<opentxs::ByteArray>{};

    return hasher(data);
}

auto hash<opentxs::FixedByteArray<24>>::operator()(
    const opentxs::FixedByteArray<24>& data) const noexcept -> std::size_t
{
    static const auto hasher = hash<opentxs::ByteArray>{};

    return hasher(data);
}

auto hash<opentxs::FixedByteArray<32>>::operator()(
    const opentxs::FixedByteArray<32>& data) const noexcept -> std::size_t
{
    static const auto hasher = hash<opentxs::ByteArray>{};

    return hasher(data);
}

auto hash<opentxs::identifier::Generic>::operator()(
    const opentxs::identifier::Generic& data) const noexcept -> std::size_t
{
    auto out = 0_uz;
    std::memcpy(&out, data.data(), std::min(sizeof(out), data.size()));

    return out;
}

auto hash<opentxs::identifier::Notary>::operator()(
    const opentxs::identifier::Notary& data) const noexcept -> std::size_t
{
    static const auto hasher = hash<opentxs::identifier::Generic>{};

    return hasher(data);
}

auto hash<opentxs::identifier::Nym>::operator()(
    const opentxs::identifier::Nym& data) const noexcept -> std::size_t
{
    static const auto hasher = hash<opentxs::identifier::Generic>{};

    return hasher(data);
}

auto hash<opentxs::identifier::UnitDefinition>::operator()(
    const opentxs::identifier::UnitDefinition& data) const noexcept
    -> std::size_t
{
    static const auto hasher = hash<opentxs::identifier::Generic>{};

    return hasher(data);
}

auto hash<opentxs::PaymentCode>::operator()(
    const opentxs::PaymentCode& rhs) const noexcept -> std::size_t
{
    static const auto hasher = hash<opentxs::identifier::Generic>{};

    return hasher(rhs.ID());
}
}  // namespace std
