// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace crypto
{
namespace key
{
class Asymmetric;
class Keypair;
}  // namespace key
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
}  // namespace proto

class Data;
class PasswordPrompt;
class Secret;
class Signature;

using OTKeypair = Pimpl<crypto::key::Keypair>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::key
{
class OPENTXS_EXPORT Keypair
{
public:
    using Keys = UnallocatedList<const Asymmetric*>;

    virtual operator bool() const noexcept = 0;

    virtual auto CheckCapability(const NymCapability& capability) const noexcept
        -> bool = 0;
    /// throws std::runtime_error if private key is missing
    virtual auto GetPrivateKey() const noexcept(false) -> const Asymmetric& = 0;
    /// throws std::runtime_error if public key is missing
    virtual auto GetPublicKey() const noexcept(false) -> const Asymmetric& = 0;
    // inclusive means, return keys when theSignature has no metadata.
    virtual auto GetPublicKeyBySignature(
        Keys& listOutput,
        const Signature& theSignature,
        bool bInclusive = false) const noexcept -> std::int32_t = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(
        proto::AsymmetricKey& serialized,
        bool privateKey) const noexcept -> bool = 0;
    virtual auto GetTransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const noexcept -> bool = 0;

    virtual ~Keypair() = default;

protected:
    Keypair() = default;

private:
    friend OTKeypair;

    virtual auto clone() const -> Keypair* = 0;

    Keypair(const Keypair&) = delete;
    Keypair(Keypair&&) = delete;
    auto operator=(const Keypair&) -> Keypair& = delete;
    auto operator=(Keypair&&) -> Keypair& = delete;
};
}  // namespace opentxs::crypto::key
