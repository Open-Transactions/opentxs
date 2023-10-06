// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/asymmetric/base/Imp.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Ciphertext.pb.h>
#include <Enums.pb.h>
#include <Signature.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/crypto/Symmetric.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/crypto/symmetric/Key.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "internal/otx/common/crypto/OTSignatureMetadata.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/SecretStyle.hpp"    // IWYU pragma: keep
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/credential/Key.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::asymmetric::implementation
{
using enum SignatureRole;
constexpr frozen::unordered_map<crypto::SignatureRole, VersionNumber, 12>
    Key::sig_version_{
        {PublicCredential, 1},
        {PrivateCredential, 1},
        {NymIDSource, 1},
        {Claim, 1},
        {ServerContract, 1},
        {UnitDefinition, 1},
        {PeerRequest, 1},
        {PeerReply, 1},
        {Context, 2},
        {Account, 2},
        {ServerRequest, 3},
        {ServerReply, 3},
    };

Key::Key(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const crypto::asymmetric::Algorithm keyType,
    const crypto::asymmetric::Role role,
    const bool hasPublic,
    const bool hasPrivate,
    const VersionNumber version,
    ByteArray&& pubkey,
    EncryptedExtractor get,
    PlaintextExtractor getPlaintext,
    allocator_type alloc) noexcept(false)
    : KeyPrivate(alloc)
    , api_(api)
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
    assert_true(0 < version);
    assert_false(nullptr == metadata_);

    if (has_private_) {
        assert_true(encrypted_key_ || (0 < plaintext_key_.size()));
    }
}

Key::Key(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const crypto::asymmetric::Algorithm keyType,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    EncryptedExtractor getEncrypted,
    allocator_type alloc) noexcept(false)
    : Key(api,
          engine,
          keyType,
          role,
          true,
          true,
          version,
          api.Factory().Data(),
          getEncrypted,
          {},
          alloc)
{
}

Key::Key(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& serialized,
    EncryptedExtractor getEncrypted,
    allocator_type alloc) noexcept(false)
    : Key(api,
          engine,
          opentxs::translate(serialized.type()),
          opentxs::translate(serialized.role()),
          true,
          proto::KEYMODE_PRIVATE == serialized.mode(),
          serialized.version(),
          serialized.has_key() ? api.Factory().DataFromBytes(serialized.key())
                               : api.Factory().Data(),
          getEncrypted,
          {},
          alloc)
{
}

Key::Key(const Key& rhs, allocator_type alloc) noexcept
    : Key(
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
          [&] { return rhs.plaintext_key_; },
          alloc)
{
}

Key::Key(
    const Key& rhs,
    const ReadView newPublic,
    allocator_type alloc) noexcept
    : Key(
          rhs.api_,
          rhs.provider_,
          rhs.type_,
          rhs.role_,
          true,
          false,
          rhs.version_,
          rhs.api_.Factory().DataFromBytes(newPublic),
          [&](auto&, auto&) -> EncryptedKey { return {}; },
          {},
          alloc)
{
}

Key::Key(
    const Key& rhs,
    ByteArray&& newPublicKey,
    Secret&& newSecretKey,
    allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , api_(rhs.api_)
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
    assert_true(0 < version_);
    assert_false(nullptr == metadata_);

    if (has_private_) {
        assert_true(encrypted_key_ || (0 < plaintext_key_.size()));
    }
}

auto Key::asPublic(allocator_type alloc) const noexcept -> asymmetric::Key
{
    auto out = asymmetric::Key{clone(alloc)};

    if (out.ErasePrivateData()) {

        return out;
    } else {
        LogError()()("failed to erase private data").Flush();

        return {};
    }
}

auto Key::CalculateHash(
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
        LogError()()("Failed to calculate hash").Flush();

        return ByteArray{};
    }

    return output;
}

auto Key::CalculateID(identifier::Generic& output) const noexcept -> bool
{
    if (false == HasPublic()) {
        LogError()()("Missing public key").Flush();

        return false;
    }

    output = api_.Factory().IdentifierFromPreimage(PublicKey());

    return false == output.empty();
}

