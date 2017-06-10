/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

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
// Each OTKeypair has 2 OTAsymmetricKeys (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/Credential.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/ChildKeyCredential.hpp"
#include "opentxs/core/crypto/ContactCredential.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/MasterCredential.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/VerificationCredential.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"

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
    CredentialSet& parent,
    const proto::Credential& serialized,
    const proto::KeyMode& mode,
    const proto::CredentialRole& purportedRole)
{
    std::unique_ptr<Credential> result;

    // This check allows all constructors to assume inputs are well-formed
    if (!proto::Check<proto::Credential>(
            serialized, 0, 0xFFFFFFFF, mode, purportedRole)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid serialized credential."
              << std::endl;

        return result;
    }

    switch (serialized.role()) {
        case proto::CREDROLE_MASTERKEY:
            result.reset(new MasterCredential(parent, serialized));

            break;
        case proto::CREDROLE_CHILDKEY:
            result.reset(new ChildKeyCredential(parent, serialized));

            break;
        case proto::CREDROLE_CONTACT:
            result.reset(new ContactCredential(parent, serialized));

            break;
        case proto::CREDROLE_VERIFY:
            result.reset(new VerificationCredential(parent, serialized));

            break;
        default:
            break;
    }

    if (!result->Validate()) { result.reset(); }

    return result;
}

Credential::Credential(
    CredentialSet& theOwner,
    const std::uint32_t version,
    const NymParameters& nymParameters)
    : ot_super(ConstNym(), version)
    , type_(nymParameters.credentialType())
    , mode_(proto::KEYMODE_PRIVATE)
    , owner_backlink_(&theOwner)
{
}

