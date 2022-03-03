// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <tuple>

#include "Proto.hpp"
#include "crypto/key/asymmetric/EllipticCurve.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/Ciphertext.pb.h"
#include "serialization/protobuf/Enums.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key

class EcdsaProvider;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
class Ciphertext;
class HDPath;
}  // namespace proto

class Data;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::key::implementation
{
class HD : virtual public key::HD, public EllipticCurve
{
public:
    auto Chaincode(const PasswordPrompt& reason) const noexcept
        -> ReadView final;
    auto ChildKey(const Bip32Index index, const PasswordPrompt& reason)
        const noexcept -> std::unique_ptr<key::HD> final;
    auto Depth() const noexcept -> int final;
    auto Fingerprint() const noexcept -> Bip32Fingerprint final;
    auto Parent() const noexcept -> Bip32Fingerprint final { return parent_; }
    auto Path() const noexcept -> const UnallocatedCString final;
    auto Path(proto::HDPath& output) const noexcept -> bool final;
    auto Xprv(const PasswordPrompt& reason) const noexcept
        -> UnallocatedCString final;
    auto Xpub(const PasswordPrompt& reason) const noexcept
        -> UnallocatedCString final;

protected:
    auto erase_private_data(const Lock& lock) -> void final;

    HD(const api::Session& api,
       const crypto::EcdsaProvider& ecdsa,
       const proto::AsymmetricKey& serializedKey)
    noexcept(false);
    HD(const api::Session& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::key::asymmetric::Algorithm keyType,
       const crypto::key::asymmetric::Role role,
       const VersionNumber version,
       const PasswordPrompt& reason)
    noexcept(false);
    HD(const api::Session& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::key::asymmetric::Algorithm keyType,
       const opentxs::Secret& privateKey,
       const Data& publicKey,
       const crypto::key::asymmetric::Role role,
       const VersionNumber version,
       key::Symmetric& sessionKey,
       const PasswordPrompt& reason)
    noexcept(false);
    HD(const api::Session& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::key::asymmetric::Algorithm keyType,
       const opentxs::Secret& privateKey,
       const opentxs::Secret& chainCode,
       const Data& publicKey,
       const proto::HDPath& path,
       const Bip32Fingerprint parent,
       const crypto::key::asymmetric::Role role,
       const VersionNumber version,
       key::Symmetric& sessionKey,
       const PasswordPrompt& reason)
    noexcept(false);
    HD(const api::Session& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::key::asymmetric::Algorithm keyType,
       const opentxs::Secret& privateKey,
       const opentxs::Secret& chainCode,
       const Data& publicKey,
       const proto::HDPath& path,
       const Bip32Fingerprint parent,
       const crypto::key::asymmetric::Role role,
       const VersionNumber version)
    noexcept(false);
    HD(const HD&) noexcept;
    HD(const HD& rhs, const ReadView newPublic) noexcept;
    HD(const HD& rhs, OTSecret&& newSecretKey) noexcept;

private:
    const std::shared_ptr<const proto::HDPath> path_;
    const std::unique_ptr<const proto::Ciphertext> chain_code_;
    mutable OTSecret plaintext_chain_code_;
    const Bip32Fingerprint parent_;

    auto chaincode(const Lock& lock, const PasswordPrompt& reason)
        const noexcept -> ReadView;
    auto get_chain_code(const Lock& lock, const PasswordPrompt& reason) const
        noexcept(false) -> Secret&;
    auto get_params() const noexcept
        -> std::tuple<bool, Bip32Depth, Bip32Index>;
    auto serialize(const Lock& lock, Serialized& serialized) const noexcept
        -> bool final;

    HD() = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;
};
}  // namespace opentxs::crypto::key::implementation
