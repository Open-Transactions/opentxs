// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"          // IWYU pragma: associated
#include "1_Internal.hpp"        // IWYU pragma: associated
#include "crypto/bip32/Imp.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "crypto/HDNode.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

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

auto Bip32::Imp::DerivePrivateKey(
    const key::HD& key,
    const Path& pathAppend,
    const PasswordPrompt& reason) const noexcept(false) -> Key
{
    const auto curve = [&] {
        if (opentxs::crypto::key::asymmetric::Algorithm::ED25519 ==
            key.keyType()) {

            return EcdsaCurve::ed25519;
        } else {

            return EcdsaCurve::secp256k1;
        }
    }();
    auto output = [&] {
        auto out{blank_.get()};
        auto& [privateKey, chainCode, publicKey, pathOut, parent] = out;
        auto path = proto::HDPath{};

        if (key.Path(path)) {
            for (const auto& child : path.child()) {
                pathOut.emplace_back(child);
            }
        }

        for (const auto child : pathAppend) { pathOut.emplace_back(child); }

        return out;
    }();

    try {
        auto& [privateKey, chainCode, publicKey, pathOut, parent] = output;
        auto node = [&] {
            auto ret = HDNode{crypto_};
            const auto privateKey = key.PrivateKey(reason);
            const auto chainCode = key.Chaincode(reason);
            const auto publicKey = key.PublicKey();

            if (false == copy(privateKey, ret.InitPrivate())) {
                throw std::runtime_error("Failed to initialize public key");
            }

            if (false == copy(chainCode, ret.InitCode())) {
                throw std::runtime_error("Failed to initialize chain code");
            }

            if (false == copy(publicKey, ret.InitPublic())) {
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

auto Bip32::Imp::DerivePublicKey(
    const key::HD& key,
    const Path& pathAppend,
    const PasswordPrompt& reason) const noexcept(false) -> Key
{
    const auto curve = [&] {
        if (crypto::key::asymmetric::Algorithm::ED25519 == key.keyType()) {

            return EcdsaCurve::ed25519;
        } else {

            return EcdsaCurve::secp256k1;
        }
    }();
    auto output = [&] {
        auto out{blank_.get()};
        auto& [privateKey, chainCode, publicKey, pathOut, parent] = out;
        auto path = proto::HDPath{};

        if (key.Path(path)) {
            for (const auto& child : path.child()) {
                pathOut.emplace_back(child);
            }
        }

        for (const auto child : pathAppend) { pathOut.emplace_back(child); }

        return out;
    }();

    try {
        auto& [privateKey, chainCode, publicKey, pathOut, parent] = output;
        auto node = [&] {
            auto ret = HDNode{crypto_};
            const auto chainCode = key.Chaincode(reason);
            const auto publicKey = key.PublicKey();

            if (false == copy(chainCode, ret.InitCode())) {
                throw std::runtime_error("Failed to initialize chain code");
            }

            if (false == copy(publicKey, ret.InitPublic())) {
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
    const AllocateOutput key,
    const AllocateOutput code,
    const AllocateOutput pub) const noexcept -> bool
{
    if ((16 > entropy.size()) || (64 < entropy.size())) {
        LogError()(OT_PRETTY_CLASS())("Invalid entropy size (")(entropy.size())(
            ")")
            .Flush();

        return false;
    }

    if (false == bool(key) || false == bool(code)) {
        LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

        return false;
    }

    auto keyOut = key(32);
    auto codeOut = code(32);

    if (false == keyOut.valid(32) || false == codeOut.valid(32)) {
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

    OT_ASSERT(64 == node.size());

    auto* start{node.data()};
    std::memcpy(keyOut, start, 32);
    std::advance(start, 32);
    std::memcpy(codeOut, start, 32);
    const auto havePub = provider(curve).ScalarMultiplyBase(
        {reinterpret_cast<const char*>(node.data()), 32}, pub);

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
