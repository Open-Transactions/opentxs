// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/AsymmetricKey.pb.h>

#include "internal/crypto/key/Keypair.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
class Key;
}  // namespace asymmetric
}  // namespace crypto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::key::blank
{
class Keypair final : virtual public key::Keypair
{
public:
    operator bool() const noexcept final { return false; }

    auto CheckCapability(const identity::NymCapability&) const noexcept
        -> bool final
    {
        return {};
    }
    auto GetPrivateKey() const noexcept(false) -> const asymmetric::Key& final
    {
        throw std::runtime_error("");
    }
    auto GetPublicKey() const noexcept(false) -> const asymmetric::Key& final
    {
        throw std::runtime_error("");
    }
    auto GetPublicKeyBySignature(Keys&, const Signature&, bool) const noexcept
        -> std::int32_t final
    {
        return {};
    }
    auto Serialize(protobuf::AsymmetricKey& serialized, bool) const noexcept
        -> bool final
    {
        serialized = {};
        return {};
    }
    auto GetTransportKey(Data&, Secret&, const PasswordPrompt&) const noexcept
        -> bool final
    {
        return {};
    }

    auto clone() const -> Keypair* final { return new Keypair; }

    ~Keypair() final = default;
};
}  // namespace opentxs::crypto::key::blank
