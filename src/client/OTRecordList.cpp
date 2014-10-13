/************************************************************
*
*  OTRecordList.cpp
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

#include <opentxs/client/OTRecordList.hpp>
#include <opentxs/client/OpenTransactions.hpp>
#include <opentxs/client/OT_ME.hpp>
#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include "Helpers.hpp"
#include <opentxs/client/OTWallet.hpp>

#include <opentxs/ext/OTPayment.hpp>

#include <opentxs/core/OTAccount.hpp>
#include <opentxs/core/OTAssetContract.hpp>
#include <opentxs/core/OTLedger.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTMessage.hpp>
#include <opentxs/core/OTPseudonym.hpp>

#include <memory>
#include <algorithm>

namespace
{

const std::string Instrument_TypeStrings[] = {
    // OTCheque is derived from OTTrackable, which is derived from OTInstrument,
    // which is
    // derived from OTScriptable, which is derived from OTContract.
    "cheque",  // A cheque drawn on a user's account.
    "voucher", // A cheque drawn on a server account (cashier's cheque aka
               // banker's cheque)
    "invoice", // A cheque with a negative amount. (Depositing this causes a
               // payment out, instead of a deposit in.)
    "payment plan",   // An OTCronItem-derived OTPaymentPlan, related to a
                      // recurring payment plan.
    "smart contract", // An OTCronItem-derived OTSmartContract, related to a
                      // smart contract.
    "cash", // An OTContract-derived OTPurse containing a list of cash OTTokens.
    "ERROR_STATE"};

const std::string& GetTypeString(int theType)
{
    return Instrument_TypeStrings[theType];
}

} // namespace

namespace opentxs
{

#define MC_UI_TEXT_TO "To: %s"
#define MC_UI_TEXT_FROM "From: %s"

//#define MC_UI_TEXT_TO "<font color='grey'>To:</font> %s"
//#define MC_UI_TEXT_FROM "<font color='grey'>From:</font> %s"

bool OT_API_Set_AddrBookCallback(OTLookupCaller& theCaller) // OTLookupCaller
                                                            // must have
                                                            // OTNameLookup
                                                            // attached already.
{
    if (!theCaller.isCallbackSet()) {
        otErr << __FUNCTION__
              << ": ERROR:\nOTLookupCaller::setCallback() "
                 "MUST be called first, with an OTNameLookup-extended "
                 "class passed to it,\n"
                 "before then invoking this function (and passing that "
                 "OTLookupCaller as a parameter "
                 "into this function.)\n";
        return false;
    }

    otWarn << __FUNCTION__
           << ": FYI, calling "
              "OTRecordList::setAddrBookCaller(theCaller) now... "
              "(which is where "
              "OT internally sets its pointer to the Java caller "
              "object, which must have been passed in as a "
              "parameter to this function.\n";

    const bool bSuccess = OTRecordList::setAddrBookCaller(theCaller);

    otWarn << __FUNCTION__
           << ": RESULT of call to OTRecordList::setAddrBookCaller: "
           << (bSuccess ? "SUCCESS" : "FAILURE") << "";

    return bSuccess;
}

// OTNameLookup CLASS

OTNameLookup::~OTNameLookup()
{
    // Note: Though the Java app will instantiate NameLookup once (its own
    // version/subclass of it)
    // and pass that instance to C++ so that OTRecordList can make calls to it
    // from inside OT, that
    // doesn't mean that C++ apps like Moneychanger won't make as many instances
    // as they want. Bottom
    // line? No error message necessary here as we have with the password
    // callback.
    //
    //    otErr << "OTNameLookup::~OTNameLookup:  (This should only happen ONCE
    // ONLY -- as the application is closing.)\n";
    //    std::cout << "OTNameLookup::~OTNameLookup()" << std:: endl;
}

// virtual
std::string OTNameLookup::GetNymName(const std::string& str_id,
                                     const std::string*) const
{
    if (str_id.empty()) return "";

    return OTAPI_Wrap::GetNym_Name(str_id);
}

// virtual
std::string OTNameLookup::GetAcctName(const std::string& str_id,
                                      const std::string*, const std::string*,
                                      const std::string*) const
{
    if (str_id.empty()) return "";

    return OTAPI_Wrap::GetAccountWallet_Name(str_id);
}

// virtual
std::string OTNameLookup::GetAddressName(const std::string&) const
{
    return ""; // There are no native OT lookups for a Bitmessage address. (Only
               // useful when overriding.)
}

// OTLookupCaller CLASS

OTLookupCaller::~OTLookupCaller()
{
    otOut << "OTLookupCaller::~OTLookupCaller: (This should only "
             "happen as the application is closing.)\n";

    delCallback();
}

void OTLookupCaller::delCallback()
{
    //    if (nullptr != _callback)  // TODO this may be a memory leak.
    //        delete _callback;    // But I know we're currently crashing from
    // deleting same object twice.
    // And since the object comes from Java, who am I to delete it? Let Java
    // clean it up.
    if (isCallbackSet())
        otOut << "OTLookupCaller::delCallback: WARNING: setting existing "
                 "callback object pointer to nullptr. "
                 "(This message doesn't trigger if it was already nullptr.)\n";
    _callback = nullptr;
}

void OTLookupCaller::setCallback(OTNameLookup* cb)
{
    otOut << "OTLookupCaller::setCallback: Attempting to set the "
             "OTNameLookup pointer...\n";

    if (nullptr == cb) {
        otOut << "OTLookupCaller::setCallback: ERROR: nullptr "
                 "OTNameLookup object passed in. (Returning.)\n";
        return;
    }

    delCallback(); // Sets _callback to nullptr, but LOGS first, if it was
                   // already
                   // set.
    _callback = cb;
    otOut << "OTLookupCaller::setCallback: FYI, the OTNameLookup "
             "pointer was set.\n";
}

bool OTLookupCaller::isCallbackSet() const
{
    return (nullptr == _callback) ? false : true;
}

std::string OTLookupCaller::GetNymName(const std::string& str_id, // NymID
                                       const std::string* p_server_id) const
{
    if (isCallbackSet()) {
        otWarn << "OTLookupCaller::GetNymName: FYI, Executing address "
                  "book callback...\n";
        return _callback->GetNymName(str_id, p_server_id);
    }
    else {
        otOut << "OTLookupCaller::GetNymName: "
                 "WARNING: Failed attempt to trigger address book "
                 "callback, due to \"it hasn't been set yet.\"\n";
    }
    return "";
}

std::string OTLookupCaller::GetAcctName(const std::string& str_id, // AcctID
                                        const std::string* p_nym_id,
                                        const std::string* p_server_id,
                                        const std::string* p_asset_id) const
{
    if (isCallbackSet()) {
        otWarn << "OTLookupCaller::GetAcctName: FYI, Executing address "
                  "book callback...\n";
        return _callback->GetAcctName(str_id, p_nym_id, p_server_id,
                                      p_asset_id);
    }
    else {
        otOut << "OTLookupCaller::GetAcctName: "
                 "WARNING: Failed attempt to trigger address book "
                 "callback, due to \"it hasn't been set yet.\"\n";
    }
    return "";
}

std::string OTLookupCaller::GetAddressName(const std::string& str_address) const
{
    if (isCallbackSet()) {
        otWarn << "OTLookupCaller::GetAddressName: FYI, Executing "
                  "address book callback...\n";
        return _callback->GetAddressName(str_address);
    }
    else {
        otOut << "OTLookupCaller::GetAddressName: "
                 "WARNING: Failed attempt to trigger address book "
                 "callback, due to \"it hasn't been set yet.\"\n";
    }
    return "";
}

// static

const std::string OTRecordList::s_blank("");
const std::string OTRecordList::s_message_type("message");

std::string OTRecordList::s_strTextTo(MC_UI_TEXT_TO);     // "To: %s"
std::string OTRecordList::s_strTextFrom(MC_UI_TEXT_FROM); // "From: %s"

OTLookupCaller* OTRecordList::s_pCaller = nullptr;

// Takes ownership. UPDATE: doesn't, since he assumes the Java side
// created it and will therefore delete it when the time comes.
// I keep a pointer, but I don't delete the thing. Let Java do it.
//
// static
bool OTRecordList::setAddrBookCaller(OTLookupCaller& theCaller)
{
    otLog3 << __FUNCTION__
           << ": Attempting to set the address book caller... \n";

    if (!theCaller.isCallbackSet()) {
        otErr << __FUNCTION__
              << ": ERROR: OTLookupCaller::setCallback() "
                 "MUST be called first, with an OTNameLookup-extended "
                 "object passed to it,\n"
                 "BEFORE calling this function with that OTLookupCaller. "
                 "(Returning false.)\n";
        return false;
    }

    if (nullptr != s_pCaller) {
        otErr
            << __FUNCTION__
            << ": WARNING: Setting the address book caller again, even though "
               "it was apparently ALREADY set... (Meaning Java has probably "
               "erroneously called this twice, "
               "possibly passing the same OTLookupCaller both times.)\n";
        //        delete s_pCaller; // Let Java delete it.
    }

    s_pCaller = &theCaller;
    otWarn << __FUNCTION__
           << ": FYI, Successfully set the address book caller object from "
              "Java (or from another SWIG language.) Returning true.\n";

    return true;
}
// static
OTLookupCaller* OTRecordList::getAddrBookCaller()
{
    return s_pCaller;
}

// SETUP:

// Set the default server here.

void OTRecordList::SetServerID(std::string str_id)
{
    ClearServers();
    AddServerID(str_id);
}

// Unless you have many servers, then use this.

void OTRecordList::AddServerID(std::string str_id)
{
    m_servers.insert(m_servers.end(), str_id);
}

// Also clears m_contents

void OTRecordList::ClearServers()
{
    ClearContents();
    m_servers.clear();
}

void OTRecordList::SetAssetID(std::string str_id)
{
    ClearAssets();
    AddAssetID(str_id);
}

void OTRecordList::AddAssetID(std::string str_id)
{
    OTWallet* pWallet = OTAPI_Wrap::OTAPI()->GetWallet(
        __FUNCTION__); // This logs and ASSERTs already.
    OT_ASSERT_MSG(nullptr != pWallet,
                  "Wallet was nullptr. Should never happen.");
    const OTString strAssetTypeID(str_id);
    const OTIdentifier theAssetTypeID(strAssetTypeID);
    std::string str_asset_name;
    // Name is dollars, fraction is cents, TLA is USD and
    // Symbol is $ (for example.) Here, we're grabbing the TLA.
    //
    OTAssetContract* pAssetContract = pWallet->GetAssetContract(theAssetTypeID);
    if (nullptr != pAssetContract) {
        str_asset_name =
            pAssetContract->GetCurrencyTLA().Get(); // This might be "USD" --
                                                    // preferable that this
                                                    // works.
        if (str_asset_name.empty())
            str_asset_name =
                pAssetContract->GetCurrencySymbol().Get(); // This might be "$".
        if (str_asset_name.empty())
            str_asset_name =
                pAssetContract->GetCurrencyName().Get(); // This might be
                                                         // "dollars".
    }
    if (str_asset_name.empty())
        str_asset_name = OTAPI_Wrap::GetAssetType_Name(
            str_id); // Otherwise we try to grab the name.
    // (Otherwise we just leave it blank. The ID is too big to cram in here.)
    m_assets.insert(
        std::pair<std::string, std::string>(str_id, str_asset_name));
}

void OTRecordList::ClearAssets()
{
    ClearContents();
    m_assets.clear();
}

void OTRecordList::SetNymID(std::string str_id)
{
    ClearNyms();
    AddNymID(str_id);
}

void OTRecordList::AddNymID(std::string str_id)
{
    m_nyms.insert(m_nyms.end(), str_id);
}

void OTRecordList::ClearNyms()
{
    ClearContents();
    m_nyms.clear();
}

void OTRecordList::SetAccountID(std::string str_id)
{
    ClearAccounts();
    AddAccountID(str_id);
}

void OTRecordList::AddAccountID(std::string str_id)
{
    m_accounts.insert(m_accounts.end(), str_id);
}

void OTRecordList::ClearAccounts()
{
    ClearContents();
    m_accounts.clear();
}

void OTRecordList::AcceptChequesAutomatically(bool bVal)
{
    m_bAutoAcceptCheques = bVal;
}
void OTRecordList::AcceptReceiptsAutomatically(bool bVal)
{
    m_bAutoAcceptReceipts = bVal;
}
void OTRecordList::AcceptTransfersAutomatically(bool bVal)
{
    m_bAutoAcceptTransfers = bVal;
}
void OTRecordList::AcceptCashAutomatically(bool bVal)
{
    m_bAutoAcceptCash = bVal;
}

bool OTRecordList::DoesAcceptChequesAutomatically() const
{
    return m_bAutoAcceptCheques;
}
bool OTRecordList::DoesAcceptReceiptsAutomatically() const
{
    return m_bAutoAcceptReceipts;
}
bool OTRecordList::DoesAcceptTransfersAutomatically() const
{
    return m_bAutoAcceptTransfers;
}
bool OTRecordList::DoesAcceptCashAutomatically() const
{
    return m_bAutoAcceptCash;
}

typedef std::map<int32_t, OTPayment*> mapOfPayments;

bool OTRecordList::PerformAutoAccept()
{
    OTWallet* pWallet = OTAPI_Wrap::OTAPI()->GetWallet(
        __FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) {
        otErr << "OTRecordList::" << __FUNCTION__
              << ": Error: Wallet is nullptr.\n";
        return false;
    }
    // LOOP NYMS
    //
    if (m_bAutoAcceptCheques || m_bAutoAcceptCash) {
        int32_t nNymIndex = -1;
        for (auto& it_nym : m_nyms) {
            ++nNymIndex;
            if (0 == nNymIndex)
                otOut << "======================================\n "
                      << __FUNCTION__ << ": Beginning auto-accept loop through "
                                         "Nyms...\n";
            const std::string& str_nym_id(it_nym);
            const OTIdentifier theNymID(str_nym_id);
            const OTString strNymID(theNymID);
            OTPseudonym* pNym = pWallet->GetNymByID(theNymID);
            if (nullptr == pNym) continue;
            // LOOP SERVERS
            //
            // For each nym, for each server, loop through its payments inbox
            //
            int32_t nServerIndex = -1;
            for (auto& it_server : m_servers) {
                ++nServerIndex;
                const std::string& str_server_id(it_server);
                const OTIdentifier theServerID(str_server_id);
                OTServerContract* pServer =
                    pWallet->GetServerContract(theServerID);
                OT_ASSERT(nullptr != pServer);
                const OTString strServerID(theServerID);
                otOut << __FUNCTION__ << ": Server " << nServerIndex
                      << ", ID: " << strServerID.Get() << "\n";
                mapOfPayments thePaymentMap;
                // OPTIMIZE FYI:
                // The "NoVerify" version is much faster, but you will lose the
                // ability to get the
                // sender/recipient name from the receipts in the box. The code
                // will, however, work
                // either way.
                //
                OTLedger* pInbox =
                    m_bRunFast
                        ? OTAPI_Wrap::OTAPI()->LoadPaymentInboxNoVerify(
                              theServerID, theNymID)
                        : OTAPI_Wrap::OTAPI()->LoadPaymentInbox(theServerID,
                                                                theNymID);
                std::unique_ptr<OTLedger> theInboxAngel(pInbox);

                // It loaded up, so let's loop through it.
                if (nullptr != pInbox) {
                    int32_t nIndex = (-1);
                    for (auto& it : pInbox->GetTransactionMap()) {
                        OTTransaction* pBoxTrans = it.second;
                        OT_ASSERT(nullptr != pBoxTrans);
                        ++nIndex; // 0 on first iteration.
                        otOut << __FUNCTION__
                              << ": Incoming payment: " << nIndex << "\n";
                        const std::string* p_str_asset_type =
                            &OTRecordList::s_blank; // <========== ASSET TYPE
                        const std::string* p_str_asset_name =
                            &OTRecordList::s_blank; // asset type display name.
                        std::string str_type;       // Instrument type.
                        OTPayment* pPayment =
                            GetInstrument(*pNym, nIndex, *pInbox);
                        // ===> Returns financial instrument by index.
                        std::unique_ptr<OTPayment> thePaymentAngel(pPayment);
                        if (nullptr ==
                            pPayment) // then we treat it like it's abbreviated.
                        {
                            otErr << __FUNCTION__
                                  << ": Payment retrieved from payments "
                                     "inbox was nullptr. (It's abbreviated?) "
                                     "Skipping.\n";
                        }
                        // We have pPayment, the instrument accompanying the
                        // receipt in the payments inbox.
                        //
                        else if (pPayment->IsValid() &&
                                 pPayment->SetTempValues()) {
                            OTIdentifier theAssetTypeID;

                            if (pPayment->GetAssetTypeID(theAssetTypeID)) {
                                OTString strTemp(theAssetTypeID);
                                const std::string str_inpmt_asset(
                                    strTemp.Get()); // The asset type we found
                                                    // on the payment (if we
                                                    // found anything.)
                                auto it_asset = m_assets.find(str_inpmt_asset);
                                if (it_asset != m_assets.end()) // Found it on
                                                                // the map of
                                                                // asset types
                                                                // we care
                                                                // about.
                                {
                                    p_str_asset_type =
                                        &(it_asset->first); // Set the asset
                                                            // type ID.
                                    p_str_asset_name =
                                        &(it_asset->second); // The CurrencyTLA.
                                                             // Examples: USD,
                                                             // BTC, etc.
                                }
                                else {
                                    // There was definitely an asset type on the
                                    // instrument, and it definitely
                                    // did not match any of the assets that we
                                    // care about.
                                    // Therefore, skip.
                                    //
                                    otErr << __FUNCTION__
                                          << ": Skipping: Incoming payment (we "
                                             "don't care about asset "
                                          << str_inpmt_asset.c_str() << ")\n";
                                    continue;
                                }
                            }
                            // By this point, p_str_asset_type and
                            // p_str_asset_name are definitely set.
                            OT_ASSERT(nullptr != p_str_asset_type); // and it's
                                                                    // either
                                                                    // blank, or
                            // it's one of
                            // the asset
                            // types we
                            // care about.
                            OT_ASSERT(nullptr != p_str_asset_name); // and it's
                                                                    // either
                                                                    // blank, or
                            // it's one of
                            // the asset
                            // types we
                            // care about.
                            // Instrument type (cheque, voucher, etc)
                            int32_t nType =
                                static_cast<int32_t>(pPayment->GetType());

                            str_type = GetTypeString(nType);
                            // For now, we only accept cash, cheques and
                            // vouchers.
                            //
                            if ((m_bAutoAcceptCheques &&
                                 ((0 == str_type.compare("cheque")) ||
                                  (0 == str_type.compare("voucher")))) ||
                                (m_bAutoAcceptCash &&
                                 (0 == str_type.compare("cash")))) {
                                otOut << __FUNCTION__
                                      << ": Adding to acceptance "
                                         "list: pending incoming "
                                      << str_type.c_str() << ".\n";
                                thePaymentMap.insert(
                                    std::pair<int32_t, OTPayment*>(
                                        nIndex, thePaymentAngel.release()));
                            }
                            else
                                otOut << __FUNCTION__
                                      << ": Unknown instrument type: "
                                      << str_type.c_str() << "\n";
                        }
                        else
                            otOut << __FUNCTION__
                                  << ": Failed in pPayment->IsValid "
                                     "or pPayment->SetTempValues()\n";
                    }
                } // looping through payments inbox.
                else
                    otWarn << __FUNCTION__
                           << ": Failed loading payments inbox. "
                              "(Probably just doesn't exist yet.)\n";
                // Above we compiled a list of purses, cheques / vouchers to
                // accept.
                // If there are any on that list, then ACCEPT them here.
                //
                if (!thePaymentMap.empty()) {
                    for (mapOfPayments::reverse_iterator it =
                             thePaymentMap.rbegin();
                         it != thePaymentMap.rend();
                         ++it) // backwards since we are processing (removing)
                               // payments by index.
                    {
                        int32_t lIndex = it->first;
                        OTPayment* pPayment = it->second;
                        if (nullptr == pPayment) {
                            otErr << __FUNCTION__
                                  << ": Error: payment pointer was "
                                     "nullptr! (Should never happen.) "
                                     "Skipping.\n";
                            continue;
                        }
                        OTString payment;
                        if (!pPayment->GetPaymentContents(payment)) {
                            otErr << __FUNCTION__
                                  << ": Error: Failed while trying to "
                                     "get payment string contents. "
                                     "(Skipping.)\n";
                            continue;
                        }
                        OTIdentifier paymentAssetType;
                        bool bGotAsset =
                            pPayment->GetAssetTypeID(paymentAssetType);

                        std::string str_asset_type_id;

                        if (bGotAsset) {
                            const OTString strAssetTypeID(paymentAssetType);
                            str_asset_type_id = strAssetTypeID.Get();
                        }
                        if (str_asset_type_id.empty()) {
                            otErr << __FUNCTION__
                                  << ": Error: Failed while trying to "
                                     "get asset type ID from payment. "
                                     "(Skipping.)\n";
                            continue;
                        }
                        // pick an account to deposit the cheque into.
                        for (auto& it_acct : m_accounts) {
                            const std::string& str_account_id(it_acct);
                            const OTIdentifier theAccountID(str_account_id);
                            OTAccount* pAccount =
                                pWallet->GetAccount(theAccountID);
                            OT_ASSERT(nullptr != pAccount);
                            const OTIdentifier& theAcctNymID =
                                pAccount->GetUserID();
                            const OTIdentifier& theAcctServerID =
                                pAccount->GetPurportedServerID();
                            const OTIdentifier& theAcctAssetID =
                                pAccount->GetAssetTypeID();
                            const std::string str_acct_type =
                                pAccount->GetTypeString();
                            //                      const OTString
                            // strAcctNymID   (theAcctNymID);
                            const OTString strAcctServerID(theAcctServerID);
                            const OTString strAcctAssetID(theAcctAssetID);
                            // If the current account is owned by the Nym, AND
                            // it has the same asset type ID
                            // as the cheque being deposited, then let's deposit
                            // the cheque into that account.
                            //
                            // TODO: we should first just see if the default
                            // account matches, instead of doing
                            // this loop in the first place.
                            //
                            if ((theNymID == theAcctNymID) &&
                                (strAcctServerID.Compare(
                                    str_server_id.c_str())) &&
                                (strAcctAssetID.Compare(
                                    str_asset_type_id.c_str())) &&
                                (0 ==
                                 str_acct_type.compare("simple"))) // No issuer
                                                                   // accounts
                            // allowed here. Only
                            // simple accounts.
                            {
                                // Accept it.
                                //
                                OTString strIndices;
                                strIndices.Format("%d", lIndex);
                                const std::string str_indices(strIndices.Get());

                                OT_ME madeEasy;
                                if (!madeEasy.accept_from_paymentbox(
                                        str_account_id, str_indices, "ANY")) {
                                    otErr << __FUNCTION__
                                          << ": Error while trying to "
                                             "accept this instrument.\n";
                                }
                                break;
                            }
                        } // loop through accounts to find one to deposit cheque
                          // into.
                    }     // Loop through payments to deposit.
                    // Empty the list and delete the payments inside.
                    //
                    for (auto& it : thePaymentMap) {
                        OTPayment* pPayment = it.second;
                        if (nullptr != pPayment) delete pPayment;
                        pPayment = nullptr;
                    }
                    thePaymentMap.clear();
                } // if (!thePaymentMap.empty())
            }
        }
    }

    // ASSET ACCOUNT -- INBOX
    //
    // Loop through the Accounts.
    //
    if (m_bAutoAcceptReceipts || m_bAutoAcceptTransfers) {
        int32_t nAccountIndex = -1;
        for (auto& it_acct : m_accounts) {
            ++nAccountIndex; // (0 on first iteration.)
            if (0 == nAccountIndex)
                otOut << "---------------------------------\n " << __FUNCTION__
                      << ": "
                         "Beginning auto-accept loop through the "
                         "accounts in the wallet...\n";
            // For each account, loop through its inbox, outbox, and record box.
            const std::string& str_account_id(it_acct);
            const OTIdentifier theAccountID(str_account_id);
            OTAccount* pAccount = pWallet->GetAccount(theAccountID);
            OT_ASSERT(nullptr != pAccount);
            const OTIdentifier& theNymID = pAccount->GetUserID();
            const OTIdentifier& theServerID = pAccount->GetPurportedServerID();
            const OTIdentifier& theAssetID = pAccount->GetAssetTypeID();
            const OTString strNymID(theNymID);
            const OTString strServerID(theServerID);
            const OTString strAssetID(theAssetID);
            otOut << "------------\n" << __FUNCTION__
                  << ": Account: " << nAccountIndex
                  << ", ID: " << str_account_id.c_str() << "\n";
            const std::string str_nym_id(strNymID.Get());
            const std::string str_server_id(strServerID.Get());
            const std::string str_asset_id(strAssetID.Get());
            // NOTE: Since this account is already on my "care about" list for
            // accounts,
            // I wouldn't bother double-checking my "care about" lists for
            // servers, nyms,
            // and asset types. But I still look up the appropriate string for
            // each, since
            // I have to pass a reference to it into the constructor for
            // OTRecord. (To a version
            // that won't be deleted, since the OTRecord will reference it. And
            // the "care about"
            // list definitely contains a copy of the string that won't be
            // deleted.)
            //
            auto it_nym = std::find(m_nyms.begin(), m_nyms.end(), str_nym_id);
            auto it_server =
                std::find(m_servers.begin(), m_servers.end(), str_server_id);
            auto it_asset = m_assets.find(str_asset_id);
            if ((m_nyms.end() == it_nym) || (m_servers.end() == it_server) ||
                (m_assets.end() == it_asset)) {
                otOut << __FUNCTION__ << ": Skipping an account ("
                      << str_account_id.c_str()
                      << ") since its Nym, or Server, "
                         "or Asset Type wasn't on my list.\n";
                continue;
            }
            // Loop through asset account INBOX.
            //
            // OPTIMIZE FYI:
            // NOTE: LoadInbox is much SLOWER than LoadInboxNoVerify, but it
            // also lets you get
            // the NAME off of the box receipt. So if you are willing to GIVE UP
            // the NAME, in
            // return for FASTER PERFORMANCE, then call SetFastMode() before
            // Populating.
            //
            OTLedger* pInbox = m_bRunFast
                                   ? OTAPI_Wrap::OTAPI()->LoadInboxNoVerify(
                                         theServerID, theNymID, theAccountID)
                                   : OTAPI_Wrap::OTAPI()->LoadInbox(
                                         theServerID, theNymID, theAccountID);
            std::unique_ptr<OTLedger> theInboxAngel(pInbox);
            if (nullptr == pInbox) {
                otOut << __FUNCTION__ << ": Skipping an account ("
                      << str_account_id.c_str()
                      << ") since its "
                         "inbox failed to load (have you downloaded "
                         "the latest one?)\n";
                continue;
            }
            const OTString strInbox(*pInbox);
            const std::string str_inbox(strInbox.Get());
            bool bFoundAnyToAccept = false;
            std::string strResponseLedger;
            int32_t nInboxIndex = -1;
            // It loaded up, so let's loop through it.
            for (auto& it : pInbox->GetTransactionMap()) {
                ++nInboxIndex; // (0 on first iteration.)
                if (0 == nInboxIndex)
                    otOut
                        << __FUNCTION__
                        << ": Beginning loop through asset account INBOX...\n";
                OTTransaction* pBoxTrans = it.second;
                OT_ASSERT(nullptr != pBoxTrans);
                otOut << __FUNCTION__ << ": Inbox index: " << nInboxIndex
                      << "\n";
                const std::string str_type(
                    pBoxTrans->GetTypeString()); // pending, chequeReceipt, etc.
                const bool bIsTransfer =
                    (OTTransaction::pending == pBoxTrans->GetType());
                const bool bIsReceipt = !bIsTransfer;
                if ((m_bAutoAcceptReceipts && bIsReceipt) ||
                    (m_bAutoAcceptTransfers && bIsTransfer)) {
                    otOut << __FUNCTION__ << ": Auto-accepting: incoming "
                          << (bIsTransfer ? "pending transfer" : "receipt")
                          << " (str_type: " << str_type.c_str() << ")\n";
                    // If we haven't found any yet, then this must be the first
                    // one!
                    //
                    if (!bFoundAnyToAccept) {
                        bFoundAnyToAccept = true;
                        OT_ME madeEasy;

                        int32_t nNumberNeeded = 20;
                        if (!madeEasy.make_sure_enough_trans_nums(
                                nNumberNeeded, // I'm just hardcoding: "Make
                                               // sure I have at least 20
                                               // transaction numbers."
                                str_server_id, str_nym_id)) {
                            otOut << "\n\nFailure: "
                                     "make_sure_enough_trans_nums: "
                                     "returned false. (Skipping inbox "
                                     "for account " << str_account_id.c_str()
                                  << ")\n";
                            continue;
                        }
                        strResponseLedger =
                            OTAPI_Wrap::It()->Ledger_CreateResponse(
                                str_server_id, str_nym_id, str_account_id,
                                str_inbox);

                        if (strResponseLedger.empty()) {
                            otOut << "\n\nFailure: "
                                     "OT_API_Ledger_CreateResponse "
                                     "returned nullptr. (Skipping inbox "
                                     "for account " << str_account_id.c_str()
                                  << ")\n";
                            continue;
                        }
                    }
                    const OTString strTrans(*pBoxTrans);
                    const std::string str_trans(strTrans.Get());
                    std::string strNEW_ResponseLEDGER =
                        OTAPI_Wrap::It()->Transaction_CreateResponse(
                            str_server_id, str_nym_id, str_account_id,
                            strResponseLedger, str_trans,
                            true); // accept = true (versus rejecting a pending
                                   // transfer, for example.)

                    if (strNEW_ResponseLEDGER.empty()) {
                        otOut << "\n\nFailure: "
                                 "OT_API_Transaction_CreateResponse "
                                 "returned nullptr. (Skipping inbox for "
                                 "account " << str_account_id.c_str() << ")\n";
                        continue;
                    }
                    strResponseLedger = strNEW_ResponseLEDGER;
                }
            }
            // Okay now we have the response ledger all ready to go, let's
            // process it!
            //
            if (bFoundAnyToAccept && !strResponseLedger.empty()) {
                std::string strFinalizedResponse =
                    OTAPI_Wrap::It()->Ledger_FinalizeResponse(
                        str_server_id, str_nym_id, str_account_id,
                        strResponseLedger);

                if (strFinalizedResponse.empty()) {
                    otOut << "\n\nFailure: "
                             "OT_API_Ledger_FinalizeResponse returned "
                             "nullptr. (Skipping inbox for account "
                          << str_account_id.c_str() << ")\n";
                    continue;
                }
                // Instantiate the "OT Made Easy" object.
                //
                OT_ME madeEasy;

                // Server communications are handled here...
                //
                std::string strResponse = madeEasy.process_inbox(
                    str_server_id, str_nym_id, str_account_id,
                    strFinalizedResponse);
                std::string strAttempt = "process_inbox";

                int32_t nInterpretReply = madeEasy.InterpretTransactionMsgReply(
                    str_server_id, str_nym_id, str_account_id, strAttempt,
                    strResponse);

                if (1 == nInterpretReply) {
                    // Download all the intermediary files (account balance,
                    // inbox, outbox, etc)
                    // since they have probably changed from this operation.
                    //
                    bool bRetrieved = madeEasy.retrieve_account(
                        str_server_id, str_nym_id, str_account_id,
                        true); // bForceDownload defaults to false.

                    otOut << "\n\nServer response (" << strAttempt.c_str()
                          << "): SUCCESS "
                             "processing/accepting inbox.\n";
                    otOut << (bRetrieved ? "Success" : "Failed")
                          << " retrieving intermediary files for account.\n";
                }
            }
        }
    }
    return true;
}

// POPULATE:

// Populates m_contents from OT API. Calls ClearContents().

bool OTRecordList::Populate()
{
    OT_ASSERT(nullptr != m_pLookup);
    ClearContents();
    // Loop through all the accounts.
    //
    // From Open-Transactions.h:
    // OTAPI_Wrap::OTAPI()->GetServerCount()
    //
    // From OTAPI.h:
    // OTAPI_Wrap::GetServerCount()  // wraps the above call.
    //
    OTWallet* pWallet = OTAPI_Wrap::OTAPI()->GetWallet(
        __FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) {
        otErr << "OTRecordList::" << __FUNCTION__
              << ": Error: Wallet is nullptr.\n";
        return false;
    }
    // Before populating, process out any items we're supposed to accept
    // automatically.
    //
    PerformAutoAccept();
    // OUTPAYMENTS, OUTMAIL, MAIL, PAYMENTS INBOX, and RECORD BOX (2 kinds.)
    // Loop through the Nyms.
    //
    int32_t nNymIndex = -1;
    for (auto& it_nym : m_nyms) {
        ++nNymIndex;
        if (0 == nNymIndex)
            otOut << "=============== " << __FUNCTION__
                  << ": Beginning loop through Nyms...\n";
        const std::string& str_nym_id(it_nym);
        const OTIdentifier theNymID(str_nym_id);
        const OTString strNymID(theNymID);
        OTPseudonym* pNym = pWallet->GetNymByID(theNymID);
        if (nullptr == pNym) continue;
        // For each Nym, loop through his OUTPAYMENTS box.
        //
        const int32_t nOutpaymentsCount =
            OTAPI_Wrap::GetNym_OutpaymentsCount(str_nym_id);

        otOut << "--------\n" << __FUNCTION__ << ": Nym " << nNymIndex
              << ", nOutpaymentsCount: " << nOutpaymentsCount
              << ", ID: " << strNymID.Get() << "\n";
        for (int32_t nCurrentOutpayment = 0;
             nCurrentOutpayment < nOutpaymentsCount; ++nCurrentOutpayment) {
            otOut << __FUNCTION__
                  << ": Outpayment instrument: " << nCurrentOutpayment << "\n";
            const OTString strOutpayment(
                OTAPI_Wrap::GetNym_OutpaymentsContentsByIndex(
                    str_nym_id, nCurrentOutpayment));
            std::string str_memo;
            OTPayment theOutPayment(strOutpayment);

            if (!theOutPayment.IsValid() || !theOutPayment.SetTempValues()) {
                otErr << __FUNCTION__
                      << ": Skipping: Unable to load outpayments "
                         "instrument from string:\n" << strOutpayment.Get()
                      << "\n";
                continue;
            }
            int64_t lAmount = 0;
            std::string str_amount; // <========== AMOUNT

            if (theOutPayment.GetAmount(lAmount)) {
                if (((OTPayment::CHEQUE == theOutPayment.GetType()) ||
                     (OTPayment::PURSE == theOutPayment.GetType()) ||
                     (OTPayment::VOUCHER == theOutPayment.GetType())) &&
                    (lAmount > 0))
                    lAmount *= (-1);

                if ((OTPayment::INVOICE == theOutPayment.GetType()) &&
                    (lAmount < 0))
                    lAmount *= (-1);

                OTString strTemp;
                strTemp.Format("%" PRId64 "", lAmount);
                str_amount = strTemp.Get();
            }
            OTIdentifier theAssetTypeID;
            const std::string* p_str_asset_type =
                &OTRecordList::s_blank; // <========== ASSET TYPE
            const std::string* p_str_asset_name =
                &OTRecordList::s_blank;   // asset type display name.
            std::string str_outpmt_asset; // The asset type we found on the
                                          // payment (if we found anything.)

            if (theOutPayment.GetAssetTypeID(theAssetTypeID)) {
                OTString strTemp(theAssetTypeID);
                str_outpmt_asset = strTemp.Get();
                auto it_asset = m_assets.find(str_outpmt_asset);
                if (it_asset != m_assets.end()) // Found it on the map of asset
                                                // types we care about.
                {
                    p_str_asset_type =
                        &(it_asset->first); // Set the asset type ID.
                    p_str_asset_name = &(it_asset->second); // The CurrencyTLA.
                                                            // Examples: USD,
                                                            // BTC, etc.
                }
                else {
                    // There was definitely an asset type on the instrument, and
                    // it definitely
                    // did not match any of the assets that we care about.
                    // Therefore, skip.
                    //
                    otOut << __FUNCTION__
                          << ": Skipping outpayment (we don't care "
                             "about asset type " << str_outpmt_asset.c_str()
                          << ")\n";
                    continue;
                }
            }
            // By this point, p_str_asset_type and p_str_asset_name are
            // definitely set.
            OT_ASSERT(nullptr != p_str_asset_type); // and it's either blank, or
            // it's one of the asset types
            // we care about.
            OT_ASSERT(nullptr != p_str_asset_name); // and it's either blank, or
            // it's one of the asset types
            // we care about.
            OTIdentifier theAccountID;
            const std::string* p_str_account =
                &OTRecordList::s_blank;     // <========== ACCOUNT
            std::string str_outpmt_account; // The accountID we found on the
                                            // payment (if we found anything.)

            if (theOutPayment.GetSenderAcctIDForDisplay(
                    theAccountID)) // Since Nym is ME, the Account must be MY
                                   // acct.
            { // (In Outpayments, the SENDER's account is MY acct.)
                OTString strTemp(theAccountID);
                str_outpmt_account = strTemp.Get();
                auto it_acct = std::find(m_accounts.begin(), m_accounts.end(),
                                         str_outpmt_account);
                if (it_acct != m_accounts.end()) // Found it on the list of
                                                 // accounts we care about.
                {
                    p_str_account = &(*it_acct);
                }
                // We don't skip vouchers since the sender account (e.g. the
                // server's account)
                // is definitely not one of my accounts -- so the voucher would
                // end up getting
                // skipped every single time.
                //
                //              else if (OTPayment::VOUCHER !=
                // theOutPayment.GetType())
                else {
                    // There was definitely an account on the instrument, and it
                    // definitely
                    // did not match any of the accounts that we care about.
                    // Therefore, skip.
                    //
                    otOut << __FUNCTION__
                          << ": Skipping outpayment (we don't care "
                             "about account " << str_outpmt_account.c_str()
                          << ")\n";
                    continue;
                }
            }
            // By this point, p_str_account is definitely set.
            OT_ASSERT(nullptr !=
                      p_str_account); // and it's either blank, or it's
                                      // one of the accounts we care
                                      // about.
            // strOutpayment contains the actual outgoing payment instrument.
            //
            const std::string str_outpmt_server =
                OTAPI_Wrap::GetNym_OutpaymentsServerIDByIndex(
                    str_nym_id, nCurrentOutpayment);
            const std::string str_outpmt_recipientID =
                OTAPI_Wrap::GetNym_OutpaymentsRecipientIDByIndex(
                    str_nym_id, nCurrentOutpayment);
            // str_outpmt_server is the server for this outpayment.
            // But is that server on our list of servers that we care about?
            // Let's see if that server is on m_servers (otherwise we can skip
            // it.)
            // Also, let's do the same for asset types.
            //
            auto it_server = std::find(m_servers.begin(), m_servers.end(),
                                       str_outpmt_server);

            if (it_server != m_servers.end()) // Found the serverID on the list
                                              // of servers we care about.
            {
                // TODO OPTIMIZE: instead of looking up the Nym's name every
                // time, look it
                // up ONCE when first adding the NymID. Add it to a map, instead
                // of a list,
                // and add the Nym's name as the second item in the map's pair.
                // (Just like I already did with the asset type.)
                //
                OTString strName(m_pLookup->GetNymName(str_outpmt_recipientID,
                                                       &(*it_server))),
                    strNameTemp;
                std::string str_name;

                if (strName.Exists())
                    strNameTemp.Format(OTRecordList::textTo(), strName.Get());
                else
                    strNameTemp.Format(OTRecordList::textTo(),
                                       str_outpmt_recipientID.c_str());

                str_name = strNameTemp.Get();
                OTString strMemo;
                if (theOutPayment.GetMemo(strMemo)) {
                    str_memo = strMemo.Get();
                }
                // For the "date" on this record we're using the "valid from"
                // date on the instrument.
                std::string str_date = "0";
                time64_t tFrom = OT_TIME_ZERO;
                time64_t tTo = OT_TIME_ZERO;

                if (theOutPayment.GetValidFrom(tFrom)) {
                    const uint64_t lFrom = OTTimeGetSecondsFromTime(tFrom);
                    OTString strFrom;
                    strFrom.Format("%" PRIu64 "", lFrom);
                    str_date = strFrom.Get();
                }
                theOutPayment.GetValidTo(tTo);
                // Instrument type (cheque, voucher, etc)
                //
                int32_t nType = static_cast<int32_t>(theOutPayment.GetType());

                const std::string& str_type = GetTypeString(nType);
                // CREATE A OTRecord AND POPULATE IT...
                //
                otOut << __FUNCTION__
                      << ": ADDED: pending outgoing instrument (str_type: "
                      << str_type.c_str() << ")\n";

                shared_ptr_OTRecord sp_Record(new OTRecord(
                    *it_server, *p_str_asset_type, *p_str_asset_name,
                    str_nym_id,     // This is the Nym WHOSE BOX IT IS.
                    *p_str_account, // This is the Nym's account according to
                                    // the payment instrument, IF that account
                                    // was found on our list of accounts we care
                                    // about. Or it's blank if no account was
                                    // found on the payment instrument.
                    // Everything above this line, it stores a reference to an
                    // external string.
                    // Everything below this line, it makes its own internal
                    // copy of the string.
                    str_name, // name of recipient (since its in outpayments
                              // box.)
                    str_date, // the "valid from" date on the instrument.
                    str_amount,
                    str_type, // cheque, voucher, smart contract, etc
                    true,  // bIsPending=true since its in the outpayments box.
                    true,  // bIsOutgoing=true. Otherwise it'd be in record box
                           // already.
                    false, // IsRecord
                    false, // IsReceipt
                    OTRecord::Instrument));
                sp_Record->SetContents(strOutpayment.Get());
                sp_Record->SetOtherNymID(str_outpmt_recipientID);
                if (!str_memo.empty()) sp_Record->SetMemo(str_memo);
                sp_Record->SetDateRange(tFrom, tTo);
                sp_Record->SetBoxIndex(nCurrentOutpayment);
                int64_t lTransNum = 0;
                theOutPayment.GetOpeningNum(lTransNum, theNymID);
                sp_Record->SetTransactionNum(lTransNum);
                m_contents.push_back(sp_Record);
            }
            else // the server for this outpayment is not on the list of
                   // servers we care about. Skip this outpayment.
            {
                otOut << __FUNCTION__
                      << ": Skipping outgoing instrument (we don't "
                         "care about server " << str_outpmt_server.c_str()
                      << ")\n";
                continue;
            }
        } // for outpayments.
        // For each Nym, loop through his MAIL box.
        //
        const int32_t nMailCount = OTAPI_Wrap::GetNym_MailCount(str_nym_id);
        for (int32_t nCurrentMail = 0; nCurrentMail < nMailCount;
             ++nCurrentMail) {
            otOut << __FUNCTION__ << ": Mail index: " << nCurrentMail << "\n";
            OTMessage* pMsg = pNym->GetMailByIndex(nCurrentMail);
            OT_ASSERT(nullptr != pMsg);
            const std::string str_mail_server =
                OTAPI_Wrap::GetNym_MailServerIDByIndex(str_nym_id,
                                                       nCurrentMail);
            const std::string str_mail_senderID =
                OTAPI_Wrap::GetNym_MailSenderIDByIndex(str_nym_id,
                                                       nCurrentMail);
            // str_mail_server is the server for this mail.
            // But is that server on our list of servers that we care about?
            // Let's see if that server is on m_servers (otherwise we can skip
            // it.)
            //
            auto it_server =
                std::find(m_servers.begin(), m_servers.end(), str_mail_server);

            if (it_server != m_servers.end()) // Found the serverID on the list
                                              // of servers we care about.
            {
                // TODO OPTIMIZE: instead of looking up the Nym's name every
                // time, look it
                // up ONCE when first adding the NymID. Add it to a map, instead
                // of a list,
                // and add the Nym's name as the second item in the map's pair.
                // (Just like I already did with the asset type.)
                //
                OTString strName(
                    m_pLookup->GetNymName(str_mail_senderID, &(*it_server))),
                    strNameTemp;
                std::string str_name;

                if (strName.Exists())
                    strNameTemp.Format(OTRecordList::textFrom(), strName.Get());
                else
                    strNameTemp.Format(OTRecordList::textFrom(),
                                       str_mail_senderID.c_str());

                str_name = strNameTemp.Get();
                const std::string* p_str_asset_type =
                    &OTRecordList::s_blank; // <========== ASSET TYPE
                const std::string* p_str_asset_name =
                    &OTRecordList::s_blank; // asset type display name.
                const std::string* p_str_account =
                    &OTRecordList::s_blank; // <========== ACCOUNT

                std::string str_amount; // There IS NO amount, on mail. (So we
                                        // leave this empty.)

                uint64_t lDate = pMsg->m_lTime;
                OTString strDate;
                strDate.Format("%" PRIu64 "", lDate);
                const std::string str_date(strDate.Get());
                // CREATE A OTRecord AND POPULATE IT...
                //
                otOut << __FUNCTION__ << ": ADDED: incoming mail.\n";

                shared_ptr_OTRecord sp_Record(new OTRecord(
                    *it_server, *p_str_asset_type, *p_str_asset_name,
                    str_nym_id,     // This is the Nym WHOSE BOX IT IS.
                    *p_str_account, // This is the Nym's account according to
                                    // the payment instrument, IF that account
                                    // was found on our list of accounts we care
                                    // about. Or it's blank if no account was
                                    // found on the payment instrument.
                    // Everything above this line, it stores a reference to an
                    // external string.
                    // Everything below this line, it makes its own internal
                    // copy of the string.
                    str_name, // name of sender (since its in incoming mail
                              // box.)
                    str_date, // How do we get the date from a mail?
                    str_amount, OTRecordList::s_message_type, // "message"
                    false, // bIsPending=false since its already received.
                    false, // bIsOutgoing=false. It's incoming mail, not
                           // outgoing mail.
                    false, // IsRecord
                    false, // IsReceipt
                    OTRecord::Mail));
                const OTString strMail(OTAPI_Wrap::GetNym_MailContentsByIndex(
                    str_nym_id, nCurrentMail));
                sp_Record->SetContents(strMail.Get());
                sp_Record->SetOtherNymID(str_mail_senderID);
                sp_Record->SetBoxIndex(nCurrentMail);
                sp_Record->SetDateRange(
                    OTTimeGetTimeFromSeconds(pMsg->m_lTime),
                    OTTimeGetTimeFromSeconds(pMsg->m_lTime));
                m_contents.push_back(sp_Record);
            }
        } // loop through incoming Mail.
        // Outmail
        //
        const int32_t nOutmailCount =
            OTAPI_Wrap::GetNym_OutmailCount(str_nym_id);
        for (int32_t nCurrentOutmail = 0; nCurrentOutmail < nOutmailCount;
             ++nCurrentOutmail) {
            otOut << __FUNCTION__ << ": Outmail index: " << nCurrentOutmail
                  << "\n";
            OTMessage* pMsg = pNym->GetOutmailByIndex(nCurrentOutmail);
            OT_ASSERT(nullptr != pMsg);
            const std::string str_mail_server =
                OTAPI_Wrap::GetNym_OutmailServerIDByIndex(str_nym_id,
                                                          nCurrentOutmail);
            const std::string str_mail_recipientID =
                OTAPI_Wrap::GetNym_OutmailRecipientIDByIndex(str_nym_id,
                                                             nCurrentOutmail);
            // str_mail_server is the server for this mail.
            // But is that server on our list of servers that we care about?
            // Let's see if that server is on m_servers (otherwise we can skip
            // it.)
            //
            auto it_server =
                std::find(m_servers.begin(), m_servers.end(), str_mail_server);

            if (it_server != m_servers.end()) // Found the serverID on the list
                                              // of servers we care about.
            {
                // TODO OPTIMIZE: instead of looking up the Nym's name every
                // time, look it
                // up ONCE when first adding the NymID. Add it to a map, instead
                // of a list,
                // and add the Nym's name as the second item in the map's pair.
                // (Just like I already did with the asset type.)
                //
                OTString strName(
                    m_pLookup->GetNymName(str_mail_recipientID, &(*it_server))),
                    strNameTemp;
                std::string str_name;

                if (strName.Exists())
                    strNameTemp.Format(OTRecordList::textTo(), strName.Get());
                else
                    strNameTemp.Format(OTRecordList::textTo(),
                                       str_mail_recipientID.c_str());

                str_name = strNameTemp.Get();
                const std::string* p_str_asset_type =
                    &OTRecordList::s_blank; // <========== ASSET TYPE
                const std::string* p_str_asset_name =
                    &OTRecordList::s_blank; // asset type display name.
                const std::string* p_str_account =
                    &OTRecordList::s_blank; // <========== ACCOUNT

                std::string str_amount; // There IS NO amount, on mail. (So we
                                        // leave this empty.)

                uint64_t lDate = pMsg->m_lTime;
                OTString strDate;
                strDate.Format("%" PRIu64 "", lDate);
                const std::string str_date(strDate.Get());
                // CREATE A OTRecord AND POPULATE IT...
                //
                otOut << __FUNCTION__ << ": ADDED: sent mail.\n";

                shared_ptr_OTRecord sp_Record(new OTRecord(
                    *it_server, *p_str_asset_type, *p_str_asset_name,
                    str_nym_id,     // This is the Nym WHOSE BOX IT IS.
                    *p_str_account, // This is the Nym's account according to
                                    // the payment instrument, IF that account
                                    // was found on our list of accounts we care
                                    // about. Or it's blank if no account was
                                    // found on the payment instrument.
                    // Everything above this line, it stores a reference to an
                    // external string.
                    // Everything below this line, it makes its own internal
                    // copy of the string.
                    str_name, // name of recipient (since its in outgoing mail
                              // box.)
                    str_date, // How do we get the date from a mail?
                    str_amount, OTRecordList::s_message_type, // "message"
                    false, // bIsPending=false since its already sent.
                    true,  // bIsOutgoing=true. It's OUTGOING mail.
                    false, // IsRecord (it's not in the record box.)
                    false, // IsReceipt
                    OTRecord::Mail));
                const OTString strOutmail(
                    OTAPI_Wrap::GetNym_OutmailContentsByIndex(str_nym_id,
                                                              nCurrentOutmail));
                sp_Record->SetContents(strOutmail.Get());
                sp_Record->SetBoxIndex(nCurrentOutmail);
                sp_Record->SetOtherNymID(str_mail_recipientID);
                sp_Record->SetDateRange(
                    OTTimeGetTimeFromSeconds(pMsg->m_lTime),
                    OTTimeGetTimeFromSeconds(pMsg->m_lTime));
                m_contents.push_back(sp_Record);
            }
        } // loop through outgoing Mail.
        // For each nym, for each server, loop through its payments inbox and
        // record box.
        //
        int32_t nServerIndex = -1;
        for (auto& it_server : m_servers) {
            ++nServerIndex;
            const OTIdentifier theServerID(it_server);
            OTServerContract* pServer = pWallet->GetServerContract(theServerID);
            OT_ASSERT(nullptr != pServer);
            const OTString strServerID(theServerID);
            otOut << __FUNCTION__ << ": Server " << nServerIndex
                  << ", ID: " << strServerID.Get() << "\n";
            // OPTIMIZE FYI:
            // The "NoVerify" version is much faster, but you will lose the
            // ability to get the
            // sender/recipient name from the receipts in the box. The code
            // will, however, work
            // either way.
            //
            OTLedger* pInbox =
                m_bRunFast
                    ? OTAPI_Wrap::OTAPI()->LoadPaymentInboxNoVerify(theServerID,
                                                                    theNymID)
                    : OTAPI_Wrap::OTAPI()->LoadPaymentInbox(theServerID,
                                                            theNymID);
            std::unique_ptr<OTLedger> theInboxAngel(pInbox);

            int32_t nIndex = (-1);
            // It loaded up, so let's loop through it.
            if (nullptr != pInbox) {
                for (auto& it : pInbox->GetTransactionMap()) {
                    OTTransaction* pBoxTrans = it.second;
                    OT_ASSERT(nullptr != pBoxTrans);
                    ++nIndex; // 0 on first iteration.
                    otOut << __FUNCTION__ << ": Incoming payment: " << nIndex
                          << "\n";
                    std::string str_name; // name of sender (since its in the
                                          // payments inbox.)
                    std::string str_sender_nym_id;
                    std::string str_sender_acct_id;

                    if (!pBoxTrans->IsAbbreviated()) {
                        OTIdentifier theSenderID;

                        if (pBoxTrans->GetSenderUserIDForDisplay(theSenderID)) {
                            const OTString strSenderID(theSenderID);
                            str_sender_nym_id = strSenderID.Get();

                            OTString strName(m_pLookup->GetNymName(
                                str_sender_nym_id, &it_server)),
                                strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(OTRecordList::textFrom(),
                                                   strName.Get());
                            else
                                strNameTemp.Format(OTRecordList::textFrom(),
                                                   str_sender_nym_id.c_str());

                            str_name = strNameTemp.Get();
                        }

                        theSenderID.clear();

                        if (pBoxTrans->GetSenderAcctIDForDisplay(theSenderID)) {
                            const OTString strSenderID(theSenderID);
                            str_sender_acct_id = strSenderID.Get();
                        }
                    }
                    time64_t tValidFrom = OT_TIME_ZERO, tValidTo = OT_TIME_ZERO;
                    std::string str_date =
                        "0"; // the "date signed" on the transaction receipt.
                    time64_t tDateSigned = pBoxTrans->GetDateSigned();

                    if (tDateSigned > OT_TIME_ZERO) {
                        tValidFrom = tDateSigned;
                        const uint64_t lDateSigned =
                            OTTimeGetSecondsFromTime(tDateSigned);
                        OTString strDateSigned;
                        strDateSigned.Format("%" PRIu64 "", lDateSigned);
                        str_date = strDateSigned.Get();
                    }
                    const std::string* p_str_asset_type =
                        &OTRecordList::s_blank; // <========== ASSET TYPE
                    const std::string* p_str_asset_name =
                        &OTRecordList::s_blank; // asset type display name.
                    std::string str_amount;     // <========== AMOUNT
                    std::string str_type;       // Instrument type.
                    std::string str_memo;
                    OTString strContents; // Instrument contents.

                    if (pBoxTrans->IsAbbreviated()) {
                        str_type =
                            pBoxTrans->GetTypeString(); // instrumentNotice,
                                                        // etc.
                        int64_t lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                        if (0 != lAmount) {
                            OTString strTemp;
                            strTemp.Format("%" PRId64 "", lAmount);
                            str_amount = strTemp.Get();
                        }
                    }
                    else // NOT abbreviated. (Full box receipt is already
                           // loaded.)
                    {
                        OTPayment* pPayment =
                            GetInstrument(*pNym, nIndex, *pInbox);
                        // ===> Returns financial instrument by index.
                        std::unique_ptr<OTPayment> thePaymentAngel(pPayment);
                        if (nullptr ==
                            pPayment) // then we treat it like it's abbreviated.
                        {
                            str_type =
                                pBoxTrans->GetTypeString(); // instrumentNotice,
                                                            // etc.
                            int64_t lAmount =
                                pBoxTrans->GetAbbrevDisplayAmount();

                            if (0 == lAmount)
                                lAmount = pBoxTrans->GetReceiptAmount();
                            if (0 != lAmount) {
                                OTString strTemp;
                                strTemp.Format("%" PRId64 "", lAmount);
                                str_amount = strTemp.Get();
                            }
                        }
                        // We have pPayment, the instrument accompanying the
                        // receipt in the payments inbox.
                        //
                        else if (pPayment->SetTempValues()) {
                            pPayment->GetValidFrom(tValidFrom);
                            pPayment->GetValidTo(tValidTo);

                            if (tValidFrom > OT_TIME_ZERO) {
                                const uint64_t lFrom =
                                    OTTimeGetSecondsFromTime(tValidFrom);
                                OTString strFrom;
                                strFrom.Format("%" PRIu64 "", lFrom);
                                str_date = strFrom.Get();
                            }
                            OTString strMemo;
                            if (pPayment->GetMemo(strMemo)) {
                                str_memo = strMemo.Get();
                            }
                            pPayment->GetPaymentContents(strContents);
                            OTIdentifier theAssetTypeID, theSenderAcctID;

                            if (pPayment->GetAssetTypeID(theAssetTypeID)) {
                                OTString strTemp(theAssetTypeID);
                                const std::string str_inpmt_asset(
                                    strTemp.Get()); // The asset type we found
                                                    // on the payment (if we
                                                    // found anything.)
                                auto it_asset = m_assets.find(str_inpmt_asset);
                                if (it_asset != m_assets.end()) // Found it on
                                                                // the map of
                                                                // asset types
                                                                // we care
                                                                // about.
                                {
                                    p_str_asset_type =
                                        &(it_asset->first); // Set the asset
                                                            // type ID.
                                    p_str_asset_name =
                                        &(it_asset->second); // The CurrencyTLA.
                                                             // Examples: USD,
                                                             // BTC, etc.
                                }
                                else {
                                    // There was definitely an asset type on the
                                    // instrument, and it definitely
                                    // did not match any of the assets that we
                                    // care about.
                                    // Therefore, skip.
                                    //
                                    otErr << __FUNCTION__
                                          << ": Skipping: Incoming payment (we "
                                             "don't care about asset "
                                          << str_inpmt_asset.c_str() << ")\n";
                                    continue;
                                }
                            }
                            if (str_sender_acct_id.empty() &&
                                pPayment->GetSenderAcctIDForDisplay(
                                    theSenderAcctID)) {
                                OTString strTemp(theSenderAcctID);
                                str_sender_acct_id = strTemp.Get();
                            }
                            // By this point, p_str_asset_type and
                            // p_str_asset_name are definitely set.
                            OT_ASSERT(nullptr != p_str_asset_type); // and it's
                                                                    // either
                                                                    // blank, or
                            // it's one of
                            // the asset
                            // types we
                            // care about.
                            OT_ASSERT(nullptr != p_str_asset_name); // and it's
                                                                    // either
                                                                    // blank, or
                            // it's one of
                            // the asset
                            // types we
                            // care about.
                            // Instrument type (cheque, voucher, etc)
                            int32_t nType =
                                static_cast<int32_t>(pPayment->GetType());

                            str_type = GetTypeString(nType);
                            int64_t lAmount = 0;

                            if (pPayment->GetAmount(lAmount)) {
                                OTString strTemp;
                                strTemp.Format("%" PRId64 "", lAmount);
                                str_amount = strTemp.Get();
                            }
                        }
                    }
                    otOut << __FUNCTION__
                          << ": ADDED: pending incoming payment (str_type: "
                          << str_type.c_str() << ")\n";

                    shared_ptr_OTRecord sp_Record(new OTRecord(
                        it_server, *p_str_asset_type, *p_str_asset_name,
                        str_nym_id, // This is the Nym WHOSE BOX IT IS.
                        OTRecordList::s_blank, // This is the Nym's account for
                                               // this box. (Blank for payments
                                               // inbox.)
                        // Everything above this line, it stores a reference to
                        // an external string.
                        // Everything below this line, it makes its own internal
                        // copy of the string.
                        str_name, // name of sender (since its in the inbox.)
                        str_date, // the "valid from" date on the instrument.
                        str_amount, str_type, // pending, chequeReceipt, etc.
                        true,  // I believe all incoming "payment inbox" items
                               // are pending. (Cheques waiting to be cashed,
                               // smart contracts waiting to be signed, etc.)
                        false, // bIsOutgoing=false. (Since this is the payment
                               // INbox, nothing is outgoing...)
                        false, // bIsRecord
                        false, // bIsReceipt
                        OTRecord::Instrument));
                    if (strContents.Exists())
                        sp_Record->SetContents(strContents.Get());
                    sp_Record->SetDateRange(tValidFrom, tValidTo);
                    sp_Record->SetBoxIndex(nIndex);
                    if (!str_memo.empty()) sp_Record->SetMemo(str_memo);
                    if (!str_sender_nym_id.empty())
                        sp_Record->SetOtherNymID(str_sender_nym_id);
                    if (!str_sender_acct_id.empty())
                        sp_Record->SetOtherAccountID(str_sender_acct_id);
                    sp_Record->SetTransNumForDisplay(
                        pBoxTrans->GetReferenceNumForDisplay());
                    sp_Record->SetTransactionNum(
                        pBoxTrans->GetTransactionNum());

                    m_contents.push_back(sp_Record);

                } // looping through inbox.
            }
            else
                otWarn << __FUNCTION__
                       << ": Failed loading payments inbox. "
                          "(Probably just doesn't exist yet.)\n";
            nIndex = (-1);

            // Also loop through its record box. For this record box, pass the
            // USER_ID twice,
            // since it's the recordbox for the Nym.
            // OPTIMIZE FYI: m_bRunFast impacts run speed here.
            OTLedger* pRecordbox =
                m_bRunFast
                    ? OTAPI_Wrap::OTAPI()->LoadRecordBoxNoVerify(
                          theServerID, theNymID, theNymID)
                    : // twice.
                    OTAPI_Wrap::OTAPI()->LoadRecordBox(theServerID, theNymID,
                                                       theNymID);
            std::unique_ptr<OTLedger> theRecordBoxAngel(pRecordbox);

            // It loaded up, so let's loop through it.
            if (nullptr != pRecordbox) {
                for (auto& it : pRecordbox->GetTransactionMap()) {
                    OTTransaction* pBoxTrans = it.second;
                    OT_ASSERT(nullptr != pBoxTrans);
                    bool bOutgoing = false;
                    ++nIndex; // 0 on first iteration.
                    otOut << __FUNCTION__
                          << ": Payment RECORD index: " << nIndex << "\n";
                    std::string str_name; // name of sender OR recipient
                                          // (depending on whether it was
                                          // originally incoming or outgoing.)
                    std::string str_other_nym_id;
                    std::string str_other_acct_id;

                    if (!pBoxTrans->IsAbbreviated()) {
                        OTIdentifier theSenderID, theSenderAcctID;
                        OTIdentifier theRecipientID, theRecipientAcctID;

                        if (pBoxTrans->GetSenderUserIDForDisplay(theSenderID)) {
                            const OTString strSenderID(theSenderID);
                            const std::string str_sender_id(strSenderID.Get());

                            // Usually, Nym is the RECIPIENT. Sometimes he's the
                            // sender.
                            // Either way, we want the OTHER ID (the other Nym)
                            // for display.
                            // So here, if Nym's CLEARLY the sender, then we
                            // want the RECIPIENT.
                            // Whereas if Nym were the recipient, then we'd want
                            // the SENDER. (For display.)
                            //
                            if (0 ==
                                str_nym_id.compare(str_sender_id)) // str_nym_id
                                                                   // IS
                            // str_sender_id.
                            // (Therefore we want
                            // recipient.)
                            {
                                bOutgoing = true; // if Nym is the sender, then
                                                  // it must have been outgoing.

                                if (pBoxTrans->GetRecipientUserIDForDisplay(
                                        theRecipientID)) {
                                    const OTString strRecipientID(
                                        theRecipientID);
                                    const std::string str_recipient_id(
                                        strRecipientID.Get());

                                    OTString strName(m_pLookup->GetNymName(
                                        str_recipient_id, &it_server)),
                                        strNameTemp;

                                    if (strName.Exists())
                                        strNameTemp.Format(
                                            OTRecordList::textTo(),
                                            strName.Get());
                                    else
                                        strNameTemp.Format(
                                            OTRecordList::textTo(),
                                            str_recipient_id.c_str());

                                    str_name = strNameTemp.Get();
                                    str_other_nym_id = str_recipient_id;
                                    if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                            theRecipientAcctID)) {
                                        const OTString strRecipientAcctID(
                                            theRecipientAcctID);
                                        str_other_acct_id =
                                            strRecipientAcctID.Get();
                                    }
                                }
                            }
                            else // str_nym_id IS NOT str_sender_id.
                                   // (Therefore we want sender.)
                            { // In this case, some OTHER Nym is the sender, so
                                // it must have been incoming. (And bOutgoing is
                                // already false.)

                                OTString strName(m_pLookup->GetNymName(
                                    str_sender_id, &it_server)),
                                    strNameTemp;

                                if (strName.Exists())
                                    strNameTemp.Format(OTRecordList::textFrom(),
                                                       strName.Get());
                                else
                                    strNameTemp.Format(OTRecordList::textFrom(),
                                                       str_sender_id.c_str());

                                str_name = strNameTemp.Get();
                                str_other_nym_id = str_sender_id;
                                if (pBoxTrans->GetSenderAcctIDForDisplay(
                                        theSenderAcctID)) {
                                    const OTString strSenderAcctID(
                                        theSenderAcctID);
                                    str_other_acct_id = strSenderAcctID.Get();
                                }
                            }
                        }
                        // In this block below, we already KNOW
                        // GetSenderUserIDForDisplay is EMPTY.
                        // (So it's "recipient or bust.")
                        else if (pBoxTrans->GetRecipientUserIDForDisplay(
                                     theRecipientID)) {
                            const OTString strRecipientID(theRecipientID);
                            const std::string str_recipient_id(
                                strRecipientID.Get());

                            if (0 !=
                                str_nym_id.compare(
                                    str_recipient_id)) // str_nym_id is NOT
                                                       // str_recipient_id.
                                                       // (Therefore we want
                                                       // str_recipient_id.)
                            {
                                // If Nym is not the recipient, then he must be
                                // the sender.
                                // (Therefore it must be outgoing.)
                                bOutgoing = true;

                                OTString strName(m_pLookup->GetNymName(
                                    str_recipient_id, &it_server)),
                                    strNameTemp;

                                if (strName.Exists())
                                    strNameTemp.Format(OTRecordList::textTo(),
                                                       strName.Get());
                                else
                                    strNameTemp.Format(
                                        OTRecordList::textTo(),
                                        str_recipient_id.c_str());

                                str_name = strNameTemp.Get();
                                str_other_nym_id = str_recipient_id;
                                if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                        theRecipientAcctID)) {
                                    const OTString strRecipientAcctID(
                                        theRecipientAcctID);
                                    str_other_acct_id =
                                        strRecipientAcctID.Get();
                                }
                            }
                        }
                    } // if not abbreviated.
                    time64_t tValidFrom = OT_TIME_ZERO, tValidTo = OT_TIME_ZERO;
                    std::string str_date =
                        "0"; // the "date signed" on the transaction receipt.
                    time64_t tDateSigned = pBoxTrans->GetDateSigned();

                    if (tDateSigned > OT_TIME_ZERO) {
                        tValidFrom = tDateSigned;
                        const uint64_t lDateSigned =
                            OTTimeGetSecondsFromTime(tDateSigned);
                        OTString strDateSigned;
                        strDateSigned.Format("%" PRIu64 "", lDateSigned);
                        str_date = strDateSigned.Get();
                    }
                    const std::string* p_str_asset_type =
                        &OTRecordList::s_blank; // <========== ASSET TYPE
                    const std::string* p_str_asset_name =
                        &OTRecordList::s_blank; // asset type display name.
                    const std::string* p_str_account =
                        &OTRecordList::s_blank; // <========== ACCOUNT
                    std::string str_amount;     // <========== AMOUNT
                    std::string str_type;       // Instrument type.
                    std::string str_memo; // Instrument memo (if applicable.)
                    OTString strContents; // Instrument contents.

                    if (pBoxTrans->IsAbbreviated()) {
                        str_type =
                            pBoxTrans->GetTypeString(); // instrumentNotice,
                                                        // etc.
                        int64_t lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                        if (0 != lAmount) {
                            OTString strTemp;
                            strTemp.Format("%" PRId64 "", lAmount);
                            str_amount = strTemp.Get();
                        }
                    }
                    else // NOT abbreviated. (Full box receipt is already
                           // loaded.)
                    {
                        OTPayment* pPayment = GetInstrument(
                            *pNym, nIndex,
                            *pRecordbox); // ===> Returns financial instrument
                                          // by index.
                        std::unique_ptr<OTPayment> thePaymentAngel(pPayment);
                        if (nullptr ==
                            pPayment) // then we treat it like it's abbreviated.
                        {
                            str_type =
                                pBoxTrans->GetTypeString(); // instrumentNotice,
                                                            // etc.
                            int64_t lAmount =
                                pBoxTrans->GetAbbrevDisplayAmount();

                            if (0 != lAmount) {
                                OTString strTemp;
                                strTemp.Format("%" PRId64 "", lAmount);
                                str_amount = strTemp.Get();
                            }
                        }
                        // We have pPayment, the instrument accompanying the
                        // receipt in the payments recordbox.
                        else if (pPayment->SetTempValues()) {
                            pPayment->GetValidFrom(tValidFrom);
                            pPayment->GetValidTo(tValidTo);

                            if (tValidFrom > OT_TIME_ZERO) {
                                const uint64_t lFrom =
                                    OTTimeGetSecondsFromTime(tValidFrom);
                                OTString strFrom;
                                strFrom.Format("%" PRIu64 "", lFrom);
                                str_date = strFrom.Get();
                            }
                            pPayment->GetPaymentContents(strContents);
                            OTIdentifier theAccountID;

                            if (bOutgoing) // Nym is sender.
                            {
                                if (pPayment->GetSenderAcctIDForDisplay(
                                        theAccountID)) // Since Nym is ME, the
                                                       // Account must be MY
                                                       // acct.
                                { // (If this record was originally OUTgoing,
                                    // then the SENDER's account is MY acct.)
                                    OTString strTemp(theAccountID);
                                    std::string str_outpmt_account =
                                        strTemp.Get(); // The accountID we found
                                                       // on the payment (only
                                                       // applies to outgoing
                                                       // payments.)
                                    auto it_acct = std::find(
                                        m_accounts.begin(), m_accounts.end(),
                                        str_outpmt_account);
                                    if (it_acct !=
                                        m_accounts.end()) // Found it on the
                                                          // list of accounts we
                                                          // care about.
                                    {
                                        p_str_account = &(*it_acct);
                                    }
                                    else {
                                        // There was definitely an account on
                                        // the instrument, and it definitely
                                        // did not match any of the accounts
                                        // that we care about.
                                        // Therefore, skip.
                                        //
                                        otOut << __FUNCTION__
                                              << ": Skipping 'sent payment' "
                                                 "record. (We don't care about "
                                                 "account "
                                              << str_outpmt_account.c_str()
                                              << ")\n";
                                        continue;
                                    }
                                }
                            }
                            else // Nym is recipient.
                            {
                                // Why is this here? Because if Nym is
                                // recipient, let's say he received an
                                // instrumentNotice containing
                                // a sendUserInstrument message containing an
                                // incoming cheque. Well... that incoming cheque
                                // (the payload
                                // on sendUserInstrument message) is ENCRYPTED.
                                // Meaning the above calls to
                                // pBoxReceipt->GetSenderAcctID
                                // on the instrumentNotice transaction will
                                // FAIL. One option is to pass pNym into
                                // GetSenderAcctID so it
                                // can decrypt the payload and return the value.
                                // But since we already have the payload
                                // decrypted here
                                // (we already have the cheque loaded up here)
                                // we can just grab the senderAcctID directly
                                // from the cheque.
                                // That's why this is here -- because this is
                                // where we KNOW we have the account ID -- so we
                                // grab it.
                                //
                                if (str_other_acct_id.empty() &&
                                    (pPayment->GetSenderAcctIDForDisplay(
                                        theAccountID))) {
                                    OTString strTemp(theAccountID);
                                    str_other_acct_id = strTemp.Get();
                                }
                            }
                            // By this point, p_str_account is definitely set.
                            OT_ASSERT(nullptr !=
                                      p_str_account); // and it's either
                                                      // blank, or it's
                                                      // one of the
                                                      // accounts we
                                                      // care about.
                            OTIdentifier theAssetTypeID;

                            if (pPayment->GetAssetTypeID(theAssetTypeID)) {
                                OTString strTemp(theAssetTypeID);
                                const std::string str_inpmt_asset(
                                    strTemp.Get()); // The asset type we found
                                                    // on the payment (if we
                                                    // found anything.)
                                auto it_asset = m_assets.find(str_inpmt_asset);
                                if (it_asset != m_assets.end()) // Found it on
                                                                // the map of
                                                                // asset types
                                                                // we care
                                                                // about.
                                {
                                    p_str_asset_type =
                                        &(it_asset->first); // Set the asset
                                                            // type ID.
                                    p_str_asset_name =
                                        &(it_asset->second); // The CurrencyTLA.
                                                             // Examples: USD,
                                                             // BTC, etc.
                                }
                                else {
                                    // There was definitely an asset type on the
                                    // instrument, and it definitely
                                    // did not match any of the assets that we
                                    // care about.
                                    // Therefore, skip.
                                    //
                                    otErr << __FUNCTION__
                                          << ": Skipping: Payment record (we "
                                             "don't care about asset type "
                                          << str_inpmt_asset.c_str() << ")\n";
                                    continue;
                                }
                            }
                            // By this point, p_str_asset_type and
                            // p_str_asset_name are definitely set.
                            OT_ASSERT(nullptr != p_str_asset_type); // and it's
                                                                    // either
                                                                    // blank, or
                            // it's one of
                            // the asset
                            // types we
                            // care about.
                            OT_ASSERT(nullptr != p_str_asset_name); // and it's
                                                                    // either
                                                                    // blank, or
                            // it's one of
                            // the asset
                            // types we
                            // care about.
                            OTString strMemo;
                            if (pPayment->GetMemo(strMemo)) {
                                str_memo = strMemo.Get();
                            }
                            // Instrument type (cheque, voucher, etc)
                            int32_t nType =
                                static_cast<int32_t>(pPayment->GetType());

                            str_type = GetTypeString(nType);
                            int64_t lAmount = 0;

                            if (pPayment->GetAmount(lAmount)) {
                                OTString strTemp;
                                strTemp.Format("%" PRId64 "", lAmount);
                                str_amount = strTemp.Get();
                            }
                        }
                    }
                    otOut << __FUNCTION__ << ": ADDED: Payment record "
                          << (bOutgoing ? "(sent)" : "(received)")
                          << " (str_type: " << str_type.c_str() << ")\n";

                    shared_ptr_OTRecord sp_Record(new OTRecord(
                        it_server, *p_str_asset_type, *p_str_asset_name,
                        str_nym_id,     // This is the Nym WHOSE BOX IT IS.
                        *p_str_account, // This is the Nym's account for this
                                        // box. (Blank for incoming, set for
                                        // outgoing.)
                        // Everything above this line, it stores a reference to
                        // an external string.
                        // Everything below this line, it makes its own internal
                        // copy of the string.
                        str_name, // name of sender or recipient (since its in
                                  // the recordbox.)
                        str_date, // the "date signed" on the receipt.
                        str_amount, str_type, // pending, chequeReceipt, etc.
                        false, // Everything in the recordbox is finished. (NOT
                               // pending.)
                        bOutgoing, // Since it's the recordbox, it contains both
                                   // incoming and outgoing receipts.
                        true,      // IsRecord
                        false,     // IsReceipt,
                        OTRecord::Instrument));
                    if (strContents.Exists())
                        sp_Record->SetContents(strContents.Get());
                    sp_Record->SetDateRange(tValidFrom, tValidTo);
                    sp_Record->SetBoxIndex(nIndex);
                    if (!str_memo.empty()) sp_Record->SetMemo(str_memo);
                    if (!str_other_nym_id.empty())
                        sp_Record->SetOtherNymID(str_other_nym_id);
                    if (!str_other_acct_id.empty())
                        sp_Record->SetOtherAccountID(str_other_acct_id);
                    sp_Record->SetTransNumForDisplay(
                        pBoxTrans->GetReferenceNumForDisplay());
                    sp_Record->SetTransactionNum(
                        pBoxTrans->GetTransactionNum());

                    m_contents.push_back(sp_Record);

                } // Loop through Recordbox
            }
            else
                otWarn << __FUNCTION__
                       << ": Failed loading payments record box. "
                          "(Probably just doesn't exist yet.)\n";

            // EXPIRED RECORDS:
            nIndex = (-1);

            // Also loop through its expired record box.
            // OPTIMIZE FYI: m_bRunFast impacts run speed here.
            OTLedger* pExpiredbox =
                m_bRunFast
                    ? OTAPI_Wrap::OTAPI()->LoadExpiredBoxNoVerify(theServerID,
                                                                  theNymID)
                    : OTAPI_Wrap::OTAPI()->LoadExpiredBox(theServerID,
                                                          theNymID);
            std::unique_ptr<OTLedger> theExpiredBoxAngel(pExpiredbox);

            // It loaded up, so let's loop through it.
            if (nullptr != pExpiredbox) {
                for (auto& it : pExpiredbox->GetTransactionMap()) {
                    OTTransaction* pBoxTrans = it.second;
                    OT_ASSERT(nullptr != pBoxTrans);
                    bool bOutgoing = false;
                    ++nIndex; // 0 on first iteration.
                    otOut << __FUNCTION__
                          << ": Expired payment RECORD index: " << nIndex
                          << "\n";
                    std::string str_name; // name of sender OR recipient
                                          // (depending on whether it was
                                          // originally incoming or outgoing.)
                    std::string str_other_nym_id;
                    std::string str_other_acct_id;

                    if (!pBoxTrans->IsAbbreviated()) {
                        OTIdentifier theSenderID, theSenderAcctID;
                        OTIdentifier theRecipientID, theRecipientAcctID;

                        if (pBoxTrans->GetSenderUserIDForDisplay(theSenderID)) {
                            const OTString strSenderID(theSenderID);
                            const std::string str_sender_id(strSenderID.Get());

                            // Usually, Nym is the RECIPIENT. Sometimes he's the
                            // sender.
                            // Either way, we want the OTHER ID (the other Nym)
                            // for display.
                            // So here, if Nym's CLEARLY the sender, then we
                            // want the RECIPIENT.
                            // Whereas if Nym were the recipient, then we'd want
                            // the SENDER. (For display.)
                            //
                            if (0 ==
                                str_nym_id.compare(str_sender_id)) // str_nym_id
                                                                   // IS
                            // str_sender_id.
                            // (Therefore we want
                            // recipient.)
                            {
                                bOutgoing = true; // if Nym is the sender, then
                                                  // it must have been outgoing.

                                if (pBoxTrans->GetRecipientUserIDForDisplay(
                                        theRecipientID)) {
                                    const OTString strRecipientID(
                                        theRecipientID);
                                    const std::string str_recipient_id(
                                        strRecipientID.Get());

                                    OTString strName(m_pLookup->GetNymName(
                                        str_recipient_id, &it_server)),
                                        strNameTemp;

                                    if (strName.Exists())
                                        strNameTemp.Format(
                                            OTRecordList::textTo(),
                                            strName.Get());
                                    else
                                        strNameTemp.Format(
                                            OTRecordList::textTo(),
                                            str_recipient_id.c_str());

                                    str_name = strNameTemp.Get();
                                    str_other_nym_id = str_recipient_id;
                                    if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                            theRecipientAcctID)) {
                                        const OTString strRecipientAcctID(
                                            theRecipientAcctID);
                                        str_other_acct_id =
                                            strRecipientAcctID.Get();
                                    }
                                }
                            }
                            else // str_nym_id IS NOT str_sender_id.
                                   // (Therefore we want sender.)
                            { // In this case, some OTHER Nym is the sender, so
                                // it must have been incoming. (And bOutgoing is
                                // already false.)

                                OTString strName(m_pLookup->GetNymName(
                                    str_sender_id, &it_server)),
                                    strNameTemp;

                                if (strName.Exists())
                                    strNameTemp.Format(OTRecordList::textFrom(),
                                                       strName.Get());
                                else
                                    strNameTemp.Format(OTRecordList::textFrom(),
                                                       str_sender_id.c_str());

                                str_name = strNameTemp.Get();
                                str_other_nym_id = str_sender_id;
                                if (pBoxTrans->GetSenderAcctIDForDisplay(
                                        theSenderAcctID)) {
                                    const OTString strSenderAcctID(
                                        theSenderAcctID);
                                    str_other_acct_id = strSenderAcctID.Get();
                                }
                            }
                        }
                        // In this block below, we already KNOW
                        // GetSenderUserIDForDisplay is EMPTY.
                        // (So it's "recipient or bust.")
                        else if (pBoxTrans->GetRecipientUserIDForDisplay(
                                     theRecipientID)) {
                            const OTString strRecipientID(theRecipientID);
                            const std::string str_recipient_id(
                                strRecipientID.Get());

                            if (0 !=
                                str_nym_id.compare(
                                    str_recipient_id)) // str_nym_id is NOT
                                                       // str_recipient_id.
                                                       // (Therefore we want
                                                       // str_recipient_id.)
                            {
                                // If Nym is not the recipient, then he must be
                                // the sender.
                                // (Therefore it must be outgoing.)
                                bOutgoing = true;

                                OTString strName(m_pLookup->GetNymName(
                                    str_recipient_id, &it_server)),
                                    strNameTemp;

                                if (strName.Exists())
                                    strNameTemp.Format(OTRecordList::textTo(),
                                                       strName.Get());
                                else
                                    strNameTemp.Format(
                                        OTRecordList::textTo(),
                                        str_recipient_id.c_str());

                                str_name = strNameTemp.Get();
                                str_other_nym_id = str_recipient_id;
                                if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                        theRecipientAcctID)) {
                                    const OTString strRecipientAcctID(
                                        theRecipientAcctID);
                                    str_other_acct_id =
                                        strRecipientAcctID.Get();
                                }
                            }
                        }
                    } // if not abbreviated.
                    time64_t tValidFrom = OT_TIME_ZERO, tValidTo = OT_TIME_ZERO;
                    std::string str_date =
                        "0"; // the "date signed" on the transaction receipt.
                    time64_t tDateSigned = pBoxTrans->GetDateSigned();

                    if (tDateSigned > OT_TIME_ZERO) {
                        tValidFrom = tDateSigned;
                        const uint64_t lDateSigned =
                            OTTimeGetSecondsFromTime(tDateSigned);
                        OTString strDateSigned;
                        strDateSigned.Format("%" PRIu64 "", lDateSigned);
                        str_date = strDateSigned.Get();
                    }
                    const std::string* p_str_asset_type =
                        &OTRecordList::s_blank; // <========== ASSET TYPE
                    const std::string* p_str_asset_name =
                        &OTRecordList::s_blank; // asset type display name.
                    const std::string* p_str_account =
                        &OTRecordList::s_blank; // <========== ACCOUNT
                    std::string str_amount;     // <========== AMOUNT
                    std::string str_type;       // Instrument type.
                    std::string str_memo; // Instrument memo (if applicable.)
                    OTString strContents; // Instrument contents.

                    if (pBoxTrans->IsAbbreviated()) {
                        str_type =
                            pBoxTrans->GetTypeString(); // instrumentNotice,
                                                        // etc.
                        int64_t lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                        if (0 != lAmount) {
                            OTString strTemp;
                            strTemp.Format("%" PRId64 "", lAmount);
                            str_amount = strTemp.Get();
                        }
                    }
                    else // NOT abbreviated. (Full box receipt is already
                           // loaded.)
                    {
                        OTPayment* pPayment = GetInstrument(
                            *pNym, nIndex,
                            *pExpiredbox); //===> Returns financial instrument
                                           // by index.
                        std::unique_ptr<OTPayment> thePaymentAngel(pPayment);
                        if (nullptr ==
                            pPayment) // then we treat it like it's abbreviated.
                        {
                            str_type =
                                pBoxTrans->GetTypeString(); // instrumentNotice,
                                                            // etc.
                            int64_t lAmount =
                                pBoxTrans->GetAbbrevDisplayAmount();

                            if (0 != lAmount) {
                                OTString strTemp;
                                strTemp.Format("%" PRId64 "", lAmount);
                                str_amount = strTemp.Get();
                            }
                        }
                        // We have pPayment, the instrument accompanying the
                        // receipt in the payments recordbox.
                        else if (pPayment->SetTempValues()) {
                            pPayment->GetValidFrom(tValidFrom);
                            pPayment->GetValidTo(tValidTo);

                            if (tValidFrom > OT_TIME_ZERO) {
                                const uint64_t lFrom =
                                    OTTimeGetSecondsFromTime(tValidFrom);
                                OTString strFrom;
                                strFrom.Format("%" PRIu64 "", lFrom);
                                str_date = strFrom.Get();
                            }
                            pPayment->GetPaymentContents(strContents);
                            OTIdentifier theAccountID;

                            if (bOutgoing) // Nym is sender.
                            {
                                if (pPayment->GetSenderAcctIDForDisplay(
                                        theAccountID)) // Since Nym is ME, the
                                                       // Account must be MY
                                                       // acct.
                                { // (If this record was originally OUTgoing,
                                    // then the SENDER's account is MY acct.)
                                    OTString strTemp(theAccountID);
                                    std::string str_outpmt_account =
                                        strTemp.Get(); // The accountID we found
                                                       // on the payment (only
                                                       // applies to outgoing
                                                       // payments.)
                                    auto it_acct = std::find(
                                        m_accounts.begin(), m_accounts.end(),
                                        str_outpmt_account);
                                    if (it_acct !=
                                        m_accounts.end()) // Found it on the
                                                          // list of accounts we
                                                          // care about.
                                    {
                                        p_str_account = &(*it_acct);
                                    }
                                    else {
                                        // There was definitely an account on
                                        // the instrument, and it definitely
                                        // did not match any of the accounts
                                        // that we care about.
                                        // Therefore, skip.
                                        //
                                        otOut
                                            << __FUNCTION__
                                            << ": Skipping 'sent payment' "
                                               "expired record. (We don't care "
                                               "about account "
                                            << str_outpmt_account.c_str()
                                            << ")\n";
                                        continue;
                                    }
                                }
                            }
                            else // Nym is recipient.
                            {
                                // Why is this here? Because if Nym is
                                // recipient, let's say he received an
                                // instrumentNotice containing
                                // a sendUserInstrument message containing an
                                // incoming cheque. Well... that incoming cheque
                                // (the payload
                                // on sendUserInstrument message) is ENCRYPTED.
                                // Meaning the above calls to
                                // pBoxReceipt->GetSenderAcctID
                                // on the instrumentNotice transaction will
                                // FAIL. One option is to pass pNym into
                                // GetSenderAcctID so it
                                // can decrypt the payload and return the value.
                                // But since we already have the payload
                                // decrypted here
                                // (we already have the cheque loaded up here)
                                // we can just grab the senderAcctID directly
                                // from the cheque.
                                // That's why this is here -- because this is
                                // where we KNOW we have the account ID -- so we
                                // grab it.
                                //
                                if (str_other_acct_id.empty() &&
                                    (pPayment->GetSenderAcctIDForDisplay(
                                        theAccountID))) {
                                    OTString strTemp(theAccountID);
                                    str_other_acct_id = strTemp.Get();
                                }
                            }
                            // By this point, p_str_account is definitely set.
                            OT_ASSERT(nullptr !=
                                      p_str_account); // and it's either
                                                      // blank, or it's
                                                      // one of the
                                                      // accounts we
                                                      // care about.
                            OTIdentifier theAssetTypeID;

                            if (pPayment->GetAssetTypeID(theAssetTypeID)) {
                                OTString strTemp(theAssetTypeID);
                                const std::string str_inpmt_asset(
                                    strTemp.Get()); // The asset type we found
                                                    // on the payment (if we
                                                    // found anything.)
                                auto it_asset = m_assets.find(str_inpmt_asset);
                                if (it_asset != m_assets.end()) // Found it on
                                                                // the map of
                                                                // asset types
                                                                // we care
                                                                // about.
                                {
                                    p_str_asset_type =
                                        &(it_asset->first); // Set the asset
                                                            // type ID.
                                    p_str_asset_name =
                                        &(it_asset->second); // The CurrencyTLA.
                                                             // Examples: USD,
                                                             // BTC, etc.
                                }
                                else {
                                    // There was definitely an asset type on the
                                    // instrument, and it definitely
                                    // did not match any of the assets that we
                                    // care about.
                                    // Therefore, skip.
                                    //
                                    otErr
                                        << __FUNCTION__
                                        << ": Skipping: Expired payment record "
                                           "(we don't care about asset type "
                                        << str_inpmt_asset.c_str() << ")\n";
                                    continue;
                                }
                            }
                            // By this point, p_str_asset_type and
                            // p_str_asset_name are definitely set.
                            OT_ASSERT(nullptr != p_str_asset_type); // and it's
                                                                    // either
                                                                    // blank, or
                            // it's one of
                            // the asset
                            // types we
                            // care about.
                            OT_ASSERT(nullptr != p_str_asset_name); // and it's
                                                                    // either
                                                                    // blank, or
                            // it's one of
                            // the asset
                            // types we
                            // care about.
                            OTString strMemo;
                            if (pPayment->GetMemo(strMemo)) {
                                str_memo = strMemo.Get();
                            }
                            // Instrument type (cheque, voucher, etc)
                            int32_t nType =
                                static_cast<int32_t>(pPayment->GetType());

                            str_type = GetTypeString(nType);
                            int64_t lAmount = 0;

                            if (pPayment->GetAmount(lAmount)) {
                                OTString strTemp;
                                strTemp.Format("%" PRId64 "", lAmount);
                                str_amount = strTemp.Get();
                            }
                        }
                    }
                    otOut << __FUNCTION__ << ": ADDED: Expired payment record "
                          << (bOutgoing ? "(sent)" : "(received)")
                          << " (str_type: " << str_type.c_str() << ")\n";

                    shared_ptr_OTRecord sp_Record(new OTRecord(
                        it_server, *p_str_asset_type, *p_str_asset_name,
                        str_nym_id,     // This is the Nym WHOSE BOX IT IS.
                        *p_str_account, // This is the Nym's account for this
                                        // box. (Blank for incoming, set for
                                        // outgoing.)
                        // Everything above this line, it stores a reference to
                        // an external string.
                        // Everything below this line, it makes its own internal
                        // copy of the string.
                        str_name, // name of sender or recipient (since its in
                                  // the recordbox.)
                        str_date, // the "date signed" on the receipt.
                        str_amount, str_type, // pending, chequeReceipt, etc.
                        false, // Everything in the recordbox is finished. (NOT
                               // pending.)
                        bOutgoing, // Since it's the recordbox, it contains both
                                   // incoming and outgoing receipts.
                        true,      // IsRecord
                        false,     // IsReceipt,
                        OTRecord::Instrument));
                    if (strContents.Exists())
                        sp_Record->SetContents(strContents.Get());
                    sp_Record->SetDateRange(tValidFrom, tValidTo);
                    sp_Record->SetExpired();
                    sp_Record->SetBoxIndex(nIndex);
                    if (!str_memo.empty()) sp_Record->SetMemo(str_memo);
                    if (!str_other_nym_id.empty())
                        sp_Record->SetOtherNymID(str_other_nym_id);
                    if (!str_other_acct_id.empty())
                        sp_Record->SetOtherAccountID(str_other_acct_id);
                    sp_Record->SetTransNumForDisplay(
                        pBoxTrans->GetReferenceNumForDisplay());
                    sp_Record->SetTransactionNum(
                        pBoxTrans->GetTransactionNum());

                    m_contents.push_back(sp_Record);

                } // Loop through ExpiredBox
            }
            else
                otWarn << __FUNCTION__
                       << ": Failed loading expired payments box. "
                          "(Probably just doesn't exist yet.)\n";

        } // Loop through servers for each Nym.
    }     // Loop through Nyms.
          // ASSET ACCOUNT -- INBOX/OUTBOX + RECORD BOX
          // Loop through the Accounts.
          //
    otOut << "================ " << __FUNCTION__
          << ": Looping through the accounts in the wallet...\n";
    int32_t nAccountIndex = -1;
    for (auto& it_acct : m_accounts) {
        ++nAccountIndex; // (0 on first iteration.)
        // For each account, loop through its inbox, outbox, and record box.
        //
        const std::string& str_account_id(it_acct);
        const OTIdentifier theAccountID(str_account_id);
        OTAccount* pAccount = pWallet->GetAccount(theAccountID);
        OT_ASSERT(nullptr != pAccount);
        const OTIdentifier& theNymID = pAccount->GetUserID();
        const OTIdentifier& theServerID = pAccount->GetPurportedServerID();
        const OTIdentifier& theAssetID = pAccount->GetAssetTypeID();
        const OTString strNymID(theNymID);
        const OTString strServerID(theServerID);
        const OTString strAssetID(theAssetID);
        otOut << "------------\n" << __FUNCTION__
              << ": Account: " << nAccountIndex
              << ", ID: " << str_account_id.c_str() << "\n";
        const std::string str_nym_id(strNymID.Get());
        const std::string str_server_id(strServerID.Get());
        const std::string str_asset_id(strAssetID.Get());
        const std::string* pstr_nym_id = &OTRecordList::s_blank;
        const std::string* pstr_server_id = &OTRecordList::s_blank;
        const std::string* pstr_asset_id = &OTRecordList::s_blank;
        const std::string* pstr_asset_name = &OTRecordList::s_blank;
        // NOTE: Since this account is already on my "care about" list for
        // accounts,
        // I wouldn't bother double-checking my "care about" lists for servers,
        // nyms,
        // and asset types. But I still look up the appropriate string for each,
        // since
        // I have to pass a reference to it into the constructor for OTRecord.
        // (To a version
        // that won't be deleted, since the OTRecord will reference it. And the
        // "care about"
        // list definitely contains a copy of the string that won't be deleted.)
        //
        auto it_nym = std::find(m_nyms.begin(), m_nyms.end(), str_nym_id);
        auto it_server =
            std::find(m_servers.begin(), m_servers.end(), str_server_id);
        auto it_asset = m_assets.find(str_asset_id);
        if ((m_nyms.end() == it_nym) || (m_servers.end() == it_server) ||
            (m_assets.end() == it_asset)) {
            otOut << __FUNCTION__ << ": Skipping an account ("
                  << str_account_id.c_str()
                  << ") since its Nym, or Server, "
                     "or Asset Type wasn't on my list.\n";
            continue;
        }
        // These pointers are what we'll use to construct each OTRecord.
        //
        pstr_nym_id = &(*it_nym);
        pstr_server_id = &(*it_server);
        pstr_asset_id = &(it_asset->first);
        pstr_asset_name = &(it_asset->second);
        // Loop through asset account INBOX.
        //
        // OPTIMIZE FYI:
        // NOTE: LoadInbox is much SLOWER than LoadInboxNoVerify, but it also
        // lets you get
        // the NAME off of the box receipt. So if you are willing to GIVE UP the
        // NAME, in
        // return for FASTER PERFORMANCE, then call SetFastMode() before
        // Populating.
        //
        OTLedger* pInbox = m_bRunFast
                               ? OTAPI_Wrap::OTAPI()->LoadInboxNoVerify(
                                     theServerID, theNymID, theAccountID)
                               : OTAPI_Wrap::OTAPI()->LoadInbox(
                                     theServerID, theNymID, theAccountID);
        std::unique_ptr<OTLedger> theInboxAngel(pInbox);

        // It loaded up, so let's loop through it.
        if (nullptr != pInbox) {
            int32_t nInboxIndex = -1;
            for (auto& it : pInbox->GetTransactionMap()) {
                ++nInboxIndex; // (0 on first iteration.)
                if (0 == nInboxIndex)
                    otOut
                        << __FUNCTION__
                        << ": Beginning loop through asset account INBOX...\n";
                OTTransaction* pBoxTrans = it.second;
                OT_ASSERT(nullptr != pBoxTrans);
                otOut << __FUNCTION__ << ": Inbox index: " << nInboxIndex
                      << "\n";
                bool bCanceled = false;
                std::string str_name; // name of sender (since its in the
                                      // inbox.)
                std::string str_other_nym_id;
                std::string str_other_acct_id;
                std::string str_memo;

                if (!pBoxTrans->IsAbbreviated()) {
                    OTString strMemo;

                    if (pBoxTrans->GetMemo(strMemo)) str_memo = strMemo.Get();
                    if (OTTransaction::pending == pBoxTrans->GetType()) {
                        // NOTE: REMOVE THE BELOW CODE. (Found a better way,
                        // above this block.)
                        //                    const OTString
                        // strBoxTrans(*pBoxTrans);
                        //
                        //                    if (strBoxTrans.Exists())
                        //                        str_memo =
                        // OTAPI_Wrap::Pending_GetNote(*pstr_server_id,
                        // *pstr_nym_id, str_account_id, strBoxTrans.Get());
                        OTIdentifier theSenderID, theSenderAcctID;

                        if (pBoxTrans->GetSenderAcctIDForDisplay(
                                theSenderAcctID)) // ACCOUNT name.
                        {
                            if (pBoxTrans->GetSenderUserIDForDisplay(
                                    theSenderID)) {
                                const OTString strSenderID(theSenderID);
                                str_other_nym_id = strSenderID.Get();
                            }
                            const OTString strSenderAcctID(theSenderAcctID);
                            const std::string str_sender_acct_id(
                                strSenderAcctID.Get());

                            str_other_acct_id = str_sender_acct_id;

                            OTString strName(m_pLookup->GetAcctName(
                                str_other_acct_id,
                                str_other_nym_id.empty()
                                    ? nullptr
                                    : &str_other_nym_id, // nym ID if known
                                pstr_server_id,          // server ID if known.
                                pstr_asset_id)),         // asset ID if known.
                                strNameTemp;

                            if (strName.Exists()) {
                                strNameTemp.Format(OTRecordList::textFrom(),
                                                   strName.Get());
                                str_name = strNameTemp.Get();
                            }
                            else if (!str_other_nym_id.empty()) {
                                OTString strNymName(m_pLookup->GetNymName(
                                    str_other_nym_id, &(*it_server))),
                                    strNymNameTemp;

                                if (strNymName.Exists()) {
                                    strNymNameTemp.Format(
                                        OTRecordList::textFrom(),
                                        strNymName.Get());
                                    str_name = strNameTemp.Get();
                                }
                            }
                            if (str_name.empty()) {
                                strNameTemp.Format(OTRecordList::textFrom(),
                                                   str_sender_acct_id.c_str());
                                str_name = strNameTemp.Get();
                            }
                        }
                        else if (pBoxTrans->GetSenderUserIDForDisplay(
                                       theSenderID)) // NYM name.
                        {
                            const OTString strSenderID(theSenderID);
                            const std::string str_sender_id(strSenderID.Get());

                            OTString strName(m_pLookup->GetNymName(
                                str_sender_id, &(*it_server))),
                                strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(OTRecordList::textFrom(),
                                                   strName.Get());
                            else
                                strNameTemp.Format(OTRecordList::textFrom(),
                                                   str_sender_id.c_str());

                            str_name = strNameTemp.Get();
                            str_other_nym_id = str_sender_id;
                        }
                        else {
                            OTString strName(OTAPI_Wrap::GetAccountWallet_Name(
                                str_account_id)),
                                strNameTemp;

                            if (strName.Exists())
                                strNameTemp = strName;
                            else
                                strNameTemp = str_account_id;

                            str_name = strNameTemp.Get();
                        }
                    }    // end: (if pending)
                    else // else it's a receipt.
                    {
                        OTIdentifier theRecipientID, theRecipientAcctID;

                        if (pBoxTrans->GetRecipientUserIDForDisplay(
                                theRecipientID)) {
                            const OTString strRecipientID(theRecipientID);
                            const std::string str_recipient_user_id(
                                strRecipientID.Get());

                            OTString strName(m_pLookup->GetNymName(
                                str_recipient_user_id, &(*it_server))),
                                strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(OTRecordList::textTo(),
                                                   strName.Get());
                            else
                                strNameTemp.Format(
                                    OTRecordList::textTo(),
                                    str_recipient_user_id.c_str());

                            str_name = strNameTemp.Get();
                            str_other_nym_id = str_recipient_user_id;
                            if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                    theRecipientAcctID)) {
                                const OTString strRecipientAcctID(
                                    theRecipientAcctID);
                                str_other_acct_id = strRecipientAcctID.Get();
                            }
                        }
                        else if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                       theRecipientAcctID)) {
                            const OTString strRecipientAcctID(
                                theRecipientAcctID);
                            const std::string str_recipient_acct_id(
                                strRecipientAcctID.Get());

                            OTString strName(m_pLookup->GetAcctName(
                                str_recipient_acct_id,
                                nullptr,         // nym ID if known
                                pstr_server_id,  // server ID if known.
                                pstr_asset_id)), // asset ID if known.
                                strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(OTRecordList::textTo(),
                                                   strName.Get());
                            else
                                strNameTemp.Format(
                                    OTRecordList::textTo(),
                                    str_recipient_acct_id.c_str());

                            str_name = strNameTemp.Get();
                            str_other_acct_id = str_recipient_acct_id;
                        }
                    } // end: (else it's a receipt.)
                }
                bCanceled = pBoxTrans->IsCancelled();
                time64_t tValidFrom = OT_TIME_ZERO, tValidTo = OT_TIME_ZERO;
                std::string str_date =
                    "0"; // the "date signed" on the transaction receipt.
                time64_t tDateSigned = pBoxTrans->GetDateSigned();

                if (tDateSigned > OT_TIME_ZERO) {
                    tValidFrom = tDateSigned;
                    const uint64_t lDateSigned =
                        OTTimeGetSecondsFromTime(tDateSigned);
                    OTString strDateSigned;
                    strDateSigned.Format("%" PRIu64 "", lDateSigned);
                    str_date = strDateSigned.Get();
                }
                std::string str_amount; // <========== AMOUNT
                int64_t lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                if (0 == lAmount) lAmount = pBoxTrans->GetReceiptAmount();
                if (0 != lAmount) {
                    OTString strTemp;
                    strTemp.Format("%" PRId64 "", lAmount);
                    str_amount = strTemp.Get();
                }
                const std::string str_type(
                    pBoxTrans->GetTypeString()); // pending, chequeReceipt, etc.
                otOut << __FUNCTION__ << ": ADDED: incoming "
                      << ((OTTransaction::pending == pBoxTrans->GetType())
                              ? "pending transfer"
                              : "receipt") << " (str_type: " << str_type.c_str()
                      << ")\n";

                shared_ptr_OTRecord sp_Record(new OTRecord(
                    *pstr_server_id, *pstr_asset_id, *pstr_asset_name,
                    *pstr_nym_id,   // This is the Nym WHOSE BOX IT IS.
                    str_account_id, // This is the Nym's account for this box.
                    // Everything above this line, it stores a reference to an
                    // external string.
                    // Everything below this line, it makes its own internal
                    // copy of the string.
                    str_name, // name of sender (since its in the inbox.)
                    str_date, // the "valid from" date on the instrument.
                    str_amount, str_type, // pending, chequeReceipt, etc.
                    (OTTransaction::pending ==
                     pBoxTrans->GetType()), // Sometimes true, often false.
                    (lAmount < 0) ? true : false, // bIsOutgoing (this is the
                                                  // inbox, but a
                    // transferReceipt in the inbox represents
                    // outgoing funds. Whereas a "pending" in the
                    // inbox represents incoming funds. For now I'm
                    // just going to go based on whether the amount
                    // is negative or not, to determine incoming /
                    // outgoing. We'll see how that works.)
                    false, // IsRecord
                    (OTTransaction::pending !=
                     pBoxTrans->GetType()), // IsReceipt,
                    (OTTransaction::pending == pBoxTrans->GetType())
                        ? OTRecord::Transfer
                        : OTRecord::Receipt));
                const OTString strContents(*pBoxTrans);
                sp_Record->SetContents(strContents.Get());
                sp_Record->SetDateRange(tValidFrom, tValidTo);
                sp_Record->SetBoxIndex(nInboxIndex);
                if (bCanceled) sp_Record->SetCanceled();
                if (!str_memo.empty()) sp_Record->SetMemo(str_memo);
                if (!str_other_nym_id.empty())
                    sp_Record->SetOtherNymID(str_other_nym_id);
                if (!str_other_acct_id.empty())
                    sp_Record->SetOtherAccountID(str_other_acct_id);
                sp_Record->SetTransNumForDisplay(
                    pBoxTrans->GetReferenceNumForDisplay());
                sp_Record->SetTransactionNum(pBoxTrans->GetTransactionNum());
                m_contents.push_back(sp_Record);
            }
        }
        // OPTIMIZE FYI:
        // NOTE: LoadOutbox is much SLOWER than LoadOutboxNoVerify, but it also
        // lets you get
        // the NAME off of the box receipt. So if you are willing to GIVE UP the
        // NAME, in
        // return for FASTER PERFORMANCE, then call SetFastMode() before running
        // Populate.
        //
        OTLedger* pOutbox = m_bRunFast
                                ? OTAPI_Wrap::OTAPI()->LoadOutboxNoVerify(
                                      theServerID, theNymID, theAccountID)
                                : OTAPI_Wrap::OTAPI()->LoadOutbox(
                                      theServerID, theNymID, theAccountID);
        std::unique_ptr<OTLedger> theOutboxAngel(pOutbox);

        // It loaded up, so let's loop through it.
        if (nullptr != pOutbox) {
            int32_t nOutboxIndex = -1;
            for (auto& it : pOutbox->GetTransactionMap()) {
                ++nOutboxIndex; // (0 on first iteration.)
                if (0 == nOutboxIndex)
                    otOut
                        << __FUNCTION__
                        << ": Beginning loop through asset account OUTBOX...\n";
                OTTransaction* pBoxTrans = it.second;
                OT_ASSERT(nullptr != pBoxTrans);
                otOut << __FUNCTION__ << ": Outbox index: " << nOutboxIndex
                      << "\n";
                std::string str_name; // name of recipient (since its in the
                                      // outbox.)
                std::string str_other_nym_id;
                std::string str_other_acct_id;
                std::string str_memo;

                if (!pBoxTrans->IsAbbreviated()) {
                    OTIdentifier theRecipientID, theRecipientAcctID;

                    if (pBoxTrans->GetRecipientUserIDForDisplay(
                            theRecipientID)) {
                        const OTString strRecipientID(theRecipientID);
                        const std::string str_recipient_id(
                            strRecipientID.Get());

                        OTString strName(m_pLookup->GetNymName(str_recipient_id,
                                                               &(*it_server))),
                            strNameTemp;

                        if (strName.Exists())
                            strNameTemp.Format(OTRecordList::textTo(),
                                               strName.Get());
                        else
                            strNameTemp.Format(OTRecordList::textTo(),
                                               str_recipient_id.c_str());

                        str_name = strNameTemp.Get();
                        str_other_nym_id = str_recipient_id;
                        if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                theRecipientAcctID)) {
                            const OTString strRecipientAcctID(
                                theRecipientAcctID);
                            str_other_acct_id = strRecipientAcctID.Get();
                        }
                    }
                    else if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                   theRecipientAcctID)) {
                        const OTString strRecipientAcctID(theRecipientAcctID);
                        const std::string str_recipient_acct_id(
                            strRecipientAcctID.Get());

                        OTString strName(m_pLookup->GetAcctName(
                            str_recipient_acct_id, nullptr, // nym ID if known
                            pstr_server_id,  // server ID if known.
                            pstr_asset_id)), // asset ID if known.
                            strNameTemp;

                        if (strName.Exists())
                            strNameTemp.Format(OTRecordList::textTo(),
                                               strName.Get());
                        else
                            strNameTemp.Format(OTRecordList::textTo(),
                                               str_recipient_acct_id.c_str());

                        str_name = strNameTemp.Get();
                        str_other_acct_id = str_recipient_acct_id;
                    }
                    if (OTTransaction::pending == pBoxTrans->GetType()) {
                        OTString strMemo;

                        if (pBoxTrans->GetMemo(strMemo))
                            str_memo = strMemo.Get();

                        // DELETE THE BELOW CODE (replaced by above code.)
                        //                    const OTString
                        // strBoxTrans(*pBoxTrans);
                        //
                        //                    if (strBoxTrans.Exists())
                        //                        str_memo =
                        // OTAPI_Wrap::Pending_GetNote(*pstr_server_id,
                        // *pstr_nym_id, str_account_id, strBoxTrans.Get());
                    }
                }
                time64_t tValidFrom = OT_TIME_ZERO, tValidTo = OT_TIME_ZERO;
                std::string str_date =
                    "0"; // the "date signed" on the transaction receipt.
                time64_t tDateSigned = pBoxTrans->GetDateSigned();

                if (tDateSigned > OT_TIME_ZERO) {
                    tValidFrom = tDateSigned;
                    const uint64_t lDateSigned =
                        OTTimeGetSecondsFromTime(tDateSigned);
                    OTString strDateSigned;
                    strDateSigned.Format("%" PRIu64 "", lDateSigned);
                    str_date = strDateSigned.Get();
                }
                std::string str_amount; // <========== AMOUNT
                int64_t lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                if (0 == lAmount) lAmount = pBoxTrans->GetReceiptAmount();
                if (lAmount >
                    0) // Outgoing transfer should display with negative amount
                    lAmount *= (-1);
                if (0 != lAmount) {
                    OTString strTemp;
                    strTemp.Format("%" PRId64 "", lAmount);
                    str_amount = strTemp.Get();
                }
                std::string str_type(
                    pBoxTrans->GetTypeString()); // pending, chequeReceipt, etc.
                if (0 == str_type.compare("pending")) str_type = "transfer";
                otOut << __FUNCTION__ << ": ADDED: "
                      << ((OTTransaction::pending == pBoxTrans->GetType())
                              ? "pending"
                              : "ERROR")
                      << " outgoing transfer (str_type: " << str_type.c_str()
                      << ").\n";

                shared_ptr_OTRecord sp_Record(new OTRecord(
                    *pstr_server_id, *pstr_asset_id, *pstr_asset_name,
                    *pstr_nym_id,   // This is the Nym WHOSE BOX IT IS.
                    str_account_id, // This is the Nym's account for this box.
                    // Everything above this line, it stores a reference to an
                    // external string.
                    // Everything below this line, it makes its own internal
                    // copy of the string.
                    str_name, // name of recipient (since its in the outbox.)
                    str_date, // the "valid from" date on the instrument.
                    str_amount, str_type, // pending, chequeReceipt, etc.
                    (OTTransaction::pending ==
                     pBoxTrans->GetType()), // Basically always true, in this
                                            // case.
                    true,  // bIsOutgoing=true. (Since this is the outbox...)
                    false, // IsRecord
                    false, // IsReceipt
                    OTRecord::Transfer));
                const OTString strContents(*pBoxTrans);
                sp_Record->SetContents(strContents.Get());
                sp_Record->SetDateRange(tValidFrom, tValidTo);
                sp_Record->SetBoxIndex(nOutboxIndex);
                if (!str_memo.empty()) sp_Record->SetMemo(str_memo);
                if (!str_other_nym_id.empty())
                    sp_Record->SetOtherNymID(str_other_nym_id);
                if (!str_other_acct_id.empty())
                    sp_Record->SetOtherAccountID(str_other_acct_id);
                sp_Record->SetTransNumForDisplay(
                    pBoxTrans->GetReferenceNumForDisplay());
                sp_Record->SetTransactionNum(pBoxTrans->GetTransactionNum());
                m_contents.push_back(sp_Record);
            }
        }
        // For this record box, pass a NymID AND an AcctID,
        // since it's the recordbox for a specific account.
        //
        // OPTIMIZE FYI:
        // NOTE: LoadRecordBox is much SLOWER than LoadRecordBoxNoVerify, but it
        // also lets you get
        // the NAME off of the box receipt. So if you are willing to GIVE UP the
        // NAME, in
        // return for FASTER PERFORMANCE, then call SetFastMode() before
        // Populating.
        //
        OTLedger* pRecordbox = m_bRunFast
                                   ? OTAPI_Wrap::OTAPI()->LoadRecordBoxNoVerify(
                                         theServerID, theNymID, theAccountID)
                                   : OTAPI_Wrap::OTAPI()->LoadRecordBox(
                                         theServerID, theNymID, theAccountID);
        std::unique_ptr<OTLedger> theRecordBoxAngel(pRecordbox);

        // It loaded up, so let's loop through it.
        if (nullptr != pRecordbox) {
            int32_t nRecordIndex = -1;
            for (auto& it : pRecordbox->GetTransactionMap()) {
                ++nRecordIndex;
                OTTransaction* pBoxTrans = it.second;
                OT_ASSERT(nullptr != pBoxTrans);
                otOut << __FUNCTION__
                      << ": Account RECORD index: " << nRecordIndex << "\n";
                bool bOutgoing = false;
                bool bCanceled = false;
                std::string str_name; // name of sender OR recipient (depending
                                      // on whether it was originally incoming
                                      // or outgoing.)
                std::string str_other_nym_id;
                std::string str_other_acct_id;
                std::string str_memo;

                if (!pBoxTrans->IsAbbreviated()) {
                    OTIdentifier theSenderID, theSenderAcctID;
                    OTIdentifier theRecipientID, theRecipientAcctID;

                    if (pBoxTrans->GetSenderAcctIDForDisplay(theSenderAcctID)) {
                        const OTString strSenderAcctID(theSenderAcctID);
                        const std::string str_sender_acct_id(
                            strSenderAcctID.Get());

                        // Usually, Nym is the RECIPIENT. Sometimes he's the
                        // sender.
                        // Either way, we want the OTHER ID (the other Nym) for
                        // display.
                        // So here, if Nym's CLEARLY the sender, then we want
                        // the RECIPIENT.
                        // Whereas if Nym were the recipient, then we'd want the
                        // SENDER. (For display.)
                        //
                        if (0 ==
                            str_account_id.compare(
                                str_sender_acct_id)) // str_account_id IS
                                                     // str_sender_acct_id.
                                                     // (Therefore we want
                                                     // recipient.)
                        {
                            bOutgoing = true; // if Nym is the sender, then it
                                              // must have been outgoing.

                            const bool bGotRecipientUserIDForDisplay =
                                pBoxTrans->GetRecipientUserIDForDisplay(
                                    theRecipientID);

                            if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                    theRecipientAcctID)) {
                                const OTString strRecipientAcctID(
                                    theRecipientAcctID);
                                const std::string str_recip_acct_id(
                                    strRecipientAcctID.Get());

                                OTString strRecipientUserID("");
                                std::string str_recip_user_id("");

                                if (bGotRecipientUserIDForDisplay) {
                                    theRecipientID.GetString(
                                        strRecipientUserID);
                                    str_recip_user_id =
                                        strRecipientUserID.Get();
                                }
                                // NOTE: We check for cancelled here so we don't
                                // accidentally
                                // cause the address book to falsely believe
                                // that str_recip_user_id
                                // is the owner of str_recip_acct_id. (If the
                                // cheque/invoice is cancelled,
                                // the recipient account will be the sender
                                // account, which is NOT owned
                                // by the recipient, obviously...)
                                //
                                if (!pBoxTrans->IsCancelled()) {
                                    OTString strName(m_pLookup->GetAcctName(
                                        str_recip_acct_id,
                                        // NOTE: we CANNOT pass
                                        // str_recip_user_id here with
                                        // str_recip_acct_id
                                        // if it's a cancelled instrument, since
                                        // in that case, the SENDER ACCT
                                        // is ALSO the RECIPIENT ACCT. So this
                                        // logic is ONLY correct since we
                                        // are inside the block of if
                                        // (!pBoxTrans->IsCancelled())
                                        // (Otherwise we'd be training the
                                        // address book to falsely believe that
                                        // the recipient Nym is the owner of the
                                        // sender acct.)
                                        bGotRecipientUserIDForDisplay
                                            ? &str_recip_user_id
                                            : nullptr,   // nym ID if known
                                        pstr_server_id,  // server ID if known.
                                        pstr_asset_id)), // asset ID if known.
                                        strNameTemp;

                                    if (strName.Exists())
                                        strNameTemp.Format(
                                            OTRecordList::textTo(),
                                            strName.Get());
                                    else
                                        strNameTemp.Format(
                                            OTRecordList::textTo(),
                                            str_recip_acct_id.c_str());

                                    str_name =
                                        strNameTemp.Get(); // We don't want to
                                                           // see our own name
                                                           // on cancelled
                                                           // cheques.
                                }
                                str_other_acct_id = str_recip_acct_id;
                            }
                            if (bGotRecipientUserIDForDisplay) {
                                const OTString strRecipientID(theRecipientID);
                                const std::string str_recipient_id(
                                    strRecipientID.Get());

                                str_other_nym_id = str_recipient_id;

                                if (str_name.empty()) {
                                    OTString strName(m_pLookup->GetNymName(
                                        str_recipient_id, &(*it_server))),
                                        strNameTemp;

                                    if (strName.Exists())
                                        strNameTemp.Format(
                                            OTRecordList::textTo(),
                                            strName.Get());
                                    else
                                        strNameTemp.Format(
                                            OTRecordList::textTo(),
                                            str_recipient_id.c_str());

                                    str_name = strNameTemp.Get();
                                }
                            }
                        }
                        else // str_account_id IS NOT str_sender_acct_id.
                               // (Therefore we want sender.)
                        { // In this case, some OTHER Nym is the sender, so it
                            // must have been incoming. (And bOutgoing is
                            // already
                            // false.)

                            if (pBoxTrans->GetSenderUserIDForDisplay(
                                    theSenderID)) {
                                const OTString strSenderUserID(theSenderID);
                                str_other_nym_id = strSenderUserID.Get();
                            }
                            OTString strName(m_pLookup->GetAcctName(
                                str_sender_acct_id,
                                str_other_nym_id.empty()
                                    ? nullptr
                                    : &str_other_nym_id, // nym ID if known
                                pstr_server_id,          // server ID if known.
                                pstr_asset_id)),         // asset ID if known.
                                strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(OTRecordList::textFrom(),
                                                   strName.Get());
                            else
                                strNameTemp.Format(OTRecordList::textFrom(),
                                                   str_sender_acct_id.c_str());

                            str_name = strNameTemp.Get();
                            str_other_acct_id = str_sender_acct_id;
                        }
                    }
                    // In this block below, we already KNOW
                    // GetSenderAcctIDForDisplay is EMPTY.
                    // (So it's "recipient or bust.")
                    else if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                 theRecipientAcctID)) {
                        if (pBoxTrans->GetRecipientUserIDForDisplay(
                                theRecipientID)) {
                            const OTString strRecipientID(theRecipientID);
                            const std::string str_recipient_user_id(
                                strRecipientID.Get());

                            str_other_nym_id = str_recipient_user_id;
                        }
                        const OTString strRecipientAcctID(theRecipientAcctID);
                        const std::string str_recipient_acct_id(
                            strRecipientAcctID.Get());

                        if (0 !=
                            str_account_id.compare(
                                str_recipient_acct_id)) // str_account_id is NOT
                        // str_recipient_acct_id.
                        // (Therefore we want
                        // str_recipient_acct_id.)
                        {
                            // If Nym is not the recipient, then he must be the
                            // sender.
                            // (Therefore it must be outgoing.)
                            bOutgoing = true;

                            OTString strName(m_pLookup->GetAcctName(
                                str_recipient_acct_id,
                                str_other_nym_id.empty()
                                    ? nullptr
                                    : &str_other_nym_id, // nym ID if known
                                pstr_server_id,          // server ID if known.
                                pstr_asset_id)),         // asset ID if known.
                                strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(OTRecordList::textTo(),
                                                   strName.Get());
                            else
                                strNameTemp.Format(
                                    OTRecordList::textTo(),
                                    str_recipient_acct_id.c_str());

                            str_name = strNameTemp.Get();
                            str_other_acct_id = str_recipient_acct_id;
                        }
                    }
                    else if (pBoxTrans->GetSenderUserIDForDisplay(
                                   theSenderID)) {
                        const OTString strSenderID(theSenderID);
                        const std::string str_sender_id(strSenderID.Get());

                        // Usually, Nym is the RECIPIENT. Sometimes he's the
                        // sender.
                        // Either way, we want the OTHER ID (the other Nym) for
                        // display.
                        // So here, if Nym's CLEARLY the sender, then we want
                        // the RECIPIENT.
                        // Whereas if Nym were the recipient, then we'd want the
                        // SENDER. (For display.)
                        //
                        if (0 ==
                            str_nym_id.compare(str_sender_id)) // str_nym_id IS
                                                               // str_sender_id.
                                                               // (Therefore we
                                                               // want
                                                               // recipient.)
                        {
                            bOutgoing = true; // if Nym is the sender, then it
                                              // must have been outgoing.

                            if (pBoxTrans->GetRecipientUserIDForDisplay(
                                    theRecipientID)) {
                                const OTString strRecipientID(theRecipientID);
                                const std::string str_recipient_id(
                                    strRecipientID.Get());

                                OTString strName(m_pLookup->GetNymName(
                                    str_recipient_id, &(*it_server))),
                                    strNameTemp;

                                if (strName.Exists())
                                    strNameTemp.Format(OTRecordList::textTo(),
                                                       strName.Get());
                                else
                                    strNameTemp.Format(
                                        OTRecordList::textTo(),
                                        str_recipient_id.c_str());

                                str_name = strNameTemp.Get();
                                str_other_nym_id = str_recipient_id;
                                if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                        theRecipientAcctID)) {
                                    const OTString strRecipientAcctID(
                                        theRecipientAcctID);
                                    str_other_acct_id =
                                        strRecipientAcctID.Get();
                                }
                            }
                        }
                        else // str_nym_id IS NOT str_sender_id. (Therefore we
                               // want sender.)
                        { // In this case, some OTHER Nym is the sender, so it
                            // must have been incoming. (And bOutgoing is
                            // already
                            // false.)

                            OTString strName(m_pLookup->GetNymName(
                                str_sender_id, &(*it_server))),
                                strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(OTRecordList::textFrom(),
                                                   strName.Get());
                            else
                                strNameTemp.Format(OTRecordList::textFrom(),
                                                   str_sender_id.c_str());

                            str_name = strNameTemp.Get();
                            str_other_nym_id = str_sender_id;
                            if (pBoxTrans->GetSenderAcctIDForDisplay(
                                    theSenderAcctID)) {
                                const OTString strSenderAcctID(theSenderAcctID);
                                str_other_acct_id = strSenderAcctID.Get();
                            }
                        }
                    }
                    // In this block below, we already KNOW
                    // GetSenderUserIDForDisplay is EMPTY.
                    // (So it's "recipient or bust.")
                    else if (pBoxTrans->GetRecipientUserIDForDisplay(
                                 theRecipientID)) {
                        const OTString strRecipientID(theRecipientID);
                        const std::string str_recipient_id(
                            strRecipientID.Get());

                        if (0 !=
                            str_nym_id.compare(str_recipient_id)) // str_nym_id
                                                                  // is NOT
                        // str_recipient_id.
                        // (Therefore we want
                        // str_recipient_id.)
                        {
                            // If Nym is not the recipient, then he must be the
                            // sender.
                            // (Therefore it must be outgoing.)
                            bOutgoing = true;

                            OTString strName(m_pLookup->GetNymName(
                                str_recipient_id, &(*it_server))),
                                strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(OTRecordList::textTo(),
                                                   strName.Get());
                            else
                                strNameTemp.Format(OTRecordList::textTo(),
                                                   str_recipient_id.c_str());

                            str_name = strNameTemp.Get();
                            str_other_nym_id = str_recipient_id;
                            if (pBoxTrans->GetRecipientAcctIDForDisplay(
                                    theRecipientAcctID)) {
                                const OTString strRecipientAcctID(
                                    theRecipientAcctID);
                                str_other_acct_id = strRecipientAcctID.Get();
                            }
                        }
                    }
                    // Get the Memo field for a transferReceipt and also for
                    // other receipts.
                    //
                    OTString strMemo;

                    if (pBoxTrans->GetMemo(strMemo)) str_memo = strMemo.Get();

                } // if not abbreviated.
                bCanceled = pBoxTrans->IsCancelled();
                time64_t tValidFrom = OT_TIME_ZERO, tValidTo = OT_TIME_ZERO;
                std::string str_date =
                    "0"; // the "date signed" on the transaction receipt.
                time64_t tDateSigned = pBoxTrans->GetDateSigned();

                if (tDateSigned > OT_TIME_ZERO) {
                    tValidFrom = tDateSigned;
                    const uint64_t lDateSigned =
                        OTTimeGetSecondsFromTime(tDateSigned);
                    OTString strDateSigned;
                    strDateSigned.Format("%" PRIu64 "", lDateSigned);
                    str_date = strDateSigned.Get();
                }
                std::string str_amount; // <========== AMOUNT
                int64_t lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                if (0 == lAmount) lAmount = pBoxTrans->GetReceiptAmount();
                const std::string str_type(
                    pBoxTrans->GetTypeString()); // pending, chequeReceipt, etc.
                if (0 == str_type.compare("transferReceipt"))
                    bOutgoing = true; // only the sender of a transfer will have
                                      // a transferReceipt.
                else if (0 == str_type.compare("pending"))
                    bOutgoing = false; // only the recipient of a transfer will
                                       // have a pending in his recordbox.
                if (0 != lAmount) {

                    //                if (lAmount < 0)
                    //                    bOutgoing = true;
                    //                else
                    //                    bOutgoing = false;

                    // A transfer receipt ALWAYS represents an outgoing
                    // transfer.
                    // If the amount is over 0, we want to display it as a
                    // negative
                    // since it represents money LEAVING my account.
                    //                if ((0 ==
                    // str_type.compare("transferReceipt")) && (lAmount > 0))
                    //                    lAmount *= (-1);

                    OTString strTemp;
                    strTemp.Format("%" PRId64 "", lAmount);
                    str_amount = strTemp.Get();
                }
                otOut << __FUNCTION__ << ": ADDED: "
                      << ((pBoxTrans->GetType() != OTTransaction::pending)
                              ? ""
                              : (bOutgoing ? "sent" : "received"))
                      << " (asset account) record (str_type: "
                      << str_type.c_str() << ")\n";
                // This line means: If it's a receipt, use a blank string.
                // Otherwise if
                // it's a transfer, then show sent/received. (This is the
                // record box, so
                // if it's a transfer, it's a completed one.)
                //
                // FYI, for Receipts we don't say "sent transferReceipt",
                // we just say "transferReceipt."

                shared_ptr_OTRecord sp_Record(new OTRecord(
                    *pstr_server_id, *pstr_asset_id, *pstr_asset_name,
                    *pstr_nym_id,   // This is the Nym WHOSE BOX IT IS.
                    str_account_id, // This is the Nym's account for this box.
                    // Everything above this line, it stores a reference to an
                    // external string.
                    // Everything below this line, it makes its own internal
                    // copy of the string.
                    str_name, // name of sender or recipient (whichever is NOT
                              // the current Nym.)
                    str_date, // the "valid from" date on the instrument.
                    str_amount, str_type, // pending, chequeReceipt, etc.
                    false, // bPending=false. If it's in the record box, then
                           // it's finished (not pending.)
                    bOutgoing, // Record box stores both old incoming, AND old
                               // outgoing, receipts.
                    true,      // IsRecord
                    pBoxTrans->GetType() != OTTransaction::pending, // IsReceipt
                    pBoxTrans->GetType() == OTTransaction::pending
                        ? OTRecord::Transfer
                        : OTRecord::Receipt));
                const OTString strContents(*pBoxTrans);
                sp_Record->SetContents(strContents.Get());
                if (bCanceled) sp_Record->SetCanceled();
                sp_Record->SetDateRange(tValidFrom, tValidTo);
                sp_Record->SetBoxIndex(nRecordIndex);
                if (!str_memo.empty()) sp_Record->SetMemo(str_memo);
                if (!str_other_nym_id.empty())
                    sp_Record->SetOtherNymID(str_other_nym_id);
                if (!str_other_acct_id.empty())
                    sp_Record->SetOtherAccountID(str_other_acct_id);
                sp_Record->SetTransNumForDisplay(
                    pBoxTrans->GetReferenceNumForDisplay());
                sp_Record->SetTransactionNum(pBoxTrans->GetTransactionNum());
                m_contents.push_back(sp_Record);
            }
        }

    } // loop through the accounts.
    // SORT the vector.
    //
    SortRecords();
    return true;
}

const list_of_strings& OTRecordList::GetNyms() const
{
    return m_nyms;
}

// Populate already sorts. But if you have to add some external records
// after Populate, then you can sort again.
//
void OTRecordList::SortRecords()
{
    // TODO OPTIMIZE: We might load everything up into a multimap, and THEN copy
    // it
    // directly over to the vector. (Since the multimap sorts automatically on
    // insert.)
    // The question is, would doing that be any faster than just sorting it
    // here?
    // (Possibly not, but I'm not sure. Re-visit later.)
    //
    // Todo optimize: any faster sorting algorithms?
    std::sort(m_contents.begin(), m_contents.end(),
              [](const shared_ptr_OTRecord& i,
                 const shared_ptr_OTRecord& j) { return j->operator<(*i); });
}

// Let's say you also want to add some Bitmessages. (Or any other external
// source.) This is where you do that. Make sure to call Populate, then use
// AddSpecialMsg a few times, then call SortRecords.
//
void OTRecordList::AddSpecialMsg(
    const std::string& str_msg_id, // The id of this message, from whatever
                                   // system it came from.
    bool bIsOutgoing, int32_t nMethodID,
    const std::string& str_contents, // Make sure to concatentate subject with
                                     // contents, before passing here.
    const std::string& str_address, const std::string& str_other_address,
    const std::string& str_type, const std::string& str_type_display,
    std::string str_my_nym_id, time64_t tDate)
{
    const char* pToFrom =
        bIsOutgoing ? OTRecordList::textTo() : OTRecordList::textFrom();
    const std::string* p_str_server =
        &OTRecordList::s_blank; // <========== Bitmessage doesn't use OT
                                // servers.
    // TODO OPTIMIZE: instead of looking up the Nym's name every time, look it
    // up ONCE when first adding the NymID. Add it to a map, instead of a list,
    // and add the Nym's name as the second item in the map's pair.
    // (Just like I already did with the asset type.)
    //
    std::string str_other_name;

    if (!str_other_address.empty())
        str_other_name = m_pLookup->GetAddressName(str_other_address);
    OTString strNameTemp;
    std::string str_name("");

    if (!str_other_name.empty())
        strNameTemp.Format(pToFrom, str_other_name.c_str());
    else if (!str_other_address.empty())
        strNameTemp.Format(pToFrom, str_other_address.c_str());

    str_name = strNameTemp.Get();
    const std::string* p_str_nym_id =
        &OTRecordList::s_blank; // <========== MY NYM ID
    const std::string* p_str_asset_type =
        &OTRecordList::s_blank; // <========== ASSET TYPE
    const std::string* p_str_asset_name =
        &OTRecordList::s_blank; // asset type display name.
    const std::string* p_str_account =
        &OTRecordList::s_blank; // <========== ACCOUNT

    std::string str_amount; // There IS NO amount, on mail. (So we leave this
                            // empty.)
    // ---------------------------------------------------
    uint64_t lDate = static_cast<uint64_t>(tDate);
    OTString strDate;
    strDate.Format("%" PRIu64 "", lDate);
    const std::string str_date(strDate.Get());
    // CREATE AN OTRecord AND POPULATE IT...
    //
    // This loop is here because normally an OTRecord's "nym id" is
    // a reference to a string found in OTRecordList's list of Nyms.
    // We can't just stick a Nym ID in there, we have to find the
    // specific Nym ID string in the OTRecordList and use THAT, otherwise
    // we'd end up with bad pointer problems.
    //
    if (!str_my_nym_id.empty()) {
        for (auto& it_nym : m_nyms) {
            const std::string& str_nym_id(it_nym);

            if (0 == str_my_nym_id.compare(str_nym_id)) {
                p_str_nym_id = &str_nym_id;
                break;
            }
        }
    }
    shared_ptr_OTRecord sp_Record(new OTRecord(
        *p_str_server, *p_str_asset_type, *p_str_asset_name,
        *p_str_nym_id,  // This is "me" (the sender Nym, if outgoing, or
                        // recipient, if incoming.)
        *p_str_account, // No OT asset account on any mail, much less on a
                        // "bitmessage" mail.
        // Everything above this line, it stores a reference to an external
        // string.
        // Everything below this line, it makes its own internal copy of the
        // string.
        str_name, // name of sender for incoming, otherwise name of recipient
                  // for outgoing.
        str_date, str_amount, OTRecordList::s_message_type, // "message"
        false,              // bIsPending=false since its already received.
        bIsOutgoing, false, // IsRecord
        false,              // IsReceipt
        OTRecord::Mail));
    sp_Record->SetSpecialMail(); // true by default. This means it's not an OT
                                 // message, but a Bitmessage (or something like
                                 // that.)
    sp_Record->SetContents(str_contents.c_str()); // "Subject: %s\n[Contents]"
    sp_Record->SetMsgID(str_msg_id);
    sp_Record->SetDateRange(OTTimeGetTimeFromSeconds(tDate),
                            OTTimeGetTimeFromSeconds(tDate));
    sp_Record->SetMethodID(nMethodID);
    sp_Record->SetAddress(str_address);
    sp_Record->SetOtherAddress(str_other_address);
    sp_Record->SetMsgType(str_type);
    sp_Record->SetMsgTypeDisplay(str_type_display);
    otWarn << __FUNCTION__
           << ": ADDED: " << (bIsOutgoing ? "outgoing" : "incoming")
           << " special mail.\n";

    m_contents.push_back(sp_Record);
}

// This one expects that s_pCaller is not nullptr.
//
OTRecordList::OTRecordList()
    : m_pLookup(nullptr)
    , m_bRunFast(false)
    , m_bAutoAcceptCheques(false)
    , m_bAutoAcceptReceipts(false)
    , m_bAutoAcceptTransfers(false)
    , m_bAutoAcceptCash(false)
{
    OT_ASSERT_MSG((nullptr != s_pCaller),
                  "Address Book Caller was nullptr! "
                  "On app startup, did you forget to call "
                  "OT_API_Set_AddrBookCallback ?\n");
    OT_ASSERT_MSG((s_pCaller->isCallbackSet()),
                  "Address Book Callback was nullptr! "
                  "On app startup, did you forget to call "
                  "OT_API_Set_AddrBookCallback ?\n");
    m_pLookup = s_pCaller->getCallback(); // <==========
}

OTRecordList::OTRecordList(const OTNameLookup& theLookup)
    : m_pLookup(&theLookup)
    , m_bRunFast(false)
    , m_bAutoAcceptCheques(false)
    , m_bAutoAcceptReceipts(false)
    , m_bAutoAcceptTransfers(false)
    , m_bAutoAcceptCash(false)
{
}

OTRecordList::~OTRecordList()
{
    //  if (nullptr != m_pLookup) // NO DELETE! We assume whatever client app is
    // using OTRecordList, will
    //      delete m_pLookup;  // delete its own address book lookup class when
    // it is good and ready.

    m_pLookup = nullptr;
}

// Clears m_contents (NOT nyms, accounts, servers, or asset types.)

void OTRecordList::ClearContents()
{
    m_contents.clear();
}

// RETRIEVE:
//

int32_t OTRecordList::size() const
{
    return m_contents.size();
}

bool OTRecordList::RemoveRecord(int32_t nIndex)
{
    OT_ASSERT((nIndex >= 0) &&
              (nIndex < static_cast<int32_t>(m_contents.size())));
    m_contents.erase(m_contents.begin() + nIndex);
    return true;
}

shared_ptr_OTRecord OTRecordList::GetRecord(int32_t nIndex)
{
    OT_ASSERT((nIndex >= 0) &&
              (nIndex < static_cast<int32_t>(m_contents.size())));
    weak_ptr_OTRecord wp_record(m_contents[nIndex]);
    shared_ptr_OTRecord sp_record(wp_record);

    return sp_record;
}

} // namespace opentxs
