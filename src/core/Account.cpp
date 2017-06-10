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

#include "opentxs/core/Account.hpp"

#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTDataFolder.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Helpers.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"

#include <inttypes.h>
#include <stdint.h>
#include <fstream>
#include <irrxml/irrXML.hpp>
#include <memory>
#include <string>

using namespace irr;
using namespace io;

namespace opentxs
{

char const* const __TypeStringsAccount[] = {
    "user",       // used by users
    "issuer",     // used by issuers    (these can only go negative.)
    "basket",     // issuer acct used by basket currencies (these can only go
                  // negative)
    "basketsub",  // used by the server (to store backing reserves for basket
                  // sub-accounts)
    "mint",       // used by mints (to store backing reserves for cash)
    "voucher",    // used by the server (to store backing reserves for vouchers)
    "stash",  // used by the server (to store backing reserves for stashes, for
              // smart contracts.)
    "err_acct"};

// Used for generating accounts, thus no accountID needed.
Account::Account(const Identifier& nymID, const Identifier& notaryID)
    : OTTransactionType()
    , stashTransNum_(0)
    , markForDeletion_(false)
{
    InitAccount();
    SetNymID(nymID);
    SetRealNotaryID(notaryID);
    SetPurportedNotaryID(notaryID);
}

Account::Account()
    : OTTransactionType()
    , stashTransNum_(0)
    , markForDeletion_(false)
{
    InitAccount();
}

Account::Account(
    const Identifier& nymID,
    const Identifier& accountId,
    const Identifier& notaryID,
    const String& name)
    : OTTransactionType(nymID, accountId, notaryID)
    , stashTransNum_(0)
    , markForDeletion_(false)
{
    InitAccount();
    m_strName = name;
}

Account::Account(
    const Identifier& nymID,
    const Identifier& accountId,
    const Identifier& notaryID)
    : OTTransactionType(nymID, accountId, notaryID)
    , stashTransNum_(0)
    , markForDeletion_(false)
{
    InitAccount();
}

Account::~Account() { Release_Account(); }

char const* Account::_GetTypeString(AccountType accountType)
{
    int32_t index = static_cast<int32_t>(accountType);
    return __TypeStringsAccount[index];
}

// Caller responsible to delete.
Ledger* Account::LoadInbox(const Nym& nym) const
{
    auto* box = new Ledger(GetNymID(), GetRealAccountID(), GetRealNotaryID());
    OT_ASSERT(box != nullptr);

    if (box->LoadInbox() && box->VerifyAccount(nym)) {
        return box;
    }

    String strNymID(GetNymID()), strAcctID(GetRealAccountID());
    otInfo << "Unable to load or verify inbox:\n"
           << strAcctID << "\n For user:\n"
           << strNymID << "\n";
    return nullptr;
}

// Caller responsible to delete.
Ledger* Account::LoadOutbox(const Nym& nym) const
{
    auto* box = new Ledger(GetNymID(), GetRealAccountID(), GetRealNotaryID());
    OT_ASSERT(nullptr != box);

    if (box->LoadOutbox() && box->VerifyAccount(nym)) {
        return box;
    }

    String strNymID(GetNymID()), strAcctID(GetRealAccountID());
    otInfo << "Unable to load or verify outbox:\n"
           << strAcctID << "\n For user:\n"
           << strNymID << "\n";
    return nullptr;
}

// hash is optional, the account will update its internal copy of the hash
// anyway.
bool Account::SaveInbox(Ledger& box, Identifier* hash)
{
    if (!IsSameAccount(box)) {
        String strAcctID(GetRealAccountID());
        String strNotaryID(GetRealNotaryID());
        String strBoxAcctID(box.GetRealAccountID());
        String strBoxSvrID(box.GetRealNotaryID());
        otErr << "OTAccount::SaveInbox: ERROR: The ledger passed in, isn't "
                 "even for this account!\n"
                 "   Acct ID: "
              << strAcctID << "\n  Other ID: " << strBoxAcctID
              << "\n Notary ID: " << strNotaryID
              << "\n Other ID: " << strBoxSvrID << "\n";
        return false;
    }

    Identifier theHash;
    if (hash == nullptr) hash = &theHash;

    bool success = box.SaveInbox(hash);

    if (success) SetInboxHash(*hash);

    return success;
}

// hash is optional, the account will update its internal copy of the hash
// anyway. If you pass the identifier in, the hash is recorded there.
bool Account::SaveOutbox(Ledger& box, Identifier* hash)
{
    if (!IsSameAccount(box)) {
        String strAcctID(GetRealAccountID());
        String strNotaryID(GetRealNotaryID());
        String strBoxAcctID(box.GetRealAccountID());
        String strBoxSvrID(box.GetRealNotaryID());
        otErr << "OTAccount::SaveOutbox: ERROR: The ledger passed in, isn't "
                 "even for this account!\n"
                 "   Acct ID: "
              << strAcctID << "\n  Other ID: " << strBoxAcctID
              << "\n Notary ID: " << strNotaryID
              << "\n Other ID: " << strBoxSvrID << "\n";
        return false;
    }

    Identifier theHash;
    if (hash == nullptr) hash = &theHash;

    bool success = box.SaveOutbox(hash);

    if (success) SetOutboxHash(*hash);

    return success;
}

void Account::SetInboxHash(const Identifier& input) { inboxHash_ = input; }

bool Account::GetInboxHash(Identifier& output)
{
    output.Release();

    if (!inboxHash_.IsEmpty()) {
        output = inboxHash_;
        return true;
    } else if (
        !GetNymID().IsEmpty() && !GetRealAccountID().IsEmpty() &&
        !GetRealNotaryID().IsEmpty()) {
        Ledger inbox(GetNymID(), GetRealAccountID(), GetRealNotaryID());

        if (inbox.LoadInbox() && inbox.CalculateInboxHash(output)) {
            SetInboxHash(output);
            return true;
        }
    }

    return false;
}

void Account::SetOutboxHash(const Identifier& input) { outboxHash_ = input; }

bool Account::GetOutboxHash(Identifier& output)
{
    output.Release();

    if (!outboxHash_.IsEmpty()) {
        output = outboxHash_;
        return true;
    } else if (
        !GetNymID().IsEmpty() && !GetRealAccountID().IsEmpty() &&
        !GetRealNotaryID().IsEmpty()) {
        Ledger outbox(GetNymID(), GetRealAccountID(), GetRealNotaryID());

        if (outbox.LoadOutbox() && outbox.CalculateOutboxHash(output)) {
            SetOutboxHash(output);
            return true;
        }
    }

    return false;
}

// TODO:  add an override so that OTAccount, when it loads up, it performs the
// check to see the NotaryID, look at the Server Contract and make sure the
// server hashes match.
//
// TODO: override "Verify". Have some way to verify a specific Nym to a specific
// account.
//
// Overriding this so I can set the filename automatically inside based on ID.
bool Account::LoadContract()
{
    String id;
    GetIdentifier(id);
    return Contract::LoadContract(OTFolders::Account().Get(), id.Get());
}

bool Account::SaveAccount()
{
    String id;
    GetIdentifier(id);
    return SaveContract(OTFolders::Account().Get(), id.Get());
}

// Debit a certain amount from the account (presumably the same amount is being
// credited somewhere else)
bool Account::Debit(const int64_t& amount)
{
    int64_t oldBalance = balanceAmount_.ToLong();
    // The MINUS here is the big difference between Debit and Credit
    int64_t newBalance = oldBalance - amount;

    // fail if integer overflow
    if ((amount > 0 && oldBalance < INT64_MIN + amount) ||
        (amount < 0 && oldBalance > INT64_MAX + amount))
        return false;

    // This is where issuer accounts get a pass. They just go negative.
    //
    // IF the new balance is less than zero...
    // AND it's a normal account... (not an issuer)
    // AND the new balance is even less than the old balance...
    // THEN FAIL. The "new less than old" requirement is recent,
    if (newBalance < 0 && !IsAllowedToGoNegative() && newBalance < oldBalance) {
        return false;
    }
    // and it means that we now allow <0 debits on normal accounts,
    // AS LONG AS the result is a HIGHER BALANCE  :-)
    else {
        balanceAmount_.Format("%" PRId64, newBalance);
        balanceDate_.Set(String(getTimestamp()));
        return true;
    }
}

// Credit a certain amount to the account (presumably the same amount is being
// debited somewhere else)
bool Account::Credit(const int64_t& amount)
{
    int64_t oldBalance = balanceAmount_.ToLong();
    // The PLUS here is the big difference between Debit and Credit.
    int64_t newBalance = oldBalance + amount;

    // fail if integer overflow
    if ((amount > 0 && oldBalance > INT64_MAX - amount) ||
        (amount < 0 && oldBalance < INT64_MIN - amount))
        return false;

    // If the balance gets too big, it may flip to negative due to us using
    // int64_t int32_t.
    // We'll maybe explicitly check that it's not negative in order to prevent
    // that. TODO.
    //    if (newBalance > 0 || (OTAccount::user != acctType_))
    //    {
    //        balanceAmount_.Format("%" PRId64 "", newBalance);
    //        return true;
    //    }

    // This is where issuer accounts get a pass. They just go negative.
    // IF the new balance is less than zero...
    // AND it's a normal account... (not an issuer)
    // AND the new balance is even less than the old balance...
    // THEN FAIL. The "new less than old" requirement is recent,
    if (newBalance < 0 && !IsAllowedToGoNegative() && newBalance < oldBalance) {
        return false;
    }
    // and it means that we now allow <0 credits on normal accounts,
    // AS LONG AS the result is a HIGHER BALANCE  :-)
    else {
        balanceAmount_.Format("%" PRId64, newBalance);
        balanceDate_.Set(String(getTimestamp()));
        return true;
    }
}

const Identifier& Account::GetInstrumentDefinitionID() const
{
    return acctInstrumentDefinitionID_;
}

void Account::InitAccount()
{
    m_strContractType = "ACCOUNT";
    acctType_ = Account::user;
}

// Verify Contract ID first, THEN Verify Owner.
// Because we use the ID in this function, so make sure that it is verified
// before calling this.
bool Account::VerifyOwner(const Nym& candidate) const
{
    Identifier ID_CANDIDATE;
    // ID_CANDIDATE now contains the ID of the Nym we're testing.
    candidate.GetIdentifier(ID_CANDIDATE);
    return m_AcctNymID == ID_CANDIDATE;
}

// TODO: when entities and roles are added, probably more will go here.
bool Account::VerifyOwnerByID(const Identifier& nymId) const
{
    return nymId == m_AcctNymID;
}

// Let's say you don't have or know the NymID, and you just want to load the
// damn thing up.
// Then call this function. It will set nymID and server ID for you.
Account* Account::LoadExistingAccount(
    const Identifier& accountId,
    const Identifier& notaryID)
{
    bool folderAlreadyExist = false;
    bool folderIsNew = false;

    String strDataFolder = "";
    String strAccountPath = "";
    if (!OTDataFolder::Get(strDataFolder)) {
        OT_FAIL;
    }
    if (!OTPaths::AppendFolder(
            strAccountPath, strDataFolder, OTFolders::Account())) {
        OT_FAIL;
    }

    if (!OTPaths::ConfirmCreateFolder(
            strAccountPath, folderAlreadyExist, folderIsNew)) {
        otErr << "Unable to find or create accounts folder: "
              << OTFolders::Account() << "\n";
        return nullptr;
    }

    Account* account = new Account;
    OT_ASSERT(account != nullptr);

    account->SetRealAccountID(accountId);
    account->SetRealNotaryID(notaryID);

    String strAcctID(accountId);

    account->m_strFoldername = OTFolders::Account().Get();
    account->m_strFilename = strAcctID.Get();

    if (!OTDB::Exists(
            account->m_strFoldername.Get(), account->m_strFilename.Get())) {
        otInfo << "OTAccount::LoadExistingAccount: File does not exist: "
               << account->m_strFoldername << Log::PathSeparator()
               << account->m_strFilename << "\n";
        delete account;
        return nullptr;
    }

    if (account->LoadContract() && account->VerifyContractID()) {
        return account;
    }

    delete account;
    return nullptr;
}

Account* Account::GenerateNewAccount(
    const Identifier& nymID,
    const Identifier& notaryID,
    const Nym& serverNym,
    const Message& message,
    Account::AccountType acctType,
    int64_t stashTransNum)
{
    Account* account = new Account(nymID, notaryID);

    if (account) {
        // This is only for stash accounts.
        if (account->GenerateNewAccount(
                serverNym, message, acctType, stashTransNum)) {
            return account;
        }

        delete account;
        account = nullptr;
    }

    return nullptr;
}

/*
 Just make sure message has these members populated:
message.m_strNymID;
message.m_strInstrumentDefinitionID;
message.m_strNotaryID;
 */
bool Account::GenerateNewAccount(
    const Nym& server,
    const Message& message,
    Account::AccountType acctType,
    int64_t stashTransNum)
{
    // First we generate a secure random number into a binary object...
    Data payload;
    // TODO: hardcoding. Plus: is 100 bytes of random a little much here?
    if (!payload.Randomize(100)) {
        otErr << __FUNCTION__ << ": Failed trying to acquire random numbers.\n";
        return false;
    }

    // Next we calculate that binary object into a message digest (an
    // OTIdentifier).
    Identifier newID;
    if (!newID.CalculateDigest(payload)) {
        otErr << __FUNCTION__ << ": Error generating new account ID.\n";
        return false;
    }

    // Next we get that digest (which is a binary hash number)
    // and extract a human-readable standard string format of that hash,
    // into an OTString.
    String strID(newID);

    // Set the account number based on what we just generated.
    SetRealAccountID(newID);
    // Might as well set them both. (Safe here to do so, for once.)
    SetPurportedAccountID(newID);
    // So it's not blank. The user can always change it.
    m_strName.Set(strID);

    // Next we create the full path filename for the account using the ID.
    m_strFoldername = OTFolders::Account().Get();
    m_strFilename = strID.Get();

    // Then we try to load it, in order to make sure that it doesn't already
    // exist.
    if (OTDB::Exists(m_strFoldername.Get(), m_strFilename.Get())) {
        otErr << __FUNCTION__ << ": Account already exists: " << m_strFilename
              << "\n";
        return false;
    }

    // Set up the various important starting values of the account.
    // Account type defaults to OTAccount::user.
    // But there are also issuer accts.
    acctType_ = acctType;

    // basket, basketsub, mint, voucher, and stash
    // accounts are all "owned" by the server.
    if (IsInternalServerAcct()) {
        server.GetIdentifier(m_AcctNymID);
    } else {
        m_AcctNymID.SetString(message.m_strNymID);
    }

    acctInstrumentDefinitionID_.SetString(message.m_strInstrumentDefinitionID);

    otLog3 << __FUNCTION__ << ": Creating new account, type:\n"
           << message.m_strInstrumentDefinitionID << "\n";

    Identifier notaryID(message.m_strNotaryID);
    // TODO: this assumes the notaryID on the message
    // is correct. It's vetted, but still...
    SetRealNotaryID(notaryID);
    SetPurportedNotaryID(notaryID);

    balanceDate_.Set(String(getTimestamp()));
    balanceAmount_.Set("0");

    if (IsStashAcct()) {
        OT_ASSERT_MSG(
            stashTransNum > 0,
            "You created a stash account, but "
            "with a zero-or-negative transaction "
            "number for its cron item.");
        stashTransNum_ = stashTransNum;
    }

    // Sign the Account (so we know that we did)... Otherwise someone could put
    // a fake
    // account file on the server if the code wasn't designed to verify the
    // signature on the
    // account.
    SignContract(server);
    SaveContract();

    // Save the Account to storage (based on its ID.)
    SaveAccount();

    // Don't know why I had this here. Putting SaveAccount() instead.
    //    OTString strFilename(m_strFilename);
    //    SaveContract(strFilename.Get()); // Saves the account to a specific
    // filename

    // No need to create the inbox and outbox ledgers...they will be created
    // automatically if they do not exist when they are needed.

    return true;
}

int64_t Account::GetBalance() const
{
    if (balanceAmount_.Exists()) {
        return balanceAmount_.ToLong();
    }
    return 0;
}

bool Account::DisplayStatistics(String& contents) const
{
    String strAccountID(GetPurportedAccountID());
    String strNotaryID(GetPurportedNotaryID());
    String strNymID(GetNymID());
    String strInstrumentDefinitionID(acctInstrumentDefinitionID_);

    String acctType;
    TranslateAccountTypeToString(acctType_, acctType);

    contents.Concatenate(
        " Asset Account (%s) Name: %s\n"
        " Last retrieved Balance: %s  on date: %s\n"
        " accountID: %s\n"
        " nymID: %s\n"
        " notaryID: %s\n"
        " instrumentDefinitionID: %s\n"
        "\n",
        acctType.Get(),
        m_strName.Get(),
        balanceAmount_.Get(),
        balanceDate_.Get(),
        strAccountID.Get(),
        strNymID.Get(),
        strNotaryID.Get(),
        strInstrumentDefinitionID.Get());

    return true;
}

bool Account::SaveContractWallet(Tag& parent) const
{
    String strAccountID(GetPurportedAccountID());
    String strNotaryID(GetPurportedNotaryID());
    String strNymID(GetNymID());
    String strInstrumentDefinitionID(acctInstrumentDefinitionID_);

    String acctType;
    TranslateAccountTypeToString(acctType_, acctType);

    // Name is in the clear in memory,
    // and base64 in storage.
    OTASCIIArmor ascName;
    if (m_strName.Exists()) {
        ascName.SetString(m_strName, false);  // linebreaks == false
    }

    TagPtr pTag(new Tag("account"));

    pTag->add_attribute("name", m_strName.Exists() ? ascName.Get() : "");
    pTag->add_attribute("accountID", strAccountID.Get());
    pTag->add_attribute("nymID", strNymID.Get());
    pTag->add_attribute("notaryID", strNotaryID.Get());

    // These are here for informational purposes only,
    // and are not ever actually loaded back up. In the
    // previous version of this code, they were written
    // only as XML comments.
    pTag->add_attribute("infoLastKnownBalance", balanceAmount_.Get());
    pTag->add_attribute("infoDateOfLastBalance", balanceDate_.Get());
    pTag->add_attribute("infoAccountType", acctType.Get());
    pTag->add_attribute(
        "infoInstrumentDefinitionID", strInstrumentDefinitionID.Get());

    parent.add_tag(pTag);

    return true;
}

// Most contracts do not override this function...
// But OTAccount does, because IF THE SIGNER has chosen to SIGN the account
// based on the current balances, then we need to update the m_xmlUnsigned
// member with the current balances and other updated information before the
// signing occurs. (Presumably this is the whole reason why the account is
// being re-signed.)
//
// Normally, in other Contract and derived classes, m_xmlUnsigned is read
// from the file and then kept read-only, since contracts do not normally
// change. But as accounts change in balance, they must be re-signed to keep the
// signatures valid.
void Account::UpdateContents()
{
    String strAssetTYPEID(acctInstrumentDefinitionID_);
    String ACCOUNT_ID(GetPurportedAccountID());
    String NOTARY_ID(GetPurportedNotaryID());
    String NYM_ID(GetNymID());

    String acctType;
    TranslateAccountTypeToString(acctType_, acctType);

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    Tag tag("account");

    tag.add_attribute("version", m_strVersion.Get());
    tag.add_attribute("type", acctType.Get());
    tag.add_attribute("accountID", ACCOUNT_ID.Get());
    tag.add_attribute("nymID", NYM_ID.Get());
    tag.add_attribute("notaryID", NOTARY_ID.Get());
    tag.add_attribute("instrumentDefinitionID", strAssetTYPEID.Get());

    if (IsStashAcct()) {
        TagPtr tagStash(new Tag("stashinfo"));
        tagStash->add_attribute("cronItemNum", formatLong(stashTransNum_));
        tag.add_tag(tagStash);
    }
    if (!inboxHash_.IsEmpty()) {
        String strHash(inboxHash_);
        TagPtr tagBox(new Tag("inboxHash"));
        tagBox->add_attribute("value", strHash.Get());
        tag.add_tag(tagBox);
    }
    if (!outboxHash_.IsEmpty()) {
        String strHash(outboxHash_);
        TagPtr tagBox(new Tag("outboxHash"));
        tagBox->add_attribute("value", strHash.Get());
        tag.add_tag(tagBox);
    }

    TagPtr tagBalance(new Tag("balance"));

    tagBalance->add_attribute("date", balanceDate_.Get());
    tagBalance->add_attribute("amount", balanceAmount_.Get());

    tag.add_tag(tagBalance);

    if (markForDeletion_) {
        tag.add_tag(
            "MARKED_FOR_DELETION",
            "THIS ACCOUNT HAS BEEN MARKED FOR DELETION AT ITS OWN REQUEST");
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t Account::ProcessXMLNode(IrrXMLReader*& xml)
{
    int32_t retval = 0;

    String strNodeName(xml->getNodeName());

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    // if (retval = OTTransactionType::ProcessXMLNode(xml))
    //    return retval;

    if (strNodeName.Compare("account")) {
        String acctType;

        m_strVersion = xml->getAttributeValue("version");
        acctType = xml->getAttributeValue("type");

        if (!acctType.Exists()) {
            otErr << "OTAccount::ProcessXMLNode: Failed: Empty account "
                     "'type' attribute.\n";
            return -1;
        }

        acctType_ = TranslateAccountTypeStringToEnum(acctType);

        if (Account::err_acct == acctType_) {
            otErr << "OTAccount::ProcessXMLNode: Failed: account 'type' "
                     "attribute contains unknown value.\n";
            return -1;
        }

        String strAcctAssetType =
            xml->getAttributeValue("instrumentDefinitionID");

        if (strAcctAssetType.Exists()) {
            acctInstrumentDefinitionID_.SetString(strAcctAssetType);
        }
        String strAccountID(xml->getAttributeValue("accountID"));
        String strNotaryID(xml->getAttributeValue("notaryID"));
        String strAcctNymID(xml->getAttributeValue("nymID"));

        Identifier ACCOUNT_ID(strAccountID);
        Identifier NOTARY_ID(strNotaryID);
        Identifier NYM_ID(strAcctNymID);

        SetPurportedAccountID(ACCOUNT_ID);
        SetPurportedNotaryID(NOTARY_ID);
        SetNymID(NYM_ID);

        String strInstrumentDefinitionID(acctInstrumentDefinitionID_);
        otLog3 << "\n\nAccount Type: " << acctType
               << "\nAccountID: " << strAccountID << "\nNymID: " << strAcctNymID
               << "\n"
                  "InstrumentDefinitionID: "
               << strInstrumentDefinitionID << "\nNotaryID: " << strNotaryID
               << "\n";

        retval = 1;
    } else if (strNodeName.Compare("inboxHash")) {

        String strHash = xml->getAttributeValue("value");
        if (strHash.Exists()) {
            inboxHash_.SetString(strHash);
        }
        otLog3 << "Account inboxHash: " << strHash << "\n";

        retval = 1;
    } else if (strNodeName.Compare("outboxHash")) {

        String strHash = xml->getAttributeValue("value");
        if (strHash.Exists()) {
            outboxHash_.SetString(strHash);
        }
        otLog3 << "Account outboxHash: " << strHash << "\n";

        retval = 1;
    } else if (strNodeName.Compare("MARKED_FOR_DELETION")) {
        markForDeletion_ = true;
        otLog3 << "This asset account has been MARKED_FOR_DELETION (at some "
                  "point prior.)\n";

        retval = 1;
    } else if (strNodeName.Compare("balance")) {
        balanceDate_ = xml->getAttributeValue("date");
        balanceAmount_ = xml->getAttributeValue("amount");

        // I convert to integer / int64_t and back to string.
        // (Just an easy way to keep the data clean.)

        time64_t date = parseTimestamp((balanceDate_.Get()));
        int64_t amount = balanceAmount_.ToLong();

        balanceDate_.Set(String(formatTimestamp(date)));
        balanceAmount_.Format("%" PRId64, amount);

        otLog3 << "\nBALANCE  --  " << balanceAmount_ << "\nDATE     --  "
               << balanceDate_ << "\n";

        retval = 1;
    } else if (strNodeName.Compare("stashinfo")) {
        if (!IsStashAcct()) {
            otErr << "OTAccount::ProcessXMLNode: Error: Encountered stashinfo "
                     "tag while loading NON-STASH account. \n";
            return -1;
        }

        int64_t lTransNum = 0;
        String strStashTransNum = xml->getAttributeValue("cronItemNum");
        if (!strStashTransNum.Exists() ||
            ((lTransNum = strStashTransNum.ToLong()) <= 0)) {
            stashTransNum_ = 0;
            otErr << "OTAccount::ProcessXMLNode: Error: Bad transaction number "
                     "for supposed corresponding cron item: "
                  << lTransNum << " \n";
            return -1;
        } else {
            stashTransNum_ = lTransNum;
        }

        otLog3 << "\nSTASH INFO:   CronItemNum     --  " << stashTransNum_
               << "\n";

        retval = 1;
    }

    return retval;
}

bool Account::IsInternalServerAcct() const
{
    switch (acctType_) {
        case Account::user:
        case Account::issuer:
            return false;
        case Account::basket:
        case Account::basketsub:
        case Account::mint:
        case Account::voucher:
        case Account::stash:
            return true;
        default:
            otErr << "OTAccount::IsInternalServerAcct: Unknown account type.\n";
            return false;
    }
    return false;
}

bool Account::IsOwnedByUser() const
{
    switch (acctType_) {
        case Account::user:
        case Account::issuer:
            return true;
        case Account::basket:
        case Account::basketsub:
        case Account::mint:
        case Account::voucher:
        case Account::stash:
            return false;
        default:
            otErr << "OTAccount::IsOwnedByUser: Unknown account type.\n";
            return false;
    }
    return false;
}

bool Account::IsOwnedByEntity() const { return false; }

bool Account::IsIssuer() const { return Account::issuer == acctType_; }

bool Account::IsAllowedToGoNegative() const
{
    switch (acctType_) {
        // issuer acct controlled by a user
        case Account::issuer:
        // basket issuer acct controlled by the server (for a basket currency)
        case Account::basket:
            return true;
        // user asset acct
        case Account::user:
        // internal server acct for storing reserves for basket sub currencies
        case Account::basketsub:
        // internal server acct for storing reserves for cash withdrawals
        case Account::mint:
        // internal server acct for storing reserves for
        // vouchers (like cashier's cheques)
        case Account::voucher:
        // internal server acct for storing reserves for
        // smart contract stashes. (Money stashed IN the contract.)
        case Account::stash:
            return false;
        default:
            otErr
                << "OTAccount::IsAllowedToGoNegative: Unknown account type.\n";
            return false;
    }
    return false;
}

void Account::Release_Account()
{
    balanceDate_.Release();
    balanceAmount_.Release();
    inboxHash_.Release();
    outboxHash_.Release();
}

void Account::Release()
{
    Release_Account();
    OTTransactionType::Release();
}

}  // namespace opentxs
