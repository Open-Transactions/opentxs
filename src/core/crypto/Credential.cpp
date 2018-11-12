// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/////////////////////////////////////////////////////////////////////

// A nym contains a list of credential sets.
// The whole purpose of a Nym is to be an identity, which can have
// master credentials.
//
// Each CredentialSet contains list of Credentials. One of the
// Credentials is a MasterCredential, and the rest are ChildCredentials
// signed by the MasterCredential.
//
// A Credential may contain keys, in which case it is a KeyCredential.
//
// Credentials without keys might be an interface to a hardware device
// or other kind of external encryption and authentication system.
//
// Non-key Credentials are not yet implemented.
//
// Each KeyCredential has 3 OTKeypairs: encryption, signing, and authentication.
// Each OTKeypair has 2 crypto::key::Asymmetrics (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

#include "stdafx.hpp"

#include "opentxs/core/crypto/Credential.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/ChildKeyCredential.hpp"
#include "opentxs/core/crypto/ContactCredential.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/MasterCredential.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/VerificationCredential.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Proto.hpp"

#include <list>
#include <memory>
#include <ostream>
#include <string>

#define OT_METHOD "opentxs::Credential::"

namespace opentxs
{

/** Contains 3 key pairs: signing, authentication, and encryption. This is
 * stored as an Contract, and it must be signed by the master key. (which is
 * also an Credential.) */
std::unique_ptr<Credential> Credential::Factory(
    const api::Core& api,
    CredentialSet& parent,
    const proto::Credential& serialized,
    const proto::KeyMode& mode,
    const proto::CredentialRole& purportedRole)
{
    std::unique_ptr<Credential> result;

    // This check allows all constructors to assume inputs are well-formed
    if (!proto::Validate<proto::Credential>(
            serialized, VERBOSE, mode, purportedRole)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid serialized credential.")
            .Flush();

        return result;
    }

    switch (serialized.role()) {
        case proto::CREDROLE_MASTERKEY: {
            result.reset(new MasterCredential(api, parent, serialized));

        } break;
        case proto::CREDROLE_CHILDKEY: {
            result.reset(new ChildKeyCredential(api, parent, serialized));

        } break;
        case proto::CREDROLE_CONTACT: {
            result.reset(new ContactCredential(api, parent, serialized));

        } break;
        case proto::CREDROLE_VERIFY: {
            result.reset(new VerificationCredential(api, parent, serialized));

        } break;
        default: {
        }
    }

    if (!result->Validate()) { result.reset(); }

    return result;
}

Credential::Credential(
    const api::Core& api,
    CredentialSet& theOwner,
    const std::uint32_t version,
    const NymParameters& nymParameters)
    : ot_super(ConstNym(), version)
    , api_(api)
    , type_(nymParameters.credentialType())
    , mode_(proto::KEYMODE_PRIVATE)
    , owner_backlink_(&theOwner)
{
}

Credential::Credential(
    const api::Core& api,
    CredentialSet& theOwner,
    const proto::Credential& serializedCred)
    : ot_super(ConstNym(), serializedCred.version())
    , api_(api)
    , type_(serializedCred.type())
    , role_(serializedCred.role())
    , mode_(serializedCred.mode())
    , owner_backlink_(&theOwner)
{
    if (serializedCred.has_nymid()) {
        nym_id_ = serializedCred.nymid();
        id_ = Identifier::Factory(serializedCred.id());
    }

    SerializedSignature sig;
    for (auto& it : serializedCred.signature()) {
        sig.reset(new proto::Signature(it));
        signatures_.push_back(sig);
    }
}

bool Credential::New(const NymParameters&)
{
    Lock lock(lock_);
    bool output = false;

    output = CalculateID(lock);

    OT_ASSERT(output);

    if (output && (proto::CREDROLE_MASTERKEY != role_)) {
        output = AddMasterSignature(lock);
    }

    OT_ASSERT(output);

    return output;
}

/** Verify that nym_id_ matches the nymID of the parent credential set */
bool Credential::VerifyNymID() const
{

    return (nym_id_ == owner_backlink_->GetNymID());
}

/** Verify that master_id_ matches the MasterID of the parent credential set */
bool Credential::VerifyMasterID() const
{
    // This check is not applicable to master credentials
    if (proto::CREDROLE_MASTERKEY == role_) { return true; }

    const std::string parent = owner_backlink_->GetMasterCredID();
    const std::string child = master_id_;

    return (parent == child);
}

/** Verifies the cryptographic integrity of a credential. Assumes the
 * CredentialSet specified by owner_backlink_ is valid. */
bool Credential::verify_internally(const Lock& lock) const
{
    OT_ASSERT(nullptr != owner_backlink_);

    if (nullptr == owner_backlink_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": This credential is not "
            "attached to a CredentialSet. Can not verify.")
            .Flush();

        return false;
    }

    if (!VerifyNymID()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": NymID for this credential "
            "does not match NymID of parent CredentialSet.")
            .Flush();

