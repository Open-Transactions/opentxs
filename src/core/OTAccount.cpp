/************************************************************
 *
 *  OTAccount.cpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/OTAccount.hpp>
#include <opentxs/core/crypto/OTCrypto.hpp>
#include <opentxs/core/util/OTDataFolder.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/OTLedger.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTMessage.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/util/OTPaths.hpp>
#include <opentxs/core/OTData.hpp>
#include <opentxs/core/OTPseudonym.hpp>

#include "Helpers.hpp"

#include <irrxml/irrXML.hpp>

#include <fstream>

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#endif

using namespace irr;
using namespace io;

namespace opentxs
{

char const* const __TypeStringsAccount[] = {
    "simple",    // used by users
    "issuer",    // used by issuers    (these can only go negative.)
    "basket",    // issuer acct used by basket currencies (these can only go
                 // negative)
    "basketsub", // used by the server (to store backing reserves for basket
                 // sub-accounts)
    "mint",      // used by mints (to store backing reserves for cash)
    "voucher",   // used by the server (to store backing reserves for vouchers)
    "stash", // used by the server (to store backing reserves for stashes, for
             // smart contracts.)
    "err_acct"};

// Used for generating accounts, thus no accountID needed.
OTAccount::OTAccount(const OTIdentifier& userId, const OTIdentifier& serverId)
    : OTTransactionType()
    , stashTransNum_(0)
    , markForDeletion_(false)
{
    InitAccount();
    SetUserID(userId);
    SetRealServerID(serverId);
    SetPurportedServerID(serverId);
}

OTAccount::OTAccount()
    : OTTransactionType()
    , stashTransNum_(0)
    , markForDeletion_(false)
{
    InitAccount();
}

OTAccount::OTAccount(const OTIdentifier& userId, const OTIdentifier& accountId,
                     const OTIdentifier& serverId, const OTString& name)
    : OTTransactionType(userId, accountId, serverId)
    , stashTransNum_(0)
    , markForDeletion_(false)
{
    InitAccount();
    m_strName = name;
}

OTAccount::OTAccount(const OTIdentifier& userId, const OTIdentifier& accountId,
                     const OTIdentifier& serverId)
    : OTTransactionType(userId, accountId, serverId)
    , stashTransNum_(0)
    , markForDeletion_(false)
{
    InitAccount();
}

OTAccount::~OTAccount()
{
    Release_Account();
}

char const* OTAccount::_GetTypeString(AccountType accountType)
{
    int32_t index = static_cast<int32_t>(accountType);
    return __TypeStringsAccount[index];
}

// Caller responsible to delete.
OTLedger* OTAccount::LoadInbox(OTPseudonym& nym) const
{
    auto* box =
        new OTLedger(GetUserID(), GetRealAccountID(), GetRealServerID());
    OT_ASSERT(box != nullptr);

    if (box->LoadInbox() && box->VerifyAccount(nym)) {
        return box;
    }

    OTString strUserID(GetUserID()), strAcctID(GetRealAccountID());
    otInfo << "Unable to load or verify inbox:\n" << strAcctID
           << "\n For user:\n" << strUserID << "\n";
    return nullptr;
}

// Caller responsible to delete.
OTLedger* OTAccount::LoadOutbox(OTPseudonym& nym) const
{
    auto* box =
        new OTLedger(GetUserID(), GetRealAccountID(), GetRealServerID());
    OT_ASSERT(nullptr != box);

    if (box->LoadOutbox() && box->VerifyAccount(nym)) {
        return box;
    }

    OTString strUserID(GetUserID()), strAcctID(GetRealAccountID());
    otInfo << "Unable to load or verify outbox:\n" << strAcctID
           << "\n For user:\n" << strUserID << "\n";
    return nullptr;
}

// hash is optional, the account will update its internal copy of the hash
// anyway.
bool OTAccount::SaveInbox(OTLedger& box, OTIdentifier* hash)
{
    if (!IsSameAccount(box)) {
        OTString strAcctID(GetRealAccountID());
        OTString strServerID(GetRealServerID());
        OTString strBoxAcctID(box.GetRealAccountID());
        OTString strBoxSvrID(box.GetRealServerID());
        otErr << "OTAccount::SaveInbox: ERROR: The ledger passed in, isn't "
                 "even for this account!\n"
                 "   Acct ID: " << strAcctID << "\n  Other ID: " << strBoxAcctID
              << "\n Server ID: " << strServerID
              << "\n Other ID: " << strBoxSvrID << "\n";
        return false;
    }

    OTIdentifier theHash;
    if (hash == nullptr) hash = &theHash;

    bool success = box.SaveInbox(hash);

    if (success) SetInboxHash(*hash);

    return success;
}

// hash is optional, the account will update its internal copy of the hash
// anyway. If you pass the identifier in, the hash is recorded there.
bool OTAccount::SaveOutbox(OTLedger& box, OTIdentifier* hash)
{
    if (!IsSameAccount(box)) {
        OTString strAcctID(GetRealAccountID());
        OTString strServerID(GetRealServerID());
        OTString strBoxAcctID(box.GetRealAccountID());
        OTString strBoxSvrID(box.GetRealServerID());
        otErr << "OTAccount::SaveOutbox: ERROR: The ledger passed in, isn't "
                 "even for this account!\n"
                 "   Acct ID: " << strAcctID << "\n  Other ID: " << strBoxAcctID
              << "\n Server ID: " << strServerID
              << "\n Other ID: " << strBoxSvrID << "\n";
        return false;
    }

    OTIdentifier theHash;
    if (hash == nullptr) hash = &theHash;

    bool success = box.SaveOutbox(hash);

    if (success) SetOutboxHash(*hash);

    return success;
}

void OTAccount::SetInboxHash(const OTIdentifier& input)
{
    inboxHash_ = input;
}

bool OTAccount::GetInboxHash(OTIdentifier& output)
{
    output.clear();

    if (!inboxHash_.empty()) {
        output = inboxHash_;
        return true;
    }
    else if (!GetUserID().empty() && !GetRealAccountID().empty() &&
               !GetRealServerID().empty()) {
        OTLedger inbox(GetUserID(), GetRealAccountID(), GetRealServerID());

        if (inbox.LoadInbox() && inbox.CalculateInboxHash(output)) {
            SetInboxHash(output);
            return true;
        }
    }

    return false;
}

void OTAccount::SetOutboxHash(const OTIdentifier& input)
{
    outboxHash_ = input;
}

bool OTAccount::GetOutboxHash(OTIdentifier& output)
{
    output.clear();

    if (!outboxHash_.empty()) {
        output = outboxHash_;
        return true;
    }
    else if (!GetUserID().empty() && !GetRealAccountID().empty() &&
               !GetRealServerID().empty()) {
        OTLedger outbox(GetUserID(), GetRealAccountID(), GetRealServerID());

        if (outbox.LoadOutbox() && outbox.CalculateOutboxHash(output)) {
            SetOutboxHash(output);
            return true;
        }
    }

    return false;
}

// TODO:  add an override so that OTAccount, when it loads up, it performs the
// check to see the ServerID, look at the Server Contract and make sure the
// server hashes match.
//
// TODO: override "Verify". Have some way to verify a specific Nym to a specific
// account.
//
// Overriding this so I can set the filename automatically inside based on ID.
bool OTAccount::LoadContract()
{
    OTString id;
    GetIdentifier(id);
    return OTContract::LoadContract(OTFolders::Account().Get(), id.Get());
}

bool OTAccount::SaveAccount()
{
    OTString id;
    GetIdentifier(id);
    return SaveContract(OTFolders::Account().Get(), id.Get());
}

// Debit a certain amount from the account (presumably the same amount is being
// credited somewhere else)
bool OTAccount::Debit(const int64_t& amount)
{
    int64_t oldBalance = atol(balanceAmount_.Get());
    // The MINUS here is the big difference between Debit and Credit
    int64_t newBalance = oldBalance - amount;

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
        balanceAmount_.Format("%lld", newBalance);
        time64_t t = OTTimeGetCurrentTime(); // Today, now.
        balanceDate_.Format("%d", t);
        return true;
    }
}

// Credit a certain amount to the account (presumably the same amount is being
// debited somewhere else)
bool OTAccount::Credit(const int64_t& amount)
{
    int64_t oldBalance = atol(balanceAmount_.Get());
    // The PLUS here is the big difference between Debit and Credit.
    int64_t newBalance = oldBalance + amount;

    // If the balance gets too big, it may flip to negative due to us using
    // int64_t int32_t.
    // We'll maybe explicitly check that it's not negative in order to prevent
    // that. TODO.
    //    if (newBalance > 0 || (OTAccount::simple != acctType_))
    //    {
    //        balanceAmount_.Format("%lld", newBalance);
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
        balanceAmount_.Format("%lld", newBalance);
        time64_t t = OTTimeGetCurrentTime(); // Today, now.
        balanceDate_.Format("%d", t);
        return true;
    }
}

const OTIdentifier& OTAccount::GetAssetTypeID() const
{
    return acctAssetTypeId_;
}

void OTAccount::InitAccount()
{
    m_strContractType = "ACCOUNT";
    acctType_ = OTAccount::simple;
}

// Verify Contract ID first, THEN Verify Owner.
// Because we use the ID in this function, so make sure that it is verified
// before calling this.
bool OTAccount::VerifyOwner(const OTPseudonym& candidate) const
{
    OTIdentifier ID_CANDIDATE;
    // ID_CANDIDATE now contains the ID of the Nym we're testing.
    candidate.GetIdentifier(ID_CANDIDATE);
    return m_AcctUserID == ID_CANDIDATE;
}

// TODO: when entities and roles are added, probably more will go here.
bool OTAccount::VerifyOwnerByID(const OTIdentifier& nymId) const
{
    return nymId == m_AcctUserID;
}

// Let's say you don't have or know the UserID, and you just want to load the
// damn thing up.
// Then call this function. It will set userID and server ID for you.
OTAccount* OTAccount::LoadExistingAccount(const OTIdentifier& accountId,
                                          const OTIdentifier& serverId)
{
    bool folderAlreadyExist = false;
    bool folderIsNew = false;

    OTString strDataFolder = "";
    OTString strAccountPath = "";
    if (!OTDataFolder::Get(strDataFolder)) {
        OT_FAIL;
    }
    if (!OTPaths::AppendFolder(strAccountPath, strDataFolder,
                               OTFolders::Account())) {
        OT_FAIL;
    }

    if (!OTPaths::ConfirmCreateFolder(strAccountPath, folderAlreadyExist,
                                      folderIsNew)) {
        otErr << "Unable to find or create accounts folder: "
              << OTFolders::Account() << "\n";
        return nullptr;
    }

    OTAccount* account = new OTAccount();
    OT_ASSERT(account != nullptr);

    account->SetRealAccountID(accountId);
    account->SetRealServerID(serverId);

    OTString strAcctID(accountId);

    account->m_strFoldername = OTFolders::Account().Get();
    account->m_strFilename = strAcctID.Get();

    if (!OTDB::Exists(account->m_strFoldername.Get(),
                      account->m_strFilename.Get())) {
        otInfo << "OTAccount::LoadExistingAccount: File does not exist: "
               << account->m_strFoldername << OTLog::PathSeparator()
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

OTAccount* OTAccount::GenerateNewAccount(const OTIdentifier& userId,
                                         const OTIdentifier& serverId,
                                         const OTPseudonym& serverNym,
                                         const OTMessage& message,
                                         OTAccount::AccountType acctType,
                                         int64_t stashTransNum)
{
    OTAccount* account = new OTAccount(userId, serverId);

    if (account) {
        // This is only for stash accounts.
        if (account->GenerateNewAccount(serverNym, message, acctType,
                                        stashTransNum)) {
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
message.m_strAssetID;
message.m_strServerID;
 */
