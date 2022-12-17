// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/library/AsymmetricProvider.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <cstddef>
#include <cstring>
#include <utility>

#include "internal/core/String.hpp"
#include "internal/otx/common/crypto/Signature.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Role.hpp"       // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Sodium.hpp"

namespace opentxs::crypto
{
auto AsymmetricProvider::CurveToKeyType(const EcdsaCurve& curve)
    -> crypto::asymmetric::Algorithm
{
    auto output = crypto::asymmetric::Algorithm::Error;

    switch (curve) {
        case (EcdsaCurve::secp256k1): {
            output = asymmetric::Algorithm::Secp256k1;

            break;
        }
        case (EcdsaCurve::ed25519): {
            output = asymmetric::Algorithm::ED25519;

            break;
        }
        case EcdsaCurve::invalid:
        default: {
        }
    }

    return output;
}

auto AsymmetricProvider::KeyTypeToCurve(
    const crypto::asymmetric::Algorithm& type) -> EcdsaCurve
{
    auto output = EcdsaCurve::invalid;
    using Type = asymmetric::Algorithm;

    switch (type) {
        case Type::Secp256k1: {
            output = EcdsaCurve::secp256k1;
        } break;
        case Type::ED25519: {
            output = EcdsaCurve::ed25519;
        } break;
        case Type::Error:
        case Type::Null:
        case Type::Legacy:
        default: {
        }
    }

    return output;
}
}  // namespace opentxs::crypto

namespace opentxs::crypto::implementation
{
AsymmetricProvider::AsymmetricProvider() noexcept
{
    if (0 > ::sodium_init()) { OT_FAIL; }
}

auto AsymmetricProvider::RandomKeypair(
    Writer&& privateKey,
    Writer&& publicKey,
    Writer&& params) const noexcept -> bool
{
    return RandomKeypair(
        std::move(privateKey),
        std::move(publicKey),
        opentxs::crypto::asymmetric::Role::Sign,
        Parameters{},
        std::move(params));
}

auto AsymmetricProvider::RandomKeypair(
    Writer&& privateKey,
    Writer&& publicKey,
    const opentxs::crypto::asymmetric::Role role,
    Writer&& params) const noexcept -> bool
{
    return RandomKeypair(
        std::move(privateKey),
        std::move(publicKey),
        role,
        Parameters{},
        std::move(params));
}

auto AsymmetricProvider::RandomKeypair(
    Writer&& privateKey,
    Writer&& publicKey,
    const Parameters& options,
    Writer&& params) const noexcept -> bool
{
    return RandomKeypair(
        std::move(privateKey),
        std::move(publicKey),
        opentxs::crypto::asymmetric::Role::Sign,
        options,
        std::move(params));
}

auto AsymmetricProvider::SeedToCurveKey(
    const ReadView seed,
    Writer&& privateKey,
    Writer&& publicKey) const noexcept -> bool
{
    auto edPublic = ByteArray{};
    auto edPrivate = Context().Factory().Secret(0);

    if (false == sodium::ExpandSeed(
                     seed,
                     edPrivate.WriteInto(Secret::Mode::Mem),
                     edPublic.WriteInto())) {
        LogError()(OT_PRETTY_CLASS())("Failed to expand seed.").Flush();

        return false;
    }

    return sodium::ToCurveKeypair(
        edPrivate.Bytes(),
        edPublic.Bytes(),
        std::move(privateKey),
        std::move(publicKey));
}

auto AsymmetricProvider::SignContract(
    const api::Session& api,
    const String& contract,
    const ReadView theKey,
    const crypto::HashType hashType,
    Signature& output) const -> bool
{
    const auto plaintext = [&] {
        const auto size = std::size_t{contract.GetLength()};
        auto out = space(size + 1u);
        std::memcpy(out.data(), contract.Get(), size);
        out.back() = std::byte{0x0};

        return out;
    }();
    auto signature = api.Factory().Data();
    bool success =
        Sign(reader(plaintext), theKey, hashType, signature.WriteInto());
    output.SetData(signature, true);  // true means, "yes, with newlines in the
                                      // b64-encoded output, please."

    if (false == success) {
        LogError()(OT_PRETTY_CLASS())("Failed to sign contract").Flush();
    }

    return success;
}

auto AsymmetricProvider::VerifyContractSignature(
    const api::Session& api,
    const String& contract,
    const ReadView key,
    const Signature& theSignature,
    const crypto::HashType hashType) const -> bool
{
    const auto plaintext = [&] {
        const auto size = std::size_t{contract.GetLength()};
        auto out = space(size + 1);
        std::memcpy(out.data(), contract.Get(), size);
        out.back() = std::byte{0x0};

        return out;
    }();
    const auto signature = [&] {
        auto out = api.Factory().Data();
        theSignature.GetData(out);

        return out;
    }();

    return Verify(reader(plaintext), key, signature.Bytes(), hashType);
}
}  // namespace opentxs::crypto::implementation
