// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "crypto/key/asymmetric/Asymmetric.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Ciphertext.pb.h>
#include <Enums.pb.h>
#include <Signature.pb.h>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/api/crypto/Symmetric.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/key/Null.hpp"
#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/otx/common/crypto/OTSignatureMetadata.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/SecretStyle.hpp"
#include "opentxs/crypto/SignatureRole.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/identity/credential/Key.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/Container.hpp"

template class opentxs::Pimpl<opentxs::crypto::key::Asymmetric>;

namespace opentxs::crypto::key
{
const VersionNumber Asymmetric::DefaultVersion{2};
const VersionNumber Asymmetric::MaxVersion{2};

auto Asymmetric::Factory() noexcept -> OTAsymmetricKey
{
    return OTAsymmetricKey(new blank::Asymmetric);
}
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
const robin_hood::unordered_flat_map<crypto::SignatureRole, VersionNumber>
    Asymmetric::sig_version_{
        {SignatureRole::PublicCredential, 1},
        {SignatureRole::PrivateCredential, 1},
        {SignatureRole::NymIDSource, 1},
        {SignatureRole::Claim, 1},
        {SignatureRole::ServerContract, 1},
        {SignatureRole::UnitDefinition, 1},
        {SignatureRole::PeerRequest, 1},
        {SignatureRole::PeerReply, 1},
        {SignatureRole::Context, 2},
        {SignatureRole::Account, 2},
        {SignatureRole::ServerRequest, 3},
        {SignatureRole::ServerReply, 3},
    };

Asymmetric::Asymmetric(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const crypto::key::asymmetric::Algorithm keyType,
    const crypto::key::asymmetric::Role role,
    const bool hasPublic,
    const bool hasPrivate,
    const VersionNumber version,
    ByteArray&& pubkey,
    EncryptedExtractor get,
    PlaintextExtractor getPlaintext) noexcept(false)
    : api_(api)
    , version_(version)
    , type_(keyType)
    , role_(role)
    , key_(std::move(pubkey))
    , plaintext_key_([&] {
        if (getPlaintext) {

            return getPlaintext();
        } else {

            return api_.Factory().Secret(0);
        }
    }())
    , lock_()
    , encrypted_key_([&] {
        if (hasPrivate && get) {

            return get(const_cast<ByteArray&>(key_), plaintext_key_);
        } else {

            return EncryptedKey{};
        }
    }())
    , provider_(engine)
    , has_public_(hasPublic)
    , metadata_(std::make_unique<OTSignatureMetadata>(api_))
    , has_private_(hasPrivate)
{
    OT_ASSERT(0 < version);
    OT_ASSERT(nullptr != metadata_);

    if (has_private_) {
        OT_ASSERT(encrypted_key_ || (0 < plaintext_key_.size()));
    }
}

Asymmetric::Asymmetric(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const crypto::key::asymmetric::Algorithm keyType,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    EncryptedExtractor getEncrypted) noexcept(false)
    : Asymmetric(
          api,
          engine,
          keyType,
          role,
          true,
          true,
          version,
          api.Factory().Data(),
          getEncrypted)
{
}

Asymmetric::Asymmetric(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& serialized,
    EncryptedExtractor getEncrypted) noexcept(false)
    : Asymmetric(
          api,
          engine,
          opentxs::translate(serialized.type()),
          opentxs::translate(serialized.role()),
          true,
          proto::KEYMODE_PRIVATE == serialized.mode(),
          serialized.version(),
          serialized.has_key() ? api.Factory().DataFromBytes(serialized.key())
                               : api.Factory().Data(),
          getEncrypted)
{
}

Asymmetric::Asymmetric(const Asymmetric& rhs) noexcept
    : Asymmetric(
          rhs.api_,
          rhs.provider_,
          rhs.type_,
          rhs.role_,
          rhs.has_public_,
          rhs.has_private_,
          rhs.version_,
          ByteArray{rhs.key_},
          [&](auto&, auto&) -> EncryptedKey {
              if (rhs.encrypted_key_) {

                  return std::make_unique<proto::Ciphertext>(
                      *rhs.encrypted_key_);
              }

              return {};
          },
          [&] { return rhs.plaintext_key_; })
{
}

Asymmetric::Asymmetric(const Asymmetric& rhs, const ReadView newPublic) noexcept
    : Asymmetric(
          rhs.api_,
          rhs.provider_,
          rhs.type_,
          rhs.role_,
          true,
          false,
          rhs.version_,
          rhs.api_.Factory().DataFromBytes(newPublic),
          [&](auto&, auto&) -> EncryptedKey { return {}; })
{
}

Asymmetric::Asymmetric(
    const Asymmetric& rhs,
    ByteArray&& newPublicKey,
    Secret&& newSecretKey) noexcept
    : api_(rhs.api_)
    , version_(rhs.version_)
    , type_(rhs.type_)
    , role_(rhs.role_)
    , key_(std::move(newPublicKey))
    , plaintext_key_(std::move(newSecretKey))
    , lock_()
    , encrypted_key_(EncryptedKey{})
    , provider_(rhs.provider_)
    , has_public_(false == key_.empty())
    , metadata_(std::make_unique<OTSignatureMetadata>(api_))
    , has_private_(false == plaintext_key_.empty())
{
    OT_ASSERT(0 < version_);
    OT_ASSERT(nullptr != metadata_);

    if (has_private_) {
        OT_ASSERT(encrypted_key_ || (0 < plaintext_key_.size()));
    }
}

Asymmetric::operator bool() const noexcept
{
    return has_public_ || has_private_;
}

auto Asymmetric::operator==(const proto::AsymmetricKey& rhs) const noexcept
    -> bool
{
    auto lhs = proto::AsymmetricKey{};

    {
        auto lock = Lock{lock_};

        if (false == serialize(lock, lhs)) { return false; }
    }

    auto LHData = SerializeKeyToData(lhs);
    auto RHData = SerializeKeyToData(rhs);

    return (LHData == RHData);
}

auto Asymmetric::CalculateHash(
    const crypto::HashType hashType,
    const PasswordPrompt& reason) const noexcept -> ByteArray
{
    auto lock = Lock{lock_};
    auto output = api_.Factory().Data();
    const auto hashed = api_.Crypto().Hash().Digest(
        hashType,
        has_private_ ? private_key(lock, reason) : PublicKey(),
        output.WriteInto());

    if (false == hashed) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate hash").Flush();

        return ByteArray{};
    }