auto Key::CalculateTag(
    const identity::Authority& nym,
    const crypto::asymmetric::Algorithm type,
    const PasswordPrompt& reason,
    std::uint32_t& tag,
    Secret& password) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == has_private_) {
        LogError()()("Not a private key.").Flush();

        return false;
    }

    try {
        const auto& cred = nym.GetTagCredential(type);
        const auto& key =
            cred.Internal()
                .asKey()
                .GetKeypair(type, opentxs::crypto::asymmetric::Role::Encrypt)
                .GetPublicKey();

        if (false == get_tag(lock, key, nym.GetMasterCredID(), reason, tag)) {
            LogError()()("Failed to calculate tag.").Flush();

            return false;
        }

        if (false == get_password(lock, key, reason, password)) {
            LogError()()("Failed to calculate session password.").Flush();

            return false;
        }

        return true;
    } catch (...) {
        LogError()()("Invalid credential").Flush();

        return false;
    }
}

auto Key::CalculateTag(
    const asymmetric::Key& dhKey,
    const identifier::Generic& credential,
    const PasswordPrompt& reason,
    std::uint32_t& tag) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == has_private_) {
        LogError()()("Not a private key.").Flush();

        return false;
    }

    return get_tag(lock, dhKey, credential, reason, tag);
}

auto Key::CalculateSessionPassword(
    const asymmetric::Key& dhKey,
    const PasswordPrompt& reason,
    Secret& password) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == has_private_) {
        LogError()()("Not a private key.").Flush();

        return false;
    }

    return get_password(lock, dhKey, reason, password);
}

auto Key::create_key(
    symmetric::Key& sessionKey,
    const crypto::AsymmetricProvider& provider,
    const Parameters& options,
    const crypto::asymmetric::Role role,
    Writer&& publicKey,
    Writer&& privateKey,
    const opentxs::Secret& prv,
    Writer&& params,
    const PasswordPrompt& reason) -> std::unique_ptr<proto::Ciphertext>
{
    generate_key(
        provider,
        options,
        role,
        std::move(publicKey),
        std::move(privateKey),
        std::move(params));
    auto pOutput = std::make_unique<proto::Ciphertext>();

    assert_false(nullptr == pOutput);

    auto& output = *pOutput;

    if (false == encrypt_key(prv.Bytes(), true, sessionKey, reason, output)) {
        throw std::runtime_error("Failed to encrypt key");
    }

    return pOutput;
}

auto Key::encrypt_key(
    ReadView plaintext,
    bool attach,
    symmetric::Key& sessionKey,
    const PasswordPrompt& reason) noexcept -> std::unique_ptr<proto::Ciphertext>
{
    auto output = std::make_unique<proto::Ciphertext>();

    if (false == bool(output)) {
        LogError()()("Failed to construct output").Flush();

        return {};
    }

    auto& ciphertext = *output;

    if (encrypt_key(plaintext, attach, sessionKey, reason, ciphertext)) {

        return output;
    } else {

        return {};
    }
}

auto Key::encrypt_key(
    ReadView plaintext,
    bool attach,
    symmetric::Key& sessionKey,
    const PasswordPrompt& reason,
    proto::Ciphertext& out) noexcept -> bool
{
    const auto encrypted =
        sessionKey.Internal().Encrypt(plaintext, out, reason, attach);

    if (false == encrypted) {
        LogError()()("Failed to encrypt key").Flush();

        return false;
    }

    return true;
}

auto Key::ErasePrivateData() noexcept -> bool
{
    auto lock = Lock{lock_};
    erase_private_data(lock);

    return true;
}

auto Key::erase_private_data(const Lock&) -> void
{
    plaintext_key_.clear();
    encrypted_key_.reset();
    has_private_ = false;
}

auto Key::generate_key(
    const crypto::AsymmetricProvider& provider,
    const Parameters& options,
    const crypto::asymmetric::Role role,
    Writer&& publicKey,
    Writer&& privateKey,
    Writer&& params) noexcept(false) -> void
{
    const auto generated = provider.RandomKeypair(
        std::move(privateKey),
        std::move(publicKey),
        role,
        options,
        std::move(params));

    if (false == generated) {
        throw std::runtime_error("Failed to generate key");
    }
}

auto Key::get_password(
    const Lock& lock,
    const asymmetric::Key& target,
    const PasswordPrompt& reason,
    Secret& password) const noexcept -> bool
{
    return provider_.SharedSecret(
        target.PublicKey(),
        private_key(lock, reason),
        SecretStyle::Default,
        password);
}

