// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Role

// IWYU pragma: no_forward_declare opentxs::identity::NymCapability

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "crypto/key/asymmetric/Keypair.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/crypto/key/Factory.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/crypto/key/Null.hpp"
#include "internal/otx/common/crypto/OTSignatureMetadata.hpp"
#include "internal/otx/common/crypto/Signature.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"

template class opentxs::Pimpl<opentxs::crypto::key::Keypair>;

namespace opentxs::factory
{
auto Keypair() noexcept -> std::unique_ptr<crypto::key::Keypair>
{
    return std::make_unique<crypto::key::blank::Keypair>();
}

auto Keypair(
    const api::Session& api,
    const opentxs::crypto::asymmetric::Role role,
    crypto::asymmetric::Key publicKey,
    crypto::asymmetric::Key privateKey) noexcept(false)
    -> std::unique_ptr<crypto::key::Keypair>
{
    using ReturnType = crypto::key::implementation::Keypair;

    return std::make_unique<ReturnType>(
        api, role, std::move(publicKey), std::move(privateKey));
}
}  // namespace opentxs::factory

namespace opentxs::crypto::key::implementation
{
Keypair::Keypair(
    const api::Session& api,
    const crypto::asymmetric::Role role,
    crypto::asymmetric::Key publicKey,
    crypto::asymmetric::Key privateKey) noexcept
    : api_(api)
    , key_private_(std::move(privateKey))
    , key_public_(std::move(publicKey))
    , role_(role)
{
}

Keypair::Keypair(const Keypair& rhs) noexcept
    : api_(rhs.api_)
    , key_private_(rhs.key_private_)
    , key_public_(rhs.key_public_)
    , role_(rhs.role_)
{
}

auto Keypair::CheckCapability(
    const identity::NymCapability& capability) const noexcept -> bool
{
    return key_private_.HasCapability(capability) ||
           key_public_.HasCapability(capability);
}

// Return the private key as an Asymmetric object
auto Keypair::GetPrivateKey() const -> const asymmetric::Key&
{
    if (key_private_.IsValid()) { return key_private_; }

    throw std::runtime_error("private key missing");
}

// Return the public key as an Asymmetric object
auto Keypair::GetPublicKey() const -> const asymmetric::Key&
{
    if (key_public_.IsValid()) { return key_public_; }

    throw std::runtime_error("public key missing");
}

auto Keypair::GetPublicKeyBySignature(
    Keys& listOutput,  // Inclusive means, return the key even
                       // when theSignature has no metadata.
    const Signature& theSignature,
    bool bInclusive) const noexcept -> std::int32_t
{
    OT_ASSERT(key_public_.IsValid());

    const auto* metadata = key_public_.Internal().GetMetadata();

    OT_ASSERT(nullptr != metadata);

    // We know that EITHER exact metadata matches must occur, and the signature
    // MUST have metadata, (bInclusive=false)
    // OR if bInclusive=true, we know that metadata is still used to eliminate
    // keys where possible, but that otherwise,
    // if the signature has no metadata, then the key is still returned, "just
    // in case."
    //
    if ((false == bInclusive) &&
        (false == theSignature.getMetaData().HasMetadata())) {
        return 0;
    }

    // Below this point, metadata is used if it's available.
    // It's assumed to be "okay" if it's not available, since any non-inclusive
    // calls would have already returned by now, if that were the case.
    // (But if it IS available, then it must match, or the key won't be
    // returned.)
    //
    // If the signature has no metadata, or if key_public_ has no metadata, or
    // if they BOTH have metadata, and their metadata is a MATCH...
    if (!theSignature.getMetaData().HasMetadata() || !metadata->HasMetadata() ||
        (metadata->HasMetadata() && theSignature.getMetaData().HasMetadata() &&
         (theSignature.getMetaData() == *(metadata)))) {
        // ...Then add key_public_ as a possible match, to listOutput.
        //
        listOutput.push_back(std::addressof(key_public_));
        return 1;
    }
    return 0;
}

auto Keypair::Serialize(proto::AsymmetricKey& serialized, bool privateKey)
    const noexcept -> bool
{
    if (privateKey) {
        if (false == key_private_.Internal().Serialize(serialized)) {
            return false;
        }
    } else {
        if (false == key_public_.Internal().Serialize(serialized)) {
            return false;
        }
    }
    return true;
}

auto Keypair::GetTransportKey(
    Data& publicKey,
    Secret& privateKey,
    const opentxs::PasswordPrompt& reason) const noexcept -> bool
{
    return key_private_.Internal().TransportKey(publicKey, privateKey, reason);
}
}  // namespace opentxs::crypto::key::implementation
