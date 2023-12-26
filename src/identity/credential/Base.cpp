// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/credential/Base.tpp"  // IWYU pragma: associated

#include <ChildCredentialParameters.pb.h>
#include <Credential.pb.h>
#include <Enums.pb.h>
#include <Signature.pb.h>
#include <memory>
#include <stdexcept>

#include "core/contract/Signable.hpp"
#include "identity/credential/Base.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/identity/Authority.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/SignatureRole.hpp"
#include "opentxs/crypto/asymmetric/Mode.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/CredentialRole.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/identity/credential/Primary.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::identity::credential::internal
{
auto Base::asKey() const noexcept -> const Key& { return Key::Blank(); }

auto Base::asKey() noexcept -> Key& { return Key::Blank(); }
}  // namespace opentxs::identity::credential::internal

namespace opentxs::identity::credential::implementation
{
Base::Base(
    const api::Session& api,
    const identity::internal::Authority& parent,
    const identity::Source& source,
    const crypto::Parameters& nymParameters,
    const VersionNumber version,
    const identity::CredentialRole role,
    const crypto::asymmetric::Mode mode,
    const identifier_type& masterID) noexcept
    : Signable(api, {}, version, {}, {})
    , parent_(parent)
    , source_(source)
    , nym_id_(source.NymID())
    , master_id_(masterID)
    , type_(nymParameters.credentialType())
    , role_(role)
    , mode_(mode)
{
}

Base::Base(
    const api::Session& api,
    const identity::internal::Authority& parent,
    const identity::Source& source,
    const proto::Credential& serialized,
    const identifier_type& masterID) noexcept(false)
    : Signable(
          api,
          {},
          serialized.version(),
          {},
          {},
          api.Factory().Internal().Identifier(serialized.id()),
          extract_signatures(serialized))
    , parent_(parent)
    , source_(source)
    , nym_id_(source.NymID())
    , master_id_(masterID)
    , type_(translate(serialized.type()))
    , role_(translate(serialized.role()))
    , mode_(translate(serialized.mode()))
{
    const auto expected = api_.Factory().Internal().NymID(serialized.nymid());

    if (expected != nym_id_) {
        throw std::runtime_error(
            "Attempting to load credential for incorrect nym");
    }
}

auto Base::add_master_signature(
    const identity::credential::internal::Primary& master,
    const PasswordPrompt& reason,
    Signatures& out) noexcept(false) -> void
{
    auto serializedMasterSignature = std::make_shared<proto::Signature>();
    auto serialized = serialize(AS_PUBLIC, WITHOUT_SIGNATURES);
    auto& signature = *serialized->add_signature();

    const auto havePublicSig = master.Sign(
        [&serialized]() -> UnallocatedCString {
            return proto::ToString(*serialized);
        },
        crypto::SignatureRole::PublicCredential,
        signature,
        reason);

    if (false == havePublicSig) {
        throw std::runtime_error("Attempting to obtain master signature");
    }

    serializedMasterSignature->CopyFrom(signature);
    out.push_back(serializedMasterSignature);
}

auto Base::asString(const bool asPrivate) const -> UnallocatedCString
{
    auto credential = SerializedType{};
    auto dataCredential = ByteArray{};
    auto stringCredential = String::Factory();
    if (false == Serialize(credential, asPrivate, WITH_SIGNATURES)) {
        return {};
    }
    dataCredential = api_.Factory().Internal().Data(credential);
    auto armoredCredential = api_.Factory().Internal().Armored(dataCredential);
    armoredCredential->WriteArmoredString(stringCredential, "Credential");

    return stringCredential->Get();
}

auto Base::calculate_id() const -> identifier_type
{
    auto idVersion = id_form();

    assert_false(nullptr == idVersion);

    return api_.Factory().Internal().Session().IdentifierFromPreimage(
        *idVersion);
}

auto Base::extract_signatures(const SerializedType& serialized) -> Signatures
{
    auto output = Signatures{};

    for (const auto& it : serialized.signature()) {
        output.push_back(std::make_shared<proto::Signature>(it));
    }

    return output;
}

auto Base::get_master_id(
    const api::Session& api,
    const proto::Credential& serialized,
    const internal::Primary& master) noexcept(false) -> const identifier_type&
{
    const auto& out = master.ID();
    const auto expected =
        api.Factory().Internal().Identifier(serialized.childdata().masterid());

    if (out != expected) {
        throw std::runtime_error(
            "Attempting to load credential for incorrect authority");
    }

    return out;
}

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
auto Base::id_form() const -> std::shared_ptr<SerializedType>
{
    auto out = std::make_shared<proto::Credential>();
    out->set_version(Version());
    out->set_type(translate((type_)));
    out->set_role(translate(role_));

    if (identity::CredentialRole::MasterKey != role_) {
        std::unique_ptr<proto::ChildCredentialParameters> parameters;
        parameters = std::make_unique<proto::ChildCredentialParameters>();
        parameters->set_version(1);
        master_id_.Internal().Serialize(*parameters->mutable_masterid());
        out->set_allocated_childdata(parameters.release());
    }

    out->set_mode(translate(crypto::asymmetric::Mode::Public));
    out->clear_signature();  // just in case...
    out->clear_id();         // just in case...
    nym_id_.Internal().Serialize(*out->mutable_nymid());

    return out;
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

auto Base::init(
    const identity::credential::internal::Primary& master,
    const PasswordPrompt& reason) noexcept(false) -> void
{
    auto sigs = Signatures{};
    sign(master, reason, sigs);
    add_signatures(std::move(sigs));

    if (false == Save()) {
        throw std::runtime_error("Failed to save master credential");
    }
}

/** Perform syntax (non-cryptographic) verifications of a credential */
auto Base::isValid() const -> bool
{
    std::shared_ptr<SerializedType> serializedProto;

    return isValid(serializedProto);
}

/** Returns the serialized form to prevent unnecessary serializations */
auto Base::isValid(std::shared_ptr<SerializedType>& credential) const -> bool
{
    auto serializationMode = AS_PUBLIC;

    if (crypto::asymmetric::Mode::Private == mode_) {
        serializationMode = AS_PRIVATE;
    }

    credential = serialize(serializationMode, WITH_SIGNATURES);

    return proto::Validate<proto::Credential>(
        *credential,
        VERBOSE,
        translate(mode_),
        translate(role_),
        true);  // with signatures
}

auto Base::MasterSignature() const -> contract::Signature
{
    auto masterSignature = contract::Signature{};
    const auto targetRole{proto::SIGROLE_PUBCREDENTIAL};
    const auto targetID = master_id_;

    for (const auto& sig : signatures()) {
        const auto id =
            api_.Factory().Internal().Identifier(sig->credentialid());

        if ((sig->role() == targetRole) && (id == targetID)) { return sig; }
    }

    return {};
}

auto Base::Save() const -> bool
{
    std::shared_ptr<SerializedType> serializedProto;

    if (!isValid(serializedProto)) {
        LogError()()("Unable to save serialized credential. Type (")(
            value(role_))("), version ")(Version())
            .Flush();

        return false;
    }

    const bool bSaved =
        api_.Wallet().Internal().SaveCredential(*serializedProto);

    if (!bSaved) {
        LogError()()("Error saving credential.").Flush();

        return false;
    }

    return true;
}

auto Base::SelfSignature(CredentialModeFlag version) const
    -> contract::Signature
{
    const auto targetRole{
        (PRIVATE_VERSION == version) ? proto::SIGROLE_PRIVCREDENTIAL
                                     : proto::SIGROLE_PUBCREDENTIAL};
    const auto& self = ID();

    for (const auto& sig : signatures()) {
        const auto id =
            api_.Factory().Internal().Identifier(sig->credentialid());

        if ((sig->role() == targetRole) && (id == self)) { return sig; }
    }

    return {};
}

auto Base::serialize(
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
    -> std::shared_ptr<Base::SerializedType>
{
    auto out = id_form();

    if (asPrivate) {
        if (crypto::asymmetric::Mode::Private == mode_) {
            out->set_mode(translate(mode_));
        } else {
            LogError()()(
                "Can't serialize a public credential as a private credential.")
                .Flush();

            return {};
        }
    }

    if (asSigned) {
        if (asPrivate) {
            auto privateSig = SelfSignature(PRIVATE_VERSION);

            if (privateSig) { *out->add_signature() = *privateSig; }
        }

        auto publicSig = SelfSignature(PUBLIC_VERSION);

        if (publicSig) { *out->add_signature() = *publicSig; }

        auto sourceSig = SourceSignature();

        if (sourceSig) { *out->add_signature() = *sourceSig; }
    }

    ID().Internal().Serialize(*out->mutable_id());

    return out;
}

auto Base::Serialize(Writer&& out) const noexcept -> bool
{
    auto serialized = proto::Credential{};
    Serialize(serialized, Private() ? AS_PRIVATE : AS_PUBLIC, WITH_SIGNATURES);

    return serialize(serialized, std::move(out));
}

auto Base::Serialize(
    SerializedType& output,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const -> bool
{
    auto serialized = serialize(asPrivate, asSigned);

    if (!serialized) { return false; }
    output = *serialized;

    return true;
}

auto Base::sign(
    const identity::credential::internal::Primary& master,
    const PasswordPrompt& reason,
    Signatures& out) noexcept(false) -> void
{
    if (identity::CredentialRole::MasterKey != role_) {
        add_master_signature(master, reason, out);
    }
}

auto Base::SourceSignature() const -> contract::Signature
{
    for (const auto& sig : signatures()) {
        const auto id =
            api_.Factory().Internal().Identifier(sig->credentialid());

        if ((sig->role() == proto::SIGROLE_NYMIDSOURCE) && (id == nym_id_)) {

            return sig;
        }
    }

    return {};
}

/** Override this method for credentials capable of deriving transport keys */
auto Base::TransportKey(Data&, Secret&, const PasswordPrompt&) const -> bool
{
    assert_true(false, "This method was called on the wrong credential.");

    return false;
}

auto Base::validate() const -> bool
{
    // Check syntax
    if (!isValid()) { return false; }

    // Check cryptographic requirements
    return verify_internally();
}

auto Base::Validate() const noexcept -> bool { return validate(); }

auto Base::Verify(
    const proto::Credential& credential,
    const identity::CredentialRole& role,
    const identifier_type& masterID,
    const proto::Signature& masterSig) const -> bool
{
    LogError()()("Non-key credentials are not able to verify signatures")
        .Flush();

    return false;
}

/** Verifies the cryptographic integrity of a credential. Assumes the
 * Authority specified by parent_ is valid. */
auto Base::verify_internally() const -> bool
{
    if (!check_id()) {
        LogError()()(
            "Purported ID for this credential does not match its actual "
            "contents.")
            .Flush();

        return false;
    }

    bool GoodMasterSignature = false;

    if (identity::CredentialRole::MasterKey == role_) {
        GoodMasterSignature = true;  // Covered by VerifySignedBySelf()
    } else {
        GoodMasterSignature = verify_master_signature();
    }

    if (!GoodMasterSignature) {
        LogError()()(
            "This credential hasn't been signed by its master credential.")
            .Flush();

        return false;
    }

    return true;
}

auto Base::verify_master_signature() const -> bool
{
    auto serialized = serialize(AS_PUBLIC, WITHOUT_SIGNATURES);
    auto masterSig = MasterSignature();

    if (!masterSig) {
        LogError()()("Missing master signature.").Flush();

        return false;
    }

    return (parent_.GetMasterCredential().Internal().Verify(
        *serialized, role_, parent_.GetMasterCredID(), *masterSig));
}
}  // namespace opentxs::identity::credential::implementation
