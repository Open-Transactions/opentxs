// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"          // IWYU pragma: associated
#include "1_Internal.hpp"        // IWYU pragma: associated
#include "crypto/bip32/Imp.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <stdexcept>

#include "internal/api/Crypto.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/HDIndex.hpp"

namespace opentxs::crypto
{
Bip32::Imp::Imp(const api::Crypto& crypto) noexcept
    : crypto_(crypto)
    , blank_()
{
}

auto Bip32::Imp::ckd_hardened(
    const HDNode& node,
    const be::big_uint32_buf_t i,
    const WritableView& data) const noexcept -> void
{
    static const auto padding = std::byte{0};
    auto out{data.as<std::byte>()};
    std::memcpy(out, &padding, sizeof(padding));
    std::advance(out, 1);
    std::memcpy(out, node.ParentPrivate().data(), 32);
    std::advance(out, 32);
    std::memcpy(out, &i, sizeof(i));
}

auto Bip32::Imp::ckd_normal(
    const HDNode& node,
    const be::big_uint32_buf_t i,
    const WritableView& data) const noexcept -> void
{
    auto out{data.as<std::byte>()};
    std::memcpy(out, node.ParentPublic().data(), 33);
    std::advance(out, 33);
    std::memcpy(out, &i, sizeof(i));
}

auto Bip32::Imp::decode(const UnallocatedCString& serialized) const noexcept
    -> OTData
{
    auto input = crypto_.Encode().IdentifierDecode(serialized);

    return Data::Factory(input.c_str(), input.size());
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
        reader(data),
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
            reader(node.ChildPrivate()(32)), node.ChildPublic());

        if (false == success) {
            LogError()(OT_PRETTY_CLASS())("Failed to calculate public key")
                .Flush();

            return false;
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }

    auto code = hash.as<std::byte>();
    std::advance(code, 32);
    std::memcpy(node.ChildCode().data(), code, 32);
    node.Next();

    return true;
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
        reader(data),
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

    auto code = hash.as<std::byte>();
    std::advance(code, 32);
    std::memcpy(node.ChildCode().data(), code, 32);
    node.Next();

    return true;
}

auto Bip32::Imp::DeserializePrivate(
    const UnallocatedCString& serialized,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode,
    Secret& key) const -> bool
{
    const auto input = decode(serialized);
    const auto size = input->size();

    if (78 != size) {
        LogError()(OT_PRETTY_CLASS())("Invalid input size (")(size)(")")
            .Flush();

        return {};
    }

    bool output = extract(input, network, depth, parent, index, chainCode);

    if (std::byte(0) != input->at(45)) {
        LogError()(OT_PRETTY_CLASS())("Invalid padding bit").Flush();

        return {};
    }

    key.Assign(&input->at(46), 32);

    return output;
}

auto Bip32::Imp::DeserializePublic(
    const UnallocatedCString& serialized,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode,
    Data& key) const -> bool
{
    const auto input = decode(serialized);
    const auto size = input->size();

    if (78 != size) {
        LogError()(OT_PRETTY_CLASS())("Invalid input size (")(size)(")")
            .Flush();

        return {};
    }

    bool output = extract(input, network, depth, parent, index, chainCode);
    output &= input->Extract(33, key, 45);

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

auto Bip32::Imp::Init(const api::Factory& factory) noexcept -> void
{
    auto& blank = const_cast<std::optional<Key>&>(blank_);
    blank.emplace(
        factory.Secret(0), factory.Secret(0), Data::Factory(), Path{}, 0);
}

auto Bip32::Imp::IsHard(const Bip32Index index) noexcept -> bool
{
    static const auto hard = Bip32Index{HDIndex{Bip32Child::HARDENED}};

    return index >= hard;
}

auto Bip32::Imp::provider(const EcdsaCurve& curve) const noexcept
    -> const crypto::EcdsaProvider&
{
    using Key = key::asymmetric::Algorithm;

    switch (curve) {
        case EcdsaCurve::ed25519: {

            return crypto_.Internal().EllipticProvider(Key::ED25519);
        }
        case EcdsaCurve::secp256k1: {

            return crypto_.Internal().EllipticProvider(Key::Secp256k1);
        }
        default: {

            return crypto_.Internal().EllipticProvider(Key::Error);
        }
    }
}

auto Bip32::Imp::SeedID(const ReadView entropy) const -> OTIdentifier
{
    auto output = Identifier::Factory();
    output->CalculateDigest(entropy);

    return output;
}

auto Bip32::Imp::SerializePrivate(
    const Bip32Network network,
    const Bip32Depth depth,
    const Bip32Fingerprint parent,
    const Bip32Index index,
    const Data& chainCode,
    const Secret& key) const -> UnallocatedCString
{
    const auto size = key.size();

    if (32 != size) {
        LogError()(OT_PRETTY_CLASS())("Invalid key size (")(size)(")").Flush();

        return {};
    }

    auto input = Data::Factory();  // TODO should be secret
    input->DecodeHex("0x00");

    OT_ASSERT(1 == input->size());

    input->Concatenate(key.Bytes());

    OT_ASSERT(33 == input->size());

    return SerializePublic(network, depth, parent, index, chainCode, input);
}

auto Bip32::Imp::SerializePublic(
    const Bip32Network network,
    const Bip32Depth depth,
    const Bip32Fingerprint parent,
    const Bip32Index index,
    const Data& chainCode,
    const Data& key) const -> UnallocatedCString
{
    auto size = key.size();

    if (33 != size) {
        LogError()(OT_PRETTY_CLASS())("Invalid key size (")(size)(")").Flush();

        return {};
    }

    size = chainCode.size();

    if (32 != size) {
        LogError()(OT_PRETTY_CLASS())("Invalid chain code size (")(size)(")")
            .Flush();

        return {};
    }

    auto output = Data::Factory(network);
    output.get() += depth;
    output.get() += parent;
    output.get() += index;
    output += chainCode;
    output += key;

    OT_ASSERT_MSG(78 == output->size(), std::to_string(output->size()).c_str());

    return crypto_.Encode().IdentifierEncode(output);
}
}  // namespace opentxs::crypto
