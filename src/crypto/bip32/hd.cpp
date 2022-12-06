// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::HashType
// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Algorithm

#include "crypto/bip32/Imp.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "internal/crypto/library/EcdsaProvider.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"              // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto
{
auto Bip32::Imp::DeriveKey(
    const EcdsaCurve& curve,
    const Secret& seed,
    const Path& path) const -> Key
{
    auto output{blank_.get()};

    try {
        auto& [privateKey, chainCode, publicKey, pathOut, parent] = output;
        pathOut = path;
        auto node = [&] {
            auto ret = HDNode{crypto_};
            const auto init = root_node(
                EcdsaCurve::secp256k1,
                seed.Bytes(),
                ret.InitPrivate(),
                ret.InitCode(),
                ret.InitPublic());

            if (false == init) {
                throw std::runtime_error("Failed to derive root node");
            }

            ret.check();

            return ret;
        }();

        for (const auto& child : path) {
            if (false == derive_private(node, parent, child)) {
                throw std::runtime_error("Failed to derive child node");
            }
        }

        node.Assign(curve, output);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return output;
}

auto Bip32::Imp::derive_private_key(
    const asymmetric::Algorithm type,
    const proto::HDPath& path,
    const ReadView parentPrivate,
    const ReadView parentChaincode,
    const ReadView parentPublic,
    const Path& pathAppend,
    const PasswordPrompt& reason) const noexcept(false) -> Key
{
    const auto curve = [&] {
        if (asymmetric::Algorithm::ED25519 == type) {

            return EcdsaCurve::ed25519;
        } else {

            return EcdsaCurve::secp256k1;
        }
    }();
    auto output = [&] {
        auto out{blank_.get()};
        auto& [privateKey, chainCode, publicKey, pathOut, parent] = out;

        for (const auto& child : path.child()) { pathOut.emplace_back(child); }

        for (const auto child : pathAppend) { pathOut.emplace_back(child); }

        return out;
    }();

    try {
        auto& [privateKey, chainCode, publicKey, pathOut, parent] = output;
        auto node = [&] {
            auto ret = HDNode{crypto_};

            if (false == copy(parentPrivate, ret.InitPrivate())) {
                throw std::runtime_error("Failed to initialize public key");
            }

            if (false == copy(parentChaincode, ret.InitCode())) {
                throw std::runtime_error("Failed to initialize chain code");
            }

            if (false == copy(parentPublic, ret.InitPublic())) {
                throw std::runtime_error("Failed to initialize public key");
            }

            ret.check();

            return ret;
        }();

        for (const auto& child : pathAppend) {
            if (false == derive_private(node, parent, child)) {
                throw std::runtime_error("Failed to derive child node");
            }
        }

        node.Assign(curve, output);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return output;
}

auto Bip32::Imp::derive_public_key(
    const asymmetric::Algorithm type,
    const proto::HDPath& path,
    const ReadView parentChaincode,
    const ReadView parentPublic,
    const Path& pathAppend,
    const PasswordPrompt& reason) const noexcept(false) -> Key
{
    const auto curve = [&] {
        if (crypto::asymmetric::Algorithm::ED25519 == type) {

            return EcdsaCurve::ed25519;
        } else {

            return EcdsaCurve::secp256k1;
        }
    }();
    auto output = [&] {
        auto out{blank_.get()};
        auto& [privateKey, chainCode, publicKey, pathOut, parent] = out;

        for (const auto& child : path.child()) { pathOut.emplace_back(child); }

        for (const auto child : pathAppend) { pathOut.emplace_back(child); }

        return out;
    }();

    try {
        auto& [privateKey, chainCode, publicKey, pathOut, parent] = output;
        auto node = [&] {
            auto ret = HDNode{crypto_};

            if (false == copy(parentChaincode, ret.InitCode())) {
                throw std::runtime_error("Failed to initialize chain code");
            }

            if (false == copy(parentPublic, ret.InitPublic())) {
                throw std::runtime_error("Failed to initialize public key");
            }

            ret.check();

            return ret;
        }();

        for (const auto& child : pathAppend) {
            if (false == derive_public(node, parent, child)) {
                throw std::runtime_error("Failed to derive child node");
            }
        }

        node.Assign(curve, output);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return output;
}

auto Bip32::Imp::root_node(
    const EcdsaCurve& curve,
    const ReadView entropy,
    Writer&& key,
    Writer&& code,
    Writer&& pub) const noexcept -> bool
{
    if ((16 > entropy.size()) || (64 < entropy.size())) {
        LogError()(OT_PRETTY_CLASS())("Invalid entropy size (")(entropy.size())(
            ")")
            .Flush();

        return false;
    }

    constexpr auto keyBytes = 32_uz;
    auto keyOut = key.Reserve(keyBytes);
    auto codeOut = code.Reserve(keyBytes);

    if (false == keyOut.IsValid(keyBytes) ||
        false == codeOut.IsValid(keyBytes)) {
        LogError()(OT_PRETTY_CLASS())("failed to allocate output space")
            .Flush();

        return false;
    }

    static const auto rootKey = UnallocatedCString{"Bitcoin seed"};
    auto node = Space{};

    if (false ==
        crypto_.Hash().HMAC(
            crypto::HashType::Sha512, rootKey, entropy, writer(node))) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate root node")
            .Flush();

        return false;
    }

    OT_ASSERT(64_uz == node.size());

    auto* start{node.data()};
    std::memcpy(keyOut, start, keyBytes);
    std::advance(start, keyBytes);
    std::memcpy(codeOut, start, keyBytes);
    const auto havePub = provider(curve).ScalarMultiplyBase(
        {reinterpret_cast<const char*>(node.data()), keyBytes}, std::move(pub));

    try {
        if (false == havePub) {
            LogError()(OT_PRETTY_CLASS())("Failed to calculate root public key")
                .Flush();

            return false;
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::crypto
