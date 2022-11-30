// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Role

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "crypto/asymmetric/key/secp256k1/Imp.hpp"  // IWYU pragma: associated

#include <utility>

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "crypto/asymmetric/key/ellipticcurve/EllipticCurvePrivate.hpp"
#include "crypto/asymmetric/key/hd/HDPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/ParameterType.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::crypto::asymmetric::key::implementation
{
Secp256k1::Secp256k1(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey,
    allocator_type alloc) noexcept(false)
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(alloc)
    , Secp256k1Private(alloc)
    , HD(api, ecdsa, serializedKey, alloc)
    , self_(this)
{
}

Secp256k1::Secp256k1(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    symmetric::Key& sessionKey,
    const PasswordPrompt& reason,
    allocator_type alloc) noexcept(false)
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(alloc)
    , Secp256k1Private(alloc)
    , HD(api,
         ecdsa,
         crypto::asymmetric::Algorithm::Secp256k1,
         role,
         version,
         sessionKey,
         reason,
         alloc)
    , self_(this)
{
    OT_ASSERT(false == plaintext_key_.empty());
}

Secp256k1::Secp256k1(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const Data& publicKey,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    symmetric::Key& sessionKey,
    const opentxs::PasswordPrompt& reason,
    allocator_type alloc) noexcept(false)
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(alloc)
    , Secp256k1Private(alloc)
    , HD(api,
         ecdsa,
         crypto::asymmetric::Algorithm::Secp256k1,
         privateKey,
         publicKey,
         role,
         version,
         sessionKey,
         reason,
         alloc)
    , self_(this)
{
}

Secp256k1::Secp256k1(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    symmetric::Key& sessionKey,
    const opentxs::PasswordPrompt& reason,
    allocator_type alloc) noexcept(false)
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(alloc)
    , Secp256k1Private(alloc)
    , HD(api,
         ecdsa,
         crypto::asymmetric::Algorithm::Secp256k1,
         privateKey,
         chainCode,
         publicKey,
         path,
         parent,
         role,
         version,
         sessionKey,
         reason,
         alloc)
    , self_(this)
{
}

Secp256k1::Secp256k1(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    allocator_type alloc) noexcept(false)
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(alloc)
    , Secp256k1Private(alloc)
    , HD(api,
         ecdsa,
         crypto::asymmetric::Algorithm::Secp256k1,
         privateKey,
         chainCode,
         publicKey,
         path,
         parent,
         role,
         version,
         alloc)
    , self_(this)
{
}

Secp256k1::Secp256k1(const Secp256k1& rhs, allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(alloc)
    , Secp256k1Private(alloc)
    , HD(rhs, alloc)
    , self_(this)
{
}

Secp256k1::Secp256k1(
    const Secp256k1& rhs,
    const ReadView newPublic,
    allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(alloc)
    , Secp256k1Private(alloc)
    , HD(rhs, newPublic, alloc)
    , self_(this)
{
}

Secp256k1::Secp256k1(
    const Secp256k1& rhs,
    Secret&& newSecretKey,
    allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(alloc)
    , Secp256k1Private(alloc)
    , HD(rhs, std::move(newSecretKey), alloc)
    , self_(this)
{
}

auto Secp256k1::clone(allocator_type alloc) const noexcept -> Secp256k1*
{
    auto pmr = alloc::PMR<Secp256k1>{alloc};
    // TODO c++20
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto Secp256k1::CreateType() const noexcept -> ParameterType
{
    return ParameterType::ed25519;
}

auto Secp256k1::get_deleter() const noexcept -> std::function<void(KeyPrivate*)>
{
    return [alloc = alloc::PMR<Secp256k1>{get_allocator()}](
               KeyPrivate* in) mutable {
        auto* p = dynamic_cast<Secp256k1*>(in);

        OT_ASSERT(nullptr != p);

        alloc.destroy(p);
        alloc.deallocate(p, 1_uz);
    };
}

auto Secp256k1::replace_public_key(
    const ReadView newPubkey,
    allocator_type alloc) const noexcept -> EllipticCurve*
{
    auto pmr = alloc::PMR<Secp256k1>{alloc};
    // TODO c++20
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this, newPubkey);

    return out;
}

auto Secp256k1::replace_secret_key(Secret&& newSecretKey, allocator_type alloc)
    const noexcept -> EllipticCurve*
{
    auto pmr = alloc::PMR<Secp256k1>{alloc};
    // TODO c++20
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this, std::move(newSecretKey));

    return out;
}

Secp256k1::~Secp256k1() { Reset(self_); }
}  // namespace opentxs::crypto::asymmetric::key::implementation