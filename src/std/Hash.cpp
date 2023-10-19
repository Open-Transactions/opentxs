// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstring>
#include <functional>
#include <iterator>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "blockchain/database/wallet/Pattern.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/crypto/Parameters.hpp"
#include "internal/serialization/protobuf/Contact.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/cfilter/Hash.hpp"
#include "opentxs/blockchain/cfilter/Header.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/token/Descriptor.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/HDSeed.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/Seed.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Sodium.hpp"

namespace opentxs
{
template <typename T>
auto cryptographic_fixed_hash_to_std(const T& val) noexcept -> std::size_t
{
    auto out = 0_uz;

    static_assert(sizeof(out) <= T::payload_size_);

    std::memcpy(std::addressof(out), val.data(), sizeof(out));

    return out;
}

template <typename T>
auto cryptographic_hash_to_std(const T& val) noexcept -> std::size_t
{
    auto out = 0_uz;

    std::memcpy(
        std::addressof(out), val.data(), std::min(sizeof(out), val.size()));

    return out;
}
}  // namespace opentxs

namespace std
{
auto hash<opentxs::blockchain::block::Block>::operator()(
    const opentxs::blockchain::block::Block& data) const noexcept -> std::size_t
{
    return hash<opentxs::blockchain::block::Hash>{}(data.Header().Hash());
}

auto hash<opentxs::blockchain::block::Hash>::operator()(
    const opentxs::blockchain::block::Hash& data) const noexcept -> std::size_t
{
    return opentxs::cryptographic_fixed_hash_to_std(data);
}

auto hash<opentxs::blockchain::block::Header>::operator()(
    const opentxs::blockchain::block::Header& data) const noexcept
    -> std::size_t
{
    return hash<opentxs::blockchain::block::Hash>{}(data.Hash());
}

auto hash<opentxs::blockchain::block::Transaction>::operator()(
    const opentxs::blockchain::block::Transaction& data) const noexcept
    -> std::size_t
{
    return hash<opentxs::blockchain::block::TransactionHash>{}(data.ID());
}

auto hash<opentxs::blockchain::block::TransactionHash>::operator()(
    const opentxs::blockchain::block::TransactionHash& data) const noexcept
    -> std::size_t
{
    return opentxs::cryptographic_fixed_hash_to_std(data);
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
    const auto& [height, blockhash] = data;
    const auto bytes = blockhash.Bytes();

    return opentxs::crypto::sodium::Siphash(
        opentxs::crypto::sodium::MakeSiphashKey(
            {reinterpret_cast<const char*>(&height), sizeof(height)}),
        {bytes.data(), std::min(bytes.size(), sizeof(std::size_t))});
}

auto hash<opentxs::blockchain::cfilter::Hash>::operator()(
    const opentxs::blockchain::cfilter::Hash& data) const noexcept
    -> std::size_t
{
    return opentxs::cryptographic_fixed_hash_to_std(data);
}

auto hash<opentxs::blockchain::cfilter::Header>::operator()(
    const opentxs::blockchain::cfilter::Header& data) const noexcept
    -> std::size_t
{
    return opentxs::cryptographic_fixed_hash_to_std(data);
}

auto hash<opentxs::blockchain::crypto::Key>::operator()(
    const opentxs::blockchain::crypto::Key& data) const noexcept -> std::size_t
{
    const auto preimage = opentxs::serialize(data);
    static const auto key = opentxs::crypto::sodium::SiphashKey{};

    return opentxs::crypto::sodium::Siphash(key, opentxs::reader(preimage));
}

auto hash<opentxs::blockchain::crypto::Target>::operator()(
    const opentxs::blockchain::crypto::Target& in) const noexcept -> std::size_t
{
    struct Visitor {
        auto operator()(opentxs::blockchain::Type in) const noexcept
        {
            return hash<opentxs::blockchain::Type>{}(in);
        }
        auto operator()(
            const opentxs::blockchain::token::Descriptor& in) const noexcept
        {
            return hash<opentxs::blockchain::token::Descriptor>{}(in);
        }
    };

    return std::visit(Visitor{}, in);
}

auto hash<opentxs::blockchain::database::wallet::db::Pattern>::operator()(
    const opentxs::blockchain::database::wallet::db::Pattern& data)
    const noexcept -> std::size_t
{
    static const auto key = opentxs::crypto::sodium::SiphashKey{};

    return opentxs::crypto::sodium::Siphash(key, opentxs::reader(data.data_));
}

auto hash<opentxs::blockchain::params::ChainData::ZMQParams>::operator()(
    const opentxs::blockchain::params::ChainData::ZMQParams& data)
    const noexcept -> std::size_t
{
    using namespace opentxs::literals;
    auto key = opentxs::crypto::sodium::SiphashKey{
        'o',
        't',
        'x',
        '_',
        'z',
        'm',
        'q',
        '_',
        'b',
        'l',
        'o',
        'c',
        'k',
        'c',
        '_',
        '\0',
    };
    const auto& [bip44, subchain] = data;
    static_assert(sizeof(subchain) == 1_uz);
    std::memcpy(
        std::next(key.data(), key.size() - 1_uz),
        std::addressof(subchain),
        sizeof(subchain));

    return opentxs::crypto::sodium::Siphash(
        key,
        opentxs::ReadView{
            reinterpret_cast<const char*>(std::addressof(bip44)),
            sizeof(bip44)});
}

auto hash<opentxs::blockchain::token::Descriptor>::operator()(
    const opentxs::blockchain::token::Descriptor& data) const noexcept
    -> std::size_t
{
    using Host = std::underlying_type<decltype(data.host_)>::type;
    using Token = std::underlying_type<decltype(data.type_)>::type;
    using Buffer = boost::endian::little_uint32_buf_t;

    static_assert(sizeof(Host) == sizeof(Buffer));
    static_assert(sizeof(Token) == sizeof(Buffer));

    const auto host = Buffer{static_cast<Host>(data.host_)};
    const auto type = Buffer{static_cast<Token>(data.type_)};
    auto key = opentxs::crypto::sodium::SiphashKey{};

    static_assert((sizeof(host) + sizeof(type)) < sizeof(key));

    std::memcpy(key.data(), std::addressof(host), sizeof(host));
    std::memcpy(
        std::next(key.data(), sizeof(host)),
        std::addressof(type),
        sizeof(type));

    return opentxs::crypto::sodium::Siphash(key, data.id_.Bytes());
}

auto hash<opentxs::contract::peer::Reply>::operator()(
    const opentxs::contract::peer::Reply& rhs) const noexcept -> std::size_t
{
    static constexpr auto hasher =
        hash<opentxs::contract::peer::Reply::identifier_type>{};

    return hasher(rhs.ID());
}

auto hash<opentxs::contract::peer::Request>::operator()(
    const opentxs::contract::peer::Request& rhs) const noexcept -> std::size_t
{
    static constexpr auto hasher =
        hash<opentxs::contract::peer::Request::identifier_type>{};

    return hasher(rhs.ID());
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
    return hash<opentxs::crypto::Seed::identifier_type>{}(rhs.ID());
}

auto hash<opentxs::identity::wot::Claim>::operator()(
    const opentxs::identity::wot::Claim& rhs) const noexcept -> std::size_t
{
    static constexpr auto hasher =
        hash<opentxs::identity::wot::Claim::identifier_type>{};

    return hasher(rhs.ID());
}

auto hash<opentxs::network::blockchain::Address>::operator()(
    const opentxs::network::blockchain::Address& rhs) const noexcept
    -> std::size_t
{
    return hash<opentxs::identifier::Generic>{}(rhs.ID());
}

auto hash<opentxs::network::zeromq::Envelope>::operator()(
    const opentxs::network::zeromq::Envelope& data) const noexcept
    -> std::size_t
{
    auto key = opentxs::crypto::sodium::SiphashKey{
        'o',
        't',
        'x',
        '_',
        'z',
        'm',
        'q',
        '_',
        'e',
        'n',
        'v',
        'e',
        'l',
        'o',
        'p',
        'e',
    };
    const auto frames = data.get();

    switch (frames.size()) {
        case 0: {

            return opentxs::crypto::sodium::Siphash(key, {});
        }
        case 1: {

            return opentxs::crypto::sodium::Siphash(key, frames[0].Bytes());
        }
        default: {
            using namespace opentxs::literals;
            auto out = opentxs::crypto::sodium::Siphash(key, frames[0].Bytes());

            for (auto n = 1_uz, stop = frames.size(); n < stop; ++n) {
                std::memcpy(
                    key.data(),
                    std::addressof(out),
                    std::min(key.size(), sizeof(out)));
                out = opentxs::crypto::sodium::Siphash(key, frames[n].Bytes());
            }

            return out;
        }
    }
}

auto hash<opentxs::network::zeromq::Frame>::operator()(
    const opentxs::network::zeromq::Frame& data) const noexcept -> std::size_t
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

auto hash<opentxs::ByteArray>::operator()(
    const opentxs::ByteArray& data) const noexcept -> std::size_t
{
    static const auto key = opentxs::crypto::sodium::SiphashKey{};

    return opentxs::crypto::sodium::Siphash(key, data.Bytes());
}

auto hash<opentxs::FixedByteArray<16>>::operator()(
    const opentxs::FixedByteArray<16>& data) const noexcept -> std::size_t
{
    return hash<opentxs::ByteArray>{}(data);
}

auto hash<opentxs::FixedByteArray<24>>::operator()(
    const opentxs::FixedByteArray<24>& data) const noexcept -> std::size_t
{
    return hash<opentxs::ByteArray>{}(data);
}

auto hash<opentxs::FixedByteArray<32>>::operator()(
    const opentxs::FixedByteArray<32>& data) const noexcept -> std::size_t
{
    return hash<opentxs::ByteArray>{}(data);
}

auto hash<opentxs::identifier::Account>::operator()(
    const opentxs::identifier::Account& data) const noexcept -> std::size_t
{
    return hash<opentxs::identifier::Generic>{}(data);
}

auto hash<opentxs::identifier::Generic>::operator()(
    const opentxs::identifier::Generic& data) const noexcept -> std::size_t
{
    return opentxs::cryptographic_hash_to_std(data);
}

auto hash<opentxs::identifier::HDSeed>::operator()(
    const opentxs::identifier::HDSeed& data) const noexcept -> std::size_t
{
    return hash<opentxs::identifier::Generic>{}(data);
}

auto hash<opentxs::identifier::Notary>::operator()(
    const opentxs::identifier::Notary& data) const noexcept -> std::size_t
{
    return hash<opentxs::identifier::Generic>{}(data);
}

auto hash<opentxs::identifier::Nym>::operator()(
    const opentxs::identifier::Nym& data) const noexcept -> std::size_t
{
    return hash<opentxs::identifier::Generic>{}(data);
}

auto hash<opentxs::identifier::UnitDefinition>::operator()(
    const opentxs::identifier::UnitDefinition& data) const noexcept
    -> std::size_t
{
    return hash<opentxs::identifier::Generic>{}(data);
}

auto hash<opentxs::PaymentCode>::operator()(
    const opentxs::PaymentCode& rhs) const noexcept -> std::size_t
{
    return hash<opentxs::identifier::Generic>{}(rhs.ID());
}
}  // namespace std