    return output;
}

auto Asymmetric::CalculateID(identifier::Generic& output) const noexcept -> bool
{
    if (false == HasPublic()) {
        LogError()(OT_PRETTY_CLASS())("Missing public key").Flush();

        return false;
    }

    output = api_.Factory().IdentifierFromPreimage(PublicKey());

    return false == output.empty();
}

auto Asymmetric::CalculateTag(
    const identity::Authority& nym,
    const crypto::key::asymmetric::Algorithm type,
    const PasswordPrompt& reason,
    std::uint32_t& tag,
    Secret& password) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == has_private_) {
        LogError()(OT_PRETTY_CLASS())("Not a private key.").Flush();

        return false;
    }

    try {
        const auto& cred = nym.GetTagCredential(type);
        const auto& key =
            cred.GetKeypair(
                    type, opentxs::crypto::key::asymmetric::Role::Encrypt)
                .GetPublicKey();

        if (false == get_tag(lock, key, nym.GetMasterCredID(), reason, tag)) {
            LogError()(OT_PRETTY_CLASS())("Failed to calculate tag.").Flush();

            return false;
        }

        if (false == get_password(lock, key, reason, password)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to calculate session password.")
                .Flush();

            return false;
        }

        return true;
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Invalid credential").Flush();

        return false;
    }
}

auto Asymmetric::CalculateTag(
    const key::Asymmetric& dhKey,
    const identifier::Generic& credential,
    const PasswordPrompt& reason,
    std::uint32_t& tag) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == has_private_) {
        LogError()(OT_PRETTY_CLASS())("Not a private key.").Flush();

        return false;
    }

    return get_tag(lock, dhKey, credential, reason, tag);
}

auto Asymmetric::CalculateSessionPassword(
    const key::Asymmetric& dhKey,
    const PasswordPrompt& reason,
    Secret& password) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == has_private_) {
        LogError()(OT_PRETTY_CLASS())("Not a private key.").Flush();

        return false;
    }

    return get_password(lock, dhKey, reason, password);
}

