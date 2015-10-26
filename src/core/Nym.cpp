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

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/Nym.hpp>
#include <opentxs/core/crypto/CredentialSet.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/Ledger.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/Message.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/crypto/OTSignedFile.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/crypto/NymParameters.hpp>
#include <opentxs/core/crypto/ChildKeyCredential.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/crypto/OTSymmetricKey.hpp>
#include <opentxs/core/util/Tag.hpp>

#include <irrxml/irrXML.hpp>

#include <algorithm>
#include <fstream>
#include <memory>

// static

namespace opentxs
{

Nym* Nym::LoadPublicNym(const Identifier& NYM_ID, const String* pstrName,
                        const char* szFuncName)
{
    const char* szFunc =
        (nullptr != szFuncName) ? szFuncName : "OTPseudonym::LoadPublicNym";

    const String strNymID(NYM_ID);

    // If name is empty, construct one way,
    // else construct a different way.
    //
    Nym* pNym = ((nullptr == pstrName) || !pstrName->Exists())
                    ? (new Nym(NYM_ID))
                    : (new Nym(*pstrName, strNymID, strNymID));
    OT_ASSERT_MSG(nullptr != pNym,
                  "OTPseudonym::LoadPublicNym: Error allocating memory.\n");

    bool bLoadedKey =
        pNym->LoadPublicKey(); // Deprecated. Only used for old-style Nyms.
                               // Eventually, remove this. Currently
                               // LoadPublicKey calls LoadCredentials, which is
                               // what we should be calling, once the nym's own
                               // keypair is eliminated.

    // First load the public key
    if (!bLoadedKey)
        otWarn << __FUNCTION__ << ": " << szFunc
               << ": Unable to find nym: " << strNymID << "\n";
    else if (!pNym->VerifyPseudonym())
        otErr << __FUNCTION__ << ": " << szFunc
              << ": Security: Failure verifying Nym: " << strNymID << "\n";
    else if (!pNym->LoadSignedNymfile(*pNym)) {
        otLog4 << "OTPseudonym::LoadPublicNym " << szFunc
               << ": Usually normal: There's no Nymfile (" << strNymID
               << "), though there IS a public "
                  "key, which checks out. It's probably just someone else's "
                  "Nym. (So I'm still returning this Nym to "
                  "the caller so he can still use the public key.)\n";
        return pNym;
    }
    else // success
        return pNym;

    delete pNym;
    pNym = nullptr;

    return nullptr;
}

/*

 Normally when I read someone ELSE'S public key, I DON'T have their Nymfile.
 Therefore I don't HAVE any of their credentials, I only have the NymID and the
 public key itself. Which was fine before, since you hashed the public key to
 verify the NymID -- and if it matched, then all was good.

 But let's say I'm reading a public key from a key credential... the hash of
 that key
 (or credential) is NOT necessarily the hash of the Nym's ID. So how can I
 verify it?

 Normally I read from:  ~/.ot/client_data/pubkeys/NYM_ID   (for the public key
 for that Nym.)

 Instead I want to read up all available credentials for that Nym.
 Let's say their IDs are stored in: ~/.ot/client_data/pubkeys/NYM_ID.cred
 And the credentials themselves:
 ~/.ot/client_data/pubkeys/credentials/NYM_ID/CREDENTIAL_ID

 So let's say I have a NymID and I need to load his credentials and use his keys
 to verify
 the Nym and also to send him messages, verify his signatures on instruments,
 etc.

 pNym->LoadPublicKey() should load the UNSIGNED Nymfile from:
 ~/.ot/client_data/pubkeys/NYM_ID.cred
 Which, in the course of loading, should automatically load all its credentials
 from
 ~/.ot/client_data/pubkeys/credentials/NYM_ID/CREDENTIAL_ID

 From there, we call VerifyPseudonym, which verifies all the credentials
 internally, including
 their signatures, and which verifies them against their source, which can also
 be hashed to
 reproduce the NymID.

 ...SINCE all the credentials verify AGAINST THEIR SOURCE, and the source hashes
 to form the NymID,
 then the Nym is therefore self-verifying, without having to sign the nymfile
 itself (for public nyms.)
 If I want to sign it anyway, well I will need a private Nym to sign it with, I
 cannot sign it with
 only a public Nym. So I will have to pass the signer in if I want to do that.
 But it's unnecessary,
 and in fact it's also unnecessary for private Nyms, (or is it?) because all the
 credentials ARE signed
 contracts already, and since they all ultimately verify to their source.

 Therefore this eliminates LoadSignedNymfile? And SaveSignedNymFile? Or augments
 -- since we will be backwards compatible...

 I'll load the local Nym's credential list and the public nym's credential list
 from the same
 place: ~/.ot/client_data/credentials/NYM_ID.cred
 EITHER WAY I'll load the actual credentials, each from
 ~/.ot/client_data/credentials/NYM_ID/CRED_ID
 Then for the local Nym, I'll load his Nymfile like normal from
 ~/.ot/client_data/nyms/NYM_ID
 (For public Nyms, I don't need to load a Nymfile or anything else -- the
 credentials are all there is.)

 But I still want my Nymfile itself signed, so people can't mess with my
 transaction numbers.
 What it comes down to is, I must put the credential list in a separate file,
 load it, verify it,
 and then use it to verify the signature on the Nymfile which contains
 everything else. So the
 "LoadSignedNymfile" process isn't going to change for the Nymfile itself, and I
 will have the credential
 list in a separate file, just like the certs were in a separate file in the
 original system (for the
 same reasons.)

 */

// static
Nym* Nym::LoadPrivateNym(const Identifier& NYM_ID, bool bChecking,
                         const String* pstrName, const char* szFuncName,
                         const OTPasswordData* pPWData,
                         const OTPassword* pImportPassword)
{
    const char* szFunc =
        (nullptr != szFuncName) ? szFuncName : "OTPseudonym::LoadPrivateNym";

    if (NYM_ID.IsEmpty()) return nullptr;

    const String strNymID(NYM_ID);

    // If name is empty, construct one way,
    // else construct a different way.
    //
    Nym* pNym = ((nullptr == pstrName) || !pstrName->Exists())
                    ? (new Nym(NYM_ID))
                    : (new Nym(*pstrName, strNymID, strNymID));
    OT_ASSERT_MSG(nullptr != pNym,
                  "OTPseudonym::LoadPrivateNym: Error allocating memory.\n");

    OTPasswordData thePWData(OT_PW_DISPLAY);
    if (nullptr == pPWData) pPWData = &thePWData;

    bool bLoadedKey = pNym->LoadCredentials(true, pPWData, pImportPassword);

    if (!bLoadedKey)
        Log::vOutput(
            bChecking ? 1 : 0,
            "%s: %s: (%s: is %s).  Unable to load credentials, "
            "cert and private key for: %s (maybe this nym doesn't exist?)\n",
            __FUNCTION__, szFunc, "bChecking", bChecking ? "true" : "false",
            strNymID.Get());
    // success loading credentials
    // failure verifying pseudonym public key.
    else if (!pNym->VerifyPseudonym()) // <====================
        otErr << __FUNCTION__ << " " << szFunc
              << ": Failure verifying Nym public key: " << strNymID << "\n";
    // success verifying pseudonym public key.
    // failure loading signed nymfile.
    else if (!pNym->LoadSignedNymfile(*pNym)) // Unlike with public key,
                                              // with private key we DO
                                              // expect nymfile to be
                                              // here.
        otErr << __FUNCTION__ << " " << szFunc
              << ": Failure calling LoadSignedNymfile: " << strNymID << "\n";
    else // ultimate success.
        return pNym;

    delete pNym;
    pNym = nullptr;

    return nullptr;
}

// pstrSourceForNymID if left nullptr, will use the Nym's public key (OT's
// original
// method.)
// Since that is already the Nym's ID (hash of public key) then providing the
// Nym's public
// key here will naturally hash to the correct ID and thus is the logical
// default action.
//
// But pstrSourceForNymID might contain a Bitcoin or Namecoin address, or the DN
// info from
// a traditional Certificate Authority, or a Freenet or Tor or I2P URL, or
// indeed any URL
// at all... We will also continue to support a simple public key, and by
// default, if none
// is provided, we will use the Nym's own public key as the source, and if the
// credential
// contents are not provided, the public cert or public key in full raw text
// shall be placed
// in this field.
//
// In the case of a traditional CA-issued cert, you would pass the unique issuer
// and subject DN
// info into the pstrSourceForNymID parameter (where the hash of that data
// results in the NymID,
// so you should only do it with a brand-new Nym in that case, since it will
// change the NymID.
// So for the API for creating new Nyms, we will force it that way in the
// interface to keep things
// safe, but at this low of a level, the power exists.) Continuing that same
// example, you would
// pass your actual public cert as pstrCredentialContents.
//
// If pstrSourceForNymID was a URL, then perhaps pstrCredentialContents would
// contain the contents
// located at that URL. As long as the contents of the string
// pstrCredentialContents can also
// be found at pstrSourceForNymID (or a list of hashes of similar strings...)
// then the master
// credential will verify. So for example, at the URL you might just post the
// same public cert,
// or you might have some CredentialSet serialized string that contains the Nym
// ID, the Master
// credential IDs and child credential IDs that are valid and invalid.
//
// Thought: Seems that there are three ways of exporting the CredentialSet as
// IDs only, as IDs
// with public keys, and as IDs with public keys and private keys.
//
// hash of pstrSourceForNymID is the NymID, and hash of pstrCredentialContents
// is the credential ID for this new master credential.
//
bool Nym::AddNewMasterCredential(
    String& strOutputMasterCredID,
    const std::shared_ptr<NymParameters>& pKeyData,
    const String* pstrSourceForNymID, // If nullptr, it uses the Nym's
                                      // (presumed) existing source
                                      // as the source.
    const OTPasswordData* pPWData,    // Pass in the string to show users here,
                                      // if/when asking for the passphrase.
    bool bChangeNymID)                // Must be explicitly set to true, to change
                                      // the Nym's ID. Other restrictions also
                                      // apply... must be your first master
                                      // credential. Must have no accounts.
                                      // Basically can only be used for brand-new
                                      // Nyms in circumstances where it's assumed
                                      // the Nym's ID is in the process of being
                                      // generated anyway. Should never be used on
                                      // some existing Nym who is already in the
                                      // wallet and who may even have accounts
                                      // somewhere already.
{
    String::Map mapPrivate, mapPublic; // Used sometimes.

    // Note: if the Nym is self-signed, then his "source" is his public key,
    // which can thus
    // never change (because it would change his ID.) We only allow this in the
    // case where a
    // Nym is first being created.
    //
    // But if the Nym has a real source (such as a URL, or Namecoin address,
    // etc) then he should
    // be able to create new credentials, and should BOTH be able to generate
    // keys on the spot

    OT_ASSERT(pKeyData);

    OTAsymmetricKey::KeyType newKeyType = pKeyData->AsymmetricKeyType();
    String newKeyTypeName = OTAsymmetricKey::KeyTypeToString(newKeyType);

    otOut << __FUNCTION__
          << "Creating new master credential with type: "
          << newKeyTypeName.Get()
          << "\n";

    //
    // SOURCE
    //
    // A source was passed in. We know it's not a public key, since its hash
    // will not match the existing
    // Nym ID. (Only the existing public key will match that when hashed, and
    // you can use that by passing nullptr
    // already, which is the following block.) Therefore it must be one of the
    // other choices: a Freenet/I2P/Tor URL,
    // or indeed any URL at all, a Bitcoin address, a Namecoin address (which is
    // actually a form of URL also.)
    // Or of course, the Issuer/Subject DN info from a traditionally-issued
    // cert. In which case we are low-level
    // enough that we will potentially change the NymID here based on that
    // option. But we won't do that for a public
    // key, unless you pass nullptr.
    //
    const String* pstrSourceToUse = nullptr;
    bool changeNymID = false;
    bool pstrSourceForNymIDValid = false;

    if ((nullptr != pstrSourceForNymID) and pstrSourceForNymID->Exists())
    {
        pstrSourceForNymIDValid = true;
    }

    if (!(m_strSourceForNymID.Exists()))
    {
        //Creating a new Nym
        changeNymID = true;
        // if pstrSourceToUse is null when the constructor for CredentialSet
        // is called, it will create a self-signed nym

        if (pstrSourceForNymIDValid)
        {
            //use the provided source
            pstrSourceToUse = pstrSourceForNymID;
        }
    }
    else
    {
        //Adding a new CredentialSet to an existing nym
        if (pstrSourceForNymIDValid)
        {
            if (!pstrSourceForNymID->Compare(m_strSourceForNymID))
            {
                otErr << "In " << __FILE__ << ", line " << __LINE__
                    << ", OTPseudonym::" << __FUNCTION__
                    << ": The Nym already has a source, but a different "
                        "source was passed in (they should be identical, since "
                        "you can't change a Nym's "
                        "source without changing his ID.)\n";
                return false;
            }
        }
        // Whether or not a source was passed in, use the existing one
        pstrSourceToUse = &m_strSourceForNymID;
    }

    // See if there are any other master credentials already. If there are, make
    // sure they have the correct nymIDSource
    //
    if (!m_mapCredentialSets.empty()) {
        for (auto& it : m_mapCredentialSets) {
            CredentialSet* pCredential = it.second;
            OT_ASSERT(nullptr != pCredential);

            if (false ==
                pstrSourceToUse->Compare(pCredential->GetSourceForNymID())) {
                otOut << __FUNCTION__
                      << ": Attempt to add new master credential failed to "
                         "match the NymID Source on an existing master "
                         "credential.\n";
                return false;
            }
        }
    }

    // Create a new CredentialSet

    CredentialSet* pNewCredentialSet = new CredentialSet(pKeyData, pPWData, pstrSourceToUse);

    if (nullptr == pNewCredentialSet) // Below this block, pNewCredentialSet must be cleaned up.
    {
        otErr << __FUNCTION__ << ": Failed trying to create a new master "
                                "credential, while calling "
                                "CredentialSet::CreateMaster.\n";
        return false;
    }

    // NOTE: The only way to verify a Master Credential is to go to its source
    // (for its NymID)
    // and see if the Master Credential ID is posted at that source. And since
    // the Master Credential
    // ID cannot possibly be known before the Master Credential is created and
    // hashed, there is no
    // way that Credential's ID could have already been posted at that source.
    // Therefore it's
    // impossible for the Credential to verify against the source at this time.
    // (Although it must
    // verify by the time the credential is registered at any OT server.)

    // We can still verify the credential internally, however. (Everything but
    // the source.)
    //

    if (changeNymID)
    {
        // No matter what happened above, the master credential has a source
        // since if none was passed it constructed as self-signed
        m_strSourceForNymID = pNewCredentialSet->GetSourceForNymID();
        m_nymID.CalculateDigest(m_strSourceForNymID);
    }

    String strNymID;
    GetIdentifier(strNymID);

    if (!pNewCredentialSet->VerifyInternally()) {
        Identifier idProposedSource;
        idProposedSource.CalculateDigest(*pstrSourceToUse);
        const String strTempID(idProposedSource);
        otErr << __FUNCTION__
            << ": Failed trying to verify the new master credential.\n"
                "Nym ID: " << strNymID << "\nHash of Source: " << strTempID
            << "\n Source: " << pstrSourceToUse->Get() << "\n";
        delete pNewCredentialSet;
        pNewCredentialSet = nullptr;
        return false;
    }

    // OTFolders::Credential() is for private credentials (I've created.)
    // OTFolders::Pubcred() is for public credentials (I've downloaded.)
    // ...(FYI.)
    //
    if (OTDB::Exists(OTFolders::Credential().Get(), strNymID.Get(),
                    pNewCredentialSet->GetMasterCredID().Get())) {
        otErr << __FUNCTION__
            << ": Failure: Apparently there is already a credential stored "
                "for this ID (even though this is a new master credential, "
                "just generated.)\n";
        delete pNewCredentialSet;
        pNewCredentialSet = nullptr;
        return false;
    }

    // It's already signed (inside CreateMaster) but it's still not added to the
    // Nym,
    // or saved to local storage. So let's do that next...
    //
    String strFoldername, strFilename;
    strFoldername.Format("%s%s%s", OTFolders::Credential().Get(),
                        Log::PathSeparator(), strNymID.Get());
    strFilename.Format("%s", pNewCredentialSet->GetMasterCredID().Get());
    const bool bSaved =
        const_cast<MasterCredential&>(pNewCredentialSet->GetMasterCredential())
            .SaveContract(strFoldername.Get(), strFilename.Get());

    if (!bSaved) {
        otErr << __FUNCTION__ << ": Failed trying to save new master "
                                "credential to local storage.\n";
        delete pNewCredentialSet;
        pNewCredentialSet = nullptr;
        return false;
    }

    m_mapCredentialSets.insert(std::pair<std::string, CredentialSet*>(
        pNewCredentialSet->GetMasterCredID().Get(), pNewCredentialSet));
    strOutputMasterCredID = pNewCredentialSet->GetMasterCredID();

    //
    // Todo: someday we'll add "source" and "altLocation" to CreateNym
    // (currently the public key is just assumed to be the source.)
    // When that time comes, we'll need to set the altLocation here as
    // well, since it will be optional whenever there is a source involved.

    SaveCredentialIDs();

    return true;
}

bool Nym::AddNewChildKeyCredential(const Identifier& idMasterCredential,
                       const std::shared_ptr<NymParameters> pKeyData, // Ignored unless pmapPrivate is nullptr.
                       const String::Map* pmapPrivate, // If nullptr, then the
                                                       // keys are
                                                       // generated in here.
                       const OTPasswordData* pPWData, String* pstrNewID)
{
    const String strMasterCredID(idMasterCredential);

    auto it = m_mapCredentialSets.find(strMasterCredID.Get());

    if (it == m_mapCredentialSets.end()) // Didn't find it.
    {
        otOut << __FUNCTION__ << ": Failed trying to add key credential to "
                                 "nonexistent master credential.\n";
        return false;
    }

    CredentialSet* pMaster = it->second;
    OT_ASSERT(nullptr != pMaster);

    ChildKeyCredential* pChildKeyCredential = nullptr;

    if (pKeyData) {
        const bool bAdded =
            pMaster->AddNewChildKeyCredential(pKeyData, pPWData, &pChildKeyCredential);

        if (!bAdded) {
            otOut
                << __FUNCTION__
                << ": Failed trying to add key credential to master credential.\n";
            return false;
        }
    }
    OT_ASSERT(nullptr != pChildKeyCredential);

    if (!pChildKeyCredential->VerifyInternally()) {

        otErr << __FUNCTION__
              << ": Failed trying to verify the new key credential.\n";
        // todo: remove it again, since it failed to verify.
        return false;
    }

    // It's already signed (inside AddNewChildKeyCredential) but it's still not added to the
    // Nym,
    // or saved to local storage. So let's do that next...
    //
    String strNymID, strChildKeyCredentialID;
    GetIdentifier(strNymID);
    pChildKeyCredential->GetIdentifier(strChildKeyCredentialID);

    // OTFolders::Credential() is for private credentials (I've created.)
    // OTFolders::Pubcred() is for public credentials (I've downloaded.)
    //
    if (OTDB::Exists(OTFolders::Credential().Get(), strNymID.Get(),
                     strChildKeyCredentialID.Get())) {
        otErr << __FUNCTION__
              << ": Failure: Apparently there is already a credential stored "
                 "for this ID (even though this is a new credential, just "
                 "generated.)\n";
        return false;
    }

    String strFoldername, strFilename;
    strFoldername.Format("%s%s%s", OTFolders::Credential().Get(),
                         Log::PathSeparator(), strNymID.Get());
    strFilename.Format("%s", strChildKeyCredentialID.Get());
    const bool bSaved =
        pChildKeyCredential->SaveContract(strFoldername.Get(), strFilename.Get());
    if (!bSaved) {
        otErr
            << __FUNCTION__
            << ": Failed trying to save new key credential to local storage.\n";
        return false;
    }

    SaveCredentialIDs();

    if (nullptr != pstrNewID) *pstrNewID = strChildKeyCredentialID;

    return true;
}

/// Though the parameter is a reference (forcing you to pass a real object),
/// the Nym DOES take ownership of the object. Therefore it MUST be allocated
/// on the heap, NOT the stack, or you will corrupt memory with this call.
///
void Nym::AddMail(Message& theMessage) // a mail message is a form of
                                       // transaction, transported via
                                       // Nymbox
{
    m_dequeMail.push_front(&theMessage);
}

/// return the number of mail items available for this Nym.
int32_t Nym::GetMailCount() const
{
    return static_cast<int32_t>(m_dequeMail.size());
}

// Look up a piece of mail by index.
// If it is, return a pointer to it, otherwise return nullptr.
Message* Nym::GetMailByIndex(int32_t nIndex) const
{
    const uint32_t uIndex = nIndex;

    // Out of bounds.
    if (m_dequeMail.empty() || (nIndex < 0) || (uIndex >= m_dequeMail.size()))
        return nullptr;

    return m_dequeMail.at(nIndex);
}

bool Nym::RemoveMailByIndex(int32_t nIndex) // if false, mail
                                            // index was bad.
{
    const uint32_t uIndex = nIndex;

    // Out of bounds.
    if (m_dequeMail.empty() || (nIndex < 0) || (uIndex >= m_dequeMail.size()))
        return false;

    Message* pMessage = m_dequeMail.at(nIndex);

    OT_ASSERT(nullptr != pMessage);

    m_dequeMail.erase(m_dequeMail.begin() + nIndex);

    delete pMessage;

    return true;
}

void Nym::ClearMail()
{
    while (GetMailCount() > 0) RemoveMailByIndex(0);
}

/// Though the parameter is a reference (forcing you to pass a real object),
/// the Nym DOES take ownership of the object. Therefore it MUST be allocated
/// on the heap, NOT the stack, or you will corrupt memory with this call.
///
void Nym::AddOutmail(Message& theMessage) // a mail message is a form
                                          // of transaction,
                                          // transported via Nymbox
{
    m_dequeOutmail.push_front(&theMessage);
}

/// return the number of mail items available for this Nym.
int32_t Nym::GetOutmailCount() const
{
    return static_cast<int32_t>(m_dequeOutmail.size());
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
Message* Nym::GetOutmailByIndex(int32_t nIndex) const
{
    const uint32_t uIndex = nIndex;

    // Out of bounds.
    if (m_dequeOutmail.empty() || (nIndex < 0) ||
        (uIndex >= m_dequeOutmail.size()))
        return nullptr;

    return m_dequeOutmail.at(nIndex);
}

bool Nym::RemoveOutmailByIndex(int32_t nIndex) // if false,
                                               // outmail index
                                               // was bad.
{
    const uint32_t uIndex = nIndex;

    // Out of bounds.
    if (m_dequeOutmail.empty() || (nIndex < 0) ||
        (uIndex >= m_dequeOutmail.size()))
        return false;

    Message* pMessage = m_dequeOutmail.at(nIndex);

    OT_ASSERT(nullptr != pMessage);

    m_dequeOutmail.erase(m_dequeOutmail.begin() + nIndex);

    delete pMessage;

    return true;
}

void Nym::ClearOutmail()
{
    while (GetOutmailCount() > 0) RemoveOutmailByIndex(0);
}

/// Though the parameter is a reference (forcing you to pass a real object),
/// the Nym DOES take ownership of the object. Therefore it MUST be allocated
/// on the heap, NOT the stack, or you will corrupt memory with this call.
///
void Nym::AddOutpayments(Message& theMessage) // a payments message is
                                              // a form of
                                              // transaction,
                                              // transported via
                                              // Nymbox
{
    m_dequeOutpayments.push_front(&theMessage);
}

/// return the number of payments items available for this Nym.
int32_t Nym::GetOutpaymentsCount() const
{
    return static_cast<int32_t>(m_dequeOutpayments.size());
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
Message* Nym::GetOutpaymentsByIndex(int32_t nIndex) const
{
    const uint32_t uIndex = nIndex;

    // Out of bounds.
    if (m_dequeOutpayments.empty() || (nIndex < 0) ||
        (uIndex >= m_dequeOutpayments.size()))
        return nullptr;

    return m_dequeOutpayments.at(nIndex);
}

// if this function returns false, outpayments index was bad.
bool Nym::RemoveOutpaymentsByIndex(int32_t nIndex, bool bDeleteIt)
{
    const uint32_t uIndex = nIndex;

    // Out of bounds.
    if (m_dequeOutpayments.empty() || (nIndex < 0) ||
        (uIndex >= m_dequeOutpayments.size())) {
        otErr << __FUNCTION__
              << ": Error: Index out of bounds: signed: " << nIndex
              << " unsigned: " << uIndex << " (size is "
              << m_dequeOutpayments.size() << ").\n";
        return false;
    }

    Message* pMessage = m_dequeOutpayments.at(nIndex);
    OT_ASSERT(nullptr != pMessage);

    m_dequeOutpayments.erase(m_dequeOutpayments.begin() + uIndex);

    if (bDeleteIt) delete pMessage;

    return true;
}

void Nym::ClearOutpayments()
{
    while (GetOutpaymentsCount() > 0) RemoveOutpaymentsByIndex(0);
}

// Instead of a "balance statement", some messages require a "transaction
// statement".
// Whenever the number of transactions changes, you must sign the new list so
// you
// aren't responsible for cleared transactions, for example. Or so you server
// will
// allow you to take responsibility for a new transaction number (only if you've
// signed off on it!)
//
// There will have to be another version of this function for when you don't
// have
// a transaction (like a processNymbox!) Otherwise you would need a transaction
// number
// in order to do a processNymbox. This function therefore is available in that
// incarnation
// even when you don't have a transaction number. It'll just attach the balance
// item to
// the message directly.
//
Item* Nym::GenerateTransactionStatement(const OTTransaction& theOwner)
{
    if ((theOwner.GetNymID() != m_nymID)) {
        otErr << "OTPseudonym::" << __FUNCTION__
              << ": Transaction has wrong owner (expected to match nym).\n";
        return nullptr;
    }

    // theOwner is the depositPaymentPlan, activateSmartContract, or marketOffer
    // that triggered the need for this transaction statement.
    // since it uses up a transaction number, I will be sure to remove that one
    // from my list before signing the list.
    Item* pBalanceItem = Item::CreateItemFromTransaction(
        theOwner, Item::transactionStatement); // <=== transactionStatement
                                               // type, with user ID, server
                                               // ID, transaction ID.

    // The above has an ASSERT, so this this will never actually happen.
    if (nullptr == pBalanceItem) return nullptr;

    // COPY THE ISSUED TRANSACTION NUMBERS FROM THE NYM

    Nym theMessageNym;

    theMessageNym.HarvestIssuedNumbers(
        theOwner.GetPurportedNotaryID(),
        *this /*unused in this case, not saving to disk*/, *this,
        false); // bSave = false;

    switch (theOwner.GetType()) {
    case OTTransaction::cancelCronItem:
        if (theOwner.GetTransactionNum() > 0) {
            theMessageNym.RemoveIssuedNum(
                theOwner.GetRealNotaryID(),
                theOwner.GetTransactionNum()); // a transaction number is being
                                               // used, and REMOVED from my list
                                               // of responsibility,
            theMessageNym.RemoveTransactionNum(
                theOwner.GetRealNotaryID(),
                theOwner.GetTransactionNum()); // so I want the new signed list
                                               // to reflect that number has
                                               // been REMOVED.
        }
        break;

    // Transaction Statements usually only have a transaction number in the case
    // of market offers and
    // payment plans, in which case the number should NOT be removed, and
    // remains in play until
    // final closure from Cron.
    case OTTransaction::marketOffer:
    case OTTransaction::paymentPlan:
    case OTTransaction::smartContract:
    default:
        break;
    }

    // What about cases where no number is being used? (Such as processNymbox)
    // Perhaps then if this function is even called, it's with a 0-number
    // transaction, in which
    // case the above Removes probably won't hurt anything.  Todo.

    String strMessageNym(theMessageNym); // Okay now we have the transaction
                                         // numbers in this MessageNym string.

    pBalanceItem->SetAttachment(strMessageNym); // <======== This is where the
                                                // server will read the
                                                // transaction numbers from (A
                                                // nym in item.m_ascAttachment)

    pBalanceItem->SignContract(*this); // <=== Sign, save, and return.
                                       // OTTransactionType needs to weasel in a
                                       // "date signed" variable.
    pBalanceItem->SaveContract();

    return pBalanceItem;
}

// use this to actually generate a new key pair and assorted nym files.
//
bool Nym::GenerateNym(const std::shared_ptr<NymParameters>& pKeyData,
                      bool bCreateFile, // By default, it
                                        // creates the various
                                        // nym files
                                        // and certs in local storage. (Pass false when
                                        // creating a temp Nym, like for OTPurse.)
                      const std::string str_id_source/*=""*/,
		      const std::string str_alt_location/*=""*/)
{
    OT_ASSERT(pKeyData);

    String strSource(str_id_source),
    strAltLocation(str_alt_location);

    SetAltLocation(strAltLocation);

    String strReason("Creating new Nym."); // NOTE:
                                           // Savex509CertAndPrivateKey
                                           // sets the ID and sometimes if
                                           // necessary, the source.
    bool bSaved = false;
    String strMasterCredID;

    const bool bAddedMaster = AddNewMasterCredential(
                                strMasterCredID,
                                pKeyData,
                                &strSource,
                                nullptr);

    if ( bAddedMaster && strMasterCredID.Exists() &&
          ( GetMasterCredentialCount() > 0 ) )
      {
      }
    else
      {
        bSaved = false;
        otErr << __FUNCTION__ << ": Failed trying to add new master "
              "credential to new Nym.\n";
      }

    if (bCreateFile) {
      bSaved = SaveSignedNymfile(*this); // Now we'll generate the
      // NymFile as well!
      // (bCreateFile will be
      // false for temp Nyms..)
    }

    if (bCreateFile && !bSaved) {
      otErr << __FUNCTION__
      << ": Failed trying to save new Nym's cert or nymfile.\n";
      
      return false;
    }

    return bSaved;
}

// If an ID is passed in, that means remove all numbers FOR THAT SERVER ID.
// If passed in, and current map doesn't match, then skip it (continue).

#ifndef CLEAR_MAP_AND_DEQUE
#define CLEAR_MAP_AND_DEQUE(the_map)                                           \
    for (auto& it : the_map) {                                                 \
        if ((nullptr != pstrNotaryID) && (str_NotaryID != it.first)) continue; \
        dequeOfTransNums* pDeque = (it.second);                                \
        OT_ASSERT(nullptr != pDeque);                                          \
        if (!(pDeque->empty())) pDeque->clear();                               \
    }
#endif // CLEAR_MAP_AND_DEQUE

// Sometimes for testing I need to clear out all the transaction numbers from a
// nym.
// So I added this method to make such a thing easy to do.
//
void Nym::RemoveAllNumbers(const String* pstrNotaryID,
                           bool bRemoveHighestNum) // Some callers
                                                   // don't want
                                                   // to wipe
// the highest num. Some do.
{
    std::string str_NotaryID(pstrNotaryID ? pstrNotaryID->Get() : "");

    // These use str_NotaryID (above)
    //
    CLEAR_MAP_AND_DEQUE(m_mapIssuedNum)
    CLEAR_MAP_AND_DEQUE(m_mapTransNum)
    CLEAR_MAP_AND_DEQUE(m_mapTentativeNum)
    CLEAR_MAP_AND_DEQUE(m_mapAcknowledgedNum)

    std::list<mapOfHighestNums::iterator> listOfHighestNums;
    std::list<mapOfIdentifiers::iterator> listOfNymboxHash;
    std::list<mapOfIdentifiers::iterator> listOfInboxHash;
    std::list<mapOfIdentifiers::iterator> listOfOutboxHash;
    std::list<mapOfIdentifiers::iterator> listOfRecentHash;

    if (bRemoveHighestNum) {
        for (auto it(m_mapHighTransNo.begin()); it != m_mapHighTransNo.end();
             ++it) {
            if ((nullptr != pstrNotaryID) &&
                (str_NotaryID != it->first)) // If passed in, and current it
                                             // doesn't match, then skip it
                                             // (continue).
                continue;

            listOfHighestNums.push_back(it);
        }
    }

    for (auto it(m_mapNymboxHash.begin()); it != m_mapNymboxHash.end(); ++it) {
        if ((nullptr != pstrNotaryID) &&
            (str_NotaryID != it->first)) // If passed in, and current it doesn't
                                         // match, then skip it (continue).
            continue;

        listOfNymboxHash.push_back(it);
    }

    // This is mapped to acct_id, not notary_id.
    // (So we just wipe them all.)
    for (auto it(m_mapInboxHash.begin()); it != m_mapInboxHash.end(); ++it) {
        listOfInboxHash.push_back(it);
    }

    // This is mapped to acct_id, not notary_id.
    // (So we just wipe them all.)
    for (auto it(m_mapOutboxHash.begin()); it != m_mapOutboxHash.end(); ++it) {
        listOfOutboxHash.push_back(it);
    }

    for (auto it(m_mapRecentHash.begin()); it != m_mapRecentHash.end(); ++it) {
        if ((nullptr != pstrNotaryID) &&
            (str_NotaryID != it->first)) // If passed in, and current it doesn't
                                         // match, then skip it (continue).
            continue;

        listOfRecentHash.push_back(it);
    }

    while (!listOfHighestNums.empty()) {
        m_mapHighTransNo.erase(listOfHighestNums.back());
        listOfHighestNums.pop_back();
    }
    while (!listOfNymboxHash.empty()) {
        m_mapNymboxHash.erase(listOfNymboxHash.back());
        listOfNymboxHash.pop_back();
    }
    while (!listOfInboxHash.empty()) {
        m_mapInboxHash.erase(listOfInboxHash.back());
        listOfInboxHash.pop_back();
    }
    while (!listOfOutboxHash.empty()) {
        m_mapOutboxHash.erase(listOfOutboxHash.back());
        listOfOutboxHash.pop_back();
    }
    while (!listOfRecentHash.empty()) {
        m_mapRecentHash.erase(listOfRecentHash.back());
        listOfRecentHash.pop_back();
    }
}

//    OTIdentifier        m_NymboxHash;       // (Server-side) Hash of the
// Nymbox
//  mapOfIdentifiers    m_mapNymboxHash;    // (Client-side) Hash of Nymbox
// (OTIdentifier) mapped by NotaryID (std::string)

bool Nym::GetNymboxHashServerSide(const Identifier& theNotaryID,
                                  Identifier& theOutput) // server-side
{
    if (m_NymboxHash.IsEmpty()) {
        Ledger theNymbox(m_nymID, m_nymID, theNotaryID);

        if (theNymbox.LoadNymbox() && theNymbox.CalculateNymboxHash(theOutput))
            return true;
    }

    return false;
}

void Nym::SetNymboxHashServerSide(const Identifier& theInput) // server-side
{
    m_NymboxHash = theInput;
}

bool Nym::GetNymboxHash(const std::string& notary_id,
                        Identifier& theOutput) const // client-side
{
    return GetHash(m_mapNymboxHash, notary_id, theOutput);
}

bool Nym::SetNymboxHash(const std::string& notary_id,
                        const Identifier& theInput) // client-side
{
    return SetHash(m_mapNymboxHash, notary_id, theInput);
}

bool Nym::GetRecentHash(const std::string& notary_id,
                        Identifier& theOutput) const // client-side
{
    return GetHash(m_mapRecentHash, notary_id, theOutput);
}

bool Nym::SetRecentHash(const std::string& notary_id,
                        const Identifier& theInput) // client-side
{
    return SetHash(m_mapRecentHash, notary_id, theInput);
}

bool Nym::GetInboxHash(const std::string& acct_id,
                       Identifier& theOutput) const // client-side
{
    return GetHash(m_mapInboxHash, acct_id, theOutput);
}

bool Nym::SetInboxHash(const std::string& acct_id,
                       const Identifier& theInput) // client-side
{
    return SetHash(m_mapInboxHash, acct_id, theInput);
}

bool Nym::GetOutboxHash(const std::string& acct_id,
                        Identifier& theOutput) const // client-side
{
    return GetHash(m_mapOutboxHash, acct_id, theOutput);
}

bool Nym::SetOutboxHash(const std::string& acct_id,
                        const Identifier& theInput) // client-side
{
    return SetHash(m_mapOutboxHash, acct_id, theInput);
}

bool Nym::GetHash(const mapOfIdentifiers& the_map, const std::string& str_id,
                  Identifier& theOutput) const // client-side
{
    bool bRetVal =
        false; // default is false: "No, I didn't find a hash for that id."
    theOutput.Release();

    // The Pseudonym has a map of its recent hashes, one for each server
    // (nymbox) or account (inbox/outbox).
    // For Server Bob, with this Pseudonym, I might have hash lkjsd987345lkj.
    // For but Server Alice, I might have hash 98345jkherkjghdf98gy.
    // (Same Nym, but different hash for each server, as well as inbox/outbox
    // hashes for each asset acct.)
    //
    // So let's loop through all the hashes I have, and if the ID on the map
    // passed in
    // matches the [server|acct] ID that was passed in, then return TRUE.
    //
    for (const auto& it : the_map) {
        if (str_id == it.first) {
            // The call has succeeded
            bRetVal = true;
            theOutput = it.second;
            break;
        }
    }

    return bRetVal;
}

bool Nym::SetHash(mapOfIdentifiers& the_map, const std::string& str_id,
                  const Identifier& theInput) // client-side
{
    bool bSuccess = false;

    auto find_it = the_map.find(str_id);

    if (the_map.end() != find_it) // found something for that str_id
    {
        // The call has succeeded
        the_map.erase(find_it);
        the_map[str_id] = theInput;
        bSuccess = true;
    }

    // If I didn't find it in the list above (whether the list is empty or
    // not....)
    // that means it does not exist. (So create it.)
    //
    if (!bSuccess) {
        the_map[str_id] = theInput;
        bSuccess = true;
    }
    //    if (bSuccess)
    //    {
    //        SaveSignedNymfile(SIGNER_NYM);
    //    }

    return bSuccess;
}

void Nym::RemoveReqNumbers(const String* pstrNotaryID)
{
    const std::string str_NotaryID(pstrNotaryID ? pstrNotaryID->Get() : "");

    for (auto it(m_mapRequestNum.begin()); it != m_mapRequestNum.end(); ++it) {
        if ((nullptr != pstrNotaryID) &&
            (str_NotaryID != it->first)) // If passed in, and current it doesn't
                                         // match, then skip it (continue).
            continue;

        m_mapRequestNum.erase(it);
    }
}

// You can't go using a Nym at a certain server, if it's not registered there...
// BTW -- if you have never called GetRequestNumber(), then this will wrongly
// return
// false!
// But as long as you call getRequestNumber() upon successsful registration (or
// whenever) this
// function will return an accurate answer after that point, and forever.
//
bool Nym::IsRegisteredAtServer(const String& strNotaryID) const
{
    bool bRetVal = false; // default is return false: "No, I'm NOT registered at
                          // that Server."
    std::string strID = strNotaryID.Get();

    // The Pseudonym has a map of the request numbers for different servers.
    // For Server Bob, with this Pseudonym, I might be on number 34.
    // For but Server Alice, I might be on number 59.
    //
    // So let's loop through all the numbers I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then return TRUE.
    for (auto& it : m_mapRequestNum) {
        
        if (strID == it.first) {

            // The call has succeeded
            bRetVal = true;

            break;
        }
    }

    return bRetVal;
}

// Removes Request Num for specific server
// (Like if Nym has deleted his account on that server...)
// Caller is responsible to save Nym after this.
//
bool Nym::UnRegisterAtServer(const String& strNotaryID)
{
    bool bRetVal = false; // default is return false: "No, I'm NOT registered at
                          // that Server."
    std::string strID = strNotaryID.Get();

    // The Pseudonym has a map of the request numbers for different servers.
    // For Server Bob, with this Pseudonym, I might be on number 34.
    // For but Server Alice, I might be on number 59.
    //
    // So let's loop through all the numbers I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then delete that one.
    //
    for (auto it(m_mapRequestNum.begin()); it != m_mapRequestNum.end(); ++it) {
        if (strID == it->first) {
            // The call has succeeded
            bRetVal = true;
            m_mapRequestNum.erase(it);
            break;
        }
    }

    return bRetVal;
}

#ifndef WIPE_MAP_AND_DEQUE
#define WIPE_MAP_AND_DEQUE(the_map)                                            \
    while (!the_map.empty()) {                                                 \
        dequeOfTransNums* pDeque = the_map.begin()->second;                    \
        OT_ASSERT(nullptr != pDeque);                                          \
        the_map.erase(the_map.begin());                                        \
        delete pDeque;                                                         \
        pDeque = nullptr;                                                      \
    }
#endif // WIPE_MAP_AND_DEQUE

void Nym::ReleaseTransactionNumbers()
{
    WIPE_MAP_AND_DEQUE(m_mapTransNum)
    WIPE_MAP_AND_DEQUE(m_mapIssuedNum)
    WIPE_MAP_AND_DEQUE(m_mapTentativeNum)
    WIPE_MAP_AND_DEQUE(m_mapAcknowledgedNum)
}

/*
 ResyncWithServer:

--    OTIdentifier        m_NymboxHash;       // (Server-side) Hash of the
Nymbox

--    mapOfIdentifiers    m_mapNymboxHash;    // (Client-side) Hash of latest
DOWNLOADED Nymbox (OTIdentifier) mapped by NotaryID (std::string)
--    mapOfIdentifiers    m_mapRecentHash;    // (Client-side) Hash of Nymbox
according to Server, based on some recent reply. (May be newer...)

--    mapOfIdentifiers    m_mapInboxHash;
--    mapOfIdentifiers    m_mapOutboxHash;

--    dequeOfMail        m_dequeMail;    // Any mail messages received by this
Nym. (And not yet deleted.)
--    dequeOfMail        m_dequeOutmail;    // Any mail messages sent by this
Nym. (And not yet deleted.)
--    dequeOfMail        m_dequeOutpayments;    // Any outoing payments sent by
this Nym. (And not yet deleted.) (payments screen.)

--    mapOfRequestNums m_mapRequestNum;    // Whenever this user makes a request
to a transaction server

**    mapOfTransNums     m_mapTransNum;    // Each Transaction Request must be
accompanied by a fresh transaction #,
**    mapOfTransNums     m_mapIssuedNum;    // If the server has issued me
(1,2,3,4,5) and I have already used 1-3,
**    mapOfTransNums     m_mapTentativeNum;

**  mapOfHighestNums m_mapHighTransNo;  // Mapped, a single int64_t to each
server (just like request numbers are.)

--    mapOfTransNums    m_mapAcknowledgedNum; // request numbers are stored
here.

    // (SERVER side)
--    std::set<int64_t> m_setOpenCronItems; // Until these Cron Items are closed
out, the server-side Nym keeps a list of them handy.

    // (SERVER side)
    // Using strings here to avoid juggling memory crap.
--    std::set<std::string> m_setAccounts; // A list of asset account IDs.
Server side only (client side uses wallet; has multiple servers.)

    // (SERVER side.)
--    int64_t    m_lUsageCredits;    // Server-side. The usage credits available
for this Nym. Infinite if negative.

 */

/*
 OTPseudonym::RemoveAllNumbers affects (**):  (-- means doesn't affect)

--    OTIdentifier        m_NymboxHash;       // (Server-side) Hash of the
Nymbox

**    mapOfIdentifiers    m_mapNymboxHash;    // (Client-side) Hash of latest
DOWNLOADED Nymbox (OTIdentifier) mapped by NotaryID (std::string)
**    mapOfIdentifiers    m_mapRecentHash;    // (Client-side) Hash of Nymbox
according to Server, based on some recent reply. (May be newer...)

**    mapOfIdentifiers    m_mapInboxHash;
**    mapOfIdentifiers    m_mapOutboxHash;

--    dequeOfMail        m_dequeMail;    // Any mail messages received by this
Nym. (And not yet deleted.)
--    dequeOfMail        m_dequeOutmail;    // Any mail messages sent by this
Nym. (And not yet deleted.)
--    dequeOfMail        m_dequeOutpayments;    // Any outoing payments sent by
this Nym. (And not yet deleted.) (payments screen.)

--    mapOfRequestNums m_mapRequestNum;

**    mapOfTransNums   m_mapTransNum;
**    mapOfTransNums   m_mapIssuedNum;
**    mapOfTransNums     m_mapTentativeNum;

**    mapOfHighestNums m_mapHighTransNo;  // Mapped, a single int64_t to each
server (just like request numbers are.)

**  mapOfTransNums     m_mapAcknowledgedNum;  // request nums are stored.

    // (SERVER side)
--    std::set<int64_t> m_setOpenCronItems; // Until these Cron Items are closed
out, the server-side Nym keeps a list of them handy.

    // (SERVER side)
--    std::set<std::string> m_setAccounts; // A list of asset account IDs.
Server side only (client side uses wallet; has multiple servers.)

    // (SERVER side.)
--    int64_t    m_lUsageCredits;    // Server-side. The usage credits available
for this Nym. Infinite if negative.



 CLEAR_MAP_AND_DEQUE(m_mapIssuedNum)
 CLEAR_MAP_AND_DEQUE(m_mapTransNum)
 CLEAR_MAP_AND_DEQUE(m_mapTentativeNum)
 CLEAR_MAP_AND_DEQUE(m_mapAcknowledgedNum)

 m_mapHighTransNo.erase(listOfHighestNums.back());
 m_mapNymboxHash.erase(listOfNymboxHash.back());
 m_mapRecentHash.erase(listOfRecentHash.back());

*/

// ** ResyncWithServer **
//
// Not for normal use! (Since you should never get out of sync with the server
// in the first place.)
// However, in testing, or if some bug messes up some data, or whatever, and you
// absolutely need to
// re-sync with a server, and you trust that server not to lie to you, then this
// function will do the trick.
// NOTE: Before calling this, you need to do a getNymbox() to download the
// latest Nymbox, and you need to do
// a registerNym() to download the server's copy of your Nym. You then
// need to load that Nymbox from
// local storage, and you need to load the server's message Nym out of the
// registerNymResponse reply, so that
// you can pass both of those objects into this function, which must assume that
// those pieces were already done
// just prior to this call.
//
bool Nym::ResyncWithServer(const Ledger& theNymbox, const Nym& theMessageNym)
{
    bool bSuccess = true;

    const Identifier& theNotaryID = theNymbox.GetRealNotaryID();
    const String strNotaryID(theNotaryID);
    const String strNymID(m_nymID);

    const int32_t nIssuedNumCount =
        theMessageNym.GetIssuedNumCount(theNotaryID);
    const int32_t nTransNumCount =
        theMessageNym.GetTransactionNumCount(theNotaryID);

    // Remove all issued, transaction, and tentative numbers for a specific
    // server ID,
    // as well as all acknowledgedNums, and the highest transaction number for
    // that notaryID,
    // from *this nym. Leave our record of the highest trans num received from
    // that server,
    // since we will want to just keep it when re-syncing. (Server doesn't store
    // that anyway.)
    //
    RemoveAllNumbers(&strNotaryID, false); // bRemoveHighestNum=true by
                                           // default. But in this case, I
                                           // keep it.

    // Any issued or trans numbers we add to *this from theMessageNym, are also
    // added here so
    // they can be used to update the "highest number" record (at the bottom of
    // this function.)
    //
    std::set<int64_t> setTransNumbers;

    // Now that *this has no issued or transaction numbers for theNotaryID, we
    // add
    // them back again from theMessageNym. (So they will match, and be 'N
    // SYNC!!!)
    //
    // Copy the issued and transaction numbers from theMessageNym onto *this.
    //
    for (int32_t n1 = 0; n1 < nIssuedNumCount; ++n1) {
        const int64_t lNum = theMessageNym.GetIssuedNum(theNotaryID, n1);

        if (!AddIssuedNum(strNotaryID, lNum)) // Add to list of
                                              // numbers that
                                              // haven't been
                                              // closed yet.
        {
            otErr << "OTPseudonym::ResyncWithServer: Failed trying to add "
                     "IssuedNum (" << lNum << ") onto *this nym: " << strNymID
                  << ", for server: " << strNotaryID << "\n";
            bSuccess = false;
        }
        else {
            setTransNumbers.insert(lNum);

            otWarn << "OTPseudonym::ResyncWithServer: Added IssuedNum (" << lNum
                   << ") onto *this nym: " << strNymID
                   << ", for server: " << strNotaryID << " \n";
        }
    }

    for (int32_t n2 = 0; n2 < nTransNumCount; ++n2) {
        const int64_t lNum = theMessageNym.GetTransactionNum(theNotaryID, n2);

        if (!AddTransactionNum(strNotaryID, lNum)) // Add to list of
                                                   // available-to-use
                                                   // numbers.
        {
            otErr << "OTPseudonym::ResyncWithServer: Failed trying to add "
                     "TransactionNum (" << lNum
                  << ") onto *this nym: " << strNymID
                  << ", for server: " << strNotaryID << "\n";
            bSuccess = false;
        }
        else {
            setTransNumbers.insert(lNum);

            otWarn << "OTPseudonym::ResyncWithServer: Added TransactionNum ("
                   << lNum << ") onto *this nym: " << strNymID
                   << ", for server: " << strNotaryID << " \n";
        }
    }

    // We already cleared all tentative numbers from *this (above in
    // RemoveAllNumbers). Next, loop through theNymbox and add Tentative numbers
    // to *this based on each successNotice in the Nymbox. This way, when the
    // notices
    // are processed, they will succeed because the Nym will believe he was
    // expecting them.
    //
    for (auto& it : theNymbox.GetTransactionMap()) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);
        //        OTString strTransaction(*pTransaction);
        //        otErr << "TRANSACTION CONTENTS:\n%s\n", strTransaction.Get());

        // (a new; ALREADY just added transaction number.)
        if ((OTTransaction::successNotice !=
             pTransaction->GetType())) // if !successNotice
            continue;

        const int64_t lNum =
            pTransaction->GetReferenceToNum(); // successNotice is inRefTo the
                                               // new transaction # that should
                                               // be on my tentative list.

        if (!AddTentativeNum(strNotaryID, lNum)) // Add to list of
        // tentatively-being-added
        // numbers.
        {
            otErr << "OTPseudonym::ResyncWithServer: Failed trying to add "
                     "TentativeNum (" << lNum
                  << ") onto *this nym: " << strNymID
                  << ", for server: " << strNotaryID << "\n";
            bSuccess = false;
        }
        else
            otWarn << "OTPseudonym::ResyncWithServer: Added TentativeNum ("
                   << lNum << ") onto *this nym: " << strNymID
                   << ", for server: " << strNotaryID << " \n";
        // There's no "else insert to setTransNumbers" here, like the other two
        // blocks above.
        // Why not? Because setTransNumbers is for updating the Highest Trans
        // Num record on this Nym,
        // and the Tentative Numbers aren't figured into that record until AFTER
        // they are accepted
        // from the Nymbox. So I leave them out, since this function is
        // basically setting us up to
        // successfully process the Nymbox, which will then naturally update the
        // highest num record
        // based on the tentatives, as it's removing them from the tentative
        // list and adding them to
        // the "available" transaction list (and issued.)
    }

    const std::string strID = strNotaryID.Get();

    for (auto& it_high_num : m_mapHighTransNo) {
        // We found it!
        if (strID == it_high_num.first) {
            // See if any numbers on the set are higher, and if so, update the
            // record to match.
            //
            for (auto& it : setTransNumbers) {
                const int64_t lTransNum = it;

                // Grab a copy of the old highest trans number
                const int64_t lOldHighestNumber = it_high_num.second;

                if (lTransNum > lOldHighestNumber) // Did we find a bigger one?
                {
                    // Then update the Nym's record!
                    m_mapHighTransNo[it_high_num.first] = lTransNum;
                    otWarn
                        << "OTPseudonym::ResyncWithServer: Updated HighestNum ("
                        << lTransNum << ") record on *this nym: " << strNymID
                        << ", for server: " << strNotaryID << " \n";
                }
            }

            // We only needed to do this for the one server, so we can break
            // now.
            break;
        }
    }

    return (SaveSignedNymfile(*this) && bSuccess);
}

/*
typedef std::deque<int64_t>                            dequeOfTransNums;
typedef std::map<std::string, dequeOfTransNums *>    mapOfTransNums;
*/

// Verify whether a certain transaction number appears on a certain list.
//
bool Nym::VerifyGenericNum(const mapOfTransNums& THE_MAP,
                           const String& strNotaryID,
                           const int64_t& lTransNum) const
{
    std::string strID = strNotaryID.Get();

    // The Pseudonym has a deque of transaction numbers for each servers.
    // These deques are mapped by Notary ID.
    //
    // So let's loop through all the deques I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then find the transaction
    // number on
    // that list, and then return true. Else return false.
    //
    for (auto& it : THE_MAP) {
        // if the NotaryID passed in matches the notaryID for the current deque
        if (strID == it.first) {
            dequeOfTransNums* pDeque = (it.second);
            OT_ASSERT(nullptr != pDeque);

            if (!(pDeque->empty())) // there are some numbers for that server ID
            {
                // Let's loop through them and see if the culprit is there
                for (uint32_t i = 0; i < pDeque->size(); i++) {
                    // Found it!
                    if (lTransNum == pDeque->at(i)) {
                        return true;
                    }
                }
            }
            break;
        }
    }

    return false;
}

// On the server side: A user has submitted a specific transaction number.
// Remove it from his file so he can't use it again.
bool Nym::RemoveGenericNum(mapOfTransNums& THE_MAP, Nym& SIGNER_NYM,
                           const String& strNotaryID, const int64_t& lTransNum)
{
    bool bRetVal = RemoveGenericNum(THE_MAP, strNotaryID, lTransNum);

    if (bRetVal) {
        SaveSignedNymfile(SIGNER_NYM);
    }

    return bRetVal;
}

// This function is a little lower level, and doesn't worry about saving. Used
// internally.
// Returns true IF it successfully finds and removes the number. Otherwise
// returns false.
//
bool Nym::RemoveGenericNum(mapOfTransNums& THE_MAP, const String& strNotaryID,
                           const int64_t& lTransNum)
{
    bool bRetVal = false;
    std::string strID = strNotaryID.Get();

    // The Pseudonym has a deque of transaction numbers for each servers.
    // These deques are mapped by Notary ID.
    //
    // So let's loop through all the deques I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then find the transaction
    // number on
    // that list, and then remove it, and return true. Else return false.
    //
    for (auto& it : THE_MAP) {
        // if the NotaryID passed in matches the notaryID for the current deque
        if (strID == it.first) {
            dequeOfTransNums* pDeque = (it.second);

            OT_ASSERT(nullptr != pDeque);

            if (!(pDeque->empty())) // there are some numbers for that server ID
            {
                // Let's loop through them and see if the culprit is there
                for (uint32_t i = 0; i < pDeque->size(); i++) {
                    // Found it!
                    if (lTransNum == pDeque->at(i)) {
                        pDeque->erase(pDeque->begin() + i);
                        bRetVal = true;
                        break;
                    }
                }
            }
            break;
        }
    }

    return bRetVal;
}

// No signer needed for this one, and save is false.
// This version is ONLY for cases where we're not saving inside this function.
bool Nym::AddGenericNum(mapOfTransNums& THE_MAP, const String& strNotaryID,
                        int64_t lTransNum)
{
    bool bSuccessFindingNotaryID = false, bSuccess = false;
    std::string strID = strNotaryID.Get();

    // The Pseudonym has a deque of transaction numbers for each server.
    // These deques are mapped by Notary ID.
    //
    // So let's loop through all the deques I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then add the transaction
    // number.
    //
    for (auto& it : THE_MAP) {
        // if the NotaryID passed in matches the notaryID for the current deque
        if (strID == it.first) {
            dequeOfTransNums* pDeque = (it.second);
            OT_ASSERT(nullptr != pDeque);

            auto iter = std::find(pDeque->begin(), pDeque->end(), lTransNum);

            if (iter == pDeque->end()) // Only add it if it's not already there.
                                       // No duplicates!
                pDeque->push_front(lTransNum);

            bSuccess = true;
            bSuccessFindingNotaryID = true;

            break;
        }
    }

    // Apparently there is not yet a deque stored for this specific notaryID.
    // Fine. Let's create it then, and then add the transaction num to that new
    // deque.
    if (!bSuccessFindingNotaryID) {
        dequeOfTransNums* pDeque = new dequeOfTransNums;

        OT_ASSERT(nullptr != pDeque);

        THE_MAP[strID] = pDeque;
        pDeque->push_front(lTransNum);
        bSuccess = true;
    }

    return bSuccess;
}

// Returns count of transaction numbers available for a given server.
//
int32_t Nym::GetGenericNumCount(const mapOfTransNums& THE_MAP,
                                const Identifier& theNotaryID) const
{
    int32_t nReturnValue = 0;

    const String strNotaryID(theNotaryID);
    std::string strID = strNotaryID.Get();

    dequeOfTransNums* pDeque = nullptr;

    // The Pseudonym has a deque of transaction numbers for each server.
    // These deques are mapped by Notary ID.
    //
    // So let's loop through all the deques I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then we found the right server.
    for (auto& it : THE_MAP) {
        // if the NotaryID passed in matches the notaryID for the current deque
        if (strID == it.first) {
            pDeque = (it.second);
            OT_ASSERT(nullptr != pDeque);

            break;
        }
    }

    // We found the right server, so let's count the transaction numbers
    // that this nym has already stored for it.
    if (nullptr != pDeque) {
        nReturnValue = static_cast<int32_t>(pDeque->size());
    }

    return nReturnValue;
}

// by index.
int64_t Nym::GetGenericNum(const mapOfTransNums& THE_MAP,
                           const Identifier& theNotaryID, int32_t nIndex) const
{
    int64_t lRetVal = 0;

    const String strNotaryID(theNotaryID);
    std::string strID = strNotaryID.Get();

    // The Pseudonym has a deque of numbers for each server.
    // These deques are mapped by Notary ID.
    //
    // So let's loop through all the deques I have, and if the server ID on the
    // maps matches the Notary ID that was passed in, then find the number on
    // that list, and then return it.
    //
    for (auto& it : THE_MAP) {
        // if the NotaryID passed in matches the notaryID for the current deque
        if (strID == it.first) {
            dequeOfTransNums* pDeque = (it.second);
            OT_ASSERT(nullptr != pDeque);

            if (!(pDeque->empty())) // there are some numbers for that server ID
            {
                // Let's loop through them and see if the culprit is there
                for (uint32_t i = 0; i < pDeque->size(); i++) {
                    // Found it!
                    if (static_cast<uint32_t>(nIndex) == i) {
                        lRetVal = pDeque->at(i); // <==== Got the number here.
                        break;
                    }
                }
            }
            break;
        }
    }

    return lRetVal;
}

// by index.
int64_t Nym::GetIssuedNum(const Identifier& theNotaryID, int32_t nIndex) const
{
    return GetGenericNum(m_mapIssuedNum, theNotaryID, nIndex);
}

// by index.
int64_t Nym::GetTransactionNum(const Identifier& theNotaryID,
                               int32_t nIndex) const
{
    return GetGenericNum(m_mapTransNum, theNotaryID, nIndex);
}

// by index.
int64_t Nym::GetAcknowledgedNum(const Identifier& theNotaryID,
                                int32_t nIndex) const
{
    return GetGenericNum(m_mapAcknowledgedNum, theNotaryID, nIndex);
}

// TRANSACTION NUM

// On the server side: A user has submitted a specific transaction number.
// Verify whether he actually has a right to use it.
bool Nym::VerifyTransactionNum(const String& strNotaryID,
                               const int64_t& lTransNum) const // doesn't save
{
    return VerifyGenericNum(m_mapTransNum, strNotaryID, lTransNum);
}

// On the server side: A user has submitted a specific transaction number.
// Remove it from his file so he can't use it again.
bool Nym::RemoveTransactionNum(Nym& SIGNER_NYM, const String& strNotaryID,
                               const int64_t& lTransNum) // saves
{
    return RemoveGenericNum(m_mapTransNum, SIGNER_NYM, strNotaryID, lTransNum);
}

bool Nym::RemoveTransactionNum(const String& strNotaryID,
                               const int64_t& lTransNum) // doesn't
                                                         // save.
{
    return RemoveGenericNum(m_mapTransNum, strNotaryID, lTransNum);
}

// Returns count of transaction numbers available for a given server.
//
int32_t Nym::GetTransactionNumCount(const Identifier& theNotaryID) const
{
    return GetGenericNumCount(m_mapTransNum, theNotaryID);
}

// No signer needed for this one, and save is false.
// This version is ONLY for cases where we're not saving inside this function.
bool Nym::AddTransactionNum(const String& strNotaryID,
                            int64_t lTransNum) // doesn't save
{
    return AddGenericNum(m_mapTransNum, strNotaryID, lTransNum);
}

// ISSUED NUM

// On the server side: A user has submitted a specific transaction number.
// Verify whether it was issued to him and still awaiting final closing.
bool Nym::VerifyIssuedNum(const String& strNotaryID,
                          const int64_t& lTransNum) const
{
    return VerifyGenericNum(m_mapIssuedNum, strNotaryID, lTransNum);
}

// On the server side: A user has accepted a specific receipt.
// Remove it from his file so he's not liable for it anymore.
bool Nym::RemoveIssuedNum(Nym& SIGNER_NYM, const String& strNotaryID,
                          const int64_t& lTransNum) // saves
{
    return RemoveGenericNum(m_mapIssuedNum, SIGNER_NYM, strNotaryID, lTransNum);
}

bool Nym::RemoveIssuedNum(const String& strNotaryID,
                          const int64_t& lTransNum) // doesn't save
{
    return RemoveGenericNum(m_mapIssuedNum, strNotaryID, lTransNum);
}

// Returns count of transaction numbers not yet cleared for a given server.
//
int32_t Nym::GetIssuedNumCount(const Identifier& theNotaryID) const
{
    return GetGenericNumCount(m_mapIssuedNum, theNotaryID);
}

// No signer needed for this one, and save is false.
// This version is ONLY for cases where we're not saving inside this function.
bool Nym::AddIssuedNum(const String& strNotaryID,
                       const int64_t& lTransNum) // doesn't save.
{
    return AddGenericNum(m_mapIssuedNum, strNotaryID, lTransNum);
}

// TENTATIVE NUM

// On the server side: A user has submitted a specific transaction number.
// Verify whether it was issued to him and still awaiting final closing.
bool Nym::VerifyTentativeNum(const String& strNotaryID,
                             const int64_t& lTransNum) const
{
    return VerifyGenericNum(m_mapTentativeNum, strNotaryID, lTransNum);
}

// On the server side: A user has accepted a specific receipt.
// Remove it from his file so he's not liable for it anymore.
bool Nym::RemoveTentativeNum(Nym& SIGNER_NYM, const String& strNotaryID,
                             const int64_t& lTransNum) // saves
{
    return RemoveGenericNum(m_mapTentativeNum, SIGNER_NYM, strNotaryID,
                            lTransNum);
}

bool Nym::RemoveTentativeNum(const String& strNotaryID,
                             const int64_t& lTransNum) // doesn't save
{
    return RemoveGenericNum(m_mapTentativeNum, strNotaryID, lTransNum);
}

// No signer needed for this one, and save is false.
// This version is ONLY for cases where we're not saving inside this function.
bool Nym::AddTentativeNum(const String& strNotaryID,
                          const int64_t& lTransNum) // doesn't save.
{
    return AddGenericNum(m_mapTentativeNum, strNotaryID, lTransNum);
}

// ACKNOWLEDGED NUM

// These are actually used for request numbers, so both sides can determine
// which
// replies are already acknowledged. Used purely for optimization, to avoid
// downloading
// a large number of box receipts (specifically the replyNotices.)

// Client side: See if I've already seen the server's reply to a certain request
// num.
// Server side: See if I've already seen the client's acknowledgment of a reply
// I sent.
//
bool Nym::VerifyAcknowledgedNum(const String& strNotaryID,
                                const int64_t& lRequestNum) const
{
    return VerifyGenericNum(m_mapAcknowledgedNum, strNotaryID, lRequestNum);
}

// On client side: server acknowledgment has been spotted in a reply message, so
// I can remove it from my ack list.
// On server side: client has removed acknowledgment from his list (as evident
// since its sent with client messages), so server can remove it as well.
//
bool Nym::RemoveAcknowledgedNum(Nym& SIGNER_NYM, const String& strNotaryID,
                                const int64_t& lRequestNum) // saves
{
    return RemoveGenericNum(m_mapAcknowledgedNum, SIGNER_NYM, strNotaryID,
                            lRequestNum);
}

bool Nym::RemoveAcknowledgedNum(const String& strNotaryID,
                                const int64_t& lRequestNum) // doesn't
                                                            // save
{
    return RemoveGenericNum(m_mapAcknowledgedNum, strNotaryID, lRequestNum);
}

// Returns count of request numbers that the client has already seen the reply
// to
// (Or in the case of server-side, the list of request numbers that the client
// has
// told me he has already seen the reply to.)
//
int32_t Nym::GetAcknowledgedNumCount(const Identifier& theNotaryID) const
{
    return GetGenericNumCount(m_mapAcknowledgedNum, theNotaryID);
}

#ifndef OT_MAX_ACK_NUMS
#define OT_MAX_ACK_NUMS 100
#endif

// No signer needed for this one, and save is false.
// This version is ONLY for cases where we're not saving inside this function.
bool Nym::AddAcknowledgedNum(const String& strNotaryID,
                             const int64_t& lRequestNum) // doesn't
                                                         // save.
{
    // We're going to call AddGenericNum, but first, let's enforce a cap on the
    // total
    // number of ackNums allowed...
    //
    std::string strID = strNotaryID.Get();

    // The Pseudonym has a deque of transaction numbers for each server.
    // These deques are mapped by Notary ID.
    //
    // So let's loop through all the deques I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then we'll pop the size of the
    // deque
    // down to our max size (off the back) before then calling AddGenericNum
    // which will
    // push the new request number onto the front.
    //
    for (auto& it : m_mapAcknowledgedNum) {
        // if the NotaryID passed in matches the notaryID for the current deque
        if (strID == it.first) {
            dequeOfTransNums* pDeque = (it.second);
            OT_ASSERT(nullptr != pDeque);

            while (pDeque->size() > OT_MAX_ACK_NUMS) {
                pDeque->pop_back(); // This fixes knotwork's issue where he had
                                    // thousands of ack nums somehow never
                                    // getting cleared out. Now we have a MAX
                                    // and always keep it clean otherwise.
            }
            break;
        }
    }

    return AddGenericNum(m_mapAcknowledgedNum, strNotaryID,
                         lRequestNum); // <=== Here we finally add the new
                                       // request number, the actual purpose of
                                       // this function.
}

// HIGHER LEVEL...

// Client side: We have received a new trans num from server. Store it.
// Now the server uses this too, for storing these numbers so it can verify them
// later.
//
bool Nym::AddTransactionNum(Nym& SIGNER_NYM, const String& strNotaryID,
                            int64_t lTransNum,
                            bool bSave) // SAVE OR NOT (your choice) High-Level.
{
    bool bSuccess1 = AddTransactionNum(
        strNotaryID,
        lTransNum); // Add to list of available-to-use, outstanding numbers.
    bool bSuccess2 =
        AddIssuedNum(strNotaryID, lTransNum); // Add to list of numbers that
                                              // haven't been closed yet.

    if (bSuccess1 && !bSuccess2)
        RemoveGenericNum(m_mapTransNum, strNotaryID, lTransNum);
    else if (bSuccess2 && !bSuccess1)
        RemoveGenericNum(m_mapIssuedNum, strNotaryID, lTransNum);

    if (bSuccess1 && bSuccess2 && bSave)
        bSave = SaveSignedNymfile(SIGNER_NYM);
    else
        bSave = true; // so the return at the bottom calculates correctly.

    return (bSuccess1 && bSuccess2 && bSave);
}

// Client side: We have received a server's successful reply to a processNymbox
// accepting a specific new transaction number(s).
// Or, if the reply was lost, then we still found out later that the acceptance
// was successful, since a notice is still dropped
// into the Nymbox. Either way, this function removes the Tentative number,
// right before calling the above AddTransactionNum()
// in order to make it available for the Nym's use on actual transactions.
//
bool Nym::RemoveTentativeNum(Nym& SIGNER_NYM, const String& strNotaryID,
                             const int64_t& lTransNum,
                             bool bSave) // SAVE OR NOT (your choice)
                                         // High-Level.
{
    bool bSuccess = RemoveTentativeNum(
        strNotaryID, lTransNum); // Remove from list of numbers that haven't
                                 // been made available for use yet, though
                                 // they're "tentative"...

    if (bSuccess && bSave)
        bSave = SaveSignedNymfile(SIGNER_NYM);
    else
        bSave = true; // so the return at the bottom calculates correctly.

    return (bSuccess && bSave);
}

// Client side: We have accepted a certain receipt. Remove the transaction
// number from my list of issued numbers.
// The server uses this too, also for keeping track of issued numbers, and
// removes them around same time as client.
// (When receipt is accepted.) Also, There is no "RemoveTransactionNum" at this
// level since GetNextTransactionNum handles that.
//
bool Nym::RemoveIssuedNum(Nym& SIGNER_NYM, const String& strNotaryID,
                          const int64_t& lTransNum,
                          bool bSave) // SAVE OR NOT (your choice)
                                      // High-Level.
{
    bool bSuccess =
        RemoveIssuedNum(strNotaryID, lTransNum); // Remove from list of numbers
                                                 // that are still signed out.

    if (bSuccess && bSave)
        bSave = SaveSignedNymfile(SIGNER_NYM);
    else
        bSave = true; // so the return at the bottom calculates correctly.

    return (bSuccess && bSave);
}

// Client keeps track of server replies it's already seen.
// Server keeps track of these client acknowledgments.
// (Server also removes from Nymbox, any acknowledged message
// that wasn't already on his list based on request num.)
// Server already removes from his list, any number the client
// has removed from his.
// This is all purely for optimization, since it allows us to avoid
// downloading all the box receipts that contain replyNotices.
//
bool Nym::RemoveAcknowledgedNum(Nym& SIGNER_NYM, const String& strNotaryID,
                                const int64_t& lRequestNum,
                                bool bSave) // SAVE OR NOT (your choice)
                                            // High-Level.
{
    bool bSuccess = RemoveAcknowledgedNum(
        strNotaryID,
        lRequestNum); // Remove from list of acknowledged request numbers.

    if (bSuccess && bSave)
        bSave = SaveSignedNymfile(SIGNER_NYM);
    else
        bSave = true; // so the return at the bottom calculates correctly.

    return (bSuccess && bSave);
}

/// OtherNym is used as container for server to send us new transaction numbers
/// Currently unused. (old) NEW USE:
/// Okay then, new use: This will be the function that does what the below
/// function
/// does (OTPseudonym::HarvestIssuedNumbers), EXCEPT it only adds numbers that
/// aren't on the TENTATIVE list. Also, it will set the new "highest" trans num
/// for the appropriate server, based on the new numbers being harvested.
void Nym::HarvestTransactionNumbers(const Identifier& theNotaryID,
                                    Nym& SIGNER_NYM, Nym& theOtherNym,
                                    bool bSave)
{
    int64_t lTransactionNumber = 0;

    std::set<int64_t> setInput, setOutputGood, setOutputBad;

    for (auto& it : theOtherNym.GetMapIssuedNum()) {
        std::string strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;

        OT_ASSERT(nullptr != pDeque);

        String OTstrNotaryID = strNotaryID.c_str();
        const Identifier theTempID(OTstrNotaryID);

        if (!(pDeque->empty()) &&
            (theNotaryID == theTempID)) // only for the matching notaryID.
        {
            for (uint32_t i = 0; i < pDeque->size(); i++) {
                lTransactionNumber = pDeque->at(i);

                // If number wasn't already on issued list, then add to BOTH
                // lists.
                // Otherwise do nothing (it's already on the issued list, and no
                // longer
                // valid on the available list--thus shouldn't be re-added there
                // anyway.)
                //
                if ((true == VerifyTentativeNum(
                                 OTstrNotaryID,
                                 lTransactionNumber)) && // If I've actually
                                                         // requested this
                                                         // number and waiting
                                                         // on it...
                    (false ==
                     VerifyIssuedNum(OTstrNotaryID,
                                     lTransactionNumber)) // and if it's not
                                                          // already on my
                                                          // issued list...
                    )
                    setInput.insert(lTransactionNumber);
            }
            break; // We found it! Might as well break out.
        }
    } // for

    // Looks like we found some numbers to harvest
    // (tentative numbers we had already been waiting for,
    // yet hadn't processed onto our issued list yet...)
    //
    if (!setInput.empty()) {
        const String strNotaryID(theNotaryID), strNymID(m_nymID);

        int64_t lViolator = UpdateHighestNum(
            SIGNER_NYM, strNotaryID, setInput, setOutputGood,
            setOutputBad); // bSave=false (saved below already, if necessary)

        // NOTE: Due to the possibility that a server reply could be processed
        // twice (due to redundancy
        // for the purposes of preventing syncing issues) then we expect we
        // might get numbers in here
        // that are below our "last highest num" (due to processing the same
        // numbers twice.) Therefore
        // we don't need to assume an error in this case. UpdateHighestNum() is
        // already smart enough to
        // only update based on the good numbers, while ignoring the bad (i.e.
        // already-processed) ones.
        // Thus we really only have a problem if we receive a (-1), which would
        // mean an error occurred.
        // Also, the above call will log an FYI that it is skipping any numbers
        // below the line, so no need
        // to log more in the case of lViolater being >0 but less than the 'last
        // highest number.'
        //
        if ((-1) == lViolator)
            otErr << "OTPseudonym::HarvestTransactionNumbers"
                  << ": ERROR: UpdateHighestNum() returned (-1), "
                     "which is an error condition. "
                     "(Should never happen.)\nNym ID: " << strNymID << " \n";
        else {
            // We only remove-tentative-num/add-transaction-num for the numbers
            // that were above our 'last highest number'.
            // The contents of setOutputBad are thus ignored for these purposes.
            //
            for (auto& it : setOutputGood) {
                const int64_t lNoticeNum = it;

                // We already know it's on the TentativeNum list, since we
                // checked that in the above for loop.
                // We also already know that it's not on the issued list, since
                // we checked that as well.
                // That's why the below calls just ASSUME those things already.
                //
                RemoveTentativeNum(
                    strNotaryID, lNoticeNum); // doesn't save (but saved below)
                AddTransactionNum(SIGNER_NYM, strNotaryID, lNoticeNum,
                                  false); // bSave = false (but saved below...)
            }

            // We save regardless of whether any removals or additions are made,
            // because data was
            // updated in UpdateHighestNum regardless.
            //
            if (bSave) SaveSignedNymfile(SIGNER_NYM);
        }
    }
}

//  OtherNym is used as container for us to send server list of issued
// transaction numbers.
//  NOTE: in more recent times, a class has been added for managing lists of
// numbers. But
//  we didn't have that back when this was written.
//
// The above function is good for accepting new numbers onto my list, numbers
// that I already
// tried to sign for and are thus waiting in my tentative list. When calling
// that function,
// I am trying to accept those new numbers (the ones I was expecting already),
// and NO other
// numbers.
//
// Whereas with the below function, I am adding as "available" all the numbers
// that I didn't
// already have issued to me, REGARDLESS of tentative status. Why would I do
// this? Well,
// perhaps a temp nym is being used during some calculations, and I want to copy
// all the numbers
// over to the temp nym, period, regardless of his tentative list, because he
// has no tentative
// list, because he's not a nym in the first place.
//
void Nym::HarvestIssuedNumbers(const Identifier& theNotaryID, Nym& SIGNER_NYM,
                               Nym& theOtherNym, bool bSave)
{
    bool bChangedTheNym = false;
    int64_t lTransactionNumber = 0;

    for (auto& it : theOtherNym.GetMapIssuedNum()) {
        std::string strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;

        OT_ASSERT(nullptr != pDeque);

        String OTstrNotaryID =
            ((strNotaryID.size()) > 0 ? strNotaryID.c_str() : "");
        const Identifier theTempID(OTstrNotaryID);

        if (!(pDeque->empty()) && (theNotaryID == theTempID)) {
            for (uint32_t i = 0; i < pDeque->size(); i++) {
                lTransactionNumber = pDeque->at(i);

                // If number wasn't already on issued list, then add to BOTH
                // lists.
                // Otherwise do nothing (it's already on the issued list, and no
                // longer
                // valid on the available list--thus shouldn't be re-added there
                // anyway.)
                //
                if (false ==
                    VerifyIssuedNum(OTstrNotaryID, lTransactionNumber)) {
                    AddTransactionNum(
                        SIGNER_NYM, OTstrNotaryID, lTransactionNumber,
                        false); // bSave = false (but saved below...)
                    bChangedTheNym = true;
                }
            }
            break; // We found it! Might as well break out.
        }
    } // for

    if (bChangedTheNym && bSave) {
        SaveSignedNymfile(SIGNER_NYM);
    }
}

/// When a number IS already on my issued list, but NOT on my available list
/// (because I already used it on some transaction) then this function will
/// verify that and then add it BACK to my available list. (Like if the
/// transaction failed and I just want to get my numbers back so I can use
/// them on a different transaction.)
///
bool Nym::ClawbackTransactionNumber(
    const Identifier& theNotaryID,
    const int64_t& lTransClawback, // the number being clawed back.
    bool bSave, // false because you might call this function 10
                // times in a loop, and not want to save EVERY
                // iteration.
    Nym* pSIGNER_NYM)
{
    if (nullptr == pSIGNER_NYM) pSIGNER_NYM = this;
    // Below this point, pSIGNER_NYM is definitely a good pointer.

    const String strNotaryID(theNotaryID);

    // Only re-add the transaction number if it's already on my issued list.
    // (Otherwise, why am I "adding it back again" if I never had it in the
    // first place? Doesn't sound like a real clawback situation in that case.)
    //
    if (true == VerifyIssuedNum(strNotaryID, lTransClawback)) {
        AddTransactionNum(*pSIGNER_NYM, strNotaryID, lTransClawback, bSave);
        return true;
    }

    return false;
}

/// Client side.
/// Get the next available transaction number for the notaryID
/// The lTransNum parameter is for the return value.
/// SAVES if successful.
bool Nym::GetNextTransactionNum(Nym& SIGNER_NYM, const String& strNotaryID,
                                int64_t& lTransNum, bool bSave)
{
    bool bRetVal = false;
    std::string strID = strNotaryID.Get();

    // The Pseudonym has a deque of transaction numbers for each server.
    // These deques are mapped by Notary ID.
    //
    // So let's loop through all the deques I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then send out the transaction
    // number.
    //
    for (auto& it : m_mapTransNum) {
        // if the NotaryID passed in matches the notaryID for the current deque
        if (strID == it.first) {
            dequeOfTransNums* pDeque = (it.second);
            OT_ASSERT(nullptr != pDeque);

            if (!(pDeque->empty())) {
                lTransNum = pDeque->back();

                pDeque->pop_back();

                // The call has succeeded
                bRetVal = true;
            }
            break;
        }
    }

    if (bRetVal && bSave) {
        if (!SaveSignedNymfile(SIGNER_NYM))
            otErr << "Error saving signed NymFile in "
                     "OTPseudonym::GetNextTransactionNum\n";
    }

    return bRetVal;
}

// returns true on success, value goes into lReqNum
// Make sure the Nym is LOADED before you call this,
// otherwise it won't be there to get.
//
bool Nym::GetHighestNum(const String& strNotaryID, int64_t& lHighestNum) const
{
    bool bRetVal = false;
    std::string strID = strNotaryID.Get();

    // The Pseudonym has a map of the highest transaction # it's received from
    // different servers.
    // For Server Bob, with this Pseudonym, I might be on number 34.
    // For but Server Alice, I might be on number 59.
    //
    // So let's loop through all the numbers I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then send out the highest
    // number.
    //
    // Since the transaction number only ever gets bigger, this is a way of
    // preventing
    // the server from EVER tricking us by trying to give us a number that we've
    // already seen before.
    //
    for (auto& it : m_mapHighTransNo) {
        if (strID == it.first) {
            // Setup return value.
            lHighestNum = (it.second);

            // The call has succeeded
            bRetVal = true;

            break;
        }
    }

    return bRetVal;
}

// Go through setNumbers and make sure none of them is lower than the highest
// number I already have for this
// server. At the same time, keep a record of the largest one in the set. If
// successful, that becomes the new
// "highest" number I've ever received that server. Otherwise fail.
// If success, returns 0. If failure, returns the number that caused us to fail
// (by being lower than the last
// highest number.) I should NEVER receive a new transaction number that is
// lower than any I've gotten before.
// They should always only get bigger. UPDATE: Unless I happen to be processing
// an old receipt twice... (which
// can happen, due to redundancy used for preventing syncing issues, such as
// Nymbox notices.)
//
int64_t Nym::UpdateHighestNum(Nym& SIGNER_NYM, const String& strNotaryID,
                              std::set<int64_t>& setNumbers,
                              std::set<int64_t>& setOutputGood,
                              std::set<int64_t>& setOutputBad, bool bSave)
{
    bool bFoundNotaryID = false;
    int64_t lReturnVal = 0; // 0 is success.

    // First find the highest and lowest numbers out of the new set.
    //
    int64_t lHighestInSet = 0;
    int64_t lLowestInSet = 0;

    for (auto& it : setNumbers) {
        const int64_t lSetNum = it;

        if (lSetNum > lHighestInSet)
            lHighestInSet = lSetNum; // Set lHighestInSet to contain the highest
                                     // number out of setNumbers (input)

        if (0 == lLowestInSet)
            lLowestInSet = lSetNum; // If lLowestInSet is still 0, then set it
                                    // to the current number (happens first
                                    // iteration.)
        else if (lSetNum < lLowestInSet)
            lLowestInSet = lSetNum; // If current number is less than
                                    // lLowestInSet, then set lLowestInSet to
                                    // current Number.
    }

    // By this point, lLowestInSet contains the lowest number in setNumbers,
    // and lHighestInSet contains the highest number in setNumbers.

    //
    // The Pseudonym has a map of the "highest transaction numbers" for
    // different servers.
    // For Server Bob, with this Pseudonym, I might be on number 34.
    // For but Server Alice, I might be on number 59.
    //
    // So let's loop through all the numbers I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then update it there (then
    // break.)
    //
    // Make sure to save the Pseudonym afterwards, so the new numbers are saved.

    std::string strID = strNotaryID.Get();

    for (auto& it : m_mapHighTransNo) {
        // We found the notaryID key on the map?
        // We now know the highest trans number for that server?
        //
        if (strID == it.first) // Iterates inside this block zero times or one
                               // time. (One if it finds it, zero if not.)
        {
            // We found it!
            // Presumably we ONLY found it because this Nym has been properly
            // loaded first.
            // Good job! Otherwise, the list would have been empty even though
            // the highest number
            // was sitting in the file.

            // Grab a copy of the old highest trans number for this server.
            //
            const int64_t lOldHighestNumber =
                it.second; // <=========== The previous "highest number".

            // Loop through the numbers passed in, and for each, see if it's
            // less than
            // the previous "highest number for this server."
            //
            // If it's less, then we can't add it (must have added it
            // already...)
            // So we add it to the bad list.
            // But if it's more,

            for (auto& it_numbers : setNumbers) {
                const int64_t lSetNum = it_numbers;

                // If the current number (this iteration) is less than or equal
                // to the
                // "old highest number", then it's not going to be added twice.
                // (It goes on the "bad list.")
                //
                if (lSetNum <= lOldHighestNumber) {
                    otWarn << "OTPseudonym::UpdateHighestNum: New transaction "
                              "number is less-than-or-equal-to "
                              "last known 'highest trans number' record. (Must "
                              "be seeing the same server reply for "
                              "a second time, due to a receipt in my Nymbox.) "
                              "FYI, last known 'highest' number received: "
                           << lOldHighestNumber
                           << " (Current 'violator': " << lSetNum
                           << ") Skipping...\n";
                    setOutputBad.insert(lSetNum);
                }

                // The current number this iteration, as it should be, is HIGHER
                // than any transaction
                // number I've ever received before. (Although sometimes old
                // messages will 'echo'.)
                // I want to replace the "highest" record with this one
                else {
                    setOutputGood.insert(lSetNum);
                }
            }

            // Here we're making sure that all the numbers in the set are larger
            // than any others
            // that we've had before for the same server (They should only ever
            // get larger.)
            //
            //            if (lLowestInSet <= lOldHighestNumber) // ERROR!!! The
            // new numbers should ALWAYS be larger than the previous ones!
            if ((lLowestInSet > 0) &&
                (lLowestInSet <= lOldHighestNumber)) // WARNING! The new numbers
                                                     // should ALWAYS be larger
                                                     // than the previous ones!
                // UPDATE: Unless we happen to be processing the same receipt
                // for a second time, due to redundancy in the system (for
                // preventing syncing errors.)
                lReturnVal = lLowestInSet; // We return the violator (otherwise
                                           // 0 if success).

            // The loop has succeeded in finding the server ID and its
            // associated "highest number" value.
            //
            bFoundNotaryID = true;
            break;
            // This main for only ever has one active iteration: the one with
            // the right server ID. Once we find it, we break (no matter what.)
        } // server ID matches.
    }

    // If we found the server ID, that means the highest number was previously
    // recorded.
    // We don't want to replace it unless we were successful in this function.
    // And if we
    // were, then we want to replace it with the new "highest number in the
    // set."
    //
    // IF we found the server ID for a previously recorded highestNum, and
    // IF this function was a success in terms of the new numbers all exceeding
    // that old record,
    // THEN ERASE that old record and replace it with the new highest number.
    //
    // Hmm: Should I require ALL new numbers to be valid? Or should I take the
    // valid ones,
    // and ignore the invalid ones?
    //
    // Update: Just found this comment from the calling function:
    // NOTE: Due to the possibility that a server reply could be processed twice
    // (due to redundancy
    // for the purposes of preventing syncing issues) then we expect we might
    // get numbers in here
    // that are below our "last highest num" (due to processing the same numbers
    // twice.) Therefore
    // we don't need to assume an error in this case. UpdateHighestNum() is
    // already smart enough to
    // only update based on the good numbers, while ignoring the bad (i.e.
    // already-processed) ones.
    // Thus we really only have a problem if we receive a (-1), which would mean
    // an error occurred.
    // Also, the above call will log an FYI that it is skipping any numbers
    // below the line, so no need
    // to log more in the case of lViolater being >0 but less than the 'last
    // highest number.'
    //
    // ===> THEREFORE, we don't need an lReturnVal of 0 in order to update the
    // highest record.
    // Instead, we just need bFoundNotaryID to be true, and we need
    // setOutputGood to not be empty
    // (we already know the numbers in setOutputGood are higher than the last
    // highest recorded trans
    // num... that's why they are in setOutputGood instead of setOutputBad.)
    //
    if (!setOutputGood.empty()) // There's numbers worth savin'!
    {
        if (bFoundNotaryID) {
            otOut << "OTPseudonym::UpdateHighestNum: Raising Highest Trans "
                     "Number from " << m_mapHighTransNo[strID] << " to "
                  << lHighestInSet << ".\n";

            // We KNOW it's there, so we can straight-away just
            // erase it and insert it afresh..
            //
            m_mapHighTransNo.erase(strID);
            m_mapHighTransNo.insert(
                std::pair<std::string, int64_t>(strID, lHighestInSet));
        }

        // If I didn't find the server in the list above (whether the list is
        // empty or not....)
        // that means the record does not yet exist. (So let's create it)--we
        // wouldn't even be
        // here unless we found valid transaction numbers and added them to
        // setOutputGood.
        // (So let's record lHighestInSet mapped to strID, just as above.)
        else {
            otOut << "OTPseudonym::UpdateHighestNum: Creating "
                     "Highest Transaction Number entry for this server as '"
                  << lHighestInSet << "'.\n";
            m_mapHighTransNo.insert(
                std::pair<std::string, int64_t>(strID, lHighestInSet));
        }

        // By this point either the record was created, or we were successful
        // above in finding it
        // and updating it. Either way, it's there now and potentially needs to
        // be saved.
        //
        if (bSave) SaveSignedNymfile(SIGNER_NYM);
    }
    else // setOutputGood was completely empty in this case...
    {      // (So there's nothing worth saving.) A repeat message.
           //
           // Should I return a -1 here or something? Let's say it's
           // a redundant message...I've already harvested these numbers. So
           // they are ignored this time, my record of 'highest' is unimpacted,
        // and if I just return lReturnVal below, it will contain 0 for success
        // or a transaction number (the min/low violator) but even that is
        // considered
        // a "success" in the sense that some of the numbers would still
        // normally be
        // expected to have passed through.
        // The caller will check for -1 in case of some drastic error, but so
        // far I don't
        // see a place here for that return value.
        //
    }

    return lReturnVal; // Defaults to 0 (success) but above, might have been set
                       // to "lLowestInSet" (if one was below the mark.)
}

// returns true on success, value goes into lReqNum
// Make sure the Nym is LOADED before you call this,
// otherwise it won't be there to get.
// and if the request number needs to be incremented,
// then make sure you call IncrementRequestNum (below)
bool Nym::GetCurrentRequestNum(const String& strNotaryID,
                               int64_t& lReqNum) const
{
    bool bRetVal = false;
    std::string strID = strNotaryID.Get();

    // The Pseudonym has a map of the request numbers for different servers.
    // For Server Bob, with this Pseudonym, I might be on number 34.
    // For but Server Alice, I might be on number 59.
    //
    // So let's loop through all the numbers I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then send out the request
    // number.
    for (auto& it : m_mapRequestNum) {
        if (strID == it.first) {
            // Setup return value.
            lReqNum = (it.second);
            // The call has succeeded
            bRetVal = true;
            break;
        }
    }

    return bRetVal;
}

// Make SURE you call SavePseudonym after you call this.
// Otherwise it will increment in memory but not in the file.
// In fact, I cannot allow that. I will call SavePseudonym myself.
// Therefore, make SURE you fully LOAD this Pseudonym before you save it.
// You don't want to overwrite what's in that file.
// THEREFORE we need a better database than the filesystem.
// I will research a good, free, secure database (or encrypt everything
// before storing it there) and soon these "load/save" commands will use that
// instead of the filesystem.
void Nym::IncrementRequestNum(Nym& SIGNER_NYM, const String& strNotaryID)
{
    bool bSuccess = false;

    // The Pseudonym has a map of the request numbers for different servers.
    // For Server Bob, with this Pseudonym, I might be on number 34.
    // For but Server Alice, I might be on number 59.
    //
    // So let's loop through all the numbers I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then send out the request
    // number and
    // increment it so it will be ready for the next request.
    //
    // Make sure to save the Pseudonym so the new request number is saved.
    std::string strID = strNotaryID.Get();

    for (auto& it : m_mapRequestNum) {
        if (strID == it.first) {
            // We found it!
            // Presumably we ONLY found it because this Nym has been properly
            // loaded first.
            // Good job! Otherwise, the list would have been empty even though
            // the request number
            // was sitting in the file.

            // Grab a copy of the old request number
            int64_t lOldRequestNumber = m_mapRequestNum[it.first];

            // Set the new request number to the old one plus one.
            m_mapRequestNum[it.first] = lOldRequestNumber + 1;

            // Now we can log BOTH, before and after... // debug here
            otLog4 << "Incremented Request Number from " << lOldRequestNumber
                   << " to " << m_mapRequestNum[it.first] << ". Saving...\n";

            // The call has succeeded
            bSuccess = true;
            break;
        }
    }

    // If I didn't find it in the list above (whether the list is empty or
    // not....)
    // that means it does not exist. So create it.

    if (!bSuccess) {
        otOut << "Creating Request Number entry as '100'. Saving...\n";
        m_mapRequestNum[strNotaryID.Get()] = 100;
        bSuccess = true;
    }

    if (bSuccess) {
        SaveSignedNymfile(SIGNER_NYM);
    }
}

// if the server sends us a getRequestNumberResponse
void Nym::OnUpdateRequestNum(Nym& SIGNER_NYM, const String& strNotaryID,
                             int64_t lNewRequestNumber)
{
    bool bSuccess = false;

    // The Pseudonym has a map of the request numbers for different servers.
    // For Server Bob, with this Pseudonym, I might be on number 34.
    // For but Server Alice, I might be on number 59.
    //
    // So let's loop through all the numbers I have, and if the server ID on the
    // map
    // matches the Notary ID that was passed in, then send out the request
    // number and
    // increment it so it will be ready for the next request.
    //
    // Make sure to save the Pseudonym so the new request number is saved.
    std::string strID = strNotaryID.Get();

    for (auto& it : m_mapRequestNum) {
        if (strID == it.first) {
            // We found it!
            // Presumably we ONLY found it because this Nym has been properly
            // loaded first.
            // Good job! Otherwise, the list would have been empty even though
            // the request number
            // was sitting in the file.

            // The call has succeeded
            bSuccess = true;

            // Grab a copy of the old request number
            int64_t lOldRequestNumber = m_mapRequestNum[it.first];

            // Set the new request number to the old one plus one.
            m_mapRequestNum[it.first] = lNewRequestNumber;

            // Now we can log BOTH, before and after...
            otLog4 << "Updated Request Number from " << lOldRequestNumber
                   << " to " << m_mapRequestNum[it.first] << ". Saving...\n";
            break;
        }
    }

    // If I didn't find it in the list above (whether the list is empty or
    // not....)
    // that means it does not exist. So create it.

    if (!bSuccess) {
        otOut << "Creating Request Number entry as '" << lNewRequestNumber
              << "'. Saving...\n";
        m_mapRequestNum[strNotaryID.Get()] = lNewRequestNumber;
        bSuccess = true;
    }

    if (bSuccess) {
        SaveSignedNymfile(SIGNER_NYM);
    }
}

size_t Nym::GetMasterCredentialCount() const
{
    return m_mapCredentialSets.size();
}

size_t Nym::GetRevokedCredentialCount() const
{
    return m_mapRevokedSets.size();
}

/*
 How will VerifyPseudonym change now that we are adding credentials?

 Before it was easy: hash the public key, and compare the result to the NymID
 as it was already set by the wallet. If they match, then this really is the
 public
 key that I was expecting. (You could not change out that key without causing
 the ID
 to also change.)

 How are things with credentials?

 The public key may not directly hash to the Nym ID, but even if it did, which
 public key?
 Aren't there THREE public keys for each credential? That is, a signing,
 authentication, and
 encryption key. Which one is used as that Key ID? Perhaps all three are hashed
 together?
 And that collective three-key is only valid for a given Nym, if its credential
 contract is
 signed by a master key for that Nym. And that master key is only valid if its
 authority
 verifies properly. And the Nym's ID should be a hash somehow, of that
 authority. All of those
 things must verify in VerifyPseudonym, in the new system.

 Is that really true?

 Pseudo-code:

 Loop through the credentials for this Nym.
 For each, the authority/source string (which resolve to the NymID) should be
 identical.
 Resolve the NymID and compare it to the one that was expected.
 An extended version will also verify the authority itself, to see if it
 verifies the key.
 For example if this Nym is issued based on a Namecoin address, then the code
 would actually
 check Namecoin itself to verify the NymID is the same one posted there.

 */

bool Nym::VerifyPseudonym() const
{
    // If there are credentials, then we verify the Nym via his credentials.
    if (!m_mapCredentialSets.empty()) {
        // Verify Nym by his own credentials.
        for (const auto& it : m_mapCredentialSets) {
            const CredentialSet* pCredential = it.second;
            OT_ASSERT(nullptr != pCredential);

            const Identifier theCredentialNymID(pCredential->GetNymID());
            if (!CompareID(theCredentialNymID)) {
                String strNymID;
                GetIdentifier(strNymID);
                otOut << __FUNCTION__ << ": Credential NymID ("
                      << pCredential->GetNymID()
                      << ") doesn't match actual NymID: " << strNymID << "\n";
                return false;
            }

            if (!pCredential->VerifyInternally()) {
                otOut << __FUNCTION__ << ": Credential ("
                      << pCredential->GetMasterCredID()
                      << ") failed its own internal verification.\n";
                return false;
            }

            // Warning: time-intensive. Todo optimize: load a contract here
            // which verifies authorization,
            // based on a signature from a separate process which did an
            // identity lookup externally.
            // Once that authorization times out, then the identity verification
            // server can just sign
            // another one.
            //
            if (!pCredential->VerifyAgainstSource()) // todo optimize,
                                                     // warning:
                                                     // time-intensive.
            {
                otOut
                    << __FUNCTION__
                    << ": Credential failed against its source. Credential ID: "
                    << pCredential->GetMasterCredID()
                    << "\n"
                       "NymID: " << pCredential->GetNymID()
                    << "\nSource: " << pCredential->GetSourceForNymID() << "\n";
                return false;
            }
        }
        return true;
    }
    otErr << "No credentials.\n";
    return false;
}

bool Nym::CompareID(const Nym& RHS) const
{
    return RHS.CompareID(m_nymID);
}

bool Nym::SavePseudonymWallet(Tag& parent) const
{
    String nymID;
    GetIdentifier(nymID);

    // Name is in the clear in memory,
    // and base64 in storage.
    OTASCIIArmor ascName;
    if (m_strName.Exists()) {
        ascName.SetString(m_strName, false); // linebreaks == false
    }

    TagPtr pTag(new Tag("pseudonym"));

    pTag->add_attribute("name", m_strName.Exists() ? ascName.Get() : "");
    pTag->add_attribute("nymID", nymID.Get());

    parent.add_tag(pTag);

    return true;
}

// pstrID is an output parameter.
bool Nym::Server_PubKeyExists(String* pstrID) // Only used
                                              // on server
                                              // side.
{

    String strID;
    if (nullptr == pstrID) {
        pstrID = &strID;
    }
    GetIdentifier(*pstrID);

    // Below this point, pstrID is a GOOD pointer, no matter what. (And no need
    // to delete it.)

    return OTDB::Exists(OTFolders::Pubkey().Get(), pstrID->Get());
}

// This version is run on the server side, and assumes only a Public Key.
// This code reads up the file, discards the bookends, and saves only the
// gibberish itself.
bool Nym::LoadPublicKey()
{
    if (LoadCredentials() && (GetMasterCredentialCount() > 0)) {
        return true;
    }
    otInfo << __FUNCTION__ << ": Failure.\n";
    return false;
}

// DISPLAY STATISTICS

void Nym::DisplayStatistics(String& strOutput)
{
    for (auto& it : m_mapRequestNum) {
        std::string strNotaryID = it.first;
        int64_t lRequestNumber = it.second;

        // Now we can log BOTH, before and after...
        strOutput.Concatenate("Req# is %" PRId64 " for server ID: %s\n",
                              lRequestNumber, strNotaryID.c_str());
    }

    for (auto& it : m_mapHighTransNo) {
        std::string strNotaryID = it.first;
        const int64_t lHighestNum = it.second;

        strOutput.Concatenate("Highest trans# was %" PRId64 " for server: %s\n",
                              lHighestNum, strNotaryID.c_str());
    }

    for (auto& it : m_mapIssuedNum) {
        std::string strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;

        OT_ASSERT(nullptr != pDeque);

        if (!(pDeque->empty())) {
            strOutput.Concatenate(
                "---- Transaction numbers still signed out from server: %s\n",
                strNotaryID.c_str());

            for (uint32_t i = 0; i < pDeque->size(); i++) {
                int64_t lTransactionNumber = pDeque->at(i);

                strOutput.Concatenate(0 == i ? "%" PRId64 : ", %" PRId64,
                                      lTransactionNumber);
            }
            strOutput.Concatenate("\n");
        }
    } // for

    for (auto& it : m_mapTransNum) {
        std::string strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;

        OT_ASSERT(nullptr != pDeque);

        if (!(pDeque->empty())) {
            strOutput.Concatenate(
                "---- Transaction numbers still usable on server: %s\n",
                strNotaryID.c_str());

            for (uint32_t i = 0; i < pDeque->size(); i++) {
                int64_t lTransactionNumber = pDeque->at(i);
                strOutput.Concatenate(0 == i ? "%" PRId64 : ", %" PRId64,
                                      lTransactionNumber);
            }
            strOutput.Concatenate("\n");
        }
    } // for

    for (auto& it : m_mapAcknowledgedNum) {
        std::string strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;

        OT_ASSERT(nullptr != pDeque);

        if (!(pDeque->empty())) {
            strOutput.Concatenate("---- Request numbers for which Nym has "
                                  "already received a reply from server: %s\n",
                                  strNotaryID.c_str());

            for (uint32_t i = 0; i < pDeque->size(); i++) {
                int64_t lRequestNumber = pDeque->at(i);
                strOutput.Concatenate(0 == i ? "%" PRId64 : ", %" PRId64,
                                      lRequestNumber);
            }
            strOutput.Concatenate("\n");
        }
    } // for

    strOutput.Concatenate("Source for ID:\n%s\n", m_strSourceForNymID.Get());
    strOutput.Concatenate("Alt. location: %s\n\n", m_strAltLocation.Get());

    const size_t nMasterCredCount = GetMasterCredentialCount();
    if (nMasterCredCount > 0) {
        for (int32_t iii = 0; iii < static_cast<int64_t>(nMasterCredCount);
             ++iii) {
            const CredentialSet* pCredential = GetMasterCredentialByIndex(iii);
            if (nullptr != pCredential) {
                strOutput.Concatenate("Credential ID: %s \n",
                                      pCredential->GetMasterCredID().Get());
                const size_t nChildCredentialCount =
                    pCredential->GetChildCredentialCount();

                if (nChildCredentialCount > 0) {
                    for (size_t vvv = 0; vvv < nChildCredentialCount; ++vvv) {
                        const std::string str_childcred_id(
                            pCredential->GetChildCredentialIDByIndex(vvv));

                        strOutput.Concatenate("   child credential: %s  \n",
                                              str_childcred_id.c_str());
                    }
                }
            }
        }
        strOutput.Concatenate("%s", "\n");
    }

    strOutput.Concatenate("==>      Name: %s   %s\n", m_strName.Get(),
                          m_bMarkForDeletion ? "(MARKED FOR DELETION)" : "");
    strOutput.Concatenate("      Version: %s\n", m_strVersion.Get());

    // This is used on server-side only. (Client side sees this value
    // by querying the server.)
    // Therefore since m_lUsageCredits is unused on client side, why display
    // it in the client API? Makes no sense.
    // strOutput.Concatenate("Usage Credits: %" PRId64 "\n", m_lUsageCredits);

    strOutput.Concatenate("       Mail count: %" PRI_SIZE "\n",
                          m_dequeMail.size());
    strOutput.Concatenate("    Outmail count: %" PRI_SIZE "\n",
                          m_dequeOutmail.size());
    strOutput.Concatenate("Outpayments count: %" PRI_SIZE "\n",
                          m_dequeOutpayments.size());

    String theStringID;
    GetIdentifier(theStringID);
    strOutput.Concatenate("Nym ID: %s\n", theStringID.Get());
}

bool Nym::SavePseudonym()
{
    if (!m_strNymfile.GetLength()) {
        String nymID;
        GetIdentifier(nymID);
        m_strNymfile.Format("%s", nymID.Get());
    }

    otInfo << "Saving nym to: " << OTFolders::Nym() << Log::PathSeparator()
           << m_strNymfile << "\n";

    return SavePseudonym(OTFolders::Nym().Get(), m_strNymfile.Get());
}

bool Nym::SavePseudonym(const char* szFoldername, const char* szFilename)
{
    OT_ASSERT(nullptr != szFoldername);
    OT_ASSERT(nullptr != szFilename);

    String strNym;
    SavePseudonym(strNym);

    bool bSaved =
        OTDB::StorePlainString(strNym.Get(), szFoldername, szFilename);
    if (!bSaved)
        otErr << __FUNCTION__ << ": Error saving file: " << szFoldername
              << Log::PathSeparator() << szFilename << "\n";

    return bSaved;
}

// Used when importing/exporting Nym into and out-of the sphere of the
// cached key in the wallet.
bool Nym::ReEncryptPrivateCredentials(bool bImporting, // bImporting=true, or
                                                       // false if exporting.
                                      const OTPasswordData* pPWData,
                                      const OTPassword* pImportPassword)
{
    const OTPassword* pExportPassphrase = nullptr;
    std::unique_ptr<const OTPassword> thePasswordAngel;

    if (nullptr == pImportPassword) {

        // whether import/export, this display string is for the OUTSIDE OF
        // WALLET
        // portion of that process.
        //
        String strDisplay(
            nullptr != pPWData
                ? pPWData->GetDisplayString()
                : (bImporting ? "Enter passphrase for the Nym being imported."
                              : "Enter passphrase for exported Nym."));
        // Circumvents the cached key.
        pExportPassphrase = OTSymmetricKey::GetPassphraseFromUser(
            &strDisplay, !bImporting); // bAskTwice is true when exporting
                                       // (since the export passphrase is being
                                       // created at that time.)
        thePasswordAngel.reset(pExportPassphrase);

        if (nullptr == pExportPassphrase) {
            otErr << __FUNCTION__ << ": Failed in GetPassphraseFromUser.\n";
            return false;
        }
    }
    else {
        pExportPassphrase = pImportPassword;
    }

    for (auto& it : m_mapCredentialSets) {
        CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        if (false ==
            pCredential->ReEncryptPrivateCredentials(*pExportPassphrase,
                                                     bImporting))
            return false;
    }

    return true;
}

// If the Nym's source is a URL, he needs to post his valid
// master credential IDs there, so they can be verified against
// their source. This method is what creates the file which you
// can post at that URL. (Containing only the valid IDs, not the
// revoked ones.)
// Optionally it also returns the contents of the public credential
// files, mapped by their credential IDs.
//
void Nym::GetPublicCredentials(String& strCredList,
                               String::Map* pmapCredFiles) const
{
    Tag tag("nymData");

    tag.add_attribute("version", m_strVersion.Get());

    String strNymID;
    GetIdentifier(strNymID);

    tag.add_attribute("nymID", strNymID.Get());

    SerializeNymIDSource(tag);

    for (auto& it : m_mapCredentialSets) {
        CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        pCredential->SerializeIDs(tag, m_listRevokedIDs,
                                  pmapCredFiles); // bShowRevoked=false by
                                                  // default, bValid=true by
                                                  // default. (True since we're
                                                  // looping m_mapCredentialSets
                                                  // only, and not
                                                  // m_mapRevokedSets.)
    }

    std::string str_result;
    tag.output(str_result);

    strCredList.Concatenate("%s", str_result.c_str());
}

void Nym::GetPrivateCredentials(String& strCredList, String::Map* pmapCredFiles)
{
    Tag tag("nymData");

    tag.add_attribute("version", m_strVersion.Get());

    String strNymID;
    GetIdentifier(strNymID);

    tag.add_attribute("nymID", strNymID.Get());

    SerializeNymIDSource(tag);

    SaveCredentialsToTag(tag, nullptr, pmapCredFiles);

    std::string str_result;
    tag.output(str_result);

    strCredList.Concatenate("%s", str_result.c_str());
}

void Nym::SerializeNymIDSource(Tag& parent) const
{
    // We encode these before storing.
    if (m_strSourceForNymID.Exists()) {
        const OTASCIIArmor ascSourceForNymID(m_strSourceForNymID);

        TagPtr pTag(new Tag("nymIDSource", ascSourceForNymID.Get()));

        if (m_strAltLocation.Exists()) {
            OTASCIIArmor ascAltLocation;
            ascAltLocation.SetString(m_strAltLocation,
                                     false); // bLineBreaks=true by default.

            pTag->add_attribute("altLocation", ascAltLocation.Get());
        }
        parent.add_tag(pTag);
    }
}

void Nym::SaveCredentialIDsToString(String& strOutput)
{
    Tag tag("nymData");

    tag.add_attribute("version", m_strVersion.Get());

    String strNymID;
    GetIdentifier(strNymID);

    tag.add_attribute("nymID", strNymID.Get());

    SerializeNymIDSource(tag);

    SaveCredentialsToTag(tag);

    std::string str_result;
    tag.output(str_result);

    strOutput.Concatenate("%s", str_result.c_str());
}

bool Nym::SaveCredentialIDs()
{
    String strNymID, strOutput;
    GetIdentifier(strNymID);

    SaveCredentialIDsToString(strOutput);

    if (strOutput.Exists()) {
        OTASCIIArmor ascOutput(strOutput);
        strOutput.Release();
        if (ascOutput.WriteArmoredString(
                strOutput, "CREDENTIAL LIST") && // bEscaped=false by default.
            strOutput.Exists()) {

            // Save it to local storage.
            String strFilename;
            strFilename.Format("%s.cred", strNymID.Get());

            std::string str_Folder = HasPrivateKey()
                                         ? OTFolders::Credential().Get()
                                         : OTFolders::Pubcred().Get();

            if (!OTDB::StorePlainString(strOutput.Get(), str_Folder,
                                        strNymID.Get(), strFilename.Get())) {
                otErr << __FUNCTION__ << ": Failure trying to store "
                      << (HasPrivateKey() ? "private" : "public")
                      << " credential list for Nym: " << strNymID << "\n";
                return false;
            }

            return true;
        }
    }
    return false;
}

// Use this to load the keys for a Nym (whether public or private), and then
// call VerifyPseudonym, and then load the actual Nymfile using
// LoadSignedNymfile.
//
bool Nym::LoadCredentials(bool bLoadPrivate, // Loads public credentials
                                             // by default. For
                                             // private, pass true.
                          const OTPasswordData* pPWData,
                          const OTPassword* pImportPassword)
{
    String strReason(nullptr == pPWData ? OT_PW_DISPLAY
                                        : pPWData->GetDisplayString());

    ClearCredentials();

    String strNymID;
    GetIdentifier(strNymID);

    String strFilename;
    strFilename.Format("%s.cred", strNymID.Get());

    otErr << "Private credential folder: " << OTFolders::Credential().Get() <<"\n";
    otErr << "Public credential folder: " << OTFolders::Pubcred().Get() <<"\n";
    otErr << "NymID: " << strNymID.Get() <<"\n";
    otErr << "Filename: " << strFilename.Get() <<"\n";
    
    const char* szFoldername = bLoadPrivate ? OTFolders::Credential().Get()
                                            : OTFolders::Pubcred().Get();
    const char* szFilename = strFilename.Get();

    otErr << "Folder: " << szFoldername <<"\n";
    otErr << "File: " << szFilename <<"\n";

    if (OTDB::Exists(szFoldername, strNymID.Get(), szFilename)) {
        String strFileContents(
            OTDB::QueryPlainString(szFoldername, strNymID.Get(), szFilename));

        // The credential list file is like the nymfile except with ONLY
        // credential IDs inside.
        // Therefore, we LOAD it like we're loading a Nymfile from string.
        // There's no need for
        // the list itself to be signed, since we verify it fully before using
        // it to verify the
        // signature on the actual nymfile.
        // How is the list safe not to be signed? Because the Nym ID's source
        // string must hash
        // to form the NymID, and any credential IDs on the list must be found
        // on a lookup to
        // that source. An attacker cannot add a credential without putting it
        // inside the source,
        // which the user controls, and the attacker cannot change the source
        // without changing
        // the NymID. Therefore the credential list file itself doesn't need to
        // be signed, for
        // the same reason that the public key didn't need to be signed: because
        // you can prove
        // its validity by hashing (the source string that validates the
        // credentials in that list,
        // or by hashing/ the public key for that Nym, if doing things the old
        // way.)
        //
        if (strFileContents.Exists() && strFileContents.DecodeIfArmored()) {
            const bool bLoaded = LoadFromString(
                strFileContents,
                nullptr, // map of credentials--if nullptr, it loads
                         // them from local storage.
                &strReason, pImportPassword); // optional to provide a
                                              // passphrase (otherwise one is
                                              // prompted for.)
            return bLoaded;
        }
        else {
            otErr << __FUNCTION__
                  << ": Failed trying to load credential list from file: "
                  << szFoldername << Log::PathSeparator() << szFilename << "\n";
        }
    }

    return false; // No log on failure, since often this may be used to SEE if
                  // credentials exist.
                  // (No need for error message every time they don't exist.)
}

void Nym::SaveCredentialsToTag(Tag& parent, String::Map* pmapPubInfo,
                               String::Map* pmapPriInfo)
{
    // IDs for revoked child credentials are saved here.
    for (auto& it : m_listRevokedIDs) {
        std::string str_revoked_id = it;
        TagPtr pTag(new Tag("revokedCredential"));
        pTag->add_attribute("ID", str_revoked_id);
        parent.add_tag(pTag);
    }

    // Serialize master and sub-credentials here.
    for (auto& it : m_mapCredentialSets) {
        CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        pCredential->SerializeIDs(
            parent, m_listRevokedIDs, pmapPubInfo, pmapPriInfo,
            true); // bShowRevoked=false by default (true here), bValid=true
    }

    // Serialize Revoked master credentials here, including their child key credentials.
    for (auto& it : m_mapRevokedSets) {
        CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        pCredential->SerializeIDs(
            parent, m_listRevokedIDs, pmapPubInfo, pmapPriInfo, true,
            false); // bShowRevoked=false by default. (Here it's true.)
                    // bValid=true by default. Here is for revoked, so false.
    }
}

// Save the Pseudonym to a string...
bool Nym::SavePseudonym(String& strNym)
{
    Tag tag("nymData");

    String nymID;
    GetIdentifier(nymID);

    tag.add_attribute("version", m_strVersion.Get());
    tag.add_attribute("nymID", nymID.Get());

    if (m_lUsageCredits != 0)
        tag.add_attribute("usageCredits", formatLong(m_lUsageCredits));

    SerializeNymIDSource(tag);

    // For now I'm saving the credential list to a separate file.
    // (And then of course, each credential also gets its own file.)
    // We load the credential list file, and any associated credentials,
    // before loading the Nymfile proper.
    // Then we use the keys from those credentials possibly to verify
    // the signature on the Nymfile (or not, in the case of the server
    // which uses its own key.)
    //
    //  SaveCredentialsToTag(tag);

    for (auto& it : m_mapRequestNum) {
        std::string strNotaryID = it.first;
        int64_t lRequestNum = it.second;

        TagPtr pTag(new Tag("requestNum"));

        pTag->add_attribute("notaryID", strNotaryID);
        pTag->add_attribute("currentRequestNum", formatLong(lRequestNum));

        tag.add_tag(pTag);
    }

    for (auto& it : m_mapHighTransNo) {
        std::string strNotaryID = it.first;
        int64_t lHighestNum = it.second;

        TagPtr pTag(new Tag("highestTransNum"));

        pTag->add_attribute("notaryID", strNotaryID);
        pTag->add_attribute("mostRecent", formatLong(lHighestNum));

        tag.add_tag(pTag);
    }

    // When you delete a Nym, it just marks it.
    // Actual deletion occurs during maintenance sweep
    // (targeting marked nyms...)
    //
    if (m_bMarkForDeletion) {
        tag.add_tag("MARKED_FOR_DELETION", "THIS NYM HAS BEEN MARKED "
                                           "FOR DELETION AT ITS OWN REQUEST");
    }

    int64_t lTransactionNumber = 0;

    for (auto& it : m_mapTransNum) {
        std::string strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;

        OT_ASSERT(nullptr != pDeque);

        if (!(pDeque->empty()) && (strNotaryID.size() > 0)) {
            NumList theList;

            for (uint32_t i = 0; i < pDeque->size(); i++) {
                lTransactionNumber = pDeque->at(i);
                theList.Add(lTransactionNumber);
            }
            String strTemp;
            if ((theList.Count() > 0) && theList.Output(strTemp) &&
                strTemp.Exists()) {
                const OTASCIIArmor ascTemp(strTemp);

                if (ascTemp.Exists()) {
                    TagPtr pTag(new Tag("transactionNums", ascTemp.Get()));
                    pTag->add_attribute("notaryID", strNotaryID);
                    tag.add_tag(pTag);
                }
            }
        }
    } // for

    lTransactionNumber = 0;

    for (auto& it : m_mapIssuedNum) {
        std::string strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;

        OT_ASSERT(nullptr != pDeque);

        if (!(pDeque->empty()) && (strNotaryID.size() > 0)) {
            NumList theList;

            for (uint32_t i = 0; i < pDeque->size(); i++) {
                lTransactionNumber = pDeque->at(i);
                theList.Add(lTransactionNumber);
            }
            String strTemp;
            if ((theList.Count() > 0) && theList.Output(strTemp) &&
                strTemp.Exists()) {
                const OTASCIIArmor ascTemp(strTemp);

                if (ascTemp.Exists()) {
                    TagPtr pTag(new Tag("issuedNums", ascTemp.Get()));
                    pTag->add_attribute("notaryID", strNotaryID);
                    tag.add_tag(pTag);
                }
            }
        }
    } // for

    lTransactionNumber = 0;

    for (auto& it : m_mapTentativeNum) {
        std::string strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;

        OT_ASSERT(nullptr != pDeque);

        if (!(pDeque->empty()) && (strNotaryID.size() > 0)) {
            NumList theList;

            for (uint32_t i = 0; i < pDeque->size(); i++) {
                lTransactionNumber = pDeque->at(i);
                theList.Add(lTransactionNumber);
            }
            String strTemp;
            if ((theList.Count() > 0) && theList.Output(strTemp) &&
                strTemp.Exists()) {
                const OTASCIIArmor ascTemp(strTemp);

                if (ascTemp.Exists()) {
                    TagPtr pTag(new Tag("tentativeNums", ascTemp.Get()));
                    pTag->add_attribute("notaryID", strNotaryID);
                    tag.add_tag(pTag);
                }
            }
        }

    } // for

    // although mapOfTransNums is used, in this case,
    // request numbers are what is actually being stored.
    // The data structure just happened to be appropriate
    // in this case, with generic manipulation functions
    // already written, so I used that pre-existing system.
    //
    for (auto& it : m_mapAcknowledgedNum) {
        std::string strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;

        OT_ASSERT(nullptr != pDeque);

        if (!(pDeque->empty()) && (strNotaryID.size() > 0)) {
            NumList theList;

            for (uint32_t i = 0; i < pDeque->size(); i++) {
                const int64_t lRequestNumber = pDeque->at(i);
                theList.Add(lRequestNumber);
            }
            String strTemp;
            if ((theList.Count() > 0) && theList.Output(strTemp) &&
                strTemp.Exists()) {
                const OTASCIIArmor ascTemp(strTemp);

                if (ascTemp.Exists()) {
                    TagPtr pTag(new Tag("ackNums", ascTemp.Get()));
                    pTag->add_attribute("notaryID", strNotaryID);
                    tag.add_tag(pTag);
                }
            }
        }

    } // for

    if (!(m_dequeMail.empty())) {
        for (uint32_t i = 0; i < m_dequeMail.size(); i++) {
            Message* pMessage = m_dequeMail.at(i);
            OT_ASSERT(nullptr != pMessage);

            String strMail(*pMessage);

            OTASCIIArmor ascMail;

            if (strMail.Exists()) ascMail.SetString(strMail);

            if (ascMail.Exists()) {
                tag.add_tag("mailMessage", ascMail.Get());
            }
        }
    }

    if (!(m_dequeOutmail.empty())) {
        for (uint32_t i = 0; i < m_dequeOutmail.size(); i++) {
            Message* pMessage = m_dequeOutmail.at(i);
            OT_ASSERT(nullptr != pMessage);

            String strOutmail(*pMessage);

            OTASCIIArmor ascOutmail;

            if (strOutmail.Exists()) ascOutmail.SetString(strOutmail);

            if (ascOutmail.Exists()) {
                tag.add_tag("outmailMessage", ascOutmail.Get());
            }
        }
    }

    if (!(m_dequeOutpayments.empty())) {
        for (uint32_t i = 0; i < m_dequeOutpayments.size(); i++) {
            Message* pMessage = m_dequeOutpayments.at(i);
            OT_ASSERT(nullptr != pMessage);

            String strOutpayments(*pMessage);

            OTASCIIArmor ascOutpayments;

            if (strOutpayments.Exists())
                ascOutpayments.SetString(strOutpayments);

            if (ascOutpayments.Exists()) {
                tag.add_tag("outpaymentsMessage", ascOutpayments.Get());
            }
        }
    }

    // These are used on the server side.
    // (That's why you don't see the server ID saved here.)
    //
    if (!(m_setOpenCronItems.empty())) {
        for (auto& it : m_setOpenCronItems) {
            int64_t lID = it;
            TagPtr pTag(new Tag("hasOpenCronItem"));
            pTag->add_attribute("ID", formatLong(lID));
            tag.add_tag(pTag);
        }
    }

    // These are used on the server side.
    // (That's why you don't see the server ID saved here.)
    //
    if (!(m_setAccounts.empty())) {
        for (auto& it : m_setAccounts) {
            std::string strID(it);
            TagPtr pTag(new Tag("ownsAssetAcct"));
            pTag->add_attribute("ID", strID);
            tag.add_tag(pTag);
        }
    }

    // client-side
    for (auto& it : m_mapNymboxHash) {
        std::string strNotaryID = it.first;
        Identifier& theID = it.second;

        if ((strNotaryID.size() > 0) && !theID.IsEmpty()) {
            const String strNymboxHash(theID);
            TagPtr pTag(new Tag("nymboxHashItem"));
            pTag->add_attribute("notaryID", strNotaryID);
            pTag->add_attribute("nymboxHash", strNymboxHash.Get());
            tag.add_tag(pTag);
        }
    } // for

    // client-side
    for (auto& it : m_mapRecentHash) {
        std::string strNotaryID = it.first;
        Identifier& theID = it.second;

        if ((strNotaryID.size() > 0) && !theID.IsEmpty()) {
            const String strRecentHash(theID);
            TagPtr pTag(new Tag("recentHashItem"));
            pTag->add_attribute("notaryID", strNotaryID);
            pTag->add_attribute("recentHash", strRecentHash.Get());
            tag.add_tag(pTag);
        }
    } // for

    // server-side
    if (!m_NymboxHash.IsEmpty()) {
        const String strNymboxHash(m_NymboxHash);
        TagPtr pTag(new Tag("nymboxHash"));
        pTag->add_attribute("value", strNymboxHash.Get());
        tag.add_tag(pTag);
    }

    // client-side
    for (auto& it : m_mapInboxHash) {
        std::string strAcctID = it.first;
        Identifier& theID = it.second;

        if ((strAcctID.size() > 0) && !theID.IsEmpty()) {
            const String strHash(theID);
            TagPtr pTag(new Tag("inboxHashItem"));
            pTag->add_attribute("accountID", strAcctID);
            pTag->add_attribute("hashValue", strHash.Get());
            tag.add_tag(pTag);
        }
    } // for

    // client-side
    for (auto& it : m_mapOutboxHash) {
        std::string strAcctID = it.first;
        Identifier& theID = it.second;

        if ((strAcctID.size() > 0) && !theID.IsEmpty()) {
            const String strHash(theID);
            TagPtr pTag(new Tag("outboxHashItem"));
            pTag->add_attribute("accountID", strAcctID);
            pTag->add_attribute("hashValue", strHash.Get());
            tag.add_tag(pTag);
        }
    } // for

    std::string str_result;
    tag.output(str_result);

    strNym.Concatenate("%s", str_result.c_str());

    return true;
}

CredentialSet* Nym::GetMasterCredential(const String& strID)
{
    auto iter = m_mapCredentialSets.find(strID.Get());
    CredentialSet* pCredential = nullptr;

    if (m_mapCredentialSets.end() != iter) // found it
        pCredential = iter->second;

    return pCredential;
}

CredentialSet* Nym::GetRevokedCredential(const String& strID)
{
    auto iter = m_mapRevokedSets.find(strID.Get());
    CredentialSet* pCredential = nullptr;

    if (m_mapCredentialSets.end() != iter) // found it
        pCredential = iter->second;

    return pCredential;
}

const CredentialSet* Nym::GetMasterCredentialByIndex(int32_t nIndex) const
{
    if ((nIndex < 0) ||
        (nIndex >= static_cast<int64_t>(m_mapCredentialSets.size()))) {
        otErr << __FUNCTION__ << ": Index out of bounds: " << nIndex << "\n";
    }
    else {
        int32_t nLoopIndex = -1;

        for (const auto& it : m_mapCredentialSets) {
            const CredentialSet* pCredential = it.second;
            OT_ASSERT(nullptr != pCredential);

            ++nLoopIndex; // 0 on first iteration.

            if (nLoopIndex == nIndex) return pCredential;
        }
    }
    return nullptr;
}

const CredentialSet* Nym::GetRevokedCredentialByIndex(int32_t nIndex) const
{
    if ((nIndex < 0) || (nIndex >= static_cast<int64_t>(m_mapRevokedSets.size()))) {
        otErr << __FUNCTION__ << ": Index out of bounds: " << nIndex << "\n";
    }
    else {
        int32_t nLoopIndex = -1;

        for (const auto& it : m_mapRevokedSets) {
            const CredentialSet* pCredential = it.second;
            OT_ASSERT(nullptr != pCredential);

            ++nLoopIndex; // 0 on first iteration.

            if (nLoopIndex == nIndex) return pCredential;
        }
    }
    return nullptr;
}

const Credential* Nym::GetChildCredential(const String& strMasterID,
                                             const String& strChildCredID) const
{
    auto iter = m_mapCredentialSets.find(strMasterID.Get());
    const CredentialSet* pMaster = nullptr;

    if (iter != m_mapCredentialSets.end()) // found it
        pMaster = iter->second;

    if (nullptr != pMaster) {
        const Credential* pSub =
            pMaster->GetChildCredential(strChildCredID, &m_listRevokedIDs);

        if (nullptr != pSub) return pSub;
    }

    return nullptr;
}

// std::set<int64_t> m_setOpenCronItems; // Until these Cron Items are closed
// out, the server-side Nym keeps a list of them handy.

// std::set<std::string> m_setAccounts; // A list of asset account IDs. Server
// side only (client side uses wallet; has multiple servers.)

/*

 Enumeration for all xml nodes which are parsed by IrrXMLReader.

 Enumeration values:

 EXN_NONE            No xml node. This is usually the node if you did not read
 anything yet.
 EXN_ELEMENT        A xml element, like <foo>.
 EXN_ELEMENT_END    End of an xml element, like </foo>.
 EXN_TEXT            Text within a xml element: <foo> this is the text. </foo>.
 EXN_COMMENT        An xml comment like <!-- I am a comment --> or a DTD
 definition.
 EXN_CDATA            An xml cdata section like <![CDATA[ this is some CDATA
 ]]>.
 EXN_UNKNOWN        Unknown element.

 Definition at line 180 of file irrXML.h.

 */
// todo optimize
bool Nym::LoadFromString(const String& strNym,
                         String::Map* pMapCredentials, // pMapCredentials can be
                                                       // passed,
                         // if you prefer to use a specific
                         // set, instead of just loading the
                         // actual set from storage (such as
                         // during registration, when the
                         // credentials have been sent
                         // inside a message.)
                         String* pstrReason, const OTPassword* pImportPassword)
{
    bool bSuccess = false;

    ClearAll(); // Since we are loading everything up... (credentials are NOT
                // cleared here. See note in OTPseudonym::ClearAll.)

    OTStringXML strNymXML(strNym); // todo optimize
    irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader(strNymXML);
    OT_ASSERT(nullptr != xml);
    std::unique_ptr<irr::io::IrrXMLReader> theCleanup(xml);

    // parse the file until end reached
    while (xml && xml->read()) {

        //        switch(xml->getNodeType())
        //        {
        //            case(EXN_NONE):
        //                otErr << "ACK NUMS: EXN_NONE --  No xml node. This is
        // usually the node if you did not read anything yet.\n";
        //                break;
        //            case(EXN_ELEMENT):
        //                otErr << "ACK NUMS: EXN_ELEMENT -- An xml element such
        // as <foo>.\n";
        //                break;
        //            case(EXN_ELEMENT_END):
        //                otErr << "ACK NUMS: EXN_ELEMENT_END -- End of an xml
        // element such as </foo>.\n";
        //                break;
        //            case(EXN_TEXT):
        //                otErr << "ACK NUMS: EXN_TEXT -- Text within an xml
        // element: <foo> this is the text. <foo>.\n";
        //                break;
        //            case(EXN_COMMENT):
        //                otErr << "ACK NUMS: EXN_COMMENT -- An xml comment like
        // <!-- I am a comment --> or a DTD definition.\n";
        //                break;
        //            case(EXN_CDATA):
        //                otErr << "ACK NUMS: EXN_CDATA -- An xml cdata section
        // like <![CDATA[ this is some CDATA ]]>.\n";
        //                break;
        //            case(EXN_UNKNOWN):
        //                otErr << "ACK NUMS: EXN_UNKNOWN -- Unknown
        // element.\n";
        //                break;
        //            default:
        //                otErr << "ACK NUMS: default!! -- SHOULD NEVER
        // HAPPEN...\n";
        //                break;
        //        }
        //        otErr << "OTPseudonym::LoadFromString: NODE DATA: %s\n",
        // xml->getNodeData());

        // strings for storing the data that we want to read out of the file
        //
        switch (xml->getNodeType()) {
        case irr::io::EXN_NONE:
        case irr::io::EXN_TEXT:
        case irr::io::EXN_COMMENT:
        case irr::io::EXN_ELEMENT_END:
        case irr::io::EXN_CDATA:
            // in this xml file, the only text which occurs is the messageText
            // messageText = xml->getNodeData();

            //            switch(xml->getNodeType())
            //            {
            //                case(EXN_NONE):
            //                    otErr << "SKIPPING: EXN_NONE --  No xml node.
            // This is usually the node if you did not read anything yet.\n";
            //                    break;
            //                case(EXN_TEXT):
            //                    otErr << "SKIPPING: EXN_TEXT -- Text within an
            // xml element: <foo> this is the text. <foo>.\n";
            //                    break;
            //                case(EXN_COMMENT):
            //                    otErr << "SKIPPING: EXN_COMMENT -- An xml
            // comment like <!-- I am a comment --> or a DTD definition.\n";
            //                    break;
            //                case(EXN_ELEMENT_END):
            //                    otErr << "SKIPPING: EXN_ELEMENT_END -- End of
            // an xml element such as </foo>.\n";
            //                    break;
            //                case(EXN_CDATA):
            //                    otErr << "SKIPPING: EXN_CDATA -- An xml cdata
            // section like <![CDATA[ this is some CDATA ]]>.\n";
            //                    break;
            //                default:
            //                    otErr << "SKIPPING: default!! -- SHOULD NEVER
            // HAPPEN...\n";
            //                    break;
            //            }

            break;
        case irr::io::EXN_ELEMENT: {
            const String strNodeName = xml->getNodeName();
            //              otErr << "PROCESSING EXN_ELEMENT: NODE NAME: %s\n",
            // strNodeName.Get());

            if (strNodeName.Compare("nymData")) {
                m_strVersion = xml->getAttributeValue("version");
                const String UserNymID = xml->getAttributeValue("nymID");

                // Server-side only...
                String strCredits = xml->getAttributeValue("usageCredits");

                if (strCredits.GetLength() > 0)
                    m_lUsageCredits = strCredits.ToLong();
                else
                    m_lUsageCredits =
                        0; // This is the default anyway, but just being safe...

                // TODO: no need to set the ID again here. We already know the
                // ID
                // at this point. Better to check and compare they are the same
                // here.
                // m_nymID.SetString(UserNymID);

                if (UserNymID.GetLength())
                    otLog3 << "\nLoading user, version: " << m_strVersion
                           << " NymID:\n" << UserNymID << "\n";
                bSuccess = true;
            }
            else if (strNodeName.Compare("nymIDSource")) {
                //                  otLog3 << "Loading nymIDSource...\n");
                OTASCIIArmor ascAltLocation =
                    xml->getAttributeValue("altLocation"); // optional.
                if (ascAltLocation.Exists())
                    ascAltLocation.GetString(
                        m_strAltLocation,
                        false); // bLineBreaks=true by default.

                if (!Contract::LoadEncodedTextField(xml, m_strSourceForNymID)) {
                    otErr << "Error in " << __FILE__ << " line " << __LINE__
                          << ": failed loading expected nymIDSource field.\n";
                    return false; // error condition
                }
            }
            else if (strNodeName.Compare("revokedCredential")) {
                const String strRevokedID = xml->getAttributeValue("ID");
                otLog3 << "revokedCredential ID: " << strRevokedID << "\n";
                auto iter =
                    std::find(m_listRevokedIDs.begin(), m_listRevokedIDs.end(),
                              strRevokedID.Get());
                if (iter == m_listRevokedIDs.end()) // It's not already there,
                                                    // so it's safe to add it.
                    m_listRevokedIDs.push_back(
                        strRevokedID.Get()); // todo optimize.
            }
            else if (strNodeName.Compare("masterCredential")) {
                const String strID    = xml->getAttributeValue("ID");
                const String strValid = xml->getAttributeValue("valid");
                const String strType  = xml->getAttributeValue("type");
                const bool bValid = strValid.Compare("true");
                otLog3 << "Loading " << (bValid ? "valid" : "invalid")
                       << " masterCredential ID: " << strID << "\n";
                String strNymID;
                GetIdentifier(strNymID);
                CredentialSet* pCredential = nullptr;

                if (nullptr == pMapCredentials) // pMapCredentials is an option
                                                // that allows you to read
                    // credentials from the map instead
                    // of from local storage. (While
                    // loading the Nym...) In this
                    // case, the option isn't being
                    // employed...
                    pCredential = CredentialSet::LoadMaster(strNymID, strID,
                                                           Credential::StringToCredentialType(strType));
                else // In this case, it potentially is on the map...
                {
                    auto it_cred = pMapCredentials->find(strID.Get());

                    if (it_cred ==
                        pMapCredentials->end()) // Nope, didn't find it on the
                                                // map. But if a Map was passed,
                                                // then it SHOULD have contained
                                                // all the listed credentials
                                                // (including the one we're
                                                // trying to load now.)
                        otErr << __FUNCTION__
                              << ": Expected master credential (" << strID
                              << ") on map of credentials, but couldn't find "
                                 "it. (Failure.)\n";
                    else // Found it on the map passed in (so no need to load
                         // from storage, we'll load from string instead.)
                    {
                        const String strMasterCredential(
                            it_cred->second.c_str());
                        if (strMasterCredential.Exists()) {
                            OTPasswordData thePWData(
                                nullptr == pstrReason
                                    ? "OTPseudonym::LoadFromString"
                                    : pstrReason->Get());
                            pCredential = CredentialSet::LoadMasterFromString(
                                strMasterCredential, strNymID, strID,
                                Credential::StringToCredentialType(strType),
                                &thePWData, pImportPassword);
                        }
                    }
                }

                if (nullptr == pCredential) {
                    otErr << __FUNCTION__
                          << ": Failed trying to load Master Credential ID: "
                          << strID << "\n";
                    return false;
                }
                else // pCredential must be cleaned up or stored somewhere.
                {
                    mapOfCredentialSets* pMap =
                        bValid ? &m_mapCredentialSets : &m_mapRevokedSets;
                    auto iter = pMap->find(strID.Get()); // todo optimize.
                    if (iter == pMap->end()) // It's not already there, so it's
                                             // safe to add it.
                        pMap->insert(std::pair<std::string, CredentialSet*>(
                            strID.Get(), pCredential)); // <=====
                    else {
                        otErr << __FUNCTION__ << ": While loading credential ("
                              << strID << "), discovered it was already there "
                                          "on my list, or one with the exact "
                                          "same ID! Therefore, failed "
                                          "adding this newer one.\n";
                        delete pCredential;
                        pCredential = nullptr;
                        return false;
                    }
                }
            }
            else if (strNodeName.Compare("keyCredential")) {
                const String strID    = xml->getAttributeValue("ID");
                const String strValid = xml->getAttributeValue(
                    "valid"); // If this is false, the ID is already on
                              // revokedCredentials list. (FYI.)
                const String strType  = xml->getAttributeValue("type");
                const String strMasterCredID =
                    xml->getAttributeValue("masterID");
                const bool bValid = strValid.Compare("true");
                otLog3 << "Loading " << (bValid ? "valid" : "invalid")
                       << " keyCredential ID: " << strID
                       << "\n ...For master credential: " << strMasterCredID
                       << "\n";
                CredentialSet* pCredential =
                    GetMasterCredential(strMasterCredID); // no need to cleanup.
                if (nullptr == pCredential)
                    pCredential = GetRevokedCredential(strMasterCredID);
                if (nullptr == pCredential) {
                    otErr << __FUNCTION__
                          << ": While loading keyCredential, failed trying to "
                             "find expected Master Credential ID: "
                          << strMasterCredID << "\n";
                    return false;
                }
                else // We found the master credential that this keyCredential
                       // belongs to.
                {
                    bool bLoaded = false;

                    if (nullptr ==
                        pMapCredentials) // pMapCredentials is an option
                                         // that allows you to read
                                         // credentials from the map
                                         // instead of from local
                                         // storage. (While loading the
                                         // Nym...) In this case, the
                                         // option isn't being
                                         // employed...
                        bLoaded = pCredential->LoadChildKeyCredential(strID, Credential::StringToCredentialType(strType));
                    else // In this case, it potentially is on the map...
                    {
                        auto it_cred = pMapCredentials->find(strID.Get());

                        if (it_cred ==
                            pMapCredentials->end()) // Nope, didn't find it on
                                                    // the map. But if a Map was
                                                    // passed, then it SHOULD
                                                    // have contained all the
                                                    // listed credentials
                                                    // (including the one we're
                                                    // trying to load now.)
                            otErr << __FUNCTION__
                                  << ": Expected keyCredential (" << strID
                                  << ") on map of credentials, but couldn't "
                                     "find it. (Failure.)\n";
                        else // Found it on the map passed in (so no need to
                             // load from storage, we'll load from string
                             // instead.)
                        {
                            const String strChildCredential(
                                it_cred->second.c_str());
                            if (strChildCredential.Exists())
                                bLoaded = pCredential->LoadChildKeyCredentialFromString(
                                    strChildCredential, strID, Credential::StringToCredentialType(strType),
                                    pImportPassword);
                        }
                    }

                    if (!bLoaded) {
                        String strNymID;
                        GetIdentifier(strNymID);
                        otErr << __FUNCTION__
                              << ": Failed loading keyCredential " << strID
                              << " for master credential " << strMasterCredID
                              << " for Nym " << strNymID << ".\n";
                        return false;
                    }
                }
            }
            else if (strNodeName.Compare("child credential")) {
                const String strID = xml->getAttributeValue("ID");
                const String strValid = xml->getAttributeValue(
                    "valid"); // If this is false, the ID is already on
                              // revokedCredentials list. (FYI.)
                const String strMasterCredID =
                    xml->getAttributeValue("masterID");
                const bool bValid = strValid.Compare("true");
                otLog3 << "Loading " << (bValid ? "valid" : "invalid")
                       << " child credential ID: " << strID
                       << "\n ...For master credential: " << strMasterCredID
                       << "\n";
                CredentialSet* pCredential =
                    GetMasterCredential(strMasterCredID); // no need to cleanup.
                if (nullptr == pCredential)
                    pCredential = GetRevokedCredential(strMasterCredID);
                if (nullptr == pCredential) {
                    otErr << __FUNCTION__
                          << ": While loading child credential, failed trying to "
                             "find expected Master Credential ID: "
                          << strMasterCredID << "\n";
                    return false;
                }
                else // We found the master credential that this child credential
                       // belongs to.
                {
                    bool bLoaded = false;

                    if (nullptr ==
                        pMapCredentials) // pMapCredentials is an option
                                         // that allows you to read
                                         // credentials from the map
                                         // instead of from local
                                         // storage. (While loading the
                                         // Nym...) In this case, the
                                         // option isn't being
                                         // employed...
                        bLoaded = pCredential->LoadCredential(strID);
                    else // In this case, it potentially is on the map...
                    {
                        auto it_cred = pMapCredentials->find(strID.Get());

                        if (it_cred ==
                            pMapCredentials->end()) // Nope, didn't find it on
                                                    // the map. But if a Map was
                                                    // passed, then it SHOULD
                                                    // have contained all the
                                                    // listed credentials
                                                    // (including the one we're
                                                    // trying to load now.)
                            otErr << __FUNCTION__
                                  << ": Expected child credential (" << strID
                                  << ") on map of credentials, but couldn't "
                                     "find it. (Failure.)\n";
                        else // Found it on the map passed in (so no need to
                             // load from storage, we'll load from string
                             // instead.)
                        {
                            const String strChildCredential(
                                it_cred->second.c_str());
                            if (strChildCredential.Exists())
                                bLoaded =
                                    pCredential->LoadCredentialFromString(
                                        strChildCredential, strID,
                                        pImportPassword);
                        }
                    }

                    if (!bLoaded) {
                        String strNymID;
                        GetIdentifier(strNymID);
                        otErr << __FUNCTION__
                              << ": Failed loading child credential " << strID
                              << " for master credential " << strMasterCredID
                              << " for Nym " << strNymID << ".\n";
                        return false;
                    }
                }
            }
            else if (strNodeName.Compare("requestNum")) {
                const String ReqNumNotaryID =
                    xml->getAttributeValue("notaryID");
                const String ReqNumCurrent =
                    xml->getAttributeValue("currentRequestNum");

                otLog3 << "\nCurrent Request Number is " << ReqNumCurrent
                       << " for NotaryID: " << ReqNumNotaryID << "\n";

                // Make sure now that I've loaded this request number, to add it
                // to my
                // internal map so that it is available for future lookups.
                m_mapRequestNum[ReqNumNotaryID.Get()] = ReqNumCurrent.ToLong();
            }
            else if (strNodeName.Compare("nymboxHash")) {
                const String strValue = xml->getAttributeValue("value");

                otLog3 << "\nNymboxHash is: " << strValue << "\n";

                if (strValue.Exists()) m_NymboxHash.SetString(strValue);
            }
            else if (strNodeName.Compare("nymboxHashItem")) {
                const String strNotaryID = xml->getAttributeValue("notaryID");
                const String strNymboxHash =
                    xml->getAttributeValue("nymboxHash");

                otLog3 << "\nNymboxHash is " << strNymboxHash
                       << " for NotaryID: " << strNotaryID << "\n";

                // Make sure now that I've loaded this nymboxHash, to add it to
                // my
                // internal map so that it is available for future lookups.
                if (strNotaryID.Exists() && strNymboxHash.Exists()) {
                    const Identifier theID(strNymboxHash);
                    m_mapNymboxHash[strNotaryID.Get()] = theID;
                }
            }
            else if (strNodeName.Compare("recentHashItem")) {
                const String strNotaryID = xml->getAttributeValue("notaryID");
                const String strRecentHash =
                    xml->getAttributeValue("recentHash");

                otLog3 << "\nRecentHash is " << strRecentHash
                       << " for NotaryID: " << strNotaryID << "\n";

                // Make sure now that I've loaded this RecentHash, to add it to
                // my
                // internal map so that it is available for future lookups.
                if (strNotaryID.Exists() && strRecentHash.Exists()) {
                    const Identifier theID(strRecentHash);
                    m_mapRecentHash[strNotaryID.Get()] = theID;
                }
            }
            else if (strNodeName.Compare("inboxHashItem")) {
                const String strAccountID = xml->getAttributeValue("accountID");
                const String strHashValue = xml->getAttributeValue("hashValue");

                otLog3 << "\nInboxHash is " << strHashValue
                       << " for Account ID: " << strAccountID << "\n";

                // Make sure now that I've loaded this InboxHash, to add it to
                // my
                // internal map so that it is available for future lookups.
                //
                if (strAccountID.Exists() && strHashValue.Exists()) {
                    const Identifier theID(strHashValue);
                    m_mapInboxHash[strAccountID.Get()] = theID;
                }
            }
            else if (strNodeName.Compare("outboxHashItem")) {
                const String strAccountID = xml->getAttributeValue("accountID");
                const String strHashValue = xml->getAttributeValue("hashValue");

                otLog3 << "\nOutboxHash is " << strHashValue
                       << " for Account ID: " << strAccountID << "\n";

                // Make sure now that I've loaded this OutboxHash, to add it to
                // my
                // internal map so that it is available for future lookups.
                //
                if (strAccountID.Exists() && strHashValue.Exists()) {
                    const Identifier theID(strHashValue);
                    m_mapOutboxHash[strAccountID.Get()] = theID;
                }
            }
            else if (strNodeName.Compare("highestTransNum")) {
                const String HighNumNotaryID =
                    xml->getAttributeValue("notaryID");
                const String HighNumRecent =
                    xml->getAttributeValue("mostRecent");

                otLog3 << "\nHighest Transaction Number ever received is "
                       << HighNumRecent << " for NotaryID: " << HighNumNotaryID
                       << "\n";

                // Make sure now that I've loaded this highest number, to add it
                // to my
                // internal map so that it is available for future lookups.
                m_mapHighTransNo[HighNumNotaryID.Get()] =
                    HighNumRecent.ToLong();
            }
            else if (strNodeName.Compare("transactionNums")) {
                const String tempNotaryID = xml->getAttributeValue("notaryID");
                String strTemp;
                if (!tempNotaryID.Exists() ||
                    !Contract::LoadEncodedTextField(xml, strTemp)) {
                    otErr << __FUNCTION__
                          << ": Error: transactionNums field without value.\n";
                    return false; // error condition
                }
                NumList theNumList;

                if (strTemp.Exists()) theNumList.Add(strTemp);

                int64_t lTemp = 0;
                while (theNumList.Peek(lTemp)) {
                    theNumList.Pop();

                    otLog3 << "Transaction Number " << lTemp
                           << " ready-to-use for NotaryID: " << tempNotaryID
                           << "\n";
                    AddTransactionNum(tempNotaryID, lTemp); // This version
                                                            // doesn't save to
                                                            // disk. (Why save
                                                            // to disk AS WE'RE
                                                            // LOADING?)
                }
            }
            else if (strNodeName.Compare("issuedNums")) {
                const String tempNotaryID = xml->getAttributeValue("notaryID");
                String strTemp;
                if (!tempNotaryID.Exists() ||
                    !Contract::LoadEncodedTextField(xml, strTemp)) {
                    otErr << __FUNCTION__
                          << ": Error: issuedNums field without value.\n";
                    return false; // error condition
                }
                NumList theNumList;

                if (strTemp.Exists()) theNumList.Add(strTemp);

                int64_t lTemp = 0;
                while (theNumList.Peek(lTemp)) {
                    theNumList.Pop();

                    otLog3 << "Currently liable for issued trans# " << lTemp
                           << " at NotaryID: " << tempNotaryID << "\n";
                    AddIssuedNum(tempNotaryID, lTemp); // This version doesn't
                                                       // save to disk. (Why
                                                       // save to disk AS WE'RE
                                                       // LOADING?)
                }
            }
            else if (strNodeName.Compare("tentativeNums")) {
                const String tempNotaryID = xml->getAttributeValue("notaryID");
                String strTemp;
                if (!tempNotaryID.Exists() ||
                    !Contract::LoadEncodedTextField(xml, strTemp)) {
                    otErr << "OTPseudonym::LoadFromString: Error: "
                             "tentativeNums field without value.\n";
                    return false; // error condition
                }
                NumList theNumList;

                if (strTemp.Exists()) theNumList.Add(strTemp);

                int64_t lTemp = 0;
                while (theNumList.Peek(lTemp)) {
                    theNumList.Pop();

                    otLog3 << "Tentative: Currently awaiting success notice, "
                              "for accepting trans# " << lTemp
                           << " for NotaryID: " << tempNotaryID << "\n";
                    AddTentativeNum(tempNotaryID, lTemp); // This version
                                                          // doesn't save to
                                                          // disk. (Why save to
                                                          // disk AS WE'RE
                                                          // LOADING?)
                }
            }
            else if (strNodeName.Compare("ackNums")) {
                const String tempNotaryID = xml->getAttributeValue("notaryID");
                String strTemp;
                if (!tempNotaryID.Exists()) {
                    otErr << __FUNCTION__
                          << ": Error: While loading ackNums "
                             "field: Missing notaryID. Nym contents:\n\n"
                          << strNym << "\n\n";
                    return false; // error condition
                }

                //                  xml->read(); // there should be a text field
                // next, with the data for the list of acknowledged numbers.
                // Note: I think I was forced to add this when the numlist was
                // empty, one time, so this may come back
                // to haunt me, but I want to fix it right, not kludge it.

                if (!Contract::LoadEncodedTextField(xml, strTemp)) {
                    otErr << __FUNCTION__
                          << ": Error: ackNums field without value "
                             "(at least, unable to LoadEncodedTextField on "
                             "that value.)\n";
                    return false; // error condition
                }
                NumList theNumList;

                if (strTemp.Exists()) theNumList.Add(strTemp);

                int64_t lTemp = 0;
                while (theNumList.Peek(lTemp)) {
                    theNumList.Pop();

                    otInfo << "Acknowledgment record exists for server reply, "
                              "for Request Number " << lTemp
                           << " for NotaryID: " << tempNotaryID << "\n";
                    AddAcknowledgedNum(tempNotaryID, lTemp); // This version
                                                             // doesn't save to
                                                             // disk. (Why save
                                                             // to disk AS WE'RE
                                                             // LOADING?)
                }
            }

            // THE BELOW FOUR ARE DEPRECATED, AND ARE REPLACED BY THE ABOVE
            // FOUR.
            else if (strNodeName.Compare("transactionNum")) {
                const String TransNumNotaryID =
                    xml->getAttributeValue("notaryID");
                const String TransNumAvailable =
                    xml->getAttributeValue("transactionNum");

                otLog3 << "Transaction Number " << TransNumAvailable
                       << " available for NotaryID: " << TransNumNotaryID
                       << "\n";

                AddTransactionNum(
                    TransNumNotaryID,
                    TransNumAvailable.ToLong()); // This version doesn't save
                                                 // to disk. (Why save to
                                                 // disk AS WE'RE LOADING?)
            }
            else if (strNodeName.Compare("issuedNum")) {
                const String TransNumNotaryID =
                    xml->getAttributeValue("notaryID");
                const String TransNumAvailable =
                    xml->getAttributeValue("transactionNum");

                otLog3 << "Currently liable for Transaction Number "
                       << TransNumAvailable
                       << ", for NotaryID: " << TransNumNotaryID << "\n";

                AddIssuedNum(TransNumNotaryID,
                             TransNumAvailable.ToLong()); // This version
                                                          // doesn't save to
                                                          // disk. (Why save
                                                          // to disk AS WE'RE
                                                          // LOADING?)
            }
            else if (strNodeName.Compare("tentativeNum")) {
                const String TransNumNotaryID =
                    xml->getAttributeValue("notaryID");
                const String TransNumAvailable =
                    xml->getAttributeValue("transactionNum");

                otLog3 << "Currently waiting on server success notice, "
                          "accepting Transaction Number " << TransNumAvailable
                       << ", for NotaryID: " << TransNumNotaryID << "\n";

                AddTentativeNum(TransNumNotaryID,
                                TransNumAvailable.ToLong()); // This version
                                                             // doesn't save
                                                             // to disk. (Why
                                                             // save to disk
                                                             // AS WE'RE
                                                             // LOADING?)
            }
            else if (strNodeName.Compare("acknowledgedNum")) {
                const String AckNumNotaryID =
                    xml->getAttributeValue("notaryID");
                const String AckNumValue = xml->getAttributeValue("requestNum");

                otLog3 << "Acknowledgment record exists for server reply, for "
                          "Request Number " << AckNumValue
                       << ", for NotaryID: " << AckNumNotaryID << "\n";

                AddAcknowledgedNum(AckNumNotaryID,
                                   AckNumValue.ToLong()); // This version
                                                          // doesn't save to
                                                          // disk. (Why save
                                                          // to disk AS WE'RE
                                                          // LOADING?)
            }
            else if (strNodeName.Compare("MARKED_FOR_DELETION")) {
                m_bMarkForDeletion = true;
                otLog3 << "This nym has been MARKED_FOR_DELETION (at some "
                          "point prior.)\n";
            }
            else if (strNodeName.Compare("hasOpenCronItem")) {
                String strID = xml->getAttributeValue("ID");

                if (strID.Exists()) {
                    const int64_t lNewID = strID.ToLong();
                    m_setOpenCronItems.insert(lNewID);
                    otLog3 << "This nym has an open cron item with ID: "
                           << strID << "\n";
                }
                else
                    otLog3 << "This nym MISSING ID when loading open cron item "
                              "record.\n";
            }
            else if (strNodeName.Compare("ownsAssetAcct")) {
                String strID = xml->getAttributeValue("ID");

                if (strID.Exists()) {
                    m_setAccounts.insert(strID.Get());
                    otLog3 << "This nym has an asset account with the ID: "
                           << strID << "\n";
                }
                else
                    otLog3 << "This nym MISSING asset account ID when loading "
                              "nym record.\n";
            }
            else if (strNodeName.Compare("mailMessage")) {
                OTASCIIArmor armorMail;
                String strMessage;

                xml->read();

                if (irr::io::EXN_TEXT == xml->getNodeType()) {
                    String strNodeData = xml->getNodeData();

                    // Sometimes the XML reads up the data with a prepended
                    // newline.
                    // This screws up my own objects which expect a consistent
                    // in/out
                    // So I'm checking here for that prepended newline, and
                    // removing it.
                    char cNewline;
                    if (strNodeData.Exists() && strNodeData.GetLength() > 2 &&
                        strNodeData.At(0, cNewline)) {
                        if ('\n' == cNewline)
                            armorMail.Set(strNodeData.Get() +
                                          1); // I know all this shit is ugly. I
                                              // refactored this in Contract.
                        else // unfortunately OTNym is like a "basic type" and
                             // isn't derived from Contract.
                            armorMail.Set(strNodeData.Get()); // TODO:
                                                              // Contract now
                                                              // has STATIC
                                                              // methods for
                                                              // this. (Start
                                                              // using them
                                                              // here...)

                        if (armorMail.GetLength() > 2) {
                            armorMail.GetString(strMessage,
                                                true); // linebreaks == true.

                            if (strMessage.GetLength() > 2) {
                                Message* pMessage = new Message;

                                OT_ASSERT(nullptr != pMessage);

                                if (pMessage->LoadContractFromString(
                                        strMessage))
                                    m_dequeMail.push_back(
                                        pMessage); // takes ownership
                                else
                                    delete pMessage;
                            }
                        } // armorMail
                    }     // strNodeData
                }         // EXN_TEXT
            }
            else if (strNodeName.Compare("outmailMessage")) {
                OTASCIIArmor armorMail;
                String strMessage;

                xml->read();

                if (irr::io::EXN_TEXT == xml->getNodeType()) {
                    String strNodeData = xml->getNodeData();

                    // Sometimes the XML reads up the data with a prepended
                    // newline.
                    // This screws up my own objects which expect a consistent
                    // in/out
                    // So I'm checking here for that prepended newline, and
                    // removing it.
                    char cNewline;
                    if (strNodeData.Exists() && strNodeData.GetLength() > 2 &&
                        strNodeData.At(0, cNewline)) {
                        if ('\n' == cNewline)
                            armorMail.Set(strNodeData.Get() + 1);
                        else
                            armorMail.Set(strNodeData.Get());

                        if (armorMail.GetLength() > 2) {
                            armorMail.GetString(strMessage,
                                                true); // linebreaks == true.

                            if (strMessage.GetLength() > 2) {
                                Message* pMessage = new Message;
                                OT_ASSERT(nullptr != pMessage);

                                if (pMessage->LoadContractFromString(
                                        strMessage))
                                    m_dequeOutmail.push_back(
                                        pMessage); // takes ownership
                                else
                                    delete pMessage;
                            }
                        } // armorMail
                    }     // strNodeData
                }         // EXN_TEXT
            }             // outpayments message
            else if (strNodeName.Compare("outpaymentsMessage")) {
                OTASCIIArmor armorMail;
                String strMessage;

                xml->read();

                if (irr::io::EXN_TEXT == xml->getNodeType()) {
                    String strNodeData = xml->getNodeData();

                    // Sometimes the XML reads up the data with a prepended
                    // newline.
                    // This screws up my own objects which expect a consistent
                    // in/out
                    // So I'm checking here for that prepended newline, and
                    // removing it.
                    char cNewline;
                    if (strNodeData.Exists() && strNodeData.GetLength() > 2 &&
                        strNodeData.At(0, cNewline)) {
                        if ('\n' == cNewline)
                            armorMail.Set(strNodeData.Get() + 1);
                        else
                            armorMail.Set(strNodeData.Get());

                        if (armorMail.GetLength() > 2) {
                            armorMail.GetString(strMessage,
                                                true); // linebreaks == true.

                            if (strMessage.GetLength() > 2) {
                                Message* pMessage = new Message;
                                OT_ASSERT(nullptr != pMessage);

                                if (pMessage->LoadContractFromString(
                                        strMessage))
                                    m_dequeOutpayments.push_back(
                                        pMessage); // takes ownership
                                else
                                    delete pMessage;
                            }
                        }
                    } // strNodeData
                }     // EXN_TEXT
            }         // outpayments message
            else {
                // unknown element type
                otErr << "Unknown element type in " << __FUNCTION__ << ": "
                      << xml->getNodeName() << "\n";
                bSuccess = false;
            }
            break;
        }
        default: {
            otLog5 << "Unknown XML type in " << __FUNCTION__ << ": "
                   << xml->getNodeName() << "\n";
            break;
        }
        } // switch
    }     // while

    return bSuccess;
}

bool Nym::LoadSignedNymfile(Nym& SIGNER_NYM)
{
    // Get the Nym's ID in string form
    String nymID;
    GetIdentifier(nymID);

    // Create an OTSignedFile object, giving it the filename (the ID) and the
    // local directory ("nyms")
    OTSignedFile theNymfile(OTFolders::Nym(), nymID);

    if (!theNymfile.LoadFile()) {
        otWarn << __FUNCTION__ << ": Failed loading a signed nymfile: " << nymID
               << "\n\n";
    }
    // We verify:
    //
    // 1. That the file even exists and loads.
    // 2. That the local subdir and filename match the versions inside the file.
    // 3. That the signature matches for the signer nym who was passed in.
    //
    else if (!theNymfile.VerifyFile()) {
        otErr << __FUNCTION__ << ": Failed verifying nymfile: " << nymID
              << "\n\n";
    }
    else if (!theNymfile.VerifySignature(SIGNER_NYM)) {
        String strSignerNymID;
        SIGNER_NYM.GetIdentifier(strSignerNymID);
        otErr << __FUNCTION__
              << ": Failed verifying signature on nymfile: " << nymID
              << "\n Signer Nym ID: " << strSignerNymID << "\n";
    }
    // NOTE: Comment out the above two blocks if you want to load a Nym without
    // having
    // to verify his information. (For development reasons. Never do that
    // normally.)
    else {
        otInfo
            << "Loaded and verified signed nymfile. Reading from string...\n";

        if (theNymfile.GetFilePayload().GetLength() > 0)
            return LoadFromString(
                theNymfile.GetFilePayload()); // <====== Success...
        else {
            const int64_t lLength =
                static_cast<int64_t>(theNymfile.GetFilePayload().GetLength());

            otErr << __FUNCTION__ << ": Bad length (" << lLength
                  << ") while loading nymfile: " << nymID << "\n";
        }
    }

    return false;
}

bool Nym::SaveSignedNymfile(Nym& SIGNER_NYM)
{
    // Get the Nym's ID in string form
    String strNymID;
    GetIdentifier(strNymID);

    // Create an OTSignedFile object, giving it the filename (the ID) and the
    // local directory ("nyms")
    OTSignedFile theNymfile(OTFolders::Nym().Get(), strNymID);
    theNymfile.GetFilename(m_strNymfile);

    otInfo << "Saving nym to: " << m_strNymfile << "\n";

    // First we save this nym to a string...
    // Specifically, the file payload string on the OTSignedFile object.
    SavePseudonym(theNymfile.GetFilePayload());

    // Now the OTSignedFile contains the path, the filename, AND the
    // contents of the Nym itself, saved to a string inside the OTSignedFile
    // object.

    if (theNymfile.SignContract(SIGNER_NYM) && theNymfile.SaveContract()) {
        const bool bSaved = theNymfile.SaveFile();

        if (!bSaved) {
            String strSignerNymID;
            SIGNER_NYM.GetIdentifier(strSignerNymID);
            otErr << __FUNCTION__
                  << ": Failed while calling theNymfile.SaveFile() for Nym "
                  << strNymID << " using Signer Nym " << strSignerNymID << "\n";
        }

        return bSaved;
    }
    else {
        String strSignerNymID;
        SIGNER_NYM.GetIdentifier(strSignerNymID);
        otErr << __FUNCTION__
              << ": Failed trying to sign and save Nymfile for Nym " << strNymID
              << " using Signer Nym " << strSignerNymID << "\n";
    }

    return false;
}

/// See if two nyms have identical lists of issued transaction numbers (#s
/// currently signed for.)
bool Nym::VerifyIssuedNumbersOnNym(Nym& THE_NYM)
{
    int64_t lTransactionNumber = 0; // Used in the loop below.

    int32_t nNumberOfTransactionNumbers1 = 0; // *this
    int32_t nNumberOfTransactionNumbers2 = 0; // THE_NYM.

    std::string strNotaryID;

    // First, loop through the Nym on my side (*this), and count how many
    // numbers total he has...
    //
    for (auto& it : GetMapIssuedNum()) {
        dequeOfTransNums* pDeque = (it.second);
        OT_ASSERT(nullptr != pDeque);

        if (!(pDeque->empty())) {
            nNumberOfTransactionNumbers1 +=
                static_cast<int32_t>(pDeque->size());
        }
    } // for

    // Next, loop through THE_NYM, and count his numbers as well...
    // But ALSO verify that each one exists on *this, so that each individual
    // number is checked.
    //
    for (auto& it : THE_NYM.GetMapIssuedNum()) {
        strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;
        OT_ASSERT(nullptr != pDeque);

        String OTstrNotaryID = strNotaryID.c_str();

        if (!(pDeque->empty())) {
            for (uint32_t i = 0; i < pDeque->size(); i++) {
                lTransactionNumber = pDeque->at(i);

                //                if ()
                {
                    nNumberOfTransactionNumbers2++;

                    if (false ==
                        VerifyIssuedNum(OTstrNotaryID, lTransactionNumber)) {
                        otOut << "OTPseudonym::" << __FUNCTION__
                              << ": Issued transaction # " << lTransactionNumber
                              << " from THE_NYM not found on *this.\n";

                        return false;
                    }
                }
            }
        }
    } // for

    // Finally, verify that the counts match...
    if (nNumberOfTransactionNumbers1 != nNumberOfTransactionNumbers2) {
        otOut << "OTPseudonym::" << __FUNCTION__
              << ": Issued transaction # Count mismatch: "
              << nNumberOfTransactionNumbers1 << " and "
              << nNumberOfTransactionNumbers2 << "\n";

        return false;
    }

    return true;
}

// This is client-side. It's called by VerifyTransactionReceipt and
// VerifyBalanceReceipt.
//
// It's okay if some issued transaction #s in THE_NYM (the receipt's Nym) aren't
// found on *this, (client-side Nym)
// since the last balance agreement may have cleaned them out after they were
// recorded in THE_NYM
// (from the transaction statement receipt).
//
// But I should never see transaction #s APPEAR in *this that aren't in THE_NYM
// on receipt, since a balance agreement
// can ONLY remove numbers, not add them. So any numbers left over should still
// be accounted for on the
// last signed receipt (which supplied THE_NYM as that list of numbers.)
//
// Conclusion: Loop through *this, which is newer, and make sure ALL numbers
// appear on THE_NYM.
// No need to check the reverse, and no need to match the count.
//
bool Nym::VerifyTransactionStatementNumbersOnNym(Nym& THE_NYM) // THE_NYM is
                                                               // from the
                                                               // receipt.
{
    int64_t lTransactionNumber = 0; // Used in the loop below.

    std::string strNotaryID;

    // First, loop through the Nym on my side (*this), and verify that all those
    // #s appear on the last receipt (THE_NYM)
    //
    for (auto& it : GetMapIssuedNum()) {
        strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;

        String OTstrNotaryID = strNotaryID.c_str();

        OT_ASSERT(nullptr != pDeque);

        if (!(pDeque->empty())) {
            for (uint32_t i = 0; i < pDeque->size(); i++) {
                lTransactionNumber = pDeque->at(i);

                if (false ==
                    THE_NYM.VerifyIssuedNum(OTstrNotaryID,
                                            lTransactionNumber)) {
                    otOut << "OTPseudonym::" << __FUNCTION__
                          << ": Issued transaction # " << lTransactionNumber
                          << " from *this not found on THE_NYM.\n";
                    return false;
                }
            }
        }
    } // for

    // Getting here means that, though issued numbers may have been removed from
    // my responsibility
    // in a subsequent balance agreement (since the transaction agreement was
    // signed), I know
    // for a fact that no numbers have been ADDED to my list of responsibility.
    // That's the most we can verify here, since we don't know the account
    // number that was
    // used for the last balance agreement.

    return true;
}

// static
bool Nym::DoesCertfileExist(const String& strNymID)
{
    String strCredListFile;
    strCredListFile.Format("%s.cred", strNymID.Get());

    return OTDB::Exists(OTFolders::Cert().Get(),
                        strNymID.Get()) || // Old-school.
           OTDB::Exists(OTFolders::Credential().Get(),
                        strNymID.Get(), strCredListFile.Get()); // New-school.
}

bool Nym::HasPublicKey() const
{
    for (auto& it : m_mapCredentialSets)
    {
        CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        if (pCredential->HasPublic())
            return true;
    }
    return false;
}

bool Nym::HasPrivateKey() const
{
    for (auto& it : m_mapCredentialSets)
    {
        CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        if (pCredential->HasPrivate())
            return true;
    }

    return false;
}

const OTAsymmetricKey& Nym::GetPrivateAuthKey() const
{
    OT_ASSERT(!m_mapCredentialSets.empty());

    const CredentialSet* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPrivateAuthKey(&m_listRevokedIDs); // success
}

const OTAsymmetricKey& Nym::GetPrivateEncrKey() const
{
    OT_ASSERT(!m_mapCredentialSets.empty());

    const CredentialSet* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPrivateEncrKey(&m_listRevokedIDs);
    ; // success
}

const OTAsymmetricKey& Nym::GetPrivateSignKey() const
{
    OT_ASSERT(!m_mapCredentialSets.empty());

    const CredentialSet* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPrivateSignKey(&m_listRevokedIDs); // success
}

const OTAsymmetricKey& Nym::GetPublicAuthKey() const
{
    OT_ASSERT(!m_mapCredentialSets.empty());

    const CredentialSet* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPublicAuthKey(&m_listRevokedIDs); // success
}

const OTAsymmetricKey& Nym::GetPublicEncrKey() const
{
    OT_ASSERT(!m_mapCredentialSets.empty());

    const CredentialSet* pCredential = nullptr;
    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPublicEncrKey(&m_listRevokedIDs); // success
}

const OTAsymmetricKey& Nym::GetPublicSignKey() const
{
    OT_ASSERT(!m_mapCredentialSets.empty());

    const CredentialSet* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPublicSignKey(&m_listRevokedIDs); // success
}

// This is being called by:
// Contract::VerifySignature(const OTPseudonym& theNym, const OTSignature&
// theSignature, OTPasswordData * pPWData=nullptr)
//
// Note: Need to change Contract::VerifySignature so that it checks all of
// these keys when verifying.
//
// OT uses the signature's metadata to narrow down its search for the correct
// public key.
// Return value is the count of public keys found that matched the metadata on
// the signature.
//
int32_t Nym::GetPublicKeysBySignature(listOfAsymmetricKeys& listOutput,
                                      const OTSignature& theSignature,
                                      char cKeyType) const
{
    // Unfortunately, theSignature can only narrow the search down (there may be
    // multiple results.)
    int32_t nCount = 0;

    for (const auto& it : m_mapCredentialSets) {
        const CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        const int32_t nTempCount = pCredential->GetPublicKeysBySignature(
            listOutput, theSignature, cKeyType);
        nCount += nTempCount;
    }

    return nCount;
}

// sets internal member based in ID passed in
void Nym::SetIdentifier(const Identifier& theIdentifier)
{
    m_nymID = theIdentifier;
}

// sets argument based on internal member
void Nym::GetIdentifier(Identifier& theIdentifier) const
{
    theIdentifier = m_nymID;
}

// sets internal member based in ID passed in
void Nym::SetIdentifier(const String& theIdentifier)
{
    m_nymID.SetString(theIdentifier);
}

// sets argument based on internal member
void Nym::GetIdentifier(String& theIdentifier) const
{
    m_nymID.GetString(theIdentifier);
}


Nym::Nym()
    : m_bMarkForDeletion(false)
    , m_lUsageCredits(0)
{
    Initialize();
}

void Nym::Initialize()
{
    m_strVersion = "1.0";
}

Nym::Nym(const String& name, const String& filename, const String& nymID)
    : m_bMarkForDeletion(false)
    , m_lUsageCredits(0)
{
    Initialize();

    m_strName = name;
    m_strNymfile = filename;

    m_nymID.SetString(nymID);
}

Nym::Nym(const Identifier& nymID)
    : m_bMarkForDeletion(false)
    , m_lUsageCredits(0)
{
    Initialize();

    m_nymID = nymID;
}

Nym::Nym(const String& strNymID)
    : m_bMarkForDeletion(false)
    , m_lUsageCredits(0)
{
    Initialize();

    m_nymID.SetString(strNymID);
}

void Nym::ClearCredentials()
{
    m_listRevokedIDs.clear();

    while (!m_mapCredentialSets.empty()) {
        CredentialSet* pCredential = m_mapCredentialSets.begin()->second;
        m_mapCredentialSets.erase(m_mapCredentialSets.begin());
        delete pCredential;
        pCredential = nullptr;
    }

    while (!m_mapRevokedSets.empty()) {
        CredentialSet* pCredential = m_mapRevokedSets.begin()->second;
        m_mapRevokedSets.erase(m_mapRevokedSets.begin());
        delete pCredential;
        pCredential = nullptr;
    }
}

void Nym::ClearAll()
{
    m_mapRequestNum.clear();
    m_mapHighTransNo.clear();

    ReleaseTransactionNumbers();
    //  m_mapTransNum.clear();
    //  m_mapIssuedNum.clear();
    //  m_mapTentativeNum.clear();
    //  m_mapAcknowledgedNum.clear();

    m_mapNymboxHash.clear();
    m_mapRecentHash.clear();
    m_mapInboxHash.clear();
    m_mapOutboxHash.clear();

    m_setAccounts.clear();
    m_setOpenCronItems.clear();

    ClearMail();
    ClearOutmail();
    ClearOutpayments();

    // We load the Nym twice... once just to load the credentials up from the
    // .cred file, and a second
    // time to load the rest of the Nym up from the Nymfile. LoadFromString
    // calls ClearAll before loading,
    // so there's no possibility of duplicated data on the Nym. And when that
    // happens, we don't want the
    // credentials to get cleared, since we want them to still be there after
    // the rest of the Nym is
    // loaded. So this is commented out.
    //  ClearCredentials();
}

Nym::~Nym()
{

    ClearAll();
    ClearCredentials();
}

} // namespace opentxs
