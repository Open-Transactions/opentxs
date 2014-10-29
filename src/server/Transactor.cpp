/************************************************************
 *
 *  Transactor.cpp
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

#include <opentxs/server/Transactor.hpp>
#include <opentxs/server/OTServer.hpp>

#include <opentxs/cash/Mint.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/OTAccount.hpp>
#include <opentxs/core/OTIdentifier.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/OTString.hpp>
#include <opentxs/core/OTAssetContract.hpp>
#include <opentxs/core/OTLog.hpp>

namespace opentxs
{

Transactor::Transactor(OTServer* server)
    : transactionNumber_(0)
    , server_(server)
{
}

Transactor::~Transactor()
{
    while (!contractsMap_.empty()) {
        auto it = contractsMap_.begin();
        OTAssetContract* pContract = it->second;
        OT_ASSERT(nullptr != pContract);
        contractsMap_.erase(it);
        delete pContract;
        pContract = nullptr;
    }

    while (!mintsMap_.empty()) {
        auto it = mintsMap_.begin();
        Mint* pMint = it->second;
        OT_ASSERT(nullptr != pMint);
        mintsMap_.erase(it);
        delete pMint;
        pMint = nullptr;
    }
}

/// Just as every request must be accompanied by a request number, so
/// every transaction request must be accompanied by a transaction number.
/// The request numbers can simply be incremented on both sides (per user.)
/// But the transaction numbers must be issued by the server and they do
/// not repeat from user to user. They are unique to transaction.
///
/// Users must ask the server to send them transaction numbers so that they
/// can be used in transaction requests. The server keeps an internal counter
/// and maintains a data file to store the latest one.
///
/// More specifically, the server file itself stores the latest transaction
/// number
/// (So it knows what number to issue and increment when the next request comes
/// in.)
///
/// But once it issues the next number, that number needs to be recorded in the
/// nym file
/// for the user it was issued to, so that it can be verified later when he
/// submits it
/// for a transaction--and so it can be removed once the transaction is complete
/// (so it
/// won't work twice.)
///
/// The option to bSaveTheNumber defaults to true for this reason. But sometimes
/// it
/// will be sent to false, in cases where the number doesn't need to be saved
/// because
/// it's never going to be verified. For example, if the server creates a
/// transaction
/// number so it can put a transaction into your inbox, it's never going to have
/// to verify
/// that it actually put it into the inbox by checking it's own nymfile for that
/// transaction
/// number. Instead it would just check its own server signature on the inbox.
/// But I digress...
///
bool Transactor::issueNextTransactionNumber(OTPseudonym& theNym,
                                            int64_t& lTransactionNumber,
                                            bool bStoreTheNumber)
{
    OTIdentifier NYM_ID(theNym), SERVER_NYM_ID(server_->m_nymServer);

    // If theNym has the same ID as server_->m_nymServer, then we'll use
    // server_->m_nymServer
    // instead of theNym.  (Since it's the same nym anyway, we'll stick to the
    // one we already loaded so any changes don't get overwritten later.)
    OTPseudonym* pNym = nullptr;

    if (NYM_ID == SERVER_NYM_ID)
        pNym = &server_->m_nymServer;
    else
        pNym = &theNym;

    // transactionNumber_ stores the last VALID AND ISSUED transaction number.
    // So first, we increment that, since we don't want to issue the same number
    // twice.
    transactionNumber_++;

    // Next, we save it to file.
    if (!server_->mainFile_.SaveMainFile()) {
        OTLog::Error("Error saving main server file.\n");
        transactionNumber_--;
        return false;
    }

    // Each Nym stores the transaction numbers that have been issued to it.
    // (On client AND server side.)
    //
    // So whenever the server issues a new number, it's to a specific Nym, then
    // it is recorded in his Nym file before being sent to the client (where it
    // is also recorded in his Nym file.)  That way the server always knows
    // which
    // numbers are valid for each Nym.
    else if (bStoreTheNumber && (false ==
                                 pNym->AddTransactionNum(server_->m_nymServer,
                                                         server_->m_strServerID,
                                                         transactionNumber_,
                                                         true))) // bSave = true
    {
        OTLog::Error("Error adding transaction number to Nym file.\n");
        transactionNumber_--;
        server_->mainFile_.SaveMainFile(); // Save it back how it was, since
                                           // we're not
                                           // issuing this
                                           // number after all.
        return false;
    }

    // SUCCESS?
    // Now the server main file has saved the latest transaction number,
    // and the number has been stored on the relevant nym file.
    // NOW we set it onto the parameter and return true.
    else {
        lTransactionNumber = transactionNumber_;
        return true;
    }
}

/// Transaction numbers are now stored in the nym file (on client and server
/// side) for whichever nym
/// they were issued to. This function verifies whether or not the transaction
/// number is present and valid
/// for any specific nym (i.e. for the nym passed in.)
bool Transactor::verifyTransactionNumber(
    OTPseudonym& theNym, const int64_t& lTransactionNumber) // passed by
                                                            // reference for
                                                            // speed, but not a
                                                            // return value.
{
    OTIdentifier NYM_ID(theNym), SERVER_NYM_ID(server_->m_nymServer);

    // If theNym has the same ID as server_->m_nymServer, then we'll use
    // server_->m_nymServer
    // instead of theNym.  (Since it's the same nym anyway, we'll stick to the
    // one we already loaded so any changes don't get overwritten later.)
    OTPseudonym* pNym = nullptr;

    if (NYM_ID == SERVER_NYM_ID)
        pNym = &server_->m_nymServer;
    else
        pNym = &theNym;
    if (pNym->VerifyTransactionNum(server_->m_strServerID, lTransactionNumber))
        return true;
    else {
        const OTString strNymID(NYM_ID);
        const OTString strIssued(
            pNym->VerifyIssuedNum(server_->m_strServerID, lTransactionNumber)
                ? "(However, that number IS issued to that Nym... He must have "
                  "already used it.)\n"
                : "(In fact, that number isn't even issued to that Nym, though "
                  "perhaps it was at some time in the past?)\n");

        OTLog::vError("%s: %lld not available for Nym %s to use. \n%s",
                      __FUNCTION__,
                      //                    " Oh, and FYI, tangentially, the
                      // current Trns# counter is: %ld\n",
                      lTransactionNumber, strNymID.Get(), strIssued.Get());
        //                    transactionNumber_);
    }

    return false;
}

/// Remove a transaction number from the Nym record once it's officially
/// used/spent.
bool Transactor::removeTransactionNumber(OTPseudonym& theNym,
                                         const int64_t& lTransactionNumber,
                                         bool bSave)
{
    OTIdentifier NYM_ID(theNym), SERVER_NYM_ID(server_->m_nymServer);

    // If theNym has the same ID as server_->m_nymServer, then we'll use
    // server_->m_nymServer
    // instead of theNym.  (Since it's the same nym anyway, we'll stick to the
    // one we already loaded so any changes don't get overwritten later.)
    OTPseudonym* pNym = nullptr;

    if (NYM_ID == SERVER_NYM_ID)
        pNym = &server_->m_nymServer;
    else
        pNym = &theNym;

    bool bRemoved = false;

    if (bSave)
        bRemoved = pNym->RemoveTransactionNum(
            server_->m_nymServer, server_->m_strServerID,
            lTransactionNumber); // the version that passes in a signer nym --
                                 // saves to local storage.
    else
        bRemoved = pNym->RemoveTransactionNum(
            server_->m_strServerID,
            lTransactionNumber); // the version that doesn't save.

    return bRemoved;
}

/// Remove an issued number from the Nym record once that nym accepts the
/// receipt from his inbox.
bool Transactor::removeIssuedNumber(OTPseudonym& theNym,
                                    const int64_t& lTransactionNumber,
                                    bool bSave)
{
    OTIdentifier NYM_ID(theNym), SERVER_NYM_ID(server_->m_nymServer);

    // If theNym has the same ID as server_->m_nymServer, then we'll use
    // server_->m_nymServer
    // instead of theNym.  (Since it's the same nym anyway, we'll stick to the
    // one we already loaded so any changes don't get overwritten later.)
    OTPseudonym* pNym = nullptr;

    if (NYM_ID == SERVER_NYM_ID)
        pNym = &server_->m_nymServer;
    else
        pNym = &theNym;

    bool bRemoved =
        pNym->RemoveIssuedNum(server_->m_nymServer, server_->m_strServerID,
                              lTransactionNumber, bSave);

    return bRemoved;
}

/// The server supports various different asset types.
/// Any user may create a new asset type by uploading the asset contract to the
/// server.
/// The server stores the contract in a directory and in its in-memory list of
/// asset types.
/// You can call this function to look up any asset contract by ID. If it
/// returns nullptr,
/// you can add it yourself by uploading the contract.  But be sure that the
/// public key
/// in the contract, used to sign the contract, is also the public key of the
/// Nym of the
/// issuer.  They must match.  In the future I may create a special key category
/// just for
/// this purpose. Right now I'm using the "contract" key which is already used
/// to verify
/// any asset or server contract.
OTAssetContract* Transactor::getAssetContract(const OTIdentifier& ASSET_TYPE_ID)
{
    for (auto& it : contractsMap_) {
        OTAssetContract* pContract = it.second;
        OT_ASSERT(nullptr != pContract);

        OTIdentifier theContractID;
        pContract->GetIdentifier(theContractID);

        if (theContractID == ASSET_TYPE_ID) return pContract;
    }

    return nullptr;
}

/// OTServer will take ownership of theContract from this point on,
/// and will be responsible for deleting it. MUST be allocated on the heap.
bool Transactor::addAssetContract(OTAssetContract& theContract)
{
    OTAssetContract* pContract = nullptr;

    OTString STR_CONTRACT_ID;
    OTIdentifier CONTRACT_ID;
    theContract.GetIdentifier(STR_CONTRACT_ID);
    theContract.GetIdentifier(CONTRACT_ID);

    pContract = getAssetContract(CONTRACT_ID);

    // already exists
    if (nullptr != pContract) // if not null
        return false;

    contractsMap_[STR_CONTRACT_ID.Get()] = &theContract;

    return true;
}

// Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID.
bool Transactor::addBasketAccountID(const OTIdentifier& BASKET_ID,
                                    const OTIdentifier& BASKET_ACCOUNT_ID,
                                    const OTIdentifier& BASKET_CONTRACT_ID)
{
    OTIdentifier theBasketAcctID;

    if (lookupBasketAccountID(BASKET_ID, theBasketAcctID)) {
        OTLog::Output(0, "User attempted to add Basket that already exists.\n");
        return false;
    }

    OTString strBasketID(BASKET_ID), strBasketAcctID(BASKET_ACCOUNT_ID),
        strBasketContractID(BASKET_CONTRACT_ID);

    idToBasketMap_[strBasketID.Get()] = strBasketAcctID.Get();
    contractIdToBasketAccountId_[strBasketContractID.Get()] =
        strBasketAcctID.Get();

    return true;
}

/// Use this to find the basket account ID for this server (which is unique to
/// this server)
/// using the contract ID to look it up. (The basket contract ID is unique to
/// this server.)
bool Transactor::lookupBasketAccountIDByContractID(
    const OTIdentifier& BASKET_CONTRACT_ID, OTIdentifier& BASKET_ACCOUNT_ID)
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : contractIdToBasketAccountId_) {
        OTString strBasketContractID = it.first.c_str();
        OTString strBasketAcctID = it.second.c_str();

        OTIdentifier id_BASKET_CONTRACT(strBasketContractID),
            id_BASKET_ACCT(strBasketAcctID);

        if (BASKET_CONTRACT_ID == id_BASKET_CONTRACT) // if the basket contract
                                                      // ID passed in matches
                                                      // this one...
        {
            BASKET_ACCOUNT_ID = id_BASKET_ACCT;
            return true;
        }
    }
    return false;
}

/// Use this to find the basket account ID for this server (which is unique to
/// this server)
/// using the contract ID to look it up. (The basket contract ID is unique to
/// this server.)
bool Transactor::lookupBasketContractIDByAccountID(
    const OTIdentifier& BASKET_ACCOUNT_ID, OTIdentifier& BASKET_CONTRACT_ID)
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : contractIdToBasketAccountId_) {
        OTString strBasketContractID = it.first.c_str();
        OTString strBasketAcctID = it.second.c_str();

        OTIdentifier id_BASKET_CONTRACT(strBasketContractID),
            id_BASKET_ACCT(strBasketAcctID);

        if (BASKET_ACCOUNT_ID == id_BASKET_ACCT) // if the basket contract ID
                                                 // passed in matches this
                                                 // one...
        {
            BASKET_CONTRACT_ID = id_BASKET_CONTRACT;
            return true;
        }
    }
    return false;
}

/// Use this to find the basket account for this server (which is unique to this
/// server)
/// using the basket ID to look it up (the Basket ID is the same for all
/// servers)
bool Transactor::lookupBasketAccountID(const OTIdentifier& BASKET_ID,
                                       OTIdentifier& BASKET_ACCOUNT_ID)
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : idToBasketMap_) {
        OTString strBasketID = it.first.c_str();
        OTString strBasketAcctID = it.second.c_str();

        OTIdentifier id_BASKET(strBasketID), id_BASKET_ACCT(strBasketAcctID);

        if (BASKET_ID ==
            id_BASKET) // if the basket ID passed in matches this one...
        {
            BASKET_ACCOUNT_ID = id_BASKET_ACCT;
            return true;
        }
    }
    return false;
}

/// Looked up the voucher account (where cashier's cheques are issued for any
/// given asset type) return a pointer to the account.  Since it's SUPPOSED to
/// exist, and since it's being requested, also will GENERATE it if it cannot
/// be found, add it to the list, and return the pointer. Should always succeed.
std::shared_ptr<OTAccount> Transactor::getVoucherAccount(
    const OTIdentifier& ASSET_TYPE_ID)
{
    std::shared_ptr<OTAccount> pAccount;
    const OTIdentifier SERVER_USER_ID(server_->m_nymServer),
        SERVER_ID(server_->m_strServerID);
    bool bWasAcctCreated = false;
    pAccount = voucherAccounts_.GetOrCreateAccount(
        server_->m_nymServer, SERVER_USER_ID, ASSET_TYPE_ID, SERVER_ID,
        bWasAcctCreated);
    if (bWasAcctCreated) {
        OTString strAcctID;
        pAccount->GetIdentifier(strAcctID);
        const OTString strAssetTypeID(ASSET_TYPE_ID);

        OTLog::vOutput(0, "OTServer::GetVoucherAccount: Successfully created "
                          "voucher account ID: %s Asset Type ID: %s\n",
                       strAcctID.Get(), strAssetTypeID.Get());

        if (!server_->mainFile_.SaveMainFile()) {
            OTLog::Error("OTServer::GetVoucherAccount: Error saving main "
                         "server file containing new account ID!!\n");
        }
    }

    return pAccount;
}

/// Lookup the current mint for any given asset type ID and series.
Mint* Transactor::getMint(const OTIdentifier& ASSET_TYPE_ID,
                          int32_t nSeries) // Each asset contract has its own
                                           // Mint.
{
    Mint* pMint = nullptr;

    for (auto& it : mintsMap_) {
        pMint = it.second;
        OT_ASSERT_MSG(nullptr != pMint,
                      "nullptr mint pointer in Transactor::getMint\n");

        OTIdentifier theID;
        pMint->GetIdentifier(theID);

        if ((ASSET_TYPE_ID ==
             theID) && // if the ID on the Mint matches the ID passed in
            (nSeries == pMint->GetSeries())) // and the series also matches...
            return pMint; // return the pointer right here, we're done.
    }
    // The mint isn't in memory for the series requested.
    const OTString ASSET_ID_STR(ASSET_TYPE_ID);

    OTString strMintFilename;
    strMintFilename.Format("%s%s%s%s%d", server_->m_strServerID.Get(),
                           OTLog::PathSeparator(), ASSET_ID_STR.Get(), ".",
                           nSeries);

    const char* szFoldername = OTFolders::Mint().Get();
    const char* szFilename = strMintFilename.Get();
    pMint = Mint::MintFactory(server_->m_strServerID,
                              server_->m_strServerUserID, ASSET_ID_STR);

    // You cannot hash the Mint to get its ID. (The ID is a hash of the asset
    // contract.)
    // Instead, you must READ the ID from the Mint file, and then compare it to
    // the one expected
    // to see if they match (similar to how Account IDs are verified.)

    OT_ASSERT_MSG(nullptr != pMint,
                  "Error allocating memory for Mint in Transactor::getMint");
    OTString strSeries;
    strSeries.Format("%s%d", ".", nSeries);
    //
    if (pMint->LoadMint(strSeries.Get())) {
        if (pMint->VerifyMint(server_->m_nymServer)) // I don't verify the
                                                     // Mint's
        // expiration date here, just its
        // signature, ID, etc.
        { // (Expiry dates are enforced on tokens during deposit--and checked
            // against mint--
            // but expiry dates are only enforced on the Mint itself during a
            // withdrawal.)
            // It's a multimap now...
            // mintsMap_[ASSET_ID_STR.Get()] = pMint;

            mintsMap_.insert(
                std::pair<std::string, Mint*>(ASSET_ID_STR.Get(), pMint));

            return pMint;
        }
        else {
            OTLog::vError(
                "Error verifying Mint in Transactor::getMint:\n%s%s%s\n",
                szFoldername, OTLog::PathSeparator(), szFilename);
        }
    }
    else {
        OTLog::vError("Error loading Mint in Transactor::getMint:\n%s%s%s\n",
                      szFoldername, OTLog::PathSeparator(), szFilename);
    }

    if (nullptr != pMint) delete pMint;
    pMint = nullptr;

    return nullptr;
}

} // namespace opentxs
