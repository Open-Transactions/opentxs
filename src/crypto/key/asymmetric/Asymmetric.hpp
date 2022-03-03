// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <robin_hood.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/SignatureRole.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/Ciphertext.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/Signature.pb.h"

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

class Parameters;
}  // namespace crypto

namespace identity
{
class Authority;
}  // namespace identity

namespace proto
{
class AsymmetricKey;
class Ciphertext;
class HDPath;
}  // namespace proto

class Identifier;
class OTSignatureMetadata;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::key::implementation
{
class Asymmetric : virtual public key::Asymmetric
{
public:
    auto CalculateHash(
        const crypto::HashType hashType,
        const PasswordPrompt& password) const noexcept -> OTData final;
    auto CalculateTag(
        const identity::Authority& nym,
        const crypto::key::asymmetric::Algorithm type,
        const PasswordPrompt& reason,
        std::uint32_t& tag,
        Secret& password) const noexcept -> bool final;
    auto CalculateTag(
        const key::Asymmetric& dhKey,
        const Identifier& credential,
        const PasswordPrompt& reason,
        std::uint32_t& tag) const noexcept -> bool final;
    auto CalculateSessionPassword(
        const key::Asymmetric& dhKey,
        const PasswordPrompt& reason,
        Secret& password) const noexcept -> bool final;
    auto CalculateID(Identifier& theOutput) const noexcept -> bool final;
    auto engine() const noexcept -> const crypto::AsymmetricProvider& final
    {
        return provider_;
    }
    auto GetMetadata() const noexcept -> const OTSignatureMetadata* final
    {
        return metadata_.get();
    }
    auto hasCapability(const NymCapability& capability) const noexcept
        -> bool override;
    auto HasPrivate() const noexcept -> bool final;
    auto HasPublic() const noexcept -> bool final { return has_public_; }
    auto keyType() const noexcept -> crypto::key::asymmetric::Algorithm final
    {
        return type_;
    }
    auto NewSignature(
        const Identifier& credentialID,
        const crypto::SignatureRole role,
        const crypto::HashType hash) const -> proto::Signature;
    auto Params() const noexcept -> ReadView override { return {}; }
    auto Path() const noexcept -> const UnallocatedCString override;
    auto Path(proto::HDPath& output) const noexcept -> bool override;
    auto PrivateKey(const PasswordPrompt& reason) const noexcept
        -> ReadView final;
    auto PublicKey() const noexcept -> ReadView final { return key_->Bytes(); }
    auto Role() const noexcept -> opentxs::crypto::key::asymmetric::Role final
    {
        return role_;
    }
    auto Serialize(Serialized& serialized) const noexcept -> bool final;
    auto SigHashType() const noexcept -> crypto::HashType override
    {
        return crypto::HashType::Blake2b256;
    }
    auto Sign(
        const GetPreimage input,
        const crypto::SignatureRole role,
        proto::Signature& signature,
        const Identifier& credential,
        const PasswordPrompt& reason,
        const crypto::HashType hash) const noexcept -> bool final;
    auto Sign(
        const ReadView preimage,
        const crypto::HashType hash,
        const AllocateOutput output,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const noexcept -> bool override;
    auto Verify(const Data& plaintext, const proto::Signature& sig)
        const noexcept -> bool final;
    auto Version() const noexcept -> VersionNumber final { return version_; }

    operator bool() const noexcept override;
    auto operator==(const proto::AsymmetricKey&) const noexcept -> bool final;

    ~Asymmetric() override;

protected:
    friend OTAsymmetricKey;

    using EncryptedKey = std::unique_ptr<proto::Ciphertext>;
    using EncryptedExtractor = std::function<EncryptedKey(Data&, Secret&)>;
    using PlaintextExtractor = std::function<OTSecret()>;

    const api::Session& api_;
    const VersionNumber version_;
    const crypto::key::asymmetric::Algorithm type_;
    const opentxs::crypto::key::asymmetric::Role role_;
    const OTData key_;
    mutable OTSecret plaintext_key_;
    mutable std::mutex lock_;
    std::unique_ptr<const proto::Ciphertext> encrypted_key_;

    static auto create_key(
        const api::Session& api,
        const crypto::AsymmetricProvider& provider,
        const Parameters& options,
        const crypto::key::asymmetric::Role role,
        const AllocateOutput publicKey,
        const AllocateOutput privateKey,
        const opentxs::Secret& prv,
        const AllocateOutput params,
        const PasswordPrompt& reason) noexcept(false)
        -> std::unique_ptr<proto::Ciphertext>;
    static auto encrypt_key(
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason,
        const bool attach,
        const ReadView plaintext) noexcept
        -> std::unique_ptr<proto::Ciphertext>;
    static auto encrypt_key(
        const api::Session& api,
        const PasswordPrompt& reason,
        const ReadView plaintext,
        proto::Ciphertext& ciphertext) noexcept -> bool;
    static auto encrypt_key(
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason,
        const bool attach,
        const ReadView plaintext,
        proto::Ciphertext& ciphertext) noexcept -> bool;
    static auto generate_key(
        const crypto::AsymmetricProvider& provider,
        const Parameters& options,
        const crypto::key::asymmetric::Role role,
        const AllocateOutput publicKey,
        const AllocateOutput privateKey,
        const AllocateOutput params) noexcept(false) -> void;

    auto get_private_key(const Lock& lock, const PasswordPrompt& reason) const
        noexcept(false) -> Secret&;
    auto has_private(const Lock& lock) const noexcept -> bool;
    auto private_key(const Lock& lock, const PasswordPrompt& reason)
        const noexcept -> ReadView;
    virtual auto serialize(const Lock& lock, Serialized& serialized)
        const noexcept -> bool;

    virtual void erase_private_data(const Lock& lock);

    Asymmetric(
        const api::Session& api,
        const crypto::AsymmetricProvider& engine,
        const crypto::key::asymmetric::Algorithm keyType,
        const crypto::key::asymmetric::Role role,
        const bool hasPublic,
        const bool hasPrivate,
        const VersionNumber version,
        OTData&& pubkey,
        EncryptedExtractor get,
        PlaintextExtractor getPlaintext = {}) noexcept(false);
    Asymmetric(
        const api::Session& api,
        const crypto::AsymmetricProvider& engine,
        const crypto::key::asymmetric::Algorithm keyType,
        const crypto::key::asymmetric::Role role,
        const VersionNumber version,
        EncryptedExtractor get) noexcept(false);
    Asymmetric(
        const api::Session& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKey& serializedKey,
        EncryptedExtractor get) noexcept(false);
    Asymmetric(const Asymmetric& rhs) noexcept;
    Asymmetric(const Asymmetric& rhs, const ReadView newPublic) noexcept;
    Asymmetric(
        const Asymmetric& rhs,
        OTData&& newPublicKey,
        OTSecret&& newSecretKey) noexcept;

private:
    using HashTypeMap =
        robin_hood::unordered_flat_map<crypto::HashType, proto::HashType>;
    using HashTypeReverseMap =
        robin_hood::unordered_flat_map<proto::HashType, crypto::HashType>;
    using SignatureRoleMap = robin_hood::
        unordered_flat_map<crypto::SignatureRole, proto::SignatureRole>;

    static const robin_hood::
        unordered_flat_map<crypto::SignatureRole, VersionNumber>
            sig_version_;

    const crypto::AsymmetricProvider& provider_;
    const bool has_public_;
    const std::unique_ptr<const OTSignatureMetadata> metadata_;
    bool has_private_;

    auto SerializeKeyToData(const proto::AsymmetricKey& rhs) const -> OTData;

    static auto hashtype_map() noexcept -> const HashTypeMap&;
    static auto signaturerole_map() noexcept -> const SignatureRoleMap&;
    static auto translate(const crypto::SignatureRole in) noexcept
        -> proto::SignatureRole;
    static auto translate(const crypto::HashType in) noexcept
        -> proto::HashType;
    static auto translate(const proto::HashType in) noexcept
        -> crypto::HashType;

    auto get_password(
        const Lock& lock,
        const key::Asymmetric& target,
        const PasswordPrompt& reason,
        Secret& password) const noexcept -> bool;
    auto get_tag(
        const Lock& lock,
        const key::Asymmetric& target,
        const Identifier& credential,
        const PasswordPrompt& reason,
        std::uint32_t& tag) const noexcept -> bool;

    Asymmetric() = delete;
    Asymmetric(Asymmetric&&) = delete;
    auto operator=(const Asymmetric&) -> Asymmetric& = delete;
    auto operator=(Asymmetric&&) -> Asymmetric& = delete;
};
}  // namespace opentxs::crypto::key::implementation
