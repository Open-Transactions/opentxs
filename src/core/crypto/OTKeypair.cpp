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

#include "opentxs/core/crypto/OTKeypair.hpp"

#include "opentxs/core/crypto/LowLevelKeyGenerator.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSignature.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include <stdint.h>
#include <memory>
#include <ostream>

// DONE: Add OTKeypair member for m_pMetadata.
// Add method to set the Metadata. Or instead of a member,
// just have the method set the public and private keys.
//
// Then a key credential can have a similar function which sets the metadata
// for its three keypairs (they are identical except for the A|E|S.)
//
// When a Nym is loaded, load up its master credentials and all their
// child credentials. Since their metadata was supposedly set properly at
// creation, verify it at load time.

// TODO: on OTNym, change GetPublicKey to GetPublicKeyForVerify or
// GetPublicKeyForEncrypt or GetPublicKeyForTransmission. Then
// rebuild so that all places using GetPublicKey are forced to choose
// one of those. Same with GetPrivateKeyForSigning, GetPrivateKeyForDecrypt,
// and GetPrivateKeyForAuthentication.

// DONE: add the methods to OTPseudonym for generating a master credential
// contract
// and a sub contract. Add ability to save / load with this data. Then go from
// there.

namespace opentxs
{

OTKeypair::OTKeypair(
    const NymParameters& nymParameters,
    const proto::KeyRole role)
    : m_pkeyPublic(OTAsymmetricKey::KeyFactory(nymParameters, role))
    , m_pkeyPrivate(OTAsymmetricKey::KeyFactory(nymParameters, role))
    , role_(role)
{
    MakeNewKeypair(nymParameters);
}

OTKeypair::OTKeypair(
    const proto::AsymmetricKey& serializedPubkey,
    const proto::AsymmetricKey& serializedPrivkey)
        : m_pkeyPublic(OTAsymmetricKey::KeyFactory(serializedPubkey))
        , m_pkeyPrivate(OTAsymmetricKey::KeyFactory(serializedPrivkey))
{
    role_ = m_pkeyPrivate->Role();
}

OTKeypair::OTKeypair(
    const proto::AsymmetricKey& serializedPubkey)
        : m_pkeyPublic(OTAsymmetricKey::KeyFactory(serializedPubkey))
{
    role_ = m_pkeyPublic->Role();
}

OTKeypair::~OTKeypair()
{
}

bool OTKeypair::HasPublicKey() const
{
    OT_ASSERT(m_pkeyPublic);

    return m_pkeyPublic->IsPublic(); // This means it actually has a public key
                                     // in it, or tried to.
}

bool OTKeypair::HasPrivateKey() const
{
    OT_ASSERT(m_pkeyPrivate);

    return m_pkeyPrivate->IsPrivate(); // This means it actually has a private
                                       // key in it, or tried to.
}

// Return the public key as an OTAsymmetricKey object
// TODO this violates encapsulation and should be deprecated
const OTAsymmetricKey& OTKeypair::GetPublicKey() const
{
    OT_ASSERT(m_pkeyPublic);

    return (*m_pkeyPublic);
}

// Return the private key as an OTAsymmetricKey object
// TODO this violates encapsulation and should be deprecated
const OTAsymmetricKey& OTKeypair::GetPrivateKey() const
{
    OT_ASSERT(m_pkeyPrivate);

    return (*m_pkeyPrivate);
}

bool OTKeypair::MakeNewKeypair(const NymParameters& nymParameters)
{
    if(!m_pkeyPrivate) {
        m_pkeyPrivate.reset(OTAsymmetricKey::KeyFactory(
                                                        nymParameters,
                                                        proto::KEYROLE_ERROR));
    }
    if(!m_pkeyPublic) {
        m_pkeyPublic.reset(OTAsymmetricKey::KeyFactory(
            nymParameters,
            proto::KEYROLE_ERROR));
    }

    LowLevelKeyGenerator lowLevelKeys(nymParameters);

    if (!lowLevelKeys.MakeNewKeypair()) {
        otErr << "OTKeypair::MakeNewKeypair"
            << ": Failed in a call to LowLevelKeyGenerator::MakeNewKeypair.\n";
        return false;
    }

    OTPasswordData passwordData("Enter or set the wallet master password.");
    return lowLevelKeys.SetOntoKeypair(*this, passwordData);

    // If true is returned:
    // Success! At this point, theKeypair's public and private keys have been
    // set.
}

// PUBLIC KEY

// Get a public key as an opentxs::String.
// This form is used in all cases except for the NymIDSource
// of a self-signed MasterCredential
bool OTKeypair::GetPublicKey(String& strKey) const
{
    OT_ASSERT(m_pkeyPublic);

    return m_pkeyPublic->GetPublicKey(strKey);
}

bool OTKeypair::CalculateID(Identifier& theOutput) const
{
    OT_ASSERT(m_pkeyPublic);

    return m_pkeyPublic->CalculateID(theOutput); // Only works for public keys.
}

int32_t OTKeypair::GetPublicKeyBySignature(
    listOfAsymmetricKeys& listOutput, // Inclusive means, return the key even
                                      // when theSignature has no metadata.
    const OTSignature& theSignature, bool bInclusive) const
{
    OT_ASSERT(m_pkeyPublic);
    OT_ASSERT(nullptr != m_pkeyPublic->m_pMetadata);

    // We know that EITHER exact metadata matches must occur, and the signature
    // MUST have metadata, (bInclusive=false)
    // OR if bInclusive=true, we know that metadata is still used to eliminate
    // keys where possible, but that otherwise,
    // if the signature has no metadata, then the key is still returned, "just
    // in case."
    //
    if ((false == bInclusive) &&
        (false == theSignature.getMetaData().HasMetadata()))
        return 0;

    // Below this point, metadata is used if it's available.
    // It's assumed to be "okay" if it's not available, since any non-inclusive
    // calls would have already returned by now, if that were the case.
    // (But if it IS available, then it must match, or the key won't be
    // returned.)
    //
    // If the signature has no metadata, or if m_pkeyPublic has no metadata, or
    // if they BOTH have metadata, and their metadata is a MATCH...
    if (!theSignature.getMetaData().HasMetadata() ||
        !m_pkeyPublic->m_pMetadata->HasMetadata() ||
        (m_pkeyPublic->m_pMetadata->HasMetadata() &&
         theSignature.getMetaData().HasMetadata() &&
         (theSignature.getMetaData() == *(m_pkeyPublic->m_pMetadata)))) {
        // ...Then add m_pkeyPublic as a possible match, to listOutput.
        //
        listOutput.push_back(m_pkeyPublic.get());
        return 1;
    }
    return 0;
}

// Used when importing/exporting a Nym to/from the wallet.
//
bool OTKeypair::ReEncrypt(const OTPassword& theExportPassword, bool bImporting)
{

    OT_ASSERT(m_pkeyPublic);
    OT_ASSERT(m_pkeyPrivate);

    OT_ASSERT(HasPublicKey());
    OT_ASSERT(HasPrivateKey());

    // If we were importing, we were in the exported format but now we're in the
    // internal format.
    // Therefore we want to use the wallet's internal cached master passphrase
    // to save. Therefore
    // strReason will be used for the import case.
    //
    // But if we were exporting, then we were in the internal format and just
    // re-encrypted to the
    // export format. So we'd want to pass the export passphrase when saving.
    //
    const String strReasonAbove(
        bImporting ? "Enter the new export passphrase. (Above "
                     "ReEncryptPrivateKey in OTKeypair::ReEncrypt)"
                   : "Enter your wallet's master passphrase. (Above "
                     "ReEncryptPrivateKey in OTKeypair::ReEncrypt)");

    const String strReasonBelow(
        bImporting ? "Enter your wallet's master passphrase. (Below "
                     "ReEncryptPrivateKey in OTKeypair::ReEncrypt)"
                   : "Enter the new export passphrase. (Below "
                     "ReEncryptPrivateKey in OTKeypair::ReEncrypt)");

    // At this point the public key was loaded from a public key, not a cert,
    // but the private key was loaded from the cert. Therefore we'll save the
    // public cert from the private key, and then use that to reload the public
    // key after ReEncrypting. (Otherwise the public key would be there, but it
    // would be missing the x509, which is only available in the cert, not the
    // pubkey alone -- and without the x509 being there, the "SaveAndReload"
    // call
    // below would fail.
    // Why don't I just stick the Cert itself into the public data, instead of
    // sticking the public key in there? Because not all key credentials will
    // use
    // certs. Some will use pubkeys from certs, and some will use pubkeys not
    // from
    // certs. But I might still just stick it in there, and code things to be
    // able to
    // load either indiscriminately. After all, that's what I'm doing already in
    // the
    // asset and server contracts. But even in those cases, there will be times
    // when
    // only a pubkey is available, not a cert, so I'll probably still find
    // myself having
    // to do this. Hmm...

    const bool bReEncrypted = m_pkeyPrivate->ReEncryptPrivateKey(
        theExportPassword, bImporting); // <==== IMPORT or EXPORT occurs here.

    if (!(bReEncrypted)) {
        otErr << __FUNCTION__ << ": Failure, either when re-encrypting, or "
                                 "when subsequently retrieving "
                                 "the public/private keys. bImporting == "
              << (bImporting ? "true" : "false") << "\n";
    }

    return (bReEncrypted);
}

serializedAsymmetricKey OTKeypair::Serialize(bool privateKey) const
{
    OT_ASSERT(m_pkeyPublic);

    if (privateKey) {
        OT_ASSERT(m_pkeyPrivate);
        return m_pkeyPrivate->Serialize();
    } else {
        return m_pkeyPublic->Serialize();
    }
}

bool OTKeypair::Verify(
    const Data& plaintext,
    const proto::Signature& sig) const
{
    if (!m_pkeyPublic) {
        otErr << __FUNCTION__ << ": Missing public key. Can not verify.\n";

        return false;
    }

    return m_pkeyPublic->Verify(plaintext, sig);
}

bool OTKeypair::TransportKey(Data& publicKey, OTPassword& privateKey) const
{
    OT_ASSERT(m_pkeyPrivate);

    return m_pkeyPrivate->TransportKey(publicKey, privateKey);
}

bool OTKeypair::hasCapability(const NymCapability& capability) const
{
    if (m_pkeyPrivate) {
        return m_pkeyPrivate->hasCapability(capability);
    }

    return false;
}
} // namespace opentxs