Credential::Credential(
    CredentialSet& theOwner,
    const proto::Credential& serializedCred)
    : ot_super(ConstNym(), serializedCred.version())
    , type_(serializedCred.type())
    , role_(serializedCred.role())
    , mode_(serializedCred.mode())
    , owner_backlink_(&theOwner)
{
    if (serializedCred.has_nymid()) {
        nym_id_ = String(serializedCred.nymid());
        id_ = Identifier(serializedCred.id());
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

    const std::string parent = owner_backlink_->GetMasterCredID().Get();
    const std::string child = master_id_.Get();

    return (parent == child);
}

/** Verifies the cryptographic integrity of a credential. Assumes the
 * CredentialSet specified by owner_backlink_ is valid. */
bool Credential::verify_internally(const Lock& lock) const
{
    OT_ASSERT(nullptr != owner_backlink_);

    if (nullptr == owner_backlink_) {
        otErr << OT_METHOD << __FUNCTION__ << ": This credential is not "
              << "attached to a CredentialSet. Can not verify.\n";

        return false;
    }

    if (!VerifyNymID()) {
        otErr << OT_METHOD << __FUNCTION__ << ": NymID for this credential "
              << "does not match NymID of parent CredentialSet.\n";

        return false;
    }

    if (!VerifyMasterID()) {
        otErr << OT_METHOD << __FUNCTION__ << ": MasterID for this credential "
              << "does not match MasterID of parent CredentialSet.\n";

        return false;
    }

    if (!CheckID(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Purported ID for this "
              << "credential does not match its actual contents.\n";

        return false;
    }

    bool GoodMasterSignature = false;

    if (proto::CREDROLE_MASTERKEY == role_) {
        GoodMasterSignature = true;  // Covered by VerifySignedBySelf()
    } else {
        GoodMasterSignature = verify_master_signature(lock);
    }

    if (!GoodMasterSignature) {
        otErr << OT_METHOD << __FUNCTION__ << ": This credential hasn't "
              << "been signed by its  master credential." << std::endl;

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
        otErr << OT_METHOD << __FUNCTION__ << ": Missing master signature."
              << std::endl;

        return false;
    }

    return (owner_backlink_->GetMasterCredential().Verify(
        *serialized, role_, Identifier(MasterID()), *masterSig));
}

SerializedSignature Credential::MasterSignature() const
{
    SerializedSignature masterSignature;
    proto::SignatureRole targetRole = proto::SIGROLE_PUBCREDENTIAL;

    const std::string master = MasterID().Get();

    for (auto& it : signatures_) {
        if ((it->role() == targetRole) &&
            (it->credentialid() == master)) {

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
bool Credential::isValid(
    const Lock& lock,
    serializedCredential& credential) const
{
    SerializationModeFlag serializationMode = AS_PUBLIC;

    if (proto::KEYMODE_PRIVATE == mode_) {
        serializationMode = AS_PRIVATE;
    }

    credential = serialize(lock, serializationMode, WITH_SIGNATURES);

    return proto::Check<proto::Credential>(
        *credential,
        0,
        0xFFFFFFFF,
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

Identifier Credential::GetID(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    serializedCredential idVersion =
        serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);

    if (idVersion->has_id()) {
        idVersion->clear_id();
    }

    Data serializedData = proto::ProtoAsData<proto::Credential>(*idVersion);
    Identifier id;

    if (!id.CalculateDigest(serializedData)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error calculating credential digest.\n";
    }

    return id;
}

String Credential::CredentialTypeToString(proto::CredentialType credentialType)
{
    String credentialString;

    switch (credentialType) {
        case proto::CREDTYPE_LEGACY:
            credentialString = "Legacy";
            break;
        case proto::CREDTYPE_HD:
            credentialString = "HD";
            break;
        default:
            credentialString = "Error";
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
        parameters->set_masterid(MasterID().Get());
        serializedCredential->set_allocated_childdata(parameters.release());
    }

    if (asPrivate) {
        if (proto::KEYMODE_PRIVATE == mode_) {
            serializedCredential->set_mode(mode_);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Can't serialized a public "
                  << "credential as a private credential." << std::endl;
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

        if (publicSig) {
            *serializedCredential->add_signature() = *publicSig;
        }

        auto sourceSig = SourceSignature();

        if (sourceSig) {
            *serializedCredential->add_signature() = *sourceSig;
        }
    } else {
        serializedCredential->clear_signature();  // just in case...
    }

    serializedCredential->set_id(String(id(lock)).Get());
    serializedCredential->set_nymid(NymID().Get());

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

    const std::string self = String(id_).Get();

    for (auto& it : signatures_) {
        if ((it->role() == targetRole) &&
            (it->credentialid() == self)) {

            return it;
        }
    }

    return nullptr;
}

SerializedSignature Credential::SourceSignature() const
{
    SerializedSignature signature;

    const std::string source = String(NymID()).Get();

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
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to save serialized "
              << "credential. Type (" << type_ << "), version " << version_
              << std::endl;

        return false;
    }

    const bool bSaved = OT::App().DB().Store(*serializedProto);

    if (!bSaved) {
        otErr << OT_METHOD << __FUNCTION__ << ": Error saving credential"
              << std::endl;

        return false;
    }

    return true;
}

Data Credential::Serialize() const
{
    serializedCredential serialized = Serialized(
        Private() ? AS_PRIVATE : AS_PUBLIC,
        WITH_SIGNATURES);

    return proto::ProtoAsData<proto::Credential>(*serialized);
}

std::string Credential::asString(const bool asPrivate) const
{
    serializedCredential credenial;
    Data dataCredential;
    String stringCredential;

    credenial = Serialized(asPrivate, WITH_SIGNATURES);
    dataCredential = proto::ProtoAsData<proto::Credential>(*credenial);

    OTASCIIArmor armoredCredential(dataCredential);

    armoredCredential.WriteArmoredString(stringCredential, "Credential");

    return stringCredential.Get();
}

// static
serializedCredential Credential::ExtractArmoredCredential(
    const String& stringCredential)
{
    OTASCIIArmor armoredCredential;
    String strTemp(stringCredential);
    armoredCredential.LoadFromString(strTemp);
    return ExtractArmoredCredential(armoredCredential);
}

// static
serializedCredential Credential::ExtractArmoredCredential(
    const OTASCIIArmor& armoredCredential)
{
    Data dataCredential(armoredCredential);

    serializedCredential serializedCred = std::make_shared<proto::Credential>();

    serializedCred->ParseFromArray(
        dataCredential.GetPointer(), dataCredential.GetSize());

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
        otErr << OT_METHOD << __FUNCTION__ << ": Missing master credential."
              << std::endl;

        return false;
    }

    SerializedSignature serializedMasterSignature =
        std::make_shared<proto::Signature>();
    auto serialized = serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);
    auto& signature = *serialized->add_signature();
    signature.set_role(proto::SIGROLE_PUBCREDENTIAL);

    bool havePublicSig =
        owner_backlink_->SignProto(
            *serialized,
            signature);

    if (!havePublicSig) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to obtain signature from master credential."
              << std::endl;

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

bool Credential::hasCapability(const NymCapability&) const
{
    return false;
}

serializedCredential Credential::Serialized(
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    Lock lock(lock_);

    return serialize(lock, asPrivate, asSigned);
}
}  // namespace opentxs