auto Key::get_private_key(const Lock&, const PasswordPrompt& reason) const
    noexcept(false) -> Secret&
{
    if (0 == plaintext_key_.size()) {
        if (false == bool(encrypted_key_)) {
            throw std::runtime_error{"Missing encrypted private key"};
        }

        const auto& privateKey = *encrypted_key_;
        std::byte b[512_uz];  // NOLINT(modernize-avoid-c-arrays)
        auto mono = alloc::MonotonicUnsync{std::addressof(b), sizeof(b)};
        auto sessionKey = api_.Crypto().Symmetric().InternalSymmetric().Key(
            privateKey.key(),
            opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305,
            {std::addressof(mono)});

        if (false == sessionKey) {
            throw std::runtime_error{"Failed to extract session key"};
        }

        const auto decrypted = sessionKey.Internal().Decrypt(
            privateKey, plaintext_key_.WriteInto(Secret::Mode::Mem), reason);

        if (false == decrypted) {
            throw std::runtime_error{"Failed to decrypt private key"};
        }
    }

    return plaintext_key_;
}

auto Key::get_tag(
    const Lock& lock,
    const asymmetric::Key& target,
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
        LogVerbose()()("Failed to calculate shared secret").Flush();

        return false;
    }

    if (false == api_.Crypto().Hash().HMAC(
                     crypto::HashType::Sha256,
                     password.Bytes(),
                     credential.Bytes(),
                     hashed.WriteInto(Secret::Mode::Mem))) {
        LogError()()("Failed to hash shared secret").Flush();

        return false;
    }

    assert_true(hashed.size() >= sizeof(tag));

    return nullptr != std::memcpy(&tag, hashed.data(), sizeof(tag));
}

auto Key::GetMetadata() const noexcept -> const OTSignatureMetadata*
{
    return metadata_.get();
}

auto Key::HasCapability(identity::NymCapability capability) const noexcept
    -> bool
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

auto Key::HasPrivate() const noexcept -> bool
{
    auto lock = Lock(lock_);

    return has_private(lock);
}

auto Key::has_private(const Lock&) const noexcept -> bool
{
    return has_private_;
}

auto Key::hashtype_map() noexcept -> const HashTypeMap&
{
    using enum crypto::HashType;
    using enum proto::HashType;
    static constexpr auto map = HashTypeMap{
        {Error, HASHTYPE_ERROR},
        {None, HASHTYPE_NONE},
        {Sha256, HASHTYPE_SHA256},
        {Sha512, HASHTYPE_SHA512},
        {Blake2b160, HASHTYPE_BLAKE2B160},
        {Blake2b256, HASHTYPE_BLAKE2B256},
        {Blake2b512, HASHTYPE_BLAKE2B512},
        {Ripemd160, HASHTYPE_RIPEMD160},
        {Sha1, HASHTYPE_SHA1},
        {Sha256D, HASHTYPE_SHA256D},
        {Sha256DC, HASHTYPE_SHA256DC},
        {Bitcoin, HASHTYPE_BITCOIN},
        {SipHash24, HASHTYPE_SIPHASH24},
        {X11, HASHTYPE_X11},
    };

    return map;
}

auto Key::IsValid() const noexcept -> bool
{
    return has_public_ || has_private_;
}

auto Key::new_signature(
    const identifier::Generic& credentialID,
    const crypto::SignatureRole role,
    const crypto::HashType hash) const -> proto::Signature
{
    proto::Signature output{};
    output.set_version(sig_version_.at(role));
    credentialID.Internal().Serialize(*output.mutable_credentialid());
    output.set_role(translate(role));
    output.set_hashtype(
        (crypto::HashType::Error == hash) ? translate(PreferredHash())
                                          : translate(hash));
    output.clear_signature();

    return output;
}

auto Key::operator==(const proto::AsymmetricKey& rhs) const noexcept -> bool
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

auto Key::Path() const noexcept -> const UnallocatedCString
{
    LogError()()("Incorrect key type.").Flush();

    return "";
}

auto Key::Path(proto::HDPath&) const noexcept -> bool
{
    LogError()()("Incorrect key type.").Flush();

    return false;
}

auto Key::PreferredHash() const noexcept -> crypto::HashType
{
    return crypto::HashType::Blake2b256;
}

auto Key::PrivateKey(const PasswordPrompt& reason) const noexcept -> ReadView
{
    auto lock = Lock{lock_};

    return private_key(lock, reason);
}

