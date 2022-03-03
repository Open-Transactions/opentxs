// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <optional>

#include "crypto/HDNode.hpp"
#include "internal/crypto/Crypto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Crypto;
class Factory;
}  // namespace api

namespace crypto
{
namespace key
{
class HD;
}  // namespace key

class EcdsaProvider;
}  // namespace crypto

class PasswordPrompt;
class Secret;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace be = boost::endian;

namespace opentxs::crypto
{
struct Bip32::Imp final : public internal::Bip32 {
public:
    auto DeriveKey(
        const EcdsaCurve& curve,
        const Secret& seed,
        const Path& path) const -> Key;
    auto DerivePrivateKey(
        const key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key;
    auto DerivePublicKey(
        const key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key;
    auto DeserializePrivate(
        const UnallocatedCString& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Secret& key) const -> bool;
    auto DeserializePublic(
        const UnallocatedCString& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Data& key) const -> bool;
    auto Init(const api::Factory& factory) noexcept -> void final;
    auto SeedID(const ReadView entropy) const -> OTIdentifier;
    auto SerializePrivate(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const Secret& key) const -> UnallocatedCString;
    auto SerializePublic(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const Data& key) const -> UnallocatedCString;

    Imp(const api::Crypto& crypto) noexcept;

    ~Imp() final = default;

private:
    using HDNode = implementation::HDNode;

    const api::Crypto& crypto_;
    const std::optional<Key> blank_;

    static auto IsHard(const Bip32Index) noexcept -> bool;

    auto ckd_hardened(
        const HDNode& node,
        const be::big_uint32_buf_t i,
        const WritableView& data) const noexcept -> void;
    auto ckd_normal(
        const HDNode& node,
        const be::big_uint32_buf_t i,
        const WritableView& data) const noexcept -> void;
    auto decode(const UnallocatedCString& serialized) const noexcept -> OTData;
    auto derive_private(
        HDNode& node,
        Bip32Fingerprint& parent,
        const Bip32Index child) const noexcept -> bool;
    auto derive_public(
        HDNode& node,
        Bip32Fingerprint& parent,
        const Bip32Index child) const noexcept -> bool;
    auto extract(
        const Data& input,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode) const noexcept -> bool;
    auto provider(const EcdsaCurve& curve) const noexcept
        -> const crypto::EcdsaProvider&;
    auto root_node(
        const EcdsaCurve& curve,
        const ReadView entropy,
        const AllocateOutput privateKey,
        const AllocateOutput code,
        const AllocateOutput publicKey) const noexcept -> bool;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::crypto