bool OTAccount::GenerateNewAccount(const OTPseudonym& server,
                                   const OTMessage& message,
                                   OTAccount::AccountType acctType,
                                   int64_t stashTransNum)
{
    // First we generate a secure random number into a binary object...
    ot_data_t payload;
    payload.resize(100);

    // TODO: hardcoding. Plus: is 100 bytes of random a little much here?
    if (!OTCrypto::It()->RandomizeMemory(payload.data(), payload.size())) {
        otErr << __FUNCTION__ << ": Failed trying to acquire random numbers.\n";
        return false;
    }

    // Next we calculate that binary object into a message digest (an
    // OTIdentifier).
    OTIdentifier newID;
    if (!newID.CalculateDigest(payload)) {
        otErr << __FUNCTION__ << ": Error generating new account ID.\n";
        return false;
    }

    // Next we get that digest (which is a binary hash number)
    // and extract a human-readable standard string format of that hash,
    // into an OTString.
    OTString strID(newID);

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
    // Account type defaults to OTAccount::simple.
    // But there are also issuer accts.
    acctType_ = acctType;

    // basket, basketsub, mint, voucher, and stash
    // accounts are all "owned" by the server.
    if (IsInternalServerAcct()) {
        server.GetIdentifier(m_AcctUserID);
    }
    else {
        m_AcctUserID.SetString(message.m_strNymID);
    }

    acctAssetTypeId_.SetString(message.m_strAssetID);

    otLog3 << __FUNCTION__ << ": Creating new account, type:\n"
           << message.m_strAssetID << "\n";

    OTIdentifier serverId(message.m_strServerID);
    // TODO: this assumes the serverID on the message
    // is correct. It's vetted, but still...
    SetRealServerID(serverId);
    SetPurportedServerID(serverId);

    time64_t t = OTTimeGetCurrentTime(); // Today, now.
    balanceDate_.Format("%d", t);

    balanceAmount_.Set("0");

    if (IsStashAcct()) {
        OT_ASSERT_MSG(stashTransNum > 0, "You created a stash account, but "
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

int64_t OTAccount::GetBalance() const
{
    if (balanceAmount_.Exists()) {
        return atol(balanceAmount_.Get());
    }
    return 0;
}

bool OTAccount::DisplayStatistics(OTString& contents) const
{
    OTString strAccountID(GetPurportedAccountID());
    OTString strServerID(GetPurportedServerID());
    OTString strUserID(GetUserID());
    OTString strAssetTypeID(acctAssetTypeId_);

    OTString acctType;
    TranslateAccountTypeToString(acctType_, acctType);

    contents.Concatenate(" Asset Account (%s) Name: %s\n"
                         " Last retrieved Balance: %s  on date: %s\n"
                         " accountID: %s\n"
                         " userID: %s\n"
                         " serverID: %s\n"
                         " assetTypeID: %s\n"
                         "\n",
                         acctType.Get(), m_strName.Get(), balanceAmount_.Get(),
                         balanceDate_.Get(), strAccountID.Get(),
                         strUserID.Get(), strServerID.Get(),
                         strAssetTypeID.Get());

    return true;
}

bool OTAccount::SaveContractWallet(OTString& contents) const
{
    OTString strAccountID(GetPurportedAccountID());
    OTString strServerID(GetPurportedServerID());
    OTString strUserID(GetUserID());
    OTString strAssetTypeID(acctAssetTypeId_);

    OTString acctType;
    TranslateAccountTypeToString(acctType_, acctType);

    OTASCIIArmor ascName;
    // name is in the clear in memory, and base64 in storage.
    if (m_strName.Exists()) {
        ascName.SetString(m_strName, false); // linebreaks == false
    }

    contents.Concatenate(
        "<!-- Last retrieved balance: %s on date: %s -->\n"
        "<!-- Account type: %s --><assetAccount name=\"%s\"\n"
        " accountID=\"%s\"\n"
        " userID=\"%s\"\n"
        " serverID=\"%s\" />\n"
        "<!-- assetTypeID: %s -->\n\n",
        balanceAmount_.Get(), balanceDate_.Get(), acctType.Get(),
        m_strName.Exists() ? ascName.Get() : "", strAccountID.Get(),
        strUserID.Get(), strServerID.Get(), strAssetTypeID.Get());
    return true;
}

// Most contracts do not override this function...
// But OTAccount does, because IF THE SIGNER has chosen to SIGN the account
// based on the current balances, then we need to update the m_xmlUnsigned
// member with the current balances and other updated information before the
// signing occurs. (Presumably this is the whole reason why the account is
// being re-signed.)
//
// Normally, in other OTContract and derived classes, m_xmlUnsigned is read
// from the file and then kept read-only, since contracts do not normally
// change. But as accounts change in balance, they must be re-signed to keep the
// signatures valid.
void OTAccount::UpdateContents()
{
    OTString strAssetTYPEID(acctAssetTypeId_);

    OTString ACCOUNT_ID(GetPurportedAccountID());
    OTString SERVER_ID(GetPurportedServerID());
    OTString USER_ID(GetUserID());

    OTString acctType;
    TranslateAccountTypeToString(acctType_, acctType);

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    m_xmlUnsigned.Concatenate("<?xml version=\"%s\"?>\n\n", "1.0");

    m_xmlUnsigned.Concatenate("<assetAccount\n version=\"%s\"\n type=\"%s\"\n "
                              "accountID=\"%s\"\n userID=\"%s\"\n"
                              " serverID=\"%s\"\n assetTypeID=\"%s\" >\n\n",
                              m_strVersion.Get(), acctType.Get(),
                              ACCOUNT_ID.Get(), USER_ID.Get(), SERVER_ID.Get(),
                              strAssetTYPEID.Get());
    if (IsStashAcct()) {
        m_xmlUnsigned.Concatenate("<stashinfo cronItemNum=\"%lld\"/>\n\n",
                                  stashTransNum_);
    }
    if (!inboxHash_.empty()) {
        OTString strHash(inboxHash_);
        m_xmlUnsigned.Concatenate("<inboxHash value=\"%s\"/>\n\n",
                                  strHash.Get());
    }
    if (!outboxHash_.empty()) {
        OTString strHash(outboxHash_);
        m_xmlUnsigned.Concatenate("<outboxHash value=\"%s\"/>\n\n",
                                  strHash.Get());
    }

    m_xmlUnsigned.Concatenate("<balance date=\"%s\" amount=\"%s\"/>\n\n",
                              balanceDate_.Get(), balanceAmount_.Get());

    if (markForDeletion_) {
        m_xmlUnsigned.Concatenate(
            "<MARKED_FOR_DELETION>\n"
            "%s</MARKED_FOR_DELETION>\n\n",
            "THIS ACCOUNT HAS BEEN MARKED FOR DELETION AT ITS OWN REQUEST");
    }
    m_xmlUnsigned.Concatenate("</assetAccount>\n");
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t OTAccount::ProcessXMLNode(IrrXMLReader*& xml)
{
    int32_t retval = 0;

    OTString strNodeName(xml->getNodeName());

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

    if (strNodeName.Compare("assetAccount")) {
        OTString acctType;

        m_strVersion = xml->getAttributeValue("version");
        acctType = xml->getAttributeValue("type");

        if (!acctType.Exists()) {
            otErr << "OTAccount::ProcessXMLNode: Failed: Empty assetAccount "
                     "'type' attribute.\n";
            return -1;
        }

        acctType_ = TranslateAccountTypeStringToEnum(acctType);

        if (OTAccount::err_acct == acctType_) {
            otErr << "OTAccount::ProcessXMLNode: Failed: assetAccount 'type' "
                     "attribute contains unknown value.\n";
            return -1;
        }

        OTString strAcctAssetType = xml->getAttributeValue("assetTypeID");

        if (strAcctAssetType.Exists()) {
            acctAssetTypeId_.SetString(strAcctAssetType);
        }
        OTString strAccountID(xml->getAttributeValue("accountID"));
        OTString strServerID(xml->getAttributeValue("serverID"));
        OTString strAcctUserID(xml->getAttributeValue("userID"));

        OTIdentifier ACCOUNT_ID(strAccountID);
        OTIdentifier SERVER_ID(strServerID);
        OTIdentifier USER_ID(strAcctUserID);

        SetPurportedAccountID(ACCOUNT_ID);
        SetPurportedServerID(SERVER_ID);
        SetUserID(USER_ID);

        OTString strAssetTypeID(acctAssetTypeId_);
        otLog3 << "\n\nAccount Type: " << acctType
               << "\nAccountID: " << strAccountID
               << "\nUserID: " << strAcctUserID
               << "\n"
                  "AssetTypeID: " << strAssetTypeID
               << "\nServerID: " << strServerID << "\n";

        retval = 1;
    }
    else if (strNodeName.Compare("inboxHash")) {

        OTString strHash = xml->getAttributeValue("value");
        if (strHash.Exists()) {
            inboxHash_.SetString(strHash);
        }
        otLog3 << "Account inboxHash: " << strHash << "\n";

        retval = 1;
    }
    else if (strNodeName.Compare("outboxHash")) {

        OTString strHash = xml->getAttributeValue("value");
        if (strHash.Exists()) {
            outboxHash_.SetString(strHash);
        }
        otLog3 << "Account outboxHash: " << strHash << "\n";

        retval = 1;
    }
    else if (strNodeName.Compare("MARKED_FOR_DELETION")) {
        markForDeletion_ = true;
        otLog3 << "This asset account has been MARKED_FOR_DELETION (at some "
                  "point prior.)\n";

        retval = 1;
    }
    else if (strNodeName.Compare("balance")) {
        balanceDate_ = xml->getAttributeValue("date");
        balanceAmount_ = xml->getAttributeValue("amount");

        // I convert to integer / int64_t and back to string.
        // (Just an easy way to keep the data clean.)

        int32_t date = atoi(balanceDate_.Get());
        int64_t amount = atol(balanceAmount_.Get());

        balanceDate_.Format("%d", date);
        balanceAmount_.Format("%lld", amount);

        otLog3 << "\nBALANCE  --  " << balanceAmount_ << "\nDATE     --  "
               << balanceDate_ << "\n";

        retval = 1;
    }
    else if (strNodeName.Compare("stashinfo")) {
        if (!IsStashAcct()) {
            otErr << "OTAccount::ProcessXMLNode: Error: Encountered stashinfo "
                     "tag while loading NON-STASH account. \n";
            return -1;
        }

        int64_t lTransNum = 0;
        OTString strStashTransNum = xml->getAttributeValue("cronItemNum");
        if (!strStashTransNum.Exists() ||
            ((lTransNum = atol(strStashTransNum.Get())) <= 0)) {
            stashTransNum_ = 0;
            otErr << "OTAccount::ProcessXMLNode: Error: Bad transaction number "
                     "for supposed corresponding cron item: " << lTransNum
                  << " \n";
            return -1;
        }
        else {
            stashTransNum_ = lTransNum;
        }

        otLog3 << "\nSTASH INFO:   CronItemNum     --  " << stashTransNum_
               << "\n";

        retval = 1;
    }

    return retval;
}

bool OTAccount::IsInternalServerAcct() const
{
    switch (acctType_) {
    case OTAccount::simple:
    case OTAccount::issuer:
        return false;
    case OTAccount::basket:
    case OTAccount::basketsub:
    case OTAccount::mint:
    case OTAccount::voucher:
    case OTAccount::stash:
        return true;
    default:
        otErr << "OTAccount::IsInternalServerAcct: Unknown account type.\n";
        return false;
    }
    return false;
}

bool OTAccount::IsOwnedByUser() const
{
    switch (acctType_) {
    case OTAccount::simple:
    case OTAccount::issuer:
        return true;
    case OTAccount::basket:
    case OTAccount::basketsub:
    case OTAccount::mint:
    case OTAccount::voucher:
    case OTAccount::stash:
        return false;
    default:
        otErr << "OTAccount::IsOwnedByUser: Unknown account type.\n";
        return false;
    }
    return false;
}

bool OTAccount::IsOwnedByEntity() const
{
    return false;
}

bool OTAccount::IsIssuer() const
{
    return OTAccount::issuer == acctType_;
}

bool OTAccount::IsAllowedToGoNegative() const
{
    switch (acctType_) {
    // issuer acct controlled by a user
    case OTAccount::issuer:
    // basket issuer acct controlled by the server (for a basket currency)
    case OTAccount::basket:
        return true;
    // user asset acct
    case OTAccount::simple:
    // internal server acct for storing reserves for basket sub currencies
    case OTAccount::basketsub:
    // internal server acct for storing reserves for cash withdrawals
    case OTAccount::mint:
    // internal server acct for storing reserves for
    // vouchers (like cashier's cheques)
    case OTAccount::voucher:
    // internal server acct for storing reserves for
    // smart contract stashes. (Money stashed IN the contract.)
    case OTAccount::stash:
        return false;
    default:
        otErr << "OTAccount::IsAllowedToGoNegative: Unknown account type.\n";
        return false;
    }
    return false;
}

void OTAccount::Release_Account()
{
    balanceDate_.Release();
    balanceAmount_.Release();
    inboxHash_.clear();
    outboxHash_.clear();
}

void OTAccount::Release()
{
    Release_Account();
    OTTransactionType::Release();
}

} // namespace opentxs
