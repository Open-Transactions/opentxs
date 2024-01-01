// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/credential/Key.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/AsymmetricKey.pb.h>
#include <opentxs/protobuf/Credential.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/KeyCredential.pb.h>
#include <opentxs/protobuf/Signature.pb.h>
#include <cstdint>
#include <memory>
#include <stdexcept>

#include "identity/credential/Base.hpp"
#include "internal/crypto/Parameters.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/identity/credential/Blank.hpp"
#include "internal/otx/common/crypto/OTSignatureMetadata.hpp"
#include "internal/otx/common/crypto/Signature.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/EcdsaCurve.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/Mode.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identifier/HDSeed.hpp"        // IWYU pragma: keep
#include "opentxs/identity/CredentialType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/NymCapability.hpp"   // IWYU pragma: keep
#include "opentxs/identity/Types.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::identity::credential::internal
{
auto Key::Blank() noexcept -> Key&
{
    static auto blank = blank::Key{};

    return blank;
}
}  // namespace opentxs::identity::credential::internal

namespace opentxs::identity::credential::implementation
{
const VersionConversionMap Key::credential_subversion_{
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {6, 2},
};
const VersionConversionMap Key::subversion_to_key_version_{
    {1, 1},
    {2, 2},
};

Key::Key(
    const api::Session& api,
    const identity::internal::Authority& parent,
    const identity::Source& source,
    const crypto::Parameters& params,
    const VersionNumber version,
    const identity::CredentialRole role,
    const PasswordPrompt& reason,
    const identifier_type& masterID,
    const bool useProvided) noexcept(false)
    : credential::implementation::Base(
          api,
          parent,
          source,
          params,
          version,
          role,
          crypto::asymmetric::Mode::Private,
          masterID)
    , subversion_(credential_subversion_.at(Version()))
    , signing_key_(signing_key(api_, params, subversion_, useProvided, reason))
    , authentication_key_(new_key(
          api_,
          protobuf::KEYROLE_AUTH,
          params,
          subversion_to_key_version_.at(subversion_),
          reason))
    , encryption_key_(new_key(
          api_,
          protobuf::KEYROLE_ENCRYPT,
          params,
          subversion_to_key_version_.at(subversion_),
          reason))
{
    if (0 == version) { throw std::runtime_error("Invalid version"); }
}

Key::Key(
    const api::Session& api,
    const identity::internal::Authority& parent,
    const identity::Source& source,
    const protobuf::Credential& serialized,
    const identifier_type& masterID) noexcept(false)
    : credential::implementation::Base(
          api,
          parent,
          source,
          serialized,
          masterID)
    , subversion_(credential_subversion_.at(Version()))
    , signing_key_(deserialize_key(api, protobuf::KEYROLE_SIGN, serialized))
    , authentication_key_(
          deserialize_key(api, protobuf::KEYROLE_AUTH, serialized))
    , encryption_key_(
          deserialize_key(api, protobuf::KEYROLE_ENCRYPT, serialized))
{
}

auto Key::addKeyCredentialtoSerializedCredential(
    std::shared_ptr<Base::SerializedType> credential,
    const bool addPrivate) const -> bool
{
    std::unique_ptr<protobuf::KeyCredential> keyCredential(
        new protobuf::KeyCredential);

    if (!keyCredential) {
        LogError()()("Failed to allocate keyCredential protobuf.").Flush();

        return false;
    }

    keyCredential->set_version(subversion_);

    // These must be serialized in this order
    const bool auth = addKeytoSerializedKeyCredential(
        *keyCredential, addPrivate, protobuf::KEYROLE_AUTH);
    const bool encrypt = addKeytoSerializedKeyCredential(
        *keyCredential, addPrivate, protobuf::KEYROLE_ENCRYPT);
    const bool sign = addKeytoSerializedKeyCredential(
        *keyCredential, addPrivate, protobuf::KEYROLE_SIGN);

    if (auth && encrypt && sign) {
        if (addPrivate) {
            keyCredential->set_mode(
                translate(crypto::asymmetric::Mode::Private));
            credential->set_allocated_privatecredential(
                keyCredential.release());

            return true;
        } else {
            keyCredential->set_mode(
                translate(crypto::asymmetric::Mode::Public));
            credential->set_allocated_publiccredential(keyCredential.release());

            return true;
        }
    }

    return false;
}

auto Key::addKeytoSerializedKeyCredential(
    protobuf::KeyCredential& credential,
    const bool getPrivate,
    const protobuf::KeyRole role) const -> bool
{
    const crypto::key::Keypair* pKey{nullptr};

    switch (role) {
        case protobuf::KEYROLE_AUTH: {
            pKey = &authentication_key_.get();
        } break;
        case protobuf::KEYROLE_ENCRYPT: {
            pKey = &encryption_key_.get();
        } break;
        case protobuf::KEYROLE_SIGN: {
            pKey = &signing_key_.get();
        } break;
        case protobuf::KEYROLE_ERROR:
        default: {
            return false;
        }
    }

    if (nullptr == pKey) { return false; }

    auto key = protobuf::AsymmetricKey{};
    if (false == pKey->Serialize(key, getPrivate)) { return false; }

    key.set_role(role);

    auto* newKey = credential.add_key();
    *newKey = key;

    return true;
}

auto Key::deserialize_key(
    const api::Session& api,
    const int index,
    const protobuf::Credential& credential) -> OTKeypair
{
    const bool hasPrivate =
        (protobuf::KEYMODE_PRIVATE == credential.mode()) ? true : false;

    const auto publicKey = credential.publiccredential().key(index - 1);

    if (hasPrivate) {
        const auto privateKey = credential.privatecredential().key(index - 1);

        return api.Factory().Internal().Session().Keypair(
            publicKey, privateKey);
    }

    return api.Factory().Internal().Session().Keypair(publicKey);
}

auto Key::GetKeypair(const opentxs::crypto::asymmetric::Role role) const
    -> const crypto::key::Keypair&
{
    return GetKeypair(crypto::asymmetric::Algorithm::Null, role);
}

auto Key::GetKeypair(
    const crypto::asymmetric::Algorithm type,
    const opentxs::crypto::asymmetric::Role role) const
    -> const crypto::key::Keypair&
{
    const crypto::key::Keypair* output{nullptr};

    switch (role) {
        case opentxs::crypto::asymmetric::Role::Auth: {
            output = &authentication_key_.get();
        } break;
        case opentxs::crypto::asymmetric::Role::Encrypt: {
            output = &encryption_key_.get();
        } break;
        case opentxs::crypto::asymmetric::Role::Sign: {
            output = &signing_key_.get();
        } break;
        case opentxs::crypto::asymmetric::Role::Error:
        default: {
            throw std::out_of_range("wrong key type");
        }
    }

    assert_false(nullptr == output);

    if (crypto::asymmetric::Algorithm::Null != type) {
        if (type != output->GetPublicKey().Type()) {
            throw std::out_of_range("wrong key type");
        }
    }

    return *output;
}

// NOTE: You might ask, if we are using theSignature's metadata to narrow down
// the key type, then why are we still passing the key type as a separate
// parameter? Good question. Because often, theSignature will have no metadata
// at all! In that case, normally we would just NOT return any keys, period.
// Because we assume, if a key credential signed it, then it WILL have metadata,
// and if it doesn't have metadata, then a key credential did NOT sign it, and
// therefore we know from the get-go that none of the keys from the key
// credentials will work to verify it, either. That's why, normally, we don't
// return any keys if theSignature has no metadata. BUT...Let's say you know
// this, that the signature has no metadata, yet you also still believe it may
// be signed with one of these keys. Further, while you don't know exactly which
// key it actually is, let's say you DO know by context that it's a signing key,
// or an authentication key, or an encryption key. So you specify that. In which
// case, OT should return all possible matching pubkeys based on that 1-letter
// criteria, instead of its normal behavior, which is to return all possible
// matching pubkeys based on a full match of the metadata.
auto Key::GetPublicKeysBySignature(
    crypto::key::Keypair::Keys& listOutput,
    const opentxs::Signature& theSignature,
    char cKeyType) const
    -> std::int32_t  // 'S' (signing key) or 'E' (encryption key)
                     // or 'A' (authentication key)
{
    // Key type was not specified, because we only want keys that match the
    // metadata on theSignature.
    // And if theSignature has no metadata, then we want to return 0 keys.
    if (('0' == cKeyType) && !theSignature.getMetaData().HasMetadata()) {
        return 0;
    }

    // By this point, we know that EITHER exact metadata matches must occur, and
    // the signature DOES have metadata, ('0')
    // OR the search is only for 'A', 'E', or 'S' candidates, based on cKeyType,
    // and that the signature's metadata
    // can additionally narrow the search down, if it's present, which in this
    // case it's not guaranteed to be.
    std::int32_t nCount = 0;

    switch (cKeyType) {
        // Specific search only for signatures with metadata.
        // FYI, theSignature.getMetaData().HasMetadata() is true, in this case.
        case '0': {
            // That's why I can just assume theSignature has a key type here:
            switch (theSignature.getMetaData().GetKeyType()) {
                case 'A':
                    nCount = authentication_key_->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                case 'E':
                    nCount = encryption_key_->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                case 'S':
                    nCount = signing_key_->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                default:
                    LogError()()("Unexpected keytype value in signature "
                                 "metadata: ")(
                        theSignature.getMetaData().GetKeyType())(" (Failure)!")
                        .Flush();
                    return 0;
            }
            break;
        }
        // Generalized search which specifies key type and returns keys
        // even for signatures with no metadata. (When metadata is present,
        // it's still used to eliminate keys.)
        case 'A':
            nCount = authentication_key_->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        case 'E':
            nCount = encryption_key_->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        case 'S':
            nCount = signing_key_->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        default:
            LogError()()("Unexpected value for cKeyType (should be 0, A, E, or "
                         "S): ")(cKeyType)(".")
                .Flush();
            return 0;
    }
    return nCount;
}

auto Key::hasCapability(const NymCapability& capability) const -> bool
{
    switch (capability) {
        case NymCapability::SIGN_MESSAGE: {
            return signing_key_->CheckCapability(capability);
        }
        case NymCapability::ENCRYPT_MESSAGE: {
            return encryption_key_->CheckCapability(capability);
        }
        case NymCapability::AUTHENTICATE_CONNECTION: {
            return authentication_key_->CheckCapability(capability);
        }
        case NymCapability::SIGN_CHILDCRED:
        default: {

            return false;
        }
    }
}

auto Key::id_form() const -> std::shared_ptr<SerializedType>
{
    auto out = Base::id_form();

    addKeyCredentialtoSerializedCredential(out, false);

    return out;
}

auto Key::new_key(
    const api::Session& api,
    const protobuf::KeyRole role,
    const crypto::Parameters& params,
    const VersionNumber version,
    const PasswordPrompt& reason,
    const ReadView dh) noexcept(false) -> OTKeypair
{
    switch (params.credentialType()) {
        case identity::CredentialType::Legacy: {
            auto revised{params};
            revised.SetDHParams(dh);

            return api.Factory().Internal().Session().Keypair(
                revised, version, translate(role), reason);
        }
        case identity::CredentialType::HD: {
            if (false == api::crypto::HaveHDKeys()) {
                throw std::runtime_error("Missing HD key support");
            }

            const auto curve =
                crypto::AsymmetricProvider::KeyTypeToCurve(params.Algorithm());

            if (crypto::EcdsaCurve::invalid == curve) {
                throw std::runtime_error("Invalid curve type");
            }

            return api.Factory().Internal().Session().Keypair(
                params.Seed(),
                params.Nym(),
                params.Credset(),
                params.CredIndex(),
                curve,
                translate(role),
                reason);
        }
        case identity::CredentialType::Error:
        default: {
            throw std::runtime_error("Unsupported credential type");
        }
    }
}

auto Key::SelfSign(
    const PasswordPrompt& reason,
    Signatures& out,
    const std::optional<Secret>,
    const bool onlyPrivate) const noexcept(false) -> bool
{
    auto publicSignature = std::make_shared<protobuf::Signature>();
    auto privateSignature = std::make_shared<protobuf::Signature>();
    bool havePublicSig = false;

    if (!onlyPrivate) {
        const auto publicVersion = serialize(AS_PUBLIC, WITHOUT_SIGNATURES);
        auto& signature = *publicVersion->add_signature();
        havePublicSig = Sign(
            [&]() -> UnallocatedCString { return to_string(*publicVersion); },
            crypto::SignatureRole::PublicCredential,
            signature,
            reason,
            opentxs::crypto::asymmetric::Role::Sign,
            crypto::HashType::Error);

        assert_true(havePublicSig);

        if (havePublicSig) {
            publicSignature->CopyFrom(signature);
            out.push_back(publicSignature);
        }
    }

    auto privateVersion = serialize(AS_PRIVATE, WITHOUT_SIGNATURES);
    auto& signature = *privateVersion->add_signature();
    const bool havePrivateSig = Sign(
        [&]() -> UnallocatedCString { return to_string(*privateVersion); },
        crypto::SignatureRole::PrivateCredential,
        signature,
        reason,
        opentxs::crypto::asymmetric::Role::Sign,
        crypto::HashType::Error);

    assert_true(havePrivateSig);

    if (havePrivateSig) {
        privateSignature->CopyFrom(signature);
        out.push_back(privateSignature);
    }

    return ((havePublicSig | onlyPrivate) && havePrivateSig);
}

auto Key::serialize(
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
    -> std::shared_ptr<Base::SerializedType>
{
    auto out = Base::serialize(asPrivate, asSigned);

    if (asPrivate) { addKeyCredentialtoSerializedCredential(out, true); }

    return out;
}

auto Key::Sign(
    const crypto::GetPreimage input,
    const crypto::SignatureRole role,
    protobuf::Signature& signature,
    const PasswordPrompt& reason,
    opentxs::crypto::asymmetric::Role key,
    const crypto::HashType hash) const -> bool
{
    const crypto::key::Keypair* keyToUse{nullptr};

    switch (key) {
        case (crypto::asymmetric::Role::Auth): {
            keyToUse = &authentication_key_.get();
        } break;
        case (crypto::asymmetric::Role::Sign): {
            keyToUse = &signing_key_.get();
        } break;
        case (crypto::asymmetric::Role::Error):
        case (crypto::asymmetric::Role::Encrypt):
        default: {
            LogError()(": Can not sign with the specified key.").Flush();
            return false;
        }
    }

    if (nullptr != keyToUse) {
        try {
            return keyToUse->GetPrivateKey().Internal().Sign(
                input, role, signature, ID(), hash, reason);
        } catch (...) {
        }
    }

    return false;
}

auto Key::signing_key(
    const api::Session& api,
    const crypto::Parameters& params,
    const VersionNumber subversion,
    const bool useProvided,
    const PasswordPrompt& reason) noexcept(false) -> OTKeypair
{
    if (useProvided) {
        if (params.Internal().Keypair()) {

            return params.Internal().Keypair();
        } else {
            throw std::runtime_error("Invalid provided keypair");
        }
    } else {

        return new_key(
            api,
            protobuf::KEYROLE_SIGN,
            params,
            subversion_to_key_version_.at(subversion),
            reason);
    }
}

auto Key::TransportKey(
    Data& publicKey,
    Secret& privateKey,
    const PasswordPrompt& reason) const -> bool
{
    return authentication_key_->GetTransportKey(publicKey, privateKey, reason);
}

auto Key::Verify(
    const Data& plaintext,
    const protobuf::Signature& sig,
    const opentxs::crypto::asymmetric::Role key) const -> bool
{
    const crypto::key::Keypair* keyToUse = nullptr;

    switch (key) {
        case crypto::asymmetric::Role::Auth: {
            keyToUse = &authentication_key_.get();
        } break;
        case crypto::asymmetric::Role::Sign: {
            keyToUse = &signing_key_.get();
        } break;
        case crypto::asymmetric::Role::Error:
        case crypto::asymmetric::Role::Encrypt:
        default: {
            LogError()()("Can not verify signatures with the "
                         "specified key.")
                .Flush();
            return false;
        }
    }

    assert_false(nullptr == keyToUse);

    try {

        return keyToUse->GetPublicKey().Internal().Verify(plaintext, sig);
    } catch (...) {
        LogError()()("Failed to verify signature.").Flush();

        return false;
    }
}

void Key::sign(
    const identity::credential::internal::Primary& master,
    const PasswordPrompt& reason,
    Signatures& out) noexcept(false)
{
    Base::sign(master, reason, out);

    if (false == SelfSign(reason, out)) {
        throw std::runtime_error("Failed to obtain self signature");
    }
}

auto Key::verify_internally() const -> bool
{
    // Perform common Credential verifications
    if (!Base::verify_internally()) { return false; }

    // All KeyCredentials must sign themselves
    if (!VerifySignedBySelf()) {
        LogConsole()()("Failed verifying key credential: it's not "
                       "signed by itself (its own signing key).")
            .Flush();
        return false;
    }

    return true;
}

auto Key::VerifySig(
    const protobuf::Signature& sig,
    const CredentialModeFlag asPrivate) const -> bool
{
    std::shared_ptr<Base::SerializedType> serialized;

    if ((crypto::asymmetric::Mode::Private != mode_) && asPrivate) {
        LogError()()(
            "Can not serialize a public credential as a private credential.")
            .Flush();
        return false;
    }

    if (asPrivate) {
        serialized = serialize(AS_PRIVATE, WITHOUT_SIGNATURES);
    } else {
        serialized = serialize(AS_PUBLIC, WITHOUT_SIGNATURES);
    }

    auto& signature = *serialized->add_signature();
    signature.CopyFrom(sig);
    signature.clear_signature();
    auto plaintext = api_.Factory().Internal().Data(*serialized);

    return Verify(plaintext, sig, opentxs::crypto::asymmetric::Role::Sign);
}

auto Key::VerifySignedBySelf() const -> bool
{
    auto publicSig = SelfSignature(PUBLIC_VERSION);

    if (!publicSig) {
        LogError()()("Could not find public self signature.").Flush();

        return false;
    }

    const bool goodPublic = VerifySig(*publicSig, PUBLIC_VERSION);

    if (!goodPublic) {
        LogError()()("Could not verify public self signature.").Flush();

        return false;
    }

    if (Private()) {
        auto privateSig = SelfSignature(PRIVATE_VERSION);

        if (!privateSig) {
            LogError()()("Could not find private self signature.").Flush();

            return false;
        }

        const bool goodPrivate = VerifySig(*privateSig, PRIVATE_VERSION);

        if (!goodPrivate) {
            LogError()()("Could not verify private self signature.").Flush();

            return false;
        }
    }

    return true;
}

Key::~Key() = default;
}  // namespace opentxs::identity::credential::implementation
