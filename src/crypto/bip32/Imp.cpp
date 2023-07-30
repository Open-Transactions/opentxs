// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/bip32/Imp.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <span>
#include <stdexcept>
#include <utility>

#include "crypto/asymmetric/key/hd/HDPrivate.hpp"
#include "internal/api/Crypto.hpp"
#include "internal/core/Core.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/crypto/library/EcdsaProvider.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/HDSeed.hpp"
#include "opentxs/crypto/Bip32Child.hpp"            // IWYU pragma: keep
#include "opentxs/crypto/HashType.hpp"              // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/HDIndex.hpp"

namespace opentxs::crypto
{
Bip32::Imp::Imp(const api::Crypto& crypto) noexcept
    : crypto_(crypto)
    , factory_()
    , blank_()
{
}

auto Bip32::Imp::ckd_hardened(
    const HDNode& node,
    const be::big_uint32_buf_t i,
    WriteBuffer& data) const noexcept -> void
{
    static const auto padding = std::byte{0};
    auto* out = data.as<std::byte>();
    std::memcpy(out, &padding, sizeof(padding));
    std::advance(out, 1);
    std::memcpy(out, node.ParentPrivate().data(), 32);
    std::advance(out, 32);
    std::memcpy(out, &i, sizeof(i));
}

auto Bip32::Imp::ckd_normal(
    const HDNode& node,
    const be::big_uint32_buf_t i,
    WriteBuffer& data) const noexcept -> void
{
    auto* out = data.as<std::byte>();
    std::memcpy(out, node.ParentPublic().data(), 33);
    std::advance(out, 33);
    std::memcpy(out, &i, sizeof(i));
}

auto Bip32::Imp::decode(std::string_view in) const noexcept -> ByteArray
{
    auto out = ByteArray{};

    if (crypto_.Encode().Base58CheckDecode(in, out.WriteInto())) {

        return out;
    } else {

        return {};
    }
}

auto Bip32::Imp::DerivePrivateKey(
    const asymmetric::key::HD& key,
    const Path& pathAppend,
    const PasswordPrompt& reason) const noexcept(false) -> Key
{
    return derive_private_key(
        key.Type(),
        [&] {
            auto out = proto::HDPath{};
            key.Internal().Path(out);

            return out;
        }(),
        key.PrivateKey(reason),
        key.Chaincode(reason),
        key.PublicKey(),
        pathAppend,
        reason);
}

auto Bip32::Imp::DerivePrivateKey(
    const asymmetric::key::HDPrivate& key,
    const Path& pathAppend,
    const PasswordPrompt& reason) const noexcept(false) -> Key
{
    return derive_private_key(
        key.Type(),
        [&] {
            auto out = proto::HDPath{};
            key.Path(out);

            return out;
        }(),
        key.PrivateKey(reason),
        key.Chaincode(reason),
        key.PublicKey(),
        pathAppend,
        reason);
}

auto Bip32::Imp::derive_private(
    HDNode& node,
    Bip32Fingerprint& parent,
    const Bip32Index child) const noexcept -> bool
{
    auto& hash = node.hash_;
    auto& data = node.data_;
    parent = node.Fingerprint();
    auto i = be::big_uint32_buf_t{child};

    if (IsHard(child)) {
        ckd_hardened(node, i, data);
    } else {
        ckd_normal(node, i, data);
    }

    auto success = crypto_.Hash().HMAC(
        crypto::HashType::Sha512,
        node.ParentCode(),
        data,
        preallocated(hash.size(), hash.data()));

    if (false == success) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate hash").Flush();

        return false;
    }

    try {
        const auto& ecdsa = provider(EcdsaCurve::secp256k1);
        success = ecdsa.ScalarAdd(
            node.ParentPrivate(), {hash.as<char>(), 32}, node.ChildPrivate());

        if (false == success) {
            LogError()(OT_PRETTY_CLASS())("Invalid scalar").Flush();

            return false;
        }

        success = ecdsa.ScalarMultiplyBase(
            node.ChildPrivate().Reserve(32), node.ChildPublic());

        if (false == success) {
            LogError()(OT_PRETTY_CLASS())("Failed to calculate public key")
                .Flush();

            return false;
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }

    auto* code = hash.as<std::byte>();
    std::advance(code, 32);
    std::memcpy(node.ChildCode().data(), code, 32);
    node.Next();

    return true;
}

auto Bip32::Imp::DerivePublicKey(
    const asymmetric::key::HD& key,
    const Path& pathAppend,
    const PasswordPrompt& reason) const noexcept(false) -> Key
{
    return derive_public_key(
        key.Type(),
        [&] {
            auto out = proto::HDPath{};
            key.Internal().Path(out);

            return out;
        }(),
        key.Chaincode(reason),
        key.PublicKey(),
        pathAppend,
        reason);
}

auto Bip32::Imp::DerivePublicKey(
    const asymmetric::key::HDPrivate& key,
    const Path& pathAppend,
    const PasswordPrompt& reason) const noexcept(false) -> Key
{
    return derive_public_key(
        key.Type(),
        [&] {
            auto out = proto::HDPath{};
            key.Path(out);

            return out;
        }(),
        key.Chaincode(reason),
        key.PublicKey(),
        pathAppend,
        reason);
}

auto Bip32::Imp::derive_public(
    HDNode& node,
    Bip32Fingerprint& parent,
    const Bip32Index child) const noexcept -> bool
{
    auto& hash = node.hash_;
    auto& data = node.data_;
    parent = node.Fingerprint();
    auto i = be::big_uint32_buf_t{child};

    if (IsHard(child)) {
        LogError()(OT_PRETTY_CLASS())(
            "Hardened public derivation is not possible")
            .Flush();

        return false;
    } else {
        ckd_normal(node, i, data);
    }

    auto success = crypto_.Hash().HMAC(
        crypto::HashType::Sha512,
        node.ParentCode(),
        data,
        preallocated(hash.size(), hash.data()));

    if (false == success) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate hash").Flush();

        return false;
    }

