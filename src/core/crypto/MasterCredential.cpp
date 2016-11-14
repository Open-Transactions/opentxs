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

#include "opentxs/core/crypto/MasterCredential.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTKeypair.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"
#endif
#include "opentxs/core/util/Assert.hpp"

#include <memory>
#include <ostream>

namespace opentxs
{

/** Verify that nym_id_ is the same as the hash of m_strSourceForNymID. Also
 * verify that *this == owner_backlink_->GetMasterCredential() (the master
 * credential.) Verify the (self-signed) signature on *this. */
bool MasterCredential::VerifyInternally() const
{
    // Perform common Key Credential verifications
    if (!ot_super::VerifyInternally()) {
        return false;
    }

    // Check that the source validates this credential
    if (!VerifyAgainstSource()) {
        otOut << __FUNCTION__ << ": Failed verifying master credential against "
              << "nym id source." << std::endl;

        return false;
    }

    return true;
}

bool MasterCredential::VerifyAgainstSource() const
{
    return owner_backlink_->Source().Verify(*this);
}

MasterCredential::MasterCredential(
    CredentialSet& theOwner,
    const proto::Credential& serializedCred)
    : ot_super(theOwner, serializedCred)
{
    role_ = proto::CREDROLE_MASTERKEY;

    std::shared_ptr<NymIDSource> source =
        std::make_shared<NymIDSource>(serializedCred.masterdata().source());

    owner_backlink_->SetSource(source);
    source_proof_.reset(
        new proto::SourceProof(serializedCred.masterdata().sourceproof()));
}

MasterCredential::MasterCredential(
    CredentialSet& theOwner,
    const NymParameters& nymParameters)
    : ot_super(theOwner, nymParameters)
{
    
    Log::vOutput(0, "MasterCredential::MasterCredential: owner and nym params\n");

    role_ = proto::CREDROLE_MASTERKEY;

    std::shared_ptr<NymIDSource> source;
    std::unique_ptr<proto::SourceProof> sourceProof;
    sourceProof.reset(new proto::SourceProof);

    
    Log::vOutput(0, "MasterCredential::MasterCredential: after new proto\n");

    proto::SourceProofType proofType = nymParameters.SourceProofType();

    if (proto::SOURCETYPE_PUBKEY == nymParameters.SourceType()) {
        OT_ASSERT_MSG(
            proto::SOURCEPROOFTYPE_SELF_SIGNATURE == proofType,
            "non self-signed credentials not yet implemented");

        source = std::make_shared<NymIDSource>(
            nymParameters, *(m_SigningKey->GetPublicKey().Serialize()));
        sourceProof->set_version(1);
        sourceProof->set_type(proto::SOURCEPROOFTYPE_SELF_SIGNATURE);

        Log::vOutput(0, "MasterCredential::MasterCredential: 1\n");

    }
    

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    else if (proto::SOURCETYPE_BIP47 == nymParameters.SourceType()) {
        Log::vOutput(0, "MasterCredential::MasterCredential: 2\n");
        

        sourceProof->set_version(1);
        sourceProof->set_type(proto::SOURCEPROOFTYPE_SIGNATURE);

        std::unique_ptr<PaymentCode> bip47Source;
        bip47Source.reset(new PaymentCode(nymParameters.Nym()));

        source = std::make_shared<NymIDSource>(bip47Source);
        Log::vOutput(0, "MasterCredential::MasterCredential: 3\n");
    }
#endif

    Log::vOutput(0, "MasterCredential::MasterCredential: 4\n");

    source_proof_.reset(sourceProof.release());
    owner_backlink_->SetSource(source);
    String nymID = owner_backlink_->GetNymID();

    nym_id_ = nymID;
    
    Log::vOutput(0, "MasterCredential::MasterCredential: (bottom)\n");

}

bool MasterCredential::New(const NymParameters& nymParameters)
{
    Log::vOutput(0, "MasterCredential::New: 1\n");

    if (!ot_super::New(nymParameters)) {
        
        Log::vOutput(0, "MasterCredential::New: Failed in (!ot_super::New(nymParameters))\n");
        
        return false;
    }
    Log::vOutput(0, "MasterCredential::New: 2\n");

    if (proto::SOURCEPROOFTYPE_SELF_SIGNATURE != source_proof_->type()) {
        SerializedSignature sig = std::make_shared<proto::Signature>();
        
        Log::vOutput(0, "MasterCredential::New: 3\n");

        
        bool haveSourceSig = owner_backlink_->Sign(*this, nymParameters, *sig);

        Log::vOutput(0, "MasterCredential::New: 4\n");

        if (haveSourceSig) {
            signatures_.push_back(sig);

            return true;
        }
        
        Log::vOutput(0, "MasterCredential::New: 5\n");

    }

    Log::vOutput(0, "MasterCredential::New: 6\n");

    return false;
}

MasterCredential::~MasterCredential() {}

serializedCredential MasterCredential::asSerialized(
    SerializationModeFlag asPrivate,
    SerializationSignatureFlag asSigned) const
{
    serializedCredential serializedCredential =
        this->ot_super::asSerialized(asPrivate, asSigned);

    proto::MasterCredentialParameters* parameters =
        new proto::MasterCredentialParameters;

    parameters->set_version(1);
    *(parameters->mutable_source()) = *(owner_backlink_->Source().Serialize());

    serializedCredential->set_allocated_masterdata(parameters);

    serializedCredential->set_role(proto::CREDROLE_MASTERKEY);
    *(serializedCredential->mutable_masterdata()->mutable_sourceproof()) =
        *source_proof_;

    return serializedCredential;
}

bool MasterCredential::Verify(const Credential& credential) const
{
    serializedCredential serializedCred = credential.asSerialized(
        AS_PUBLIC, WITHOUT_SIGNATURES);

    if (!proto::Check<proto::Credential>(
            *serializedCred,
            0,
            0xFFFFFFFF,
            proto::KEYMODE_PUBLIC,
            credential.Role(),
            false)) {
                otErr << __FUNCTION__ << ": Invalid credential syntax."
                      << std::endl;

                return false;
    }

    bool sameMaster = (id_ == Identifier(credential.MasterID()));

    if (!sameMaster) {
        otErr << __FUNCTION__ << ": Credential does not designate this"
              << " credential as its master.\n";
        return false;
    }

    SerializedSignature masterSig = credential.MasterSignature();

    if (!masterSig) {
        otErr << __FUNCTION__ << ": Missing master signature.\n";
        return false;
    }

    auto& signature = *serializedCred->add_signature();
    signature.CopyFrom(*masterSig);
    signature.clear_signature();

    return Verify(
        proto::ProtoAsData<proto::Credential>(*serializedCred), *masterSig);
}

bool MasterCredential::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::SIGN_CHILDCRED) : {
            if (m_SigningKey) {
                return m_SigningKey->hasCapability(capability);
            }

            break;
        }
        default : {}
    }

    return false;
}
}  // namespace opentxs