auto Key::private_key(const Lock& lock, const PasswordPrompt& reason)
    const noexcept -> ReadView
{
    try {

        return get_private_key(lock, reason).Bytes();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}

auto Key::PublicKey() const noexcept -> ReadView { return key_.Bytes(); }

auto Key::Serialize(Serialized& output) const noexcept -> bool
{
    auto lock = Lock{lock_};

    return serialize(lock, output);
}

auto Key::serialize(const Lock&, Serialized& output) const noexcept -> bool
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

auto Key::SerializeKeyToData(const proto::AsymmetricKey& serializedKey) const
    -> ByteArray
{
    return api_.Factory().Internal().Data(serializedKey);
}

auto Key::Sign(
    const GetPreimage input,
    const crypto::SignatureRole role,
    proto::Signature& signature,
    const identifier::Generic& credential,
    const PasswordPrompt& reason) const noexcept -> bool
{
    return Sign(input, role, signature, credential, PreferredHash(), reason);
}

auto Key::Sign(
    const GetPreimage input,
    const crypto::SignatureRole role,
    proto::Signature& signature,
    const identifier::Generic& credential,
    const crypto::HashType hash,
    const PasswordPrompt& reason) const noexcept -> bool
{
    const auto type{(crypto::HashType::Error == hash) ? PreferredHash() : hash};

    try {
        signature = new_signature(credential, role, type);
    } catch (...) {
        LogError()()("Invalid signature role.").Flush();

        return false;
    }

    const auto preimage = input();
    auto& output = *signature.mutable_signature();

    return Sign(preimage, writer(output), type, reason);
}

auto Key::Sign(
    ReadView preimage,
    Writer&& output,
    crypto::HashType hash,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == has_private_) {
        LogError()()("Missing private key").Flush();

        return false;
    }

    bool success = provider_.Sign(
        preimage, private_key(lock, reason), hash, std::move(output));

    if (false == success) { LogError()()("Failed to sign preimage").Flush(); }

    return success;
}

auto Key::signaturerole_map() noexcept -> const SignatureRoleMap&
{
    using enum SignatureRole;
    using enum proto::SignatureRole;
    static constexpr auto map = SignatureRoleMap{
        {PublicCredential, SIGROLE_PUBCREDENTIAL},
        {PrivateCredential, SIGROLE_PRIVCREDENTIAL},
        {NymIDSource, SIGROLE_NYMIDSOURCE},
        {Claim, SIGROLE_CLAIM},
        {ServerContract, SIGROLE_SERVERCONTRACT},
        {UnitDefinition, SIGROLE_UNITDEFINITION},
        {PeerRequest, SIGROLE_PEERREQUEST},
        {PeerReply, SIGROLE_PEERREPLY},
        {Context, SIGROLE_CONTEXT},
        {Account, SIGROLE_ACCOUNT},
        {ServerRequest, SIGROLE_SERVERREQUEST},
        {ServerReply, SIGROLE_SERVERREPLY},
    };

    return map;
}

auto Key::translate(const crypto::SignatureRole in) noexcept
    -> proto::SignatureRole
{
    try {
        return signaturerole_map().at(in);
    } catch (...) {
        return proto::SIGROLE_ERROR;
    }
}

auto Key::translate(const crypto::HashType in) noexcept -> proto::HashType
{
    try {
        return hashtype_map().at(in);
    } catch (...) {
        return proto::HASHTYPE_ERROR;
    }
}

auto Key::translate(const proto::HashType in) noexcept -> crypto::HashType
{
    static const auto map = frozen::invert_unordered_map(hashtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::HashType::Error;
    }
}

auto Key::TransportKey(
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

auto Key::Verify(ReadView plaintext, ReadView sig) const noexcept -> bool
{
    if (false == HasPublic()) {
        LogError()()("Missing public key").Flush();

        return false;
    }

    const auto proto = proto::Factory<proto::Signature>(sig);
    const auto output = provider_.Verify(
        plaintext, PublicKey(), proto.signature(), translate(proto.hashtype()));

    if (false == output) { LogError()()("Invalid signature").Flush(); }

    return output;
}

auto Key::Verify(const Data& plaintext, const proto::Signature& sig)
    const noexcept -> bool
{
    if (false == HasPublic()) {
        LogError()()("Missing public key").Flush();

        return false;
    }

    const auto output = provider_.Verify(
        plaintext.Bytes(),
        PublicKey(),
        sig.signature(),
        translate(sig.hashtype()));

    if (false == output) { LogError()()("Invalid signature").Flush(); }

    return output;
}

Key::~Key() = default;
}  // namespace opentxs::crypto::asymmetric::implementation
