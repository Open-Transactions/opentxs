// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "crypto/key/asymmetric/Keypair.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "internal/crypto/key/Factory.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/crypto/key/Null.hpp"
#include "internal/otx/common/crypto/OTSignatureMetadata.hpp"
#include "internal/otx/common/crypto/Signature.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/util/Pimpl.hpp"

template class opentxs::Pimpl<opentxs::crypto::key::Keypair>;

namespace opentxs::factory
{
auto Keypair() noexcept -> std::unique_ptr<crypto::key::Keypair>
{
    return std::make_unique<crypto::key::blank::Keypair>();
}

auto Keypair(
    const api::Session& api,
    const opentxs::crypto::key::asymmetric::Role role,
    std::unique_ptr<crypto::key::Asymmetric> publicKey,
    std::unique_ptr<crypto::key::Asymmetric> privateKey) noexcept(false)
    -> std::unique_ptr<crypto::key::Keypair>
{
    using ReturnType = crypto::key::implementation::Keypair;

    if (false == bool(publicKey)) {
        throw std::runtime_error("Invalid public key");
    }

    if (false == bool(privateKey)) {
        throw std::runtime_error("Invalid private key");
    }

    return std::make_unique<ReturnType>(
        api, role, std::move(publicKey), std::move(privateKey));
}
}  // namespace opentxs::factory

namespace opentxs::crypto::key::implementation
{
Keypair::Keypair(
    const api::Session& api,
    const opentxs::crypto::key::asymmetric::Role role,
    std::unique_ptr<crypto::key::Asymmetric> publicKey,
    std::unique_ptr<crypto::key::Asymmetric> privateKey) noexcept
    : api_(api)
    , key_private_(privateKey.release())
    , key_public_(publicKey.release())
    , role_(role)
{
    OT_ASSERT(key_public_.get());
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
    bool output{false};

    if (key_private_.get()) {
        output |= key_private_->hasCapability(capability);
    } else if (key_public_.get()) {
        output |= key_public_->hasCapability(capability);
    }

    return output;
}

// Return the private key as an Asymmetric object
auto Keypair::GetPrivateKey() const -> const key::Asymmetric&
{
    if (key_private_.get()) { return key_private_; }

    throw std::runtime_error("private key missing");
}

// Return the public key as an Asymmetric object
auto Keypair::GetPublicKey() const -> const key::Asymmetric&
{
    if (key_public_.get()) { return key_public_; }

    throw std::runtime_error("public key missing");
}

auto Keypair::GetPublicKeyBySignature(
    Keys& listOutput,  // Inclusive means, return the key even
                       // when theSignature has no metadata.
    const Signature& theSignature,
    bool bInclusive) const noexcept -> std::int32_t
{
    OT_ASSERT(key_public_.get());

    const auto* metadata = key_public_->GetMetadata();

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
        listOutput.push_back(&key_public_.get());
        return 1;
    }
    return 0;
}

auto Keypair::Serialize(proto::AsymmetricKey& serialized, bool privateKey)
    const noexcept -> bool
{
    OT_ASSERT(key_public_.get());

    if (privateKey) {
        OT_ASSERT(key_private_.get());

        if (false == key_private_->Serialize(serialized)) { return false; }
    } else {
        if (false == key_public_->Serialize(serialized)) { return false; }
    }
    return true;
}

auto Keypair::GetTransportKey(
    Data& publicKey,
    Secret& privateKey,
    const opentxs::PasswordPrompt& reason) const noexcept -> bool
{
    return key_private_->TransportKey(publicKey, privateKey, reason);
}
}  // namespace opentxs::crypto::key::implementation