auto Asymmetric::create_key(
    const api::Session& api,
    const crypto::AsymmetricProvider& provider,
    const Parameters& options,
    const crypto::key::asymmetric::Role role,
    const AllocateOutput publicKey,
    const AllocateOutput privateKey,
    const opentxs::Secret& prv,
    const AllocateOutput params,
    const PasswordPrompt& reason) -> std::unique_ptr<proto::Ciphertext>
{
    generate_key(provider, options, role, publicKey, privateKey, params);
    auto pOutput = std::make_unique<proto::Ciphertext>();

    OT_ASSERT(pOutput);

    auto& output = *pOutput;

    if (false == encrypt_key(api, reason, prv.Bytes(), output)) {
        throw std::runtime_error("Failed to encrypt key");
    }

    return pOutput;
}

auto Asymmetric::encrypt_key(
    key::Symmetric& sessionKey,
    const PasswordPrompt& reason,
    const bool attach,
    const ReadView plaintext) noexcept -> std::unique_ptr<proto::Ciphertext>
{
    auto output = std::make_unique<proto::Ciphertext>();

    if (false == bool(output)) {
        LogError()(OT_PRETTY_STATIC(Asymmetric))("Failed to construct output")
            .Flush();

        return {};
    }

    auto& ciphertext = *output;

    if (encrypt_key(sessionKey, reason, attach, plaintext, ciphertext)) {

        return output;
    } else {

        return {};
    }
}

auto Asymmetric::encrypt_key(
    const api::Session& api,
    const PasswordPrompt& reason,
    const ReadView plaintext,
    proto::Ciphertext& ciphertext) noexcept -> bool
{
    auto sessionKey = api.Crypto().Symmetric().Key(reason);

    return encrypt_key(sessionKey, reason, true, plaintext, ciphertext);
}

auto Asymmetric::encrypt_key(
    key::Symmetric& sessionKey,
    const PasswordPrompt& reason,
    const bool attach,
    const ReadView plaintext,
    proto::Ciphertext& ciphertext) noexcept -> bool
{
    const auto encrypted =
        sessionKey.Encrypt(plaintext, reason, ciphertext, attach);

    if (false == encrypted) {
        LogError()(OT_PRETTY_STATIC(Asymmetric))("Failed to encrypt key")
            .Flush();

        return false;
    }

    return true;
}

auto Asymmetric::erase_private_data(const Lock&) -> void
{
    plaintext_key_.clear();
    encrypted_key_.reset();
    has_private_ = false;
}

auto Asymmetric::generate_key(
    const crypto::AsymmetricProvider& provider,
    const Parameters& options,
    const crypto::key::asymmetric::Role role,
    const AllocateOutput publicKey,
    const AllocateOutput privateKey,
    const AllocateOutput params) noexcept(false) -> void
{
    const auto generated =
        provider.RandomKeypair(privateKey, publicKey, role, options, params);

    if (false == generated) {
        throw std::runtime_error("Failed to generate key");
    }
}

auto Asymmetric::get_password(
    const Lock& lock,
    const key::Asymmetric& target,
    const PasswordPrompt& reason,
    Secret& password) const noexcept -> bool
{
    return provider_.SharedSecret(
        target.PublicKey(),
        private_key(lock, reason),
        SecretStyle::Default,
        password);
}

auto Asymmetric::get_private_key(const Lock&, const PasswordPrompt& reason)
    const noexcept(false) -> Secret&
{
    if (0 == plaintext_key_.size()) {
        if (false == bool(encrypted_key_)) {
            throw std::runtime_error{"Missing encrypted private key"};
        }

        const auto& privateKey = *encrypted_key_;
        auto sessionKey = api_.Crypto().Symmetric().InternalSymmetric().Key(
            privateKey.key(),
            opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305);

        if (false == sessionKey.get()) {
            throw std::runtime_error{"Failed to extract session key"};
        }

        auto allocator = plaintext_key_.WriteInto(Secret::Mode::Mem);

        if (false == sessionKey->Decrypt(privateKey, reason, allocator)) {
            throw std::runtime_error{"Failed to decrypt private key"};
        }
    }

    return plaintext_key_;
}

