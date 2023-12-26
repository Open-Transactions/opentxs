// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "crypto/asymmetric/key/ellipticcurve/Imp.hpp"

#include <memory>
#include <tuple>

#include "crypto/asymmetric/key/hd/HDPrivate.hpp"
#include "internal/crypto/asymmetric/key/HD.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace symmetric
{
class Key;
}  // namespace symmetric

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
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::key::implementation
{
class HD : virtual public HDPrivate, public EllipticCurve
{
public:
    auto asHD() const noexcept -> const internal::key::HD& override
    {
        return *this;
    }
    [[nodiscard]] auto asHDPrivate() const noexcept
        -> const key::HDPrivate* override
    {
        return this;
    }
    auto Chaincode(const PasswordPrompt& reason) const noexcept
        -> ReadView final;
    auto ChildKey(
        const Bip32Index index,
        const PasswordPrompt& reason,
        allocator_type alloc) const noexcept -> asymmetric::key::HD final;
    auto Depth() const noexcept -> int final;
    auto Fingerprint() const noexcept -> Bip32Fingerprint final;
    auto Parent() const noexcept -> Bip32Fingerprint final { return parent_; }
    auto Path() const noexcept -> const UnallocatedCString final;
    auto Path(proto::HDPath& output) const noexcept -> bool final;
    auto Xprv(const PasswordPrompt& reason, Writer&& out) const noexcept
        -> bool final;
    auto Xpub(const PasswordPrompt& reason, Writer&& out) const noexcept
        -> bool final;

    auto asHD() noexcept -> internal::key::HD& override { return *this; }
    [[nodiscard]] auto asHDPrivate() noexcept -> key::HDPrivate* override
    {
        return this;
    }

    HD() = delete;
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;

    ~HD() override;

protected:
    auto erase_private_data(const Lock& lock) -> void final;

    HD(const api::Session& api,
       const crypto::EcdsaProvider& ecdsa,
       const proto::AsymmetricKey& serializedKey,
       allocator_type alloc) noexcept(false);
    HD(const api::Session& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::asymmetric::Algorithm keyType,
       const crypto::asymmetric::Role role,
       const VersionNumber version,
       symmetric::Key& sessionKey,
       const PasswordPrompt& reason,
       allocator_type alloc) noexcept(false);
    HD(const api::Session& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::asymmetric::Algorithm keyType,
       const opentxs::Secret& privateKey,
       const Data& publicKey,
       const crypto::asymmetric::Role role,
       const VersionNumber version,
       symmetric::Key& sessionKey,
       const PasswordPrompt& reason,
       allocator_type alloc) noexcept(false);
    HD(const api::Session& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::asymmetric::Algorithm keyType,
       const opentxs::Secret& privateKey,
       const opentxs::Secret& chainCode,
       const Data& publicKey,
       const proto::HDPath& path,
       const Bip32Fingerprint parent,
       const crypto::asymmetric::Role role,
       const VersionNumber version,
       symmetric::Key& sessionKey,
       const PasswordPrompt& reason,
       allocator_type alloc) noexcept(false);
    HD(const api::Session& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::asymmetric::Algorithm keyType,
       const opentxs::Secret& privateKey,
       const opentxs::Secret& chainCode,
       const Data& publicKey,
       const proto::HDPath& path,
       const Bip32Fingerprint parent,
       const crypto::asymmetric::Role role,
       const VersionNumber version,
       allocator_type alloc) noexcept(false);
    HD(const HD&, allocator_type alloc) noexcept;
    HD(const HD& rhs, const ReadView newPublic, allocator_type alloc) noexcept;
    HD(const HD& rhs, Secret&& newSecretKey, allocator_type alloc) noexcept;

private:
    const std::shared_ptr<const proto::HDPath> path_;
    const std::unique_ptr<const proto::Ciphertext> chain_code_;
    mutable Secret plaintext_chain_code_;
    const Bip32Fingerprint parent_;

    auto chaincode(const Lock& lock, const PasswordPrompt& reason)
        const noexcept -> ReadView;
    auto get_chain_code(const Lock& lock, const PasswordPrompt& reason) const
        noexcept(false) -> Secret&;
    auto get_params() const noexcept
        -> std::tuple<bool, Bip32Depth, Bip32Index>;
    auto serialize(const Lock& lock, Serialized& serialized) const noexcept
        -> bool final;
};
}  // namespace opentxs::crypto::asymmetric::key::implementation
