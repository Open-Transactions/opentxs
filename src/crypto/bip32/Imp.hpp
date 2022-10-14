// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <future>
#include <memory>
#include <optional>
#include <string_view>

#include "crypto/HDNode.hpp"
#include "internal/crypto/Crypto.hpp"
#include "internal/util/AsyncConst.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Factory;
}  // namespace api

namespace crypto
{
namespace asymmetric
{
namespace key
{
class HD;
class HDPrivate;
}  // namespace key
}  // namespace asymmetric

class EcdsaProvider;
}  // namespace crypto

namespace identifier
{
class Generic;
}  // namespace identifier

namespace proto
{
class HDPath;
}  // namespace proto

class ByteArray;
class Data;
class PasswordPrompt;
class Secret;
class WriteBuffer;
class Writer;
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
        const asymmetric::key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key;
    auto DerivePrivateKey(
        const asymmetric::key::HDPrivate& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key final;
    auto DerivePublicKey(
        const asymmetric::key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key;
    auto DerivePublicKey(
        const asymmetric::key::HDPrivate& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key final;
    auto DeserializePrivate(
        std::string_view serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Secret& key) const -> bool;
    auto DeserializePublic(
        std::string_view serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Data& key) const -> bool;
    auto Init(const std::shared_ptr<const api::Factory>& factory) noexcept
        -> void final;
    auto SeedID(const ReadView entropy) const -> identifier::Generic;
    auto SerializePrivate(
        Bip32Network network,
        Bip32Depth depth,
        Bip32Fingerprint parent,
        Bip32Index index,
        ReadView chainCode,
        ReadView key,
        Writer&& out) const noexcept -> bool;
    auto SerializePublic(
        Bip32Network network,
        Bip32Depth depth,
        Bip32Fingerprint parent,
        Bip32Index index,
        ReadView chainCode,
        ReadView key,
        Writer&& out) const noexcept -> bool;

    Imp(const api::Crypto& crypto) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() final;

private:
    using HDNode = implementation::HDNode;

    const api::Crypto& crypto_;
    std::weak_ptr<const api::Factory> factory_;
    AsyncConst<Key> blank_;

    static auto IsHard(const Bip32Index) noexcept -> bool;

    auto ckd_hardened(
        const HDNode& node,
        const be::big_uint32_buf_t i,
        WriteBuffer& data) const noexcept -> void;
    auto ckd_normal(
        const HDNode& node,
        const be::big_uint32_buf_t i,
        WriteBuffer& data) const noexcept -> void;
    auto decode(std::string_view serialized) const noexcept -> ByteArray;
    auto derive_private(
        HDNode& node,
        Bip32Fingerprint& parent,
        const Bip32Index child) const noexcept -> bool;
    auto derive_private_key(
        const asymmetric::Algorithm type,
        const proto::HDPath& path,
        const ReadView parentPrivate,
        const ReadView parentChaincode,
        const ReadView parentPublic,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key;
    auto derive_public_key(
        const asymmetric::Algorithm type,
        const proto::HDPath& path,
        const ReadView parentChaincode,
        const ReadView parentPublic,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key;
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
        Writer&& privateKey,
        Writer&& code,
        Writer&& publicKey) const noexcept -> bool;
};
}  // namespace opentxs::crypto
