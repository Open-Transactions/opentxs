// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

//////////////////////////////////////////////////////////////////////

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
// Each OTKeypair has 2 Asymmetrics (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

#include "stdafx.hpp"

#include "opentxs/core/crypto/LowLevelKeyGenerator.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/Signature.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"

#include <ostream>

#include "Keypair.hpp"

#define OT_METHOD "opentxs::crypto::key::implementation::Keypair::"

template class opentxs::Pimpl<opentxs::crypto::key::Keypair>;

namespace opentxs::crypto::key
{
OTKeypair Keypair::Factory(
    const NymParameters& nymParameters,
    const proto::KeyRole role)
{
    return OTKeypair{new implementation::Keypair(nymParameters, role)};
}

OTKeypair Keypair::Factory(
    const proto::AsymmetricKey& serializedPubkey,
    const proto::AsymmetricKey& serializedPrivkey)
{
    return OTKeypair{
        new implementation::Keypair(serializedPubkey, serializedPrivkey)};
}

OTKeypair Keypair::Factory(const proto::AsymmetricKey& serializedPubkey)
{
    return OTKeypair{new implementation::Keypair(serializedPubkey)};
}
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
Keypair::Keypair(const NymParameters& nymParameters, const proto::KeyRole role)
    : m_pkeyPublic{Asymmetric::Factory(nymParameters, role)}
    , m_pkeyPrivate{Asymmetric::Factory(nymParameters, role)}
    , role_{role}
{
    make_new_keypair(nymParameters);
}

Keypair::Keypair(
    const proto::AsymmetricKey& serializedPubkey,
    const proto::AsymmetricKey& serializedPrivkey)
    : m_pkeyPublic{Asymmetric::Factory(serializedPubkey)}
    , m_pkeyPrivate{Asymmetric::Factory(serializedPrivkey)}
    , role_{m_pkeyPrivate->Role()}
{
}

Keypair::Keypair(const proto::AsymmetricKey& serializedPubkey)
    : m_pkeyPublic{Asymmetric::Factory(serializedPubkey)}
    , m_pkeyPrivate{Asymmetric::Factory()}
    , role_{m_pkeyPublic->Role()}
{
}

Keypair::Keypair(const Keypair& rhs)
    : key::Keypair{}
    , m_pkeyPublic{rhs.m_pkeyPublic}
    , m_pkeyPrivate{rhs.m_pkeyPrivate}
    , role_{rhs.role_}
{
}

bool Keypair::CalculateID(Identifier& theOutput) const
{
    OT_ASSERT(m_pkeyPublic.get());

    return m_pkeyPublic->CalculateID(theOutput);  // Only works for public keys.
}

Keypair* Keypair::clone() const { return new Keypair(*this); }

// Return the private key as an Asymmetric object
// TODO this violates encapsulation and should be deprecated
const Asymmetric& Keypair::GetPrivateKey() const
{
    OT_ASSERT(m_pkeyPrivate.get());

    return m_pkeyPrivate;
}

// Return the public key as an Asymmetric object
// TODO this violates encapsulation and should be deprecated
const Asymmetric& Keypair::GetPublicKey() const
{
    OT_ASSERT(m_pkeyPublic.get());

    return m_pkeyPublic;
}

// Get a public key as an opentxs::String.
// This form is used in all cases except for the NymIDSource
// of a self-signed MasterCredential
bool Keypair::GetPublicKey(String& strKey) const
{
    OT_ASSERT(m_pkeyPublic.get());

    return m_pkeyPublic->GetPublicKey(strKey);
}

std::int32_t Keypair::GetPublicKeyBySignature(
    Keys& listOutput,  // Inclusive means, return the key even
                       // when theSignature has no metadata.
    const Signature& theSignature,
    bool bInclusive) const
{
    OT_ASSERT(m_pkeyPublic.get());

    const auto* metadata = m_pkeyPublic->GetMetadata();

    OT_ASSERT(nullptr != metadata);

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
    if (!theSignature.getMetaData().HasMetadata() || !metadata->HasMetadata() ||
        (metadata->HasMetadata() && theSignature.getMetaData().HasMetadata() &&
         (theSignature.getMetaData() == *(metadata)))) {
        // ...Then add m_pkeyPublic as a possible match, to listOutput.
        //
        listOutput.push_back(&m_pkeyPublic.get());
        return 1;
    }
    return 0;
}

bool Keypair::hasCapability(const NymCapability& capability) const
{
    if (m_pkeyPrivate.get()) {
        return m_pkeyPrivate->hasCapability(capability);
    }

    return false;
}

bool Keypair::HasPrivateKey() const
{
    OT_ASSERT(m_pkeyPrivate.get());

    return m_pkeyPrivate->IsPrivate();  // This means it actually has a private
                                        // key in it, or tried to.
}

bool Keypair::HasPublicKey() const
{
    OT_ASSERT(m_pkeyPublic.get());

    return m_pkeyPublic->IsPublic();  // This means it actually has a public key
                                      // in it, or tried to.
}

bool Keypair::make_new_keypair(const NymParameters& nymParameters)
{
    LowLevelKeyGenerator lowLevelKeys(nymParameters);

    if (!lowLevelKeys.MakeNewKeypair()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed in a call to LowLevelKeyGenerator::MakeNewKeypair.")
            .Flush();
        return false;
    }

    OTPasswordData passwordData("Enter or set the wallet master password.");
    return lowLevelKeys.SetOntoKeypair(*this, passwordData);

    // If true is returned:
    // Success! At this point, theKeypair's public and private keys have been
    // set.
}

// Used when importing/exporting a Nym to/from the wallet.
//
bool Keypair::ReEncrypt(const OTPassword& theExportPassword, bool bImporting)
{

    OT_ASSERT(m_pkeyPublic.get());
    OT_ASSERT(m_pkeyPrivate.get());
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
    const auto strReasonAbove = String::Factory(
        bImporting ? "Enter the new export passphrase. (Above "
                     "ReEncryptPrivateKey in OTKeypair::ReEncrypt)"
                   : "Enter your wallet's master passphrase. (Above "
                     "ReEncryptPrivateKey in OTKeypair::ReEncrypt)");

    const auto strReasonBelow = String::Factory(
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
        theExportPassword, bImporting);  // <==== IMPORT or EXPORT occurs here.

    if (!(bReEncrypted)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure, either when re-encrypting, or "
            "when subsequently retrieving "
            "the public/private keys. bImporting == ")(
            bImporting ? "true" : "false")(".")
            .Flush();
    }

    return (bReEncrypted);
}

std::shared_ptr<proto::AsymmetricKey> Keypair::Serialize(bool privateKey) const
{
    OT_ASSERT(m_pkeyPublic.get());

    if (privateKey) {
        OT_ASSERT(m_pkeyPrivate.get());

        return m_pkeyPrivate->Serialize();
    } else {
        return m_pkeyPublic->Serialize();
    }
}

bool Keypair::TransportKey(Data& publicKey, OTPassword& privateKey) const
{
    OT_ASSERT(m_pkeyPrivate.get());

    return m_pkeyPrivate->TransportKey(publicKey, privateKey);
}

bool Keypair::Verify(const Data& plaintext, const proto::Signature& sig) const
{
    if (!m_pkeyPublic.get()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Missing public key. Can not verify.")
            .Flush();

        return false;
    }

    return m_pkeyPublic->Verify(plaintext, sig);
}
}  // namespace opentxs::crypto::key::implementation