auto Asymmetric::get_tag(
    const Lock& lock,
    const key::Asymmetric& target,
    const identifier::Generic& credential,
    const PasswordPrompt& reason,
    std::uint32_t& tag) const noexcept -> bool
{
    auto hashed = api_.Factory().Secret(0);
    auto password = api_.Factory().Secret(0);

    if (false == provider_.SharedSecret(
                     target.PublicKey(),
                     private_key(lock, reason),
                     SecretStyle::Default,
                     password)) {
        LogVerbose()(OT_PRETTY_CLASS())("Failed to calculate shared secret")
            .Flush();

        return false;
    }

    if (false == api_.Crypto().Hash().HMAC(
                     crypto::HashType::Sha256,
                     password.Bytes(),
                     credential.Bytes(),
                     hashed.WriteInto(Secret::Mode::Mem))) {
        LogError()(OT_PRETTY_CLASS())("Failed to hash shared secret").Flush();

        return false;
    }

    OT_ASSERT(hashed.size() >= sizeof(tag));

    return nullptr != std::memcpy(&tag, hashed.data(), sizeof(tag));
}

auto Asymmetric::hasCapability(
    const identity::NymCapability& capability) const noexcept -> bool
{
    switch (capability) {
        case (identity::NymCapability::SIGN_CHILDCRED):
        case (identity::NymCapability::SIGN_MESSAGE):
        case (identity::NymCapability::ENCRYPT_MESSAGE):
        case (identity::NymCapability::AUTHENTICATE_CONNECTION): {

            return true;
        }
        default: {
        }
    }

    return false;
}

auto Asymmetric::HasPrivate() const noexcept -> bool
{
    auto lock = Lock(lock_);

    return has_private(lock);
}

auto Asymmetric::has_private(const Lock&) const noexcept -> bool
{
    return has_private_;
}

auto Asymmetric::hashtype_map() noexcept -> const HashTypeMap&
{
    static const auto map = HashTypeMap{
        {crypto::HashType::Error, proto::HASHTYPE_ERROR},
        {crypto::HashType::None, proto::HASHTYPE_NONE},
        {crypto::HashType::Sha256, proto::HASHTYPE_SHA256},
        {crypto::HashType::Sha512, proto::HASHTYPE_SHA512},
        {crypto::HashType::Blake2b160, proto::HASHTYPE_BLAKE2B160},
        {crypto::HashType::Blake2b256, proto::HASHTYPE_BLAKE2B256},
        {crypto::HashType::Blake2b512, proto::HASHTYPE_BLAKE2B512},
        {crypto::HashType::Ripemd160, proto::HASHTYPE_RIPEMD160},
        {crypto::HashType::Sha1, proto::HASHTYPE_SHA1},
        {crypto::HashType::Sha256D, proto::HASHTYPE_SHA256D},
        {crypto::HashType::Sha256DC, proto::HASHTYPE_SHA256DC},
        {crypto::HashType::Bitcoin, proto::HASHTYPE_BITCOIN},
        {crypto::HashType::SipHash24, proto::HASHTYPE_SIPHASH24},
    };

    return map;
}

auto Asymmetric::NewSignature(
    const identifier::Generic& credentialID,
    const crypto::SignatureRole role,
    const crypto::HashType hash) const -> proto::Signature
{
    proto::Signature output{};
    output.set_version(sig_version_.at(role));
    output.set_credentialid(credentialID.asBase58(api_.Crypto()));
    output.set_role(translate(role));
    output.set_hashtype(
        (crypto::HashType::Error == hash) ? translate(SigHashType())
                                          : translate(hash));
    output.clear_signature();

    return output;
}

auto Asymmetric::Path() const noexcept -> const UnallocatedCString
{
    LogError()(OT_PRETTY_CLASS())("Incorrect key type.").Flush();

    return "";
}

auto Asymmetric::Path(proto::HDPath&) const noexcept -> bool
{
    LogError()(OT_PRETTY_CLASS())("Incorrect key type.").Flush();

    return false;
}

auto Asymmetric::PrivateKey(const PasswordPrompt& reason) const noexcept
    -> ReadView
{
    auto lock = Lock{lock_};

    return private_key(lock, reason);
}

