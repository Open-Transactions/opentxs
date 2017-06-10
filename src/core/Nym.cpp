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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/Nym.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/core/crypto/Bip39.hpp"
#endif
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSignedFile.hpp"
#include "opentxs/core/crypto/OTSymmetricKey.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/storage/Storage.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/server/OTServer.hpp"      // TODO remove this
#include "opentxs/server/ServerLoader.hpp"  // TODO remove this

#include <inttypes.h>
#include <sodium/crypto_box.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <array>
#include <fstream>
#include <irrxml/irrXML.hpp>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace opentxs
{

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
Nym* Nym::LoadPrivateNym(
    const Identifier& NYM_ID,
    bool bChecking,
    const String* pstrName,
    const char* szFuncName,
    const OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    const char* szFunc =
        (nullptr != szFuncName) ? szFuncName : "OTPseudonym::LoadPrivateNym";

    if (NYM_ID.IsEmpty()) return nullptr;

    const String strNymID(NYM_ID);

    // If name is empty, construct one way,
    // else construct a different way.
    std::unique_ptr<Nym> pNym;
    pNym.reset(
        ((nullptr == pstrName) || !pstrName->Exists())
            ? (new Nym(NYM_ID))
            : (new Nym(*pstrName, strNymID, strNymID)));
    OT_ASSERT_MSG(
        nullptr != pNym,
        "OTPseudonym::LoadPrivateNym: Error allocating memory.\n");

    OTPasswordData thePWData(OT_PW_DISPLAY);
    if (nullptr == pPWData) pPWData = &thePWData;

    bool bLoadedKey = pNym->LoadCredentials(true, pPWData, pImportPassword);

    if (!bLoadedKey) {
        Log::vOutput(
            bChecking ? 1 : 0,
            "%s: %s: (%s: is %s).  Unable to load credentials, "
            "cert and private key for: %s (maybe this nym doesn't exist?)\n",
            __FUNCTION__,
            szFunc,
            "bChecking",
            bChecking ? "true" : "false",
            strNymID.Get());
        // success loading credentials
        // failure verifying pseudonym public key.
    } else if (!pNym->VerifyPseudonym()) {
        otErr << __FUNCTION__ << " " << szFunc
              << ": Failure verifying Nym public key: " << strNymID << "\n";
        // success verifying pseudonym public key.
        // failure loading signed nymfile.
    } else if (!pNym->LoadSignedNymfile(*pNym)) {  // Unlike with public key,
                                                   // with private key we DO
                                                   // expect nymfile to be
                                                   // here.
        otErr << __FUNCTION__ << " " << szFunc
              << ": Failure calling LoadSignedNymfile: " << strNymID << "\n";
    } else {  // ultimate success.
        if (pNym->hasCapability(NymCapability::SIGN_MESSAGE)) {

            return pNym.release();
        }
        otErr << __FUNCTION__ << " " << szFunc << ": Loaded nym: " << strNymID
              << ", but it's not a private nym." << std::endl;
    }

    return nullptr;
}

/// Though the parameter is a reference (forcing you to pass a real object),
/// the Nym DOES take ownership of the object. Therefore it MUST be allocated
/// on the heap, NOT the stack, or you will corrupt memory with this call.
///
void Nym::AddOutpayments(Message& theMessage)  // a payments message is
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

const std::set<TransactionNumber> removing;

// Sometimes for testing I need to clear out all the transaction numbers from a
// nym. So I added this method to make such a thing easy to do.
void Nym::RemoveAllNumbers(const String* pstrNotaryID)
{
    std::list<mapOfIdentifiers::iterator> listOfInboxHash;
    std::list<mapOfIdentifiers::iterator> listOfOutboxHash;

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

    while (!listOfInboxHash.empty()) {
        m_mapInboxHash.erase(listOfInboxHash.back());
        listOfInboxHash.pop_back();
    }
    while (!listOfOutboxHash.empty()) {
        m_mapOutboxHash.erase(listOfOutboxHash.back());
        listOfOutboxHash.pop_back();
    }
}

bool Nym::GetInboxHash(
    const std::string& acct_id,
    Identifier& theOutput) const  // client-side
{
    return GetHash(m_mapInboxHash, acct_id, theOutput);
}

bool Nym::SetInboxHash(
    const std::string& acct_id,
    const Identifier& theInput)  // client-side
{
    return SetHash(m_mapInboxHash, acct_id, theInput);
}

bool Nym::GetOutboxHash(
    const std::string& acct_id,
    Identifier& theOutput) const  // client-side
{
    return GetHash(m_mapOutboxHash, acct_id, theOutput);
}

bool Nym::SetOutboxHash(
    const std::string& acct_id,
    const Identifier& theInput)  // client-side
{
    return SetHash(m_mapOutboxHash, acct_id, theInput);
}

bool Nym::GetHash(
    const mapOfIdentifiers& the_map,
    const std::string& str_id,
    Identifier& theOutput) const  // client-side
{
    bool bRetVal =
        false;  // default is false: "No, I didn't find a hash for that id."
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

bool Nym::SetHash(
    mapOfIdentifiers& the_map,
    const std::string& str_id,
    const Identifier& theInput)  // client-side
{
    bool bSuccess = false;

    auto find_it = the_map.find(str_id);

    if (the_map.end() != find_it)  // found something for that str_id
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

/*
 ResyncWithServer:

--    mapOfIdentifiers    m_mapInboxHash;
--    mapOfIdentifiers    m_mapOutboxHash;

--    dequeOfMail        m_dequeMail;    // Any mail messages received by this
Nym. (And not yet deleted.)
--    dequeOfMail        m_dequeOutmail;    // Any mail messages sent by this
Nym. (And not yet deleted.)
--    dequeOfMail        m_dequeOutpayments;    // Any outoing payments sent by
this Nym. (And not yet deleted.) (payments screen.)

**    mapOfTransNums     m_mapTransNum;    // Each Transaction Request must be
accompanied by a fresh transaction #,
**    mapOfTransNums     m_mapIssuedNum;    // If the server has issued me
(1,2,3,4,5) and I have already used 1-3,

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

**    mapOfIdentifiers    m_mapInboxHash;
**    mapOfIdentifiers    m_mapOutboxHash;

--    dequeOfMail        m_dequeMail;    // Any mail messages received by this
Nym. (And not yet deleted.)
--    dequeOfMail        m_dequeOutmail;    // Any mail messages sent by this
Nym. (And not yet deleted.)
--    dequeOfMail        m_dequeOutpayments;    // Any outoing payments sent by
this Nym. (And not yet deleted.) (payments screen.)

**    mapOfTransNums   m_mapTransNum;
**    mapOfTransNums   m_mapIssuedNum;

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

    auto context =
        OT::App().Contract().mutable_ServerContext(m_nymID, theNotaryID);

    // Remove all issued, transaction, and tentative numbers for a specific
    // server ID,
    // as well as all acknowledgedNums, and the highest transaction number for
    // that notaryID,
    // from *this nym. Leave our record of the highest trans num received from
    // that server,
    // since we will want to just keep it when re-syncing. (Server doesn't store
    // that anyway.)
    //
    RemoveAllNumbers(&strNotaryID);

    std::set<TransactionNumber> setTransNumbers;

    // loop through theNymbox and add Tentative numbers to *this based on each
    // successNotice in the Nymbox. This way, when the notices are processed,
    // they will succeed because the Nym will believe he was expecting them.
    for (auto& it : theNymbox.GetTransactionMap()) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);
        //        OTString strTransaction(*pTransaction);
        //        otErr << "TRANSACTION CONTENTS:\n%s\n", strTransaction.Get());

        // (a new; ALREADY just added transaction number.)
        if ((OTTransaction::successNotice !=
             pTransaction->GetType()))  // if !successNotice
            continue;

        const int64_t lNum =
            pTransaction->GetReferenceToNum();  // successNotice is inRefTo the
                                                // new transaction # that should
                                                // be on my tentative list.

        // Add to list of tentatively-being-added numbers.
        if (!context.It().AddTentativeNumber(lNum)) {
            otErr << "OTPseudonym::ResyncWithServer: Failed trying to add "
                     "TentativeNum ("
                  << lNum << ") onto *this nym: " << strNymID
                  << ", for server: " << strNotaryID << "\n";
            bSuccess = false;
        } else
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

    std::set<TransactionNumber> notUsed;
    context.It().UpdateHighest(setTransNumbers, notUsed, notUsed);

    return (SaveSignedNymfile(*this) && bSuccess);
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

            // Verify all Credentials in the CredentialSet, including source
            // verification for the master credential.
            if (!pCredential->VerifyInternally()) {
                otOut << __FUNCTION__ << ": Credential ("
                      << pCredential->GetMasterCredID()
                      << ") failed its own internal verification." << std::endl;
                return false;
            }
        }
        return true;
    }
    otErr << "No credentials.\n";
    return false;
}

bool Nym::CompareID(const Nym& RHS) const { return RHS.CompareID(m_nymID); }

bool Nym::SavePseudonymWallet(Tag& parent) const
{
    String nymID;
    GetIdentifier(nymID);

    // Name is in the clear in memory,
    // and base64 in storage.
    OTASCIIArmor ascName;
    if (!alias_.empty()) {
        ascName.SetString(String(alias_), false);  // linebreaks == false
    }

    TagPtr pTag(new Tag("pseudonym"));

    pTag->add_attribute("name", !alias_.empty() ? ascName.Get() : "");
    pTag->add_attribute("nymID", nymID.Get());

    parent.add_tag(pTag);

    return true;
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
    strOutput.Concatenate("Source for ID:\n%s\n", source_->asString().Get());
    strOutput.Concatenate("Description: %s\n\n", m_strDescription.Get());

    const size_t nMasterCredCount = GetMasterCredentialCount();
    if (nMasterCredCount > 0) {
        for (int32_t iii = 0; iii < static_cast<int64_t>(nMasterCredCount);
             ++iii) {
            const CredentialSet* pCredentialSet =
                GetMasterCredentialByIndex(iii);
            if (nullptr != pCredentialSet) {
                const String strCredType = Credential::CredentialTypeToString(
                    pCredentialSet->GetMasterCredential().Type());
                strOutput.Concatenate(
                    "%s Master Credential ID: %s \n",
                    strCredType.Get(),
                    pCredentialSet->GetMasterCredID().Get());
                const size_t nChildCredentialCount =
                    pCredentialSet->GetChildCredentialCount();

                if (nChildCredentialCount > 0) {
                    for (size_t vvv = 0; vvv < nChildCredentialCount; ++vvv) {
                        const Credential* pChild =
                            pCredentialSet->GetChildCredentialByIndex(vvv);
                        const String strChildCredType =
                            Credential::CredentialTypeToString(pChild->Type());
                        const std::string str_childcred_id(
                            pCredentialSet->GetChildCredentialIDByIndex(vvv));

                        strOutput.Concatenate(
                            "   %s child credential ID: %s \n",
                            strChildCredType.Get(),
                            str_childcred_id.c_str());
                    }
                }
            }
        }
        strOutput.Concatenate("%s", "\n");
    }

    strOutput.Concatenate(
        "==>      Name: %s   %s\n",
        alias_.c_str(),
        m_bMarkForDeletion ? "(MARKED FOR DELETION)" : "");
    strOutput.Concatenate("      Version: %s\n", m_strVersion.Get());
    strOutput.Concatenate(
        "Outpayments count: %" PRI_SIZE "\n", m_dequeOutpayments.size());

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
bool Nym::ReEncryptPrivateCredentials(
    bool bImporting,  // bImporting=true, or
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
            &strDisplay, !bImporting);  // bAskTwice is true when exporting
                                        // (since the export passphrase is being
                                        // created at that time.)
        thePasswordAngel.reset(pExportPassphrase);

        if (nullptr == pExportPassphrase) {
            otErr << __FUNCTION__ << ": Failed in GetPassphraseFromUser.\n";
            return false;
        }
    } else {
        pExportPassphrase = pImportPassword;
    }

    for (auto& it : m_mapCredentialSets) {
        CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        if (false ==
            pCredential->ReEncryptPrivateCredentials(
                *pExportPassphrase, bImporting))
            return false;
    }

    return true;
}

const serializedCredentialIndex Nym::asPublicNym() const
{
    return SerializeCredentialIndex(CREDENTIAL_INDEX_MODE_FULL_CREDS);
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
    if (source_) {

        TagPtr pTag(new Tag("nymIDSource", source_->asString().Get()));

        if (m_strDescription.Exists()) {
            OTASCIIArmor ascDescription;
            ascDescription.SetString(
                m_strDescription,
                false);  // bLineBreaks=true by default.

            pTag->add_attribute("Description", ascDescription.Get());
        }
        parent.add_tag(pTag);
    }
}

bool Nym::SaveCredentialIDs() const
{
    String strNymID;
    GetIdentifier(strNymID);
    serializedCredentialIndex index =
        SerializeCredentialIndex(CREDENTIAL_INDEX_MODE_ONLY_IDS);

    const bool valid = proto::Check(index, index.version(), index.version());

    if (!valid) {
        return false;
    }

    if (!OT::App().DB().Store(index, alias_)) {
        otErr << __FUNCTION__ << ": Failure trying to store "
              << " credential list for Nym: " << strNymID << std::endl;

        return false;
    }

    otWarn << "Credentials saved." << std::endl;

    return true;
}

// Use this to load the keys for a Nym (whether public or private), and then
// call VerifyPseudonym, and then load the actual Nymfile using
// LoadSignedNymfile.
bool Nym::LoadCredentials(
    bool bLoadPrivate,
    const OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    ClearCredentials();

    String strNymID;
    GetIdentifier(strNymID);
    std::shared_ptr<proto::CredentialIndex> index;

    if (OT::App().DB().Load(strNymID.Get(), index)) {
        return LoadCredentialIndex(*index);
    } else {
        otErr << __FUNCTION__
              << ": Failed trying to load credential list for nym: " << strNymID
              << std::endl;
    }

    return false;
}

void Nym::SaveCredentialsToTag(
    Tag& parent,
    String::Map* pmapPubInfo,
    String::Map* pmapPriInfo) const
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
            parent,
            m_listRevokedIDs,
            pmapPubInfo,
            pmapPriInfo,
            true);  // bShowRevoked=false by default (true here), bValid=true
    }

    // Serialize Revoked master credentials here, including their child key
    // credentials.
    for (auto& it : m_mapRevokedSets) {
        CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        pCredential->SerializeIDs(
            parent,
            m_listRevokedIDs,
            pmapPubInfo,
            pmapPriInfo,
            true,
            false);  // bShowRevoked=false by default. (Here it's true.)
                     // bValid=true by default. Here is for revoked, so false.
    }
}

serializedCredentialIndex Nym::SerializeCredentialIndex(
    const CredentialIndexModeFlag mode) const
{
    serializedCredentialIndex index;

    auto version = version_;

    // Upgrade to version 2
    if (2 > version_) {
        version = 2;
    }

    index.set_version(version);
    String nymID(m_nymID);
    index.set_nymid(nymID.Get());

    if (CREDENTIAL_INDEX_MODE_ONLY_IDS == mode) {
        index.set_mode(mode_);

        if (proto::CREDINDEX_PRIVATE == mode_) {
            index.set_index(index_);
        }
    } else {
        index.set_mode(proto::CREDINDEX_PUBLIC);
    }

    index.set_revision(revision_.load());
    *(index.mutable_source()) = *(source_->Serialize());

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            SerializedCredentialSet credset = it.second->Serialize(mode);
            auto pCredSet = index.add_activecredentials();
            *pCredSet = *credset;
            pCredSet = nullptr;
        }
    }

    for (auto& it : m_mapRevokedSets) {
        if (nullptr != it.second) {
            SerializedCredentialSet credset = it.second->Serialize(mode);
            auto pCredSet = index.add_revokedcredentials();
            *pCredSet = *credset;
            pCredSet = nullptr;
        }
    }

    return index;
}

bool Nym::LoadCredentialIndex(const serializedCredentialIndex& index)
{
    if (!proto::Check<proto::CredentialIndex>(index, 0, 0xFFFFFFFF)) {
        otErr << __FUNCTION__ << ": Unable to load invalid serialized"
              << " credential index." << std::endl;

        return false;
    }

    version_ = index.version();
    index_ = index.index();
    revision_.store(index.revision());
    mode_ = index.mode();
    Identifier nymID(index.nymid());
    m_nymID = nymID;
    SetSource(NymIDSource(index.source()));
    proto::KeyMode mode = (proto::CREDINDEX_PRIVATE == mode_)
                              ? proto::KEYMODE_PRIVATE
                              : proto::KEYMODE_PUBLIC;

    for (auto& it : index.activecredentials()) {
        CredentialSet* newSet = new CredentialSet(mode, it);

        if (nullptr != newSet) {
            m_mapCredentialSets.insert(std::pair<std::string, CredentialSet*>(
                newSet->GetMasterCredID().Get(), newSet));
        }
    }

    for (auto& it : index.revokedcredentials()) {
        CredentialSet* newSet = new CredentialSet(mode, it);

        if (nullptr != newSet) {
            m_mapCredentialSets.insert(std::pair<std::string, CredentialSet*>(
                newSet->GetMasterCredID().Get(), newSet));
        }
    }

    return true;
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

    // When you delete a Nym, it just marks it.
    // Actual deletion occurs during maintenance sweep
    // (targeting marked nyms...)
    //
    if (m_bMarkForDeletion) {
        tag.add_tag(
            "MARKED_FOR_DELETION",
            "THIS NYM HAS BEEN MARKED "
            "FOR DELETION AT ITS OWN REQUEST");
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
    if (!(m_setAccounts.empty())) {
        for (auto& it : m_setAccounts) {
            std::string strID(it);
            TagPtr pTag(new Tag("ownsAssetAcct"));
            pTag->add_attribute("ID", strID);
            tag.add_tag(pTag);
        }
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
    }  // for

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
    }  // for

    std::string str_result;
    tag.output(str_result);

    strNym.Concatenate("%s", str_result.c_str());

    return true;
}

CredentialSet* Nym::GetMasterCredential(const String& strID)
{
    auto iter = m_mapCredentialSets.find(strID.Get());
    CredentialSet* pCredential = nullptr;

    if (m_mapCredentialSets.end() != iter)  // found it
        pCredential = iter->second;

    return pCredential;
}

const CredentialSet* Nym::MasterCredential(const String& strID) const
{
    auto iter = m_mapCredentialSets.find(strID.Get());
    CredentialSet* pCredential = nullptr;

    if (m_mapCredentialSets.end() != iter)  // found it
        pCredential = iter->second;

    return pCredential;
}

std::shared_ptr<const proto::Credential> Nym::MasterCredentialContents(
    const std::string& id) const
{
    std::shared_ptr<const proto::Credential> output;
    auto credential = MasterCredential(String(id));

    if (nullptr != credential) {
        output = credential->GetMasterCredential().Serialized(
            AS_PUBLIC, WITH_SIGNATURES);
    }

    return output;
}

int32_t Nym::ChildCredentialCount(const std::string& id) const
{
    int32_t output = 0;
    auto credential = MasterCredential(String(id));

    if (nullptr != credential) {
        output = credential->GetChildCredentialCount();
    }

    return output;
}

std::string Nym::ChildCredentialID(
    const std::string& masterID,
    const uint32_t index) const
{
    std::string output = "";
    auto credential = MasterCredential(String(masterID));

    if (nullptr != credential) {
        output = credential->GetChildCredentialIDByIndex(index);
    }

    return output;
}

std::shared_ptr<const proto::Credential> Nym::ChildCredentialContents(
    const std::string& masterID,
    const std::string& childID) const
{
    std::shared_ptr<const proto::Credential> output;
    auto credential = MasterCredential(String(masterID));

    if (nullptr != credential) {
        output = credential->GetChildCredential(String(childID))
                     ->Serialized(AS_PUBLIC, WITH_SIGNATURES);
    }

    return output;
}

std::shared_ptr<const proto::Credential> Nym::RevokedCredentialContents(
    const std::string& id) const
{
    std::shared_ptr<const proto::Credential> output;

    auto iter = m_mapRevokedSets.find(id);

    if (m_mapRevokedSets.end() != iter) {
        output = iter->second->GetMasterCredential().Serialized(
            AS_PUBLIC, WITH_SIGNATURES);
    }

    return output;
}

CredentialSet* Nym::GetRevokedCredential(const String& strID)
{
    auto iter = m_mapRevokedSets.find(strID.Get());
    CredentialSet* pCredential = nullptr;

    if (m_mapCredentialSets.end() != iter)  // found it
        pCredential = iter->second;

    return pCredential;
}

const CredentialSet* Nym::GetMasterCredentialByIndex(int32_t nIndex) const
{
    if ((nIndex < 0) ||
        (nIndex >= static_cast<int64_t>(m_mapCredentialSets.size()))) {
        otErr << __FUNCTION__ << ": Index out of bounds: " << nIndex << "\n";
    } else {
        int32_t nLoopIndex = -1;

        for (const auto& it : m_mapCredentialSets) {
            const CredentialSet* pCredential = it.second;
            OT_ASSERT(nullptr != pCredential);

            ++nLoopIndex;  // 0 on first iteration.

            if (nLoopIndex == nIndex) return pCredential;
        }
    }
    return nullptr;
}

const CredentialSet* Nym::GetRevokedCredentialByIndex(int32_t nIndex) const
{
    if ((nIndex < 0) ||
        (nIndex >= static_cast<int64_t>(m_mapRevokedSets.size()))) {
        otErr << __FUNCTION__ << ": Index out of bounds: " << nIndex << "\n";
    } else {
        int32_t nLoopIndex = -1;

        for (const auto& it : m_mapRevokedSets) {
            const CredentialSet* pCredential = it.second;
            OT_ASSERT(nullptr != pCredential);

            ++nLoopIndex;  // 0 on first iteration.

            if (nLoopIndex == nIndex) return pCredential;
        }
    }
    return nullptr;
}

const Credential* Nym::GetChildCredential(
    const String& strMasterID,
    const String& strChildCredID) const
{
    auto iter = m_mapCredentialSets.find(strMasterID.Get());
    const CredentialSet* pMaster = nullptr;

    if (iter != m_mapCredentialSets.end())  // found it
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
bool Nym::LoadNymFromString(
    const String& strNym,
    bool& converted,
    String::Map* pMapCredentials,  // pMapCredentials can be
                                   // passed,
    // if you prefer to use a specific
    // set, instead of just loading the
    // actual set from storage (such as
    // during registration, when the
    // credentials have been sent
    // inside a message.)
    String* pstrReason,
    const OTPassword* pImportPassword)
{
    bool bSuccess = false;
    bool convert = false;
    converted = false;
    ClearAll();  // Since we are loading everything up... (credentials are NOT
                 // cleared here. See note in OTPseudonym::ClearAll.)
    OTStringXML strNymXML(strNym);  // todo optimize
    irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader(strNymXML);
    OT_ASSERT(nullptr != xml);
    std::unique_ptr<irr::io::IrrXMLReader> theCleanup(xml);

    auto server = ServerLoader::getServer();
    const bool serverMode = (nullptr != server);
    Identifier serverID;

    if (serverMode) {
        serverID = server->GetServerNym().ID();
    }

    // parse the file until end reached
    while (xml && xml->read()) {

        // strings for storing the data that we want to read out of the file
        //
        switch (xml->getNodeType()) {
            case irr::io::EXN_NONE:
            case irr::io::EXN_TEXT:
            case irr::io::EXN_COMMENT:
            case irr::io::EXN_ELEMENT_END:
            case irr::io::EXN_CDATA:
                break;
            case irr::io::EXN_ELEMENT: {
                const String strNodeName = xml->getNodeName();

                if (strNodeName.Compare("nymData")) {
                    m_strVersion = xml->getAttributeValue("version");
                    const String UserNymID = xml->getAttributeValue("nymID");

                    // Server-side only...
                    String strCredits = xml->getAttributeValue("usageCredits");

                    if (strCredits.GetLength() > 0)
                        m_lUsageCredits = strCredits.ToLong();
                    else
                        m_lUsageCredits = 0;  // This is the default anyway, but
                                              // just being safe...

                    m_nymID.SetString(UserNymID);

                    if (UserNymID.GetLength())
                        otLog3 << "\nLoading user, version: " << m_strVersion
                               << " NymID:\n"
                               << UserNymID << "\n";
                    bSuccess = true;
                    convert = (String("1.0") == m_strVersion);

                    if (convert) {
                        otErr << __FUNCTION__
                              << ": Converting nymfile with version "
                              << m_strVersion << std::endl;
                    } else {
                        otWarn << __FUNCTION__
                               << ": Not converting nymfile because version is "
                               << m_strVersion << std::endl;
                    }
                } else if (strNodeName.Compare("nymIDSource")) {
                    //                  otLog3 << "Loading nymIDSource...\n");
                    OTASCIIArmor ascDescription =
                        xml->getAttributeValue("Description");  // optional.
                    if (ascDescription.Exists())
                        ascDescription.GetString(
                            m_strDescription,
                            false);  // bLineBreaks=true by default.

                    OTASCIIArmor stringSource;
                    if (!Contract::LoadEncodedTextField(xml, stringSource)) {
                        otErr
                            << "Error in " << __FILE__ << " line " << __LINE__
                            << ": failed loading expected nymIDSource field.\n";
                        return false;  // error condition
                    }
                    serializedNymIDSource source =
                        NymIDSource::ExtractArmoredSource(stringSource);
                    source_.reset(new NymIDSource(*source));
                } else if (strNodeName.Compare("revokedCredential")) {
                    const String strRevokedID = xml->getAttributeValue("ID");
                    otLog3 << "revokedCredential ID: " << strRevokedID << "\n";
                    auto iter = std::find(
                        m_listRevokedIDs.begin(),
                        m_listRevokedIDs.end(),
                        strRevokedID.Get());
                    if (iter ==
                        m_listRevokedIDs.end())  // It's not already there,
                                                 // so it's safe to add it.
                        m_listRevokedIDs.push_back(
                            strRevokedID.Get());  // todo optimize.
                } else if (strNodeName.Compare("masterCredential")) {
                    const String strID = xml->getAttributeValue("ID");
                    const String strValid = xml->getAttributeValue("valid");
                    const String strType = xml->getAttributeValue("type");
                    const bool bValid = strValid.Compare("true");
                    otLog3 << "Loading " << (bValid ? "valid" : "invalid")
                           << " masterCredential ID: " << strID << "\n";
                    String strNymID;
                    GetIdentifier(strNymID);
                    CredentialSet* pCredential = nullptr;

                    if (nullptr ==
                        pMapCredentials)  // pMapCredentials is an option
                                          // that allows you to read
                        // credentials from the map instead
                        // of from local storage. (While
                        // loading the Nym...) In this
                        // case, the option isn't being
                        // employed...
                        pCredential =
                            CredentialSet::LoadMaster(strNymID, strID);
                    else  // In this case, it potentially is on the map...
                    {
                        auto it_cred = pMapCredentials->find(strID.Get());

                        if (it_cred == pMapCredentials->end())  // Nope, didn't
                                                                // find it on
                                                                // the
                            // map. But if a Map was passed,
                            // then it SHOULD have contained
                            // all the listed credentials
                            // (including the one we're
                            // trying to load now.)
                            otErr
                                << __FUNCTION__
                                << ": Expected master credential (" << strID
                                << ") on map of credentials, but couldn't find "
                                   "it. (Failure.)\n";
                        else  // Found it on the map passed in (so no need to
                              // load
                              // from storage, we'll load from string instead.)
                        {
                            const String strMasterCredential(
                                it_cred->second.c_str());
                            if (strMasterCredential.Exists()) {
                                OTPasswordData thePWData(
                                    nullptr == pstrReason
                                        ? "OTPseudonym::LoadFromString"
                                        : pstrReason->Get());
                                pCredential =
                                    CredentialSet::LoadMasterFromString(
                                        strMasterCredential,
                                        strNymID,
                                        strID,
                                        &thePWData,
                                        pImportPassword);
                            }
                        }
                    }

                    if (nullptr == pCredential) {
                        otErr
                            << __FUNCTION__
                            << ": Failed trying to load Master Credential ID: "
                            << strID << "\n";
                        return false;
                    } else  // pCredential must be cleaned up or stored
                            // somewhere.
                    {
                        mapOfCredentialSets* pMap =
                            bValid ? &m_mapCredentialSets : &m_mapRevokedSets;
                        auto iter = pMap->find(strID.Get());  // todo optimize.
                        if (iter ==
                            pMap->end())  // It's not already there, so it's
                                          // safe to add it.
                            pMap->insert(std::pair<std::string, CredentialSet*>(
                                strID.Get(), pCredential));  // <=====
                        else {
                            otErr << __FUNCTION__
                                  << ": While loading credential (" << strID
                                  << "), discovered it was already there "
                                     "on my list, or one with the exact "
                                     "same ID! Therefore, failed "
                                     "adding this newer one.\n";
                            delete pCredential;
                            pCredential = nullptr;
                            return false;
                        }
                    }
                } else if (strNodeName.Compare("keyCredential")) {
                    const String strID = xml->getAttributeValue("ID");
                    const String strValid = xml->getAttributeValue(
                        "valid");  // If this is false, the ID is already on
                                   // revokedCredentials list. (FYI.)
                    const String strType = xml->getAttributeValue("type");
                    const String strMasterCredID =
                        xml->getAttributeValue("masterID");
                    const bool bValid = strValid.Compare("true");
                    otLog3 << "Loading " << (bValid ? "valid" : "invalid")
                           << " keyCredential ID: " << strID
                           << "\n ...For master credential: " << strMasterCredID
                           << "\n";
                    CredentialSet* pCredential = GetMasterCredential(
                        strMasterCredID);  // no need to cleanup.
                    if (nullptr == pCredential)
                        pCredential = GetRevokedCredential(strMasterCredID);
                    if (nullptr == pCredential) {
                        otErr << __FUNCTION__
                              << ": While loading keyCredential, failed trying "
                                 "to "
                                 "find expected Master Credential ID: "
                              << strMasterCredID << "\n";
                        return false;
                    } else  // We found the master credential that this
                            // keyCredential
                            // belongs to.
                    {
                        bool bLoaded = false;

                        if (nullptr ==
                            pMapCredentials)  // pMapCredentials is an option
                                              // that allows you to read
                                              // credentials from the map
                                              // instead of from local
                                              // storage. (While loading the
                                              // Nym...) In this case, the
                                              // option isn't being
                                              // employed...
                            bLoaded =
                                pCredential->LoadChildKeyCredential(strID);
                        else  // In this case, it potentially is on the map...
                        {
                            auto it_cred = pMapCredentials->find(strID.Get());

                            if (it_cred == pMapCredentials->end())  // Nope,
                                                                    // didn't
                                                                    // find it
                                                                    // on
                                // the map. But if a Map was
                                // passed, then it SHOULD
                                // have contained all the
                                // listed credentials
                                // (including the one we're
                                // trying to load now.)
                                otErr
                                    << __FUNCTION__
                                    << ": Expected keyCredential (" << strID
                                    << ") on map of credentials, but couldn't "
                                       "find it. (Failure.)\n";
                            else  // Found it on the map passed in (so no need
                                  // to
                                  // load from storage, we'll load from string
                                  // instead.)
                            {
                                const String strChildCredential(
                                    it_cred->second.c_str());
                                if (strChildCredential.Exists())
                                    bLoaded =
                                        pCredential
                                            ->LoadChildKeyCredentialFromString(
                                                strChildCredential,
                                                strID,
                                                pImportPassword);
                            }
                        }

                        if (!bLoaded) {
                            String strNymID;
                            GetIdentifier(strNymID);
                            otErr << __FUNCTION__
                                  << ": Failed loading keyCredential " << strID
                                  << " for master credential "
                                  << strMasterCredID << " for Nym " << strNymID
                                  << ".\n";
                            return false;
                        }
                    }
                } else if (strNodeName.Compare("requestNum") && convert) {
                    const String ReqNumNotaryID =
                        xml->getAttributeValue("notaryID");
                    const String ReqNumCurrent =
                        xml->getAttributeValue("currentRequestNum");

                    otLog3 << "\nCurrent Request Number is " << ReqNumCurrent
                           << " for NotaryID: " << ReqNumNotaryID << "\n";

                    // Migrate to Context class.
                    Identifier local, remote;

                    if (serverMode) {
                        local = serverID;
                        remote = m_nymID;
                        auto context =
                            OT::App().Contract().mutable_ClientContext(
                                local, remote);
                        context.It().SetRequest(ReqNumCurrent.ToLong());
                    } else {
                        local = m_nymID;
                        remote = Identifier(ReqNumNotaryID);
                        auto context =
                            OT::App().Contract().mutable_ServerContext(
                                local, remote);
                        context.It().SetRequest(ReqNumCurrent.ToLong());
                    }

                    converted = true;
                } else if (strNodeName.Compare("nymboxHash") && convert) {
                    const String strValue = xml->getAttributeValue("value");

                    otLog3 << "\nNymboxHash is: " << strValue << "\n";

                    if (strValue.Exists()) {
                        // Migrate to Context class.
                        auto context =
                            OT::App().Contract().mutable_ClientContext(
                                serverID, m_nymID);
                        context.It().SetLocalNymboxHash(Identifier(strValue));
                    }

                    converted = true;
                } else if (strNodeName.Compare("nymboxHashItem") && convert) {
                    const String strNotaryID =
                        xml->getAttributeValue("notaryID");
                    const String strNymboxHash =
                        xml->getAttributeValue("nymboxHash");

                    otLog3 << "\nNymboxHash is " << strNymboxHash
                           << " for NotaryID: " << strNotaryID << "\n";

                    // Convert to Context class
                    if (strNotaryID.Exists() && strNymboxHash.Exists()) {
                        auto context =
                            OT::App().Contract().mutable_ServerContext(
                                m_nymID, Identifier(strNotaryID));
                        context.It().SetLocalNymboxHash(
                            Identifier(strNymboxHash));
                    }

                    converted = true;
                } else if (strNodeName.Compare("recentHashItem") && convert) {
                    const String strNotaryID =
                        xml->getAttributeValue("notaryID");
                    const String strRecentHash =
                        xml->getAttributeValue("recentHash");

                    otLog3 << "\nRecentHash is " << strRecentHash
                           << " for NotaryID: " << strNotaryID << "\n";

                    // Convert to Context class
                    if (strNotaryID.Exists() && strRecentHash.Exists()) {
                        auto context =
                            OT::App().Contract().mutable_ServerContext(
                                m_nymID, Identifier(strNotaryID));
                        context.It().SetRemoteNymboxHash(
                            Identifier(strRecentHash));
                    }

                    converted = true;
                } else if (strNodeName.Compare("inboxHashItem")) {
                    const String strAccountID =
                        xml->getAttributeValue("accountID");
                    const String strHashValue =
                        xml->getAttributeValue("hashValue");

                    otLog3 << "\nInboxHash is " << strHashValue
                           << " for Account ID: " << strAccountID << "\n";

                    // Make sure now that I've loaded this InboxHash, to add it
                    // to
                    // my
                    // internal map so that it is available for future lookups.
                    //
                    if (strAccountID.Exists() && strHashValue.Exists()) {
                        const Identifier theID(strHashValue);
                        m_mapInboxHash[strAccountID.Get()] = theID;
                    }
                } else if (strNodeName.Compare("outboxHashItem")) {
                    const String strAccountID =
                        xml->getAttributeValue("accountID");
                    const String strHashValue =
                        xml->getAttributeValue("hashValue");

                    otLog3 << "\nOutboxHash is " << strHashValue
                           << " for Account ID: " << strAccountID << "\n";

                    // Make sure now that I've loaded this OutboxHash, to add it
                    // to
                    // my
                    // internal map so that it is available for future lookups.
                    //
                    if (strAccountID.Exists() && strHashValue.Exists()) {
                        const Identifier theID(strHashValue);
                        m_mapOutboxHash[strAccountID.Get()] = theID;
                    }
                } else if (strNodeName.Compare("highestTransNum") && convert) {
                    const String HighNumNotaryID =
                        xml->getAttributeValue("notaryID");
                    const String HighNumRecent =
                        xml->getAttributeValue("mostRecent");

                    otLog3 << "\nHighest Transaction Number ever received is "
                           << HighNumRecent
                           << " for NotaryID: " << HighNumNotaryID << "\n";

                    // Migrate to Context class.
                    auto context = OT::App().Contract().mutable_ServerContext(
                        m_nymID, Identifier(HighNumNotaryID));
                    context.It().SetHighest(HighNumRecent.ToLong());

                    converted = true;
                } else if (strNodeName.Compare("transactionNums") && convert) {
                    const String tempNotaryID =
                        xml->getAttributeValue("notaryID");
                    String strTemp;

                    if (!tempNotaryID.Exists() ||
                        !Contract::LoadEncodedTextField(xml, strTemp)) {
                        otErr << __FUNCTION__ << ": Error: transactionNums "
                                                 "field without value.\n";
                        return false;  // error condition
                    }

                    NumList theNumList;

                    if (strTemp.Exists()) {
                        theNumList.Add(strTemp);
                    }

                    TransactionNumber lTemp = 0;

                    while (theNumList.Peek(lTemp)) {
                        theNumList.Pop();
                        // Migrate to Context class.
                        Identifier local, remote;

                        if (serverMode) {
                            local = serverID;
                            remote = m_nymID;
                            auto context =
                                OT::App().Contract().mutable_ClientContext(
                                    local, remote);
                            context.It().insert_available_number(lTemp);
                        } else {
                            local = m_nymID;
                            remote = Identifier(tempNotaryID);
                            auto context =
                                OT::App().Contract().mutable_ServerContext(
                                    local, remote);
                            context.It().insert_available_number(lTemp);
                        }
                    }

                    converted = true;
                } else if (strNodeName.Compare("issuedNums") && convert) {
                    const String tempNotaryID =
                        xml->getAttributeValue("notaryID");
                    String strTemp;
                    if (!tempNotaryID.Exists() ||
                        !Contract::LoadEncodedTextField(xml, strTemp)) {
                        otErr << __FUNCTION__
                              << ": Error: issuedNums field without value.\n";
                        return false;  // error condition
                    }
                    NumList theNumList;

                    if (strTemp.Exists()) theNumList.Add(strTemp);

                    TransactionNumber lTemp = 0;

                    while (theNumList.Peek(lTemp)) {
                        theNumList.Pop();

                        otLog3 << "Currently liable for issued trans# " << lTemp
                               << " at NotaryID: " << tempNotaryID << "\n";

                        // Migrate to Context class.
                        Identifier local, remote;

                        if (serverMode) {
                            local = serverID;
                            remote = m_nymID;
                            auto context =
                                OT::App().Contract().mutable_ClientContext(
                                    local, remote);
                            auto existing = context.It().Request();

                            if (0 == existing) {
                                context.It().insert_issued_number(lTemp);
                            }
                        } else {
                            local = m_nymID;
                            remote = Identifier(tempNotaryID);
                            auto context =
                                OT::App().Contract().mutable_ServerContext(
                                    local, remote);
                            auto existing = context.It().Request();

                            if (0 == existing) {
                                context.It().insert_issued_number(lTemp);
                            }
                        }
                    }

                    converted = true;
                } else if (strNodeName.Compare("tentativeNums") && convert) {
                    const String tempNotaryID =
                        xml->getAttributeValue("notaryID");
                    String strTemp;
                    if (!tempNotaryID.Exists() ||
                        !Contract::LoadEncodedTextField(xml, strTemp)) {
                        otErr << "OTPseudonym::LoadFromString: Error: "
                                 "tentativeNums field without value.\n";
                        return false;  // error condition
                    }
                    NumList theNumList;

                    if (strTemp.Exists()) theNumList.Add(strTemp);

                    TransactionNumber lTemp = 0;
                    while (theNumList.Peek(lTemp)) {
                        theNumList.Pop();

                        otLog3
                            << "Tentative: Currently awaiting success notice, "
                               "for accepting trans# "
                            << lTemp << " for NotaryID: " << tempNotaryID
                            << "\n";

                        // Convert to Context class
                        auto context =
                            OT::App().Contract().mutable_ServerContext(
                                m_nymID, Identifier(tempNotaryID));
                        context.It().AddTentativeNumber(lTemp);
                    }

                    converted = true;
                } else if (strNodeName.Compare("ackNums") && convert) {
                    const String tempNotaryID =
                        xml->getAttributeValue("notaryID");
                    String strTemp;
                    if (!tempNotaryID.Exists()) {
                        otErr << __FUNCTION__
                              << ": Error: While loading ackNums "
                                 "field: Missing notaryID. Nym contents:\n\n"
                              << strNym << "\n\n";
                        return false;  // error condition
                    }

                    //                  xml->read(); // there should be a text
                    //                  field
                    // next, with the data for the list of acknowledged numbers.
                    // Note: I think I was forced to add this when the numlist
                    // was
                    // empty, one time, so this may come back
                    // to haunt me, but I want to fix it right, not kludge it.

                    if (!Contract::LoadEncodedTextField(xml, strTemp)) {
                        otErr << __FUNCTION__
                              << ": Error: ackNums field without value "
                                 "(at least, unable to LoadEncodedTextField on "
                                 "that value.)\n";
                        return false;  // error condition
                    }
                    NumList theNumList;

                    if (strTemp.Exists()) {
                        theNumList.Add(strTemp);
                    }

                    RequestNumber lTemp = 0;

                    while (theNumList.Peek(lTemp)) {
                        theNumList.Pop();
                        // Migrate to Context class.
                        Identifier local, remote;

                        if (serverMode) {
                            local = serverID;
                            remote = m_nymID;
                            auto context =
                                OT::App().Contract().mutable_ClientContext(
                                    local, remote);
                            context.It().AddAcknowledgedNumber(lTemp);
                        } else {
                            local = m_nymID;
                            remote = Identifier(tempNotaryID);
                            auto context =
                                OT::App().Contract().mutable_ServerContext(
                                    local, remote);
                            context.It().AddAcknowledgedNumber(lTemp);
                        }
                    }

                    converted = true;
                } else if (strNodeName.Compare("MARKED_FOR_DELETION")) {
                    m_bMarkForDeletion = true;
                    otLog3 << "This nym has been MARKED_FOR_DELETION (at some "
                              "point prior.)\n";
                } else if (strNodeName.Compare("hasOpenCronItem") && convert) {
                    String strID = xml->getAttributeValue("ID");

                    if (strID.Exists()) {
                        auto context =
                            OT::App().Contract().mutable_ClientContext(
                                serverID, m_nymID);
                        context.It().OpenCronItem(strID.ToLong());
                        otLog3 << "This nym has an open cron item with ID: "
                               << strID << "\n";
                    } else
                        otLog3 << "This nym MISSING ID when loading open cron "
                                  "item "
                                  "record.\n";

                    converted = true;
                } else if (strNodeName.Compare("ownsAssetAcct")) {
                    String strID = xml->getAttributeValue("ID");

                    if (strID.Exists()) {
                        m_setAccounts.insert(strID.Get());
                        otLog3 << "This nym has an asset account with the ID: "
                               << strID << "\n";
                    } else
                        otLog3
                            << "This nym MISSING asset account ID when loading "
                               "nym record.\n";
                }
                // Convert nyms created with opentxs-1.0
                // TODO: Remove this code after support for opentxs-1.0 ends
                else if (strNodeName.Compare("mailMessage")) {
                    OTASCIIArmor armorMail;
                    String strMessage;

                    xml->read();

                    if (irr::io::EXN_TEXT == xml->getNodeType()) {
                        String strNodeData = xml->getNodeData();

                        // Sometimes the XML reads up the data with a prepended
                        // newline.
                        // This screws up my own objects which expect a
                        // consistent
                        // in/out
                        // So I'm checking here for that prepended newline, and
                        // removing it.
                        char cNewline;
                        if (strNodeData.Exists() &&
                            strNodeData.GetLength() > 2 &&
                            strNodeData.At(0, cNewline)) {
                            if ('\n' == cNewline)
                                armorMail.Set(
                                    strNodeData.Get() +
                                    1);  // I know all this shit is ugly. I
                                         // refactored this in Contract.
                            else  // unfortunately OTNym is like a "basic type"
                                  // and
                                  // isn't derived from Contract.
                                armorMail.Set(strNodeData.Get());  // TODO:
                            // Contract now
                            // has STATIC
                            // methods for
                            // this. (Start
                            // using them
                            // here...)

                            if (armorMail.GetLength() > 2) {
                                armorMail.GetString(
                                    strMessage,
                                    true);  // linebreaks == true.

                                if (strMessage.GetLength() > 2) {
                                    std::unique_ptr<Message> pMessage(
                                        new Message);

                                    OT_ASSERT(pMessage);

                                    const bool loaded =
                                        pMessage->LoadContractFromString(
                                            strMessage);

                                    if (loaded) {
                                        OT::App().Contract().Mail(
                                            m_nymID,
                                            *pMessage,
                                            StorageBox::MAILINBOX);
                                    }
                                }
                            }  // armorMail
                        }      // strNodeData
                    }          // EXN_TEXT
                }
                // Convert nyms created with opentxs-1.0
                // TODO: Remove this code after support for opentxs-1.0 ends
                else if (strNodeName.Compare("outmailMessage")) {
                    OTASCIIArmor armorMail;
                    String strMessage;

                    xml->read();

                    if (irr::io::EXN_TEXT == xml->getNodeType()) {
                        String strNodeData = xml->getNodeData();

                        // Sometimes the XML reads up the data with a prepended
                        // newline.
                        // This screws up my own objects which expect a
                        // consistent
                        // in/out
                        // So I'm checking here for that prepended newline, and
                        // removing it.
                        char cNewline;
                        if (strNodeData.Exists() &&
                            strNodeData.GetLength() > 2 &&
                            strNodeData.At(0, cNewline)) {
                            if ('\n' == cNewline)
                                armorMail.Set(strNodeData.Get() + 1);
                            else
                                armorMail.Set(strNodeData.Get());

                            if (armorMail.GetLength() > 2) {
                                armorMail.GetString(
                                    strMessage,
                                    true);  // linebreaks == true.

                                if (strMessage.GetLength() > 2) {
                                    std::unique_ptr<Message> pMessage(
                                        new Message);

                                    OT_ASSERT(pMessage);

                                    const bool loaded =
                                        pMessage->LoadContractFromString(
                                            strMessage);

                                    if (loaded) {
                                        OT::App().Contract().Mail(
                                            m_nymID,
                                            *pMessage,
                                            StorageBox::MAILOUTBOX);
                                    }
                                }
                            }  // armorMail
                        }      // strNodeData
                    }          // EXN_TEXT
                }              // outpayments message
                else if (strNodeName.Compare("outpaymentsMessage")) {
                    OTASCIIArmor armorMail;
                    String strMessage;

                    xml->read();

                    if (irr::io::EXN_TEXT == xml->getNodeType()) {
                        String strNodeData = xml->getNodeData();

                        // Sometimes the XML reads up the data with a prepended
                        // newline.
                        // This screws up my own objects which expect a
                        // consistent
                        // in/out
                        // So I'm checking here for that prepended newline, and
                        // removing it.
                        char cNewline;
                        if (strNodeData.Exists() &&
                            strNodeData.GetLength() > 2 &&
                            strNodeData.At(0, cNewline)) {
                            if ('\n' == cNewline)
                                armorMail.Set(strNodeData.Get() + 1);
                            else
                                armorMail.Set(strNodeData.Get());

                            if (armorMail.GetLength() > 2) {
                                armorMail.GetString(
                                    strMessage,
                                    true);  // linebreaks == true.

                                if (strMessage.GetLength() > 2) {
                                    Message* pMessage = new Message;
                                    OT_ASSERT(nullptr != pMessage);

                                    if (pMessage->LoadContractFromString(
                                            strMessage))
                                        m_dequeOutpayments.push_back(
                                            pMessage);  // takes ownership
                                    else
                                        delete pMessage;
                                }
                            }
                        }  // strNodeData
                    }      // EXN_TEXT
                }          // outpayments message
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
        }  // switch
    }      // while

    if (converted) {
        m_strVersion = "1.1";
    }

    return bSuccess;
}

bool Nym::LoadSignedNymfile(const Nym& SIGNER_NYM)
{
    // Get the Nym's ID in string form
    String nymID;
    GetIdentifier(nymID);

    // Create an OTSignedFile object, giving it the filename (the ID) and the
    // local directory ("nyms")
    OTSignedFile theNymfile(OTFolders::Nym(), nymID);

    if (!theNymfile.LoadFile()) {
        otWarn << __FUNCTION__ << ": Failed loading a signed nymfile: " << nymID
               << std::endl;

        return false;
    }

    // We verify:
    //
    // 1. That the file even exists and loads.
    // 2. That the local subdir and filename match the versions inside the file.
    // 3. That the signature matches for the signer nym who was passed in.
    //
    if (!theNymfile.VerifyFile()) {
        otErr << __FUNCTION__ << ": Failed verifying nymfile: " << nymID
              << "\n\n";

        return false;
    }

    if (!theNymfile.VerifySignature(SIGNER_NYM)) {
        String strSignerNymID;
        SIGNER_NYM.GetIdentifier(strSignerNymID);
        otErr << __FUNCTION__
              << ": Failed verifying signature on nymfile: " << nymID
              << "\n Signer Nym ID: " << strSignerNymID << "\n";

        return false;
    }

    otInfo << "Loaded and verified signed nymfile. Reading from string...\n";

    if (1 > theNymfile.GetFilePayload().GetLength()) {
        const auto lLength = theNymfile.GetFilePayload().GetLength();

        otErr << __FUNCTION__ << ": Bad length (" << lLength
              << ") while loading nymfile: " << nymID << "\n";
    }

    bool converted = false;
    const bool loaded =
        LoadNymFromString(theNymfile.GetFilePayload(), converted);

    if (!loaded) {
        return false;
    }

    if (converted) {
        // This will ensure that none of the old tags will be present the next
        // time this nym is loaded.
        // Converting a nym more than once is likely to cause sync issues.
        SaveSignedNymfile(SIGNER_NYM);
    }

    return true;
}

bool Nym::SaveSignedNymfile(const Nym& SIGNER_NYM)
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
    } else {
        String strSignerNymID;
        SIGNER_NYM.GetIdentifier(strSignerNymID);
        otErr << __FUNCTION__
              << ": Failed trying to sign and save Nymfile for Nym " << strNymID
              << " using Signer Nym " << strSignerNymID << "\n";
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

    return pCredential->GetPrivateAuthKey(&m_listRevokedIDs);  // success
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
    ;  // success
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

    return pCredential->GetPrivateSignKey(&m_listRevokedIDs);  // success
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

    return pCredential->GetPublicAuthKey(&m_listRevokedIDs);  // success
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

    return pCredential->GetPublicEncrKey(&m_listRevokedIDs);  // success
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

    return pCredential->GetPublicSignKey(&m_listRevokedIDs);  // success
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
int32_t Nym::GetPublicKeysBySignature(
    listOfAsymmetricKeys& listOutput,
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

Nym::Nym(const NymParameters& nymParameters)
    : version_(2)
    , mode_(proto::CREDINDEX_PRIVATE)
    , m_bMarkForDeletion(false)
    , m_lUsageCredits(0)
{
    revision_.store(1);
    Initialize();

    NymParameters revisedParameters = nymParameters;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    revisedParameters.SetCredset(index_++);
    std::uint32_t nymIndex = 0;
    std::string fingerprint = nymParameters.Seed();
    auto seed = OT::App().Crypto().BIP39().Seed(fingerprint, nymIndex);

    OT_ASSERT(seed);

    const bool defaultIndex = (0 == nymParameters.Nym());

    if (!defaultIndex) {
        otErr << __FUNCTION__ << ": Re-creating nym at specified path. "
              << std::endl;

        nymIndex = nymParameters.Nym();
    }

    const std::uint32_t newIndex = nymIndex + 1;

    OT::App().Crypto().BIP39().UpdateIndex(fingerprint, newIndex);
    revisedParameters.SetEntropy(*seed);
    revisedParameters.SetSeed(fingerprint);
    revisedParameters.SetNym(nymIndex);
#endif
    CredentialSet* pNewCredentialSet = new CredentialSet(revisedParameters);

    SetSource(pNewCredentialSet->Source());
    m_nymID = source_->NymID();

    SetDescription(source_->Description());

    m_mapCredentialSets.insert(std::pair<std::string, CredentialSet*>(
        pNewCredentialSet->GetMasterCredID().Get(), pNewCredentialSet));

    SaveCredentialIDs();
    SaveSignedNymfile(*this);
}

void Nym::Initialize() { m_strVersion = "1.1"; }

Nym::Nym(const String& name, const String& filename, const String& nymID)
    : m_bMarkForDeletion(false)
    , m_lUsageCredits(0)
{
    Initialize();

    alias_ = name.Get();
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
    m_mapInboxHash.clear();
    m_mapOutboxHash.clear();
    m_setAccounts.clear();
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

bool Nym::WriteCredentials() const
{
    for (auto& it : m_mapCredentialSets) {
        if (!it.second->WriteCredentials()) {
            otErr << __FUNCTION__ << ": Failed to save credentials."
                  << std::endl;

            return false;
        }
    }

    if (!SaveCredentialIDs()) {
        otErr << __FUNCTION__ << ": Failed to save credential lists."
              << std::endl;

        return false;
    }

    return true;
}

std::unique_ptr<proto::ContactData> Nym::ContactData() const
{
    std::unique_ptr<proto::ContactData> contactData;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            it.second->GetContactData(contactData);

            break;
        }
    }

    return contactData;
}

std::unique_ptr<proto::VerificationSet> Nym::VerificationSet() const
{
    std::unique_ptr<proto::VerificationSet> verificationSet;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            it.second->GetVerificationSet(verificationSet);
        }
    }

    return verificationSet;
}

bool Nym::SetContactData(const proto::ContactData& data)
{
    std::list<std::string> revokedIDs;
    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            it.second->RevokeContactCredentials(revokedIDs);
        }
    }

    for (auto& it : revokedIDs) {
        m_listRevokedIDs.push_back(it);
    }

    bool added = false;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            if (it.second->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                it.second->AddContactCredential(data);
                added = true;

                break;
            }
        }
    }

    if (added) {
        if (VerifyPseudonym()) {
            revision_++;
            SaveCredentialIDs();

            return true;
        }
    }

    return false;
}

bool Nym::SetVerificationSet(const proto::VerificationSet& data)
{
    std::list<std::string> revokedIDs;
    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            it.second->RevokeVerificationCredentials(revokedIDs);
        }
    }

    for (auto& it : revokedIDs) {
        m_listRevokedIDs.push_back(it);
    }

    bool added = false;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            if (it.second->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                it.second->AddVerificationCredential(data);
                added = true;

                break;
            }
        }
    }

    if (added) {
        if (VerifyPseudonym()) {
            revision_++;
            SaveCredentialIDs();

            return true;
        }
    }

    return false;
}

bool Nym::Verify(const Data& plaintext, const proto::Signature& sig) const
{
    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            if (it.second->Verify(plaintext, sig)) {
                return true;
            }
        }
    }

    return false;
}

zcert_t* Nym::TransportKey() const
{
    OTPassword privateKey;
    Data publicKey;

    bool generated = false;
    zcert_t* output = nullptr;

    for (auto& it : m_mapCredentialSets) {
        OT_ASSERT(nullptr != it.second);

        if (nullptr != it.second) {
            const CredentialSet& credSet = *it.second;

            if (credSet.TransportKey(publicKey, privateKey)) {
                generated = true;
                break;
            }
        }
    }

    if (generated) {
        output = zcert_new_from(
            static_cast<unsigned char*>(
                const_cast<void*>(publicKey.GetPointer())),
            static_cast<unsigned char*>(privateKey.getMemoryWritable()));
    }

    return output;
}

bool Nym::hasCapability(const NymCapability& capability) const
{
    for (auto& it : m_mapCredentialSets) {
        OT_ASSERT(nullptr != it.second);

        if (nullptr != it.second) {
            const CredentialSet& credSet = *it.second;

            if (credSet.hasCapability(capability)) {
                return true;
            }
        }
    }

    return false;
}

std::string Nym::AddChildKeyCredential(
    const Identifier& masterID,
    const NymParameters& nymParameters)
{
    std::string output;
    std::string master = String(masterID).Get();
    auto it = m_mapCredentialSets.find(master);
    const bool noMaster = (it == m_mapCredentialSets.end());

    if (noMaster) {
        otErr << __FUNCTION__ << ": master ID not found." << std::endl;

        return output;
    }

    if (it->second) {
        output = it->second->AddChildKeyCredential(nymParameters);
    }

    return output;
}

std::string Nym::Alias() const { return alias_; }

bool Nym::SetAlias(const std::string& alias)
{
    alias_ = alias;
    revision_++;

    if (SaveCredentialIDs()) {

        return OT::App().Contract().SetNymAlias(m_nymID, alias);
    }

    return false;
}

std::uint64_t Nym::Revision() const { return revision_.load(); }

std::string Nym::PaymentCode() const
{
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    if (!source_) {
        return "";
    }

    if (proto::SOURCETYPE_BIP47 != source_->Type()) {
        return "";
    }

    auto serialized = source_->Serialize();

    if (!serialized) {
        return "";
    }

    class PaymentCode paymentCode(serialized->paymentcode());

    return paymentCode.asBase58();

#else
    return "";
#endif
}
}  // namespace opentxs