        return false;
    }

    if (!VerifyMasterID()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": MasterID for this credential "
            "does not match MasterID of parent CredentialSet.")
            .Flush();

        return false;
    }

    if (!CheckID(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Purported ID for this "
            "credential does not match its actual contents.")
            .Flush();

        return false;
    }

    bool GoodMasterSignature = false;

    if (proto::CREDROLE_MASTERKEY == role_) {
        GoodMasterSignature = true;  // Covered by VerifySignedBySelf()
    } else {
        GoodMasterSignature = verify_master_signature(lock);
    }

    if (!GoodMasterSignature) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": This credential hasn't "
            "been signed by its  master credential.")
            .Flush();

        return false;
    }

    return true;
}

bool Credential::verify_master_signature(const Lock& lock) const
{
    OT_ASSERT(owner_backlink_);

    auto serialized = serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);

    auto masterSig = MasterSignature();

    if (!masterSig) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing master signature.")
            .Flush();

        return false;
    }

    return (owner_backlink_->GetMasterCredential().Verify(
        *serialized, role_, Identifier::Factory(MasterID()), *masterSig));
}

SerializedSignature Credential::MasterSignature() const
{
    SerializedSignature masterSignature;
    proto::SignatureRole targetRole = proto::SIGROLE_PUBCREDENTIAL;

    const std::string master = MasterID();

    for (auto& it : signatures_) {
        if ((it->role() == targetRole) && (it->credentialid() == master)) {

            masterSignature = it;
            break;
        }
    }

    return masterSignature;
}

/** Perform syntax (non-cryptographic) verifications of a credential */
bool Credential::isValid(const Lock& lock) const
{
    serializedCredential serializedProto;

    return isValid(lock, serializedProto);
}

/** Returns the serialized form to prevent unnecessary serializations */
bool Credential::isValid(const Lock& lock, serializedCredential& credential)
    const
{
    SerializationModeFlag serializationMode = AS_PUBLIC;

    if (proto::KEYMODE_PRIVATE == mode_) { serializationMode = AS_PRIVATE; }

    credential = serialize(lock, serializationMode, WITH_SIGNATURES);

    return proto::Validate<proto::Credential>(
        *credential,
        VERBOSE,
        mode_,
        role_,
        true);  // with signatures
}

bool Credential::validate(const Lock& lock) const
{
    // Check syntax
    if (!isValid(lock)) { return false; }

    // Check cryptographic requirements
    return verify_internally(lock);
}

bool Credential::Validate() const
{
    Lock lock(lock_);

    return validate(lock);
}

OTIdentifier Credential::GetID(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    serializedCredential idVersion =
        serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);

    if (idVersion->has_id()) { idVersion->clear_id(); }

    auto serializedData = proto::ProtoAsData(*idVersion);
    auto id = Identifier::Factory();

    if (!id->CalculateDigest(serializedData)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error calculating credential digest.")
            .Flush();
    }

    return id;
}

OTString Credential::CredentialTypeToString(
    proto::CredentialType credentialType)
{
    auto credentialString = String::Factory();

    switch (credentialType) {
        case proto::CREDTYPE_LEGACY:
            credentialString = String::Factory("Legacy");
            break;
        case proto::CREDTYPE_HD:
            credentialString = String::Factory("HD");
            break;
        default:
            credentialString = String::Factory("Error");
    }
    return credentialString;
}

proto::CredentialType Credential::Type() const { return type_; }

serializedCredential Credential::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    serializedCredential serializedCredential =
        std::make_shared<proto::Credential>();

    serializedCredential->set_version(version_);
    serializedCredential->set_type(static_cast<proto::CredentialType>(type_));
    serializedCredential->set_role(static_cast<proto::CredentialRole>(role_));

    if (proto::CREDROLE_MASTERKEY != role_) {
        std::unique_ptr<proto::ChildCredentialParameters> parameters;
        parameters.reset(new proto::ChildCredentialParameters);

        parameters->set_version(1);
        parameters->set_masterid(MasterID());
        serializedCredential->set_allocated_childdata(parameters.release());
    }

    if (asPrivate) {
        if (proto::KEYMODE_PRIVATE == mode_) {
            serializedCredential->set_mode(mode_);
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Can't serialized a public "
                "credential as a private credential.")
                .Flush();
        }
    } else {
        serializedCredential->set_mode(proto::KEYMODE_PUBLIC);
    }

    if (asSigned) {
        if (asPrivate) {
            auto privateSig = SelfSignature(PRIVATE_VERSION);

            if (privateSig) {
                *serializedCredential->add_signature() = *privateSig;
            }
        }

        auto publicSig = SelfSignature(PUBLIC_VERSION);

        if (publicSig) { *serializedCredential->add_signature() = *publicSig; }

        auto sourceSig = SourceSignature();

        if (sourceSig) { *serializedCredential->add_signature() = *sourceSig; }
    } else {
        serializedCredential->clear_signature();  // just in case...
    }

    serializedCredential->set_id(id(lock)->str());
    serializedCredential->set_nymid(NymID());

    return serializedCredential;
}