auto Asymmetric::private_key(const Lock& lock, const PasswordPrompt& reason)
    const noexcept -> ReadView
{
    try {

        return get_private_key(lock, reason).Bytes();
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto Asymmetric::Serialize(Serialized& output) const noexcept -> bool
{
    auto lock = Lock{lock_};

    return serialize(lock, output);
}

auto Asymmetric::serialize(const Lock&, Serialized& output) const noexcept
    -> bool
{
    output.set_version(version_);
    output.set_role(opentxs::translate(role_));
    output.set_type(static_cast<proto::AsymmetricKeyType>(type_));
    output.set_key(key_.data(), key_.size());

    if (has_private_) {
        output.set_mode(proto::KEYMODE_PRIVATE);

        if (encrypted_key_) {
            *output.mutable_encryptedkey() = *encrypted_key_;
        }
    } else {
        output.set_mode(proto::KEYMODE_PUBLIC);
    }

    return true;
}

auto Asymmetric::SerializeKeyToData(
    const proto::AsymmetricKey& serializedKey) const -> ByteArray
{
    return api_.Factory().InternalSession().Data(serializedKey);
}

auto Asymmetric::Sign(
    const GetPreimage input,
    const crypto::SignatureRole role,
    proto::Signature& signature,
    const identifier::Generic& credential,
    const PasswordPrompt& reason,
    const crypto::HashType hash) const noexcept -> bool
{
    const auto type{(crypto::HashType::Error == hash) ? SigHashType() : hash};

    try {
        signature = NewSignature(credential, role, type);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Invalid signature role.").Flush();

        return false;
    }

    const auto preimage = input();
    auto& output = *signature.mutable_signature();

    return Sign(preimage, type, writer(output), reason);
}

auto Asymmetric::Sign(
    const ReadView preimage,
    const crypto::HashType hash,
    const AllocateOutput output,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == has_private_) {
        LogError()(OT_PRETTY_CLASS())("Missing private key").Flush();

        return false;
    }

    bool success =
        engine().Sign(preimage, private_key(lock, reason), hash, output);

    if (false == success) {
        LogError()(OT_PRETTY_CLASS())("Failed to sign preimage").Flush();
    }

    return success;
}

auto Asymmetric::signaturerole_map() noexcept -> const SignatureRoleMap&
{
    static const auto map = Asymmetric::SignatureRoleMap{
        {SignatureRole::PublicCredential, proto::SIGROLE_PUBCREDENTIAL},
        {SignatureRole::PrivateCredential, proto::SIGROLE_PRIVCREDENTIAL},
        {SignatureRole::NymIDSource, proto::SIGROLE_NYMIDSOURCE},
        {SignatureRole::Claim, proto::SIGROLE_CLAIM},
        {SignatureRole::ServerContract, proto::SIGROLE_SERVERCONTRACT},
        {SignatureRole::UnitDefinition, proto::SIGROLE_UNITDEFINITION},
        {SignatureRole::PeerRequest, proto::SIGROLE_PEERREQUEST},
        {SignatureRole::PeerReply, proto::SIGROLE_PEERREPLY},
        {SignatureRole::Context, proto::SIGROLE_CONTEXT},
        {SignatureRole::Account, proto::SIGROLE_ACCOUNT},
        {SignatureRole::ServerRequest, proto::SIGROLE_SERVERREQUEST},
        {SignatureRole::ServerReply, proto::SIGROLE_SERVERREPLY},
    };

    return map;
}

auto Asymmetric::translate(const crypto::SignatureRole in) noexcept
    -> proto::SignatureRole
{
    try {
        return signaturerole_map().at(in);
    } catch (...) {
        return proto::SIGROLE_ERROR;
    }
}

auto Asymmetric::translate(const crypto::HashType in) noexcept
    -> proto::HashType
{
    try {
        return hashtype_map().at(in);
    } catch (...) {
        return proto::HASHTYPE_ERROR;
    }
}

auto Asymmetric::translate(const proto::HashType in) noexcept
    -> crypto::HashType
{
    static const auto map = reverse_arbitrary_map<
        crypto::HashType,
        proto::HashType,
        HashTypeReverseMap>(hashtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::HashType::Error;
    }
}

auto Asymmetric::TransportKey(
    Data& publicKey,
    Secret& privateKey,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == has_private(lock)) { return false; }

    return provider_.SeedToCurveKey(
        private_key(lock, reason),
        privateKey.WriteInto(Secret::Mode::Mem),
        publicKey.WriteInto());
}

auto Asymmetric::Verify(const Data& plaintext, const proto::Signature& sig)
    const noexcept -> bool
{
    if (false == HasPublic()) {
        LogError()(OT_PRETTY_CLASS())("Missing public key").Flush();

        return false;
    }

    const auto output = engine().Verify(
        plaintext.Bytes(),
        PublicKey(),
        sig.signature(),
        translate(sig.hashtype()));

    if (false == output) {
        LogError()(OT_PRETTY_CLASS())("Invalid signature").Flush();
    }

    return output;
}

Asymmetric::~Asymmetric() = default;
}  // namespace opentxs::crypto::key::implementation