    try {
        const auto& ecdsa = provider(EcdsaCurve::secp256k1);
        success = ecdsa.PubkeyAdd(
            node.ParentPublic(), {hash.as<char>(), 32}, node.ChildPublic());

        if (false == success) {
            LogError()(OT_PRETTY_CLASS())("Failed to calculate public key")
                .Flush();

            return false;
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }

    auto* code = hash.as<std::byte>();
    std::advance(code, 32);
    std::memcpy(node.ChildCode().data(), code, 32);
    node.Next();

    return true;
}

auto Bip32::Imp::DeserializePrivate(
    std::string_view serialized,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode,
    Secret& key) const -> bool
{
    const auto input = decode(serialized);
    const auto size = input.size();

    if (78 != size) {
        LogError()(OT_PRETTY_CLASS())("Invalid input size (")(size)(")")
            .Flush();

        return {};
    }

    bool output = extract(input, network, depth, parent, index, chainCode);

    if (std::byte(0) != input.get()[45]) {
        LogError()(OT_PRETTY_CLASS())("Invalid padding bit").Flush();

        return {};
    }

    key.Assign(&input.get()[46], 32);

    return output;
}

auto Bip32::Imp::DeserializePublic(
    std::string_view serialized,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode,
    Data& key) const -> bool
{
    const auto input = decode(serialized);
    const auto size = input.size();

    if (78 != size) {
        LogError()(OT_PRETTY_CLASS())("Invalid input size (")(size)(")")
            .Flush();

        return {};
    }

    bool output = extract(input, network, depth, parent, index, chainCode);
    output &= input.Extract(33, key, 45);

    return output;
}

auto Bip32::Imp::extract(
    const Data& input,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode) const noexcept -> bool
{
    bool output{true};
    output &= input.Extract(network);
    output &= input.Extract(depth, 4);
    output &= input.Extract(parent, 5);
    output &= input.Extract(index, 9);
    output &= input.Extract(32, chainCode, 13);

    return output;
}

auto Bip32::Imp::Init(
    const std::shared_ptr<const api::Factory>& factory) noexcept -> void
{
    OT_ASSERT(factory);

    factory_ = factory;
    blank_.set_value(
        Key{factory->Secret(0), factory->Secret(0), ByteArray{}, Path{}, 0});
}

auto Bip32::Imp::IsHard(const Bip32Index index) noexcept -> bool
{
    static const auto hard = Bip32Index{HDIndex{Bip32Child::HARDENED}};

    return index >= hard;
}

auto Bip32::Imp::provider(const EcdsaCurve& curve) const noexcept
    -> const crypto::EcdsaProvider&
{
    using enum asymmetric::Algorithm;

    switch (curve) {
        case EcdsaCurve::ed25519: {

            return crypto_.Internal().EllipticProvider(ED25519);
        }
        case EcdsaCurve::secp256k1: {

            return crypto_.Internal().EllipticProvider(Secp256k1);
        }
        case EcdsaCurve::invalid:
        default: {

            return crypto_.Internal().EllipticProvider(Error);
        }
    }
}

auto Bip32::Imp::SeedID(const ReadView entropy) const noexcept -> crypto::SeedID
{
    const auto f = factory_.lock();

    OT_ASSERT(f);

    return f->SeedIDFromPreimage(entropy);
}

auto Bip32::Imp::SerializePrivate(
    Bip32Network network,
    Bip32Depth depth,
    Bip32Fingerprint parent,
    Bip32Index index,
    ReadView chainCode,
    ReadView key,
    Writer&& out) const noexcept -> bool
{
    const auto size = key.size();

    if (32_uz != size) {
        LogError()(OT_PRETTY_CLASS())("Invalid key size (")(size)(")").Flush();

        return {};
    }

    auto input = factory::Secret(0_uz);
    input.DecodeHex("0x00");

    OT_ASSERT(1_uz == input.size());

    input.Concatenate(key);

    OT_ASSERT(33_uz == input.size());

    return SerializePublic(
        network,
        depth,
        parent,
        index,
        chainCode,
        input.Bytes(),
        std::move(out));
}

auto Bip32::Imp::SerializePublic(
    Bip32Network network,
    Bip32Depth depth,
    Bip32Fingerprint parent,
    Bip32Index index,
    ReadView chainCode,
    ReadView key,
    Writer&& out) const noexcept -> bool
{
    auto size = key.size();

    if (33_uz != size) {
        LogError()(OT_PRETTY_CLASS())("Invalid key size (")(size)(")").Flush();

        return {};
    }

    size = chainCode.size();

    if (32_uz != size) {
        LogError()(OT_PRETTY_CLASS())("Invalid chain code size (")(size)(")")
            .Flush();

        return {};
    }

    auto output = ByteArray{network};
    output += depth;
    output += parent;
    output += index;
    output += chainCode;
    output += key;

    OT_ASSERT_MSG(78 == output.size(), std::to_string(output.size()).c_str());

    return crypto_.Encode().Base58CheckEncode(output.Bytes(), std::move(out));
}

Bip32::Imp::~Imp() = default;
}  // namespace opentxs::crypto