SerializedSignature Credential::SelfSignature(CredentialModeFlag version) const
{
    proto::SignatureRole targetRole;

    if (PRIVATE_VERSION == version) {
        targetRole = proto::SIGROLE_PRIVCREDENTIAL;
    } else {
        targetRole = proto::SIGROLE_PUBCREDENTIAL;
    }

    const std::string self = String::Factory(id_)->Get();

    for (auto& it : signatures_) {
        if ((it->role() == targetRole) && (it->credentialid() == self)) {

            return it;
        }
    }

    return nullptr;
}

SerializedSignature Credential::SourceSignature() const
{
    SerializedSignature signature;

    const std::string source = String::Factory(NymID())->Get();

    for (auto& it : signatures_) {
        if ((it->role() == proto::SIGROLE_NYMIDSOURCE) &&
            (it->credentialid() == source)) {
            signature = std::make_shared<proto::Signature>(*it);

            break;
        }
    }
    return signature;
}

bool Credential::Save() const
{
    Lock lock(lock_);

    serializedCredential serializedProto;

    if (!isValid(lock, serializedProto)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to save serialized "
            "credential. Type (")(role_)("), version ")(version_)(".")
            .Flush();

        return false;
    }

    const bool bSaved = api_.Wallet().SaveCredential(*serializedProto);

    if (!bSaved) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error saving credential.")
            .Flush();

        return false;
    }

    return true;
}

OTData Credential::Serialize() const
{
    serializedCredential serialized =
        Serialized(Private() ? AS_PRIVATE : AS_PUBLIC, WITH_SIGNATURES);

    return proto::ProtoAsData<proto::Credential>(*serialized);
}

std::string Credential::asString(const bool asPrivate) const
{
    serializedCredential credenial;
    auto dataCredential = Data::Factory();
    auto stringCredential = String::Factory();
    credenial = Serialized(asPrivate, WITH_SIGNATURES);
    dataCredential = proto::ProtoAsData<proto::Credential>(*credenial);
    auto armoredCredential = Armored::Factory(dataCredential);
    armoredCredential->WriteArmoredString(stringCredential, "Credential");

    return stringCredential->Get();
}

// static
serializedCredential Credential::ExtractArmoredCredential(
    const String& stringCredential)
{
    auto armoredCredential = Armored::Factory();
    auto strTemp = String::Factory(stringCredential.Get());
    armoredCredential->LoadFromString(strTemp);

    return ExtractArmoredCredential(armoredCredential);
}

// static
serializedCredential Credential::ExtractArmoredCredential(
    const Armored& armoredCredential)
{
    auto dataCredential = Data::Factory(armoredCredential);
    serializedCredential serializedCred = std::make_shared<proto::Credential>();
    serializedCred->ParseFromArray(
        dataCredential->data(), dataCredential->size());

    return serializedCred;
}

void Credential::ReleaseSignatures(const bool onlyPrivate)
{
    for (auto i = signatures_.begin(); i != signatures_.end();) {
        if (!onlyPrivate ||
            (onlyPrivate && (proto::SIGROLE_PRIVCREDENTIAL == (*i)->role()))) {
            i = signatures_.erase(i);
        } else {
            i++;
        }
    }
}

bool Credential::AddMasterSignature(const Lock& lock)
{
    if (nullptr == owner_backlink_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing master credential.")
            .Flush();

        return false;
    }

    SerializedSignature serializedMasterSignature =
        std::make_shared<proto::Signature>();
    auto serialized = serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);
    auto& signature = *serialized->add_signature();
    signature.set_role(proto::SIGROLE_PUBCREDENTIAL);

    bool havePublicSig = owner_backlink_->SignProto(*serialized, signature);

    if (!havePublicSig) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain signature from master credential.")
            .Flush();

        return false;
    }
    serializedMasterSignature->CopyFrom(signature);
    signatures_.push_back(serializedMasterSignature);

    return true;
}

/** Override this method for credentials capable of returning contact data. */
bool Credential::GetContactData(std::unique_ptr<proto::ContactData>&) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.");

    return false;
}

/** Override this method for credentials capable of returning verification sets.
 */
bool Credential::GetVerificationSet(
    std::unique_ptr<proto::VerificationSet>&) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.");

    return false;
}

/** Override this method for credentials capable of verifying signatures */
bool Credential::Verify(
    const Data&,
    const proto::Signature&,
    const proto::KeyRole) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.");

    return false;
}

/** Override this method for credentials capable of verifying other credentials
 */
bool Credential::Verify(
    const proto::Credential&,
    const proto::CredentialRole&,
    const Identifier&,
    const proto::Signature&) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.");

    return false;
}

/** Override this method for credentials capable of deriving transport keys */
bool Credential::TransportKey(Data&, OTPassword&) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.");

    return false;
}

bool Credential::hasCapability(const NymCapability&) const { return false; }

std::string Credential::Name() const { return id_->str(); }

serializedCredential Credential::Serialized(
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    Lock lock(lock_);

    return serialize(lock, asPrivate, asSigned);
}
}  // namespace opentxs
