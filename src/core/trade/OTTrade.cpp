/************************************************************
 *
 *  OTTrade.cpp
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

#include <opentxs/core/trade/OTTrade.hpp>
#include <opentxs/core/trade/OTMarket.hpp>
#include <opentxs/core/trade/OTOffer.hpp>
#include <opentxs/core/Account.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTPseudonym.hpp>

#include <irrxml/irrXML.hpp>

#include <memory>

namespace opentxs
{

enum { TradeProcessIntervalSeconds = 10 };

// This class is like: you are placing an order to do a trade.
// Your order will continue processing until it is complete.
// PART of that process is putting an offer on the market. See OTOffer for that.
//
// Trades are like cron items, they can expire, they can have rules.
//
// An OTTrade is derived from OTCronItem. OTCron has a list of those items.

// Used to be I could just call pTrade->VerifySignature(nym), which is what
// I still call here, inside this function. But that's a special case -- an
// override
// from the OTScriptable / OTSmartContract version, which verifies parties and
// agents, etc.
//
bool OTTrade::VerifyNymAsAgent(OTPseudonym& nym,
                               OTPseudonym&, // Not needed in this version of
                                             // the override.
                               mapOfNyms*) const
{
    return VerifySignature(nym);
}

// This is an override. See note above.
//
bool OTTrade::VerifyNymAsAgentForAccount(OTPseudonym& nym,
                                         Account& account) const
{
    return account.VerifyOwner(nym);
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t OTTrade::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    int32_t returnVal = 0;

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    //
    if (0 != (returnVal = ot_super::ProcessXMLNode(xml))) return returnVal;

    if (!strcmp("trade", xml->getNodeName())) {
        m_strVersion = xml->getAttributeValue("version");
        tradesAlreadyDone_ = atoi(xml->getAttributeValue("completedNoTrades"));

        SetTransactionNum(
            String::StringToLong(xml->getAttributeValue("transactionNum")));

        const String creationStr = xml->getAttributeValue("creationDate");
        const String validFromStr = xml->getAttributeValue("validFrom");
        const String validToStr = xml->getAttributeValue("validTo");

        int64_t creation = creationStr.ToLong();
        int64_t validFrom = validFromStr.ToLong();
        int64_t validTo = validToStr.ToLong();

        SetCreationDate(OTTimeGetTimeFromSeconds(creation));
        SetValidFrom(OTTimeGetTimeFromSeconds(validFrom));
        SetValidTo(OTTimeGetTimeFromSeconds(validTo));

        String activated(xml->getAttributeValue("hasActivated"));

        if (activated.Compare("true"))
            hasTradeActivated_ = true;
        else
            hasTradeActivated_ = false;

        const String serverID(xml->getAttributeValue("serverID")),
            userID(xml->getAttributeValue("userID")),
            assetTypeID(xml->getAttributeValue("assetTypeID")),
            assetAcctID(xml->getAttributeValue("assetAcctID")),
            currencyTypeID(xml->getAttributeValue("currencyTypeID")),
            currencyAcctID(xml->getAttributeValue("currencyAcctID"));

        const Identifier SERVER_ID(serverID), USER_ID(userID),
            ASSET_TYPE_ID(assetTypeID), ASSET_ACCT_ID(assetAcctID),
            CURRENCY_TYPE_ID(currencyTypeID), CURRENCY_ACCT_ID(currencyAcctID);

        SetServerID(SERVER_ID);
        SetSenderUserID(USER_ID);
        SetAssetID(ASSET_TYPE_ID);
        SetSenderAcctID(ASSET_ACCT_ID);
        SetCurrencyID(CURRENCY_TYPE_ID);
        SetCurrencyAcctID(CURRENCY_ACCT_ID);

        otLog3 << "\n\nTrade. Transaction Number: " << m_lTransactionNum
               << "   Completed # of Trades: " << tradesAlreadyDone_ << "\n";

        otWarn << " Creation Date: " << creation
               << "   Valid From: " << validFrom << "\n Valid To: " << validTo
               << "\n"
                  " assetTypeID: " << assetTypeID
               << "\n assetAcctID: " << assetAcctID << "\n"
                                                       " ServerID: " << serverID
               << "\n UserID: " << userID
               << "\n "
                  " currencyTypeID: " << currencyTypeID
               << "\n currencyAcctID: " << currencyAcctID << "\n ";

        returnVal = 1;
    }

    if (!strcmp("stopOrder", xml->getNodeName())) {
        String sign(xml->getAttributeValue("sign"));

        if (sign.Compare("0")) {
            stopSign_ = 0; // Zero means it isn't a stop order. So why is the
                           // tag in the file?
            otErr << "Strange: Stop order tag found in trade, but sign "
                     "character set to 0.\n"
                     "(Zero means: NOT a stop order.)\n";
            return (-1);
        }
        else if (sign.Compare("<"))
            stopSign_ = '<';
        else if (sign.Compare(">"))
            stopSign_ = '>';
        else {
            stopSign_ = 0;
            otErr << "Unexpected or nonexistent value in stop order sign: "
                  << sign << "\n";
            return (-1);
        }

        // Now we know the sign is properly formed, let's grab the price value.

        stopPrice_ = String::StringToLong(xml->getAttributeValue("price"));

        String activated(xml->getAttributeValue("hasActivated"));

        if (activated.Compare("true"))
            stopActivated_ = true;
        else
            stopActivated_ = false;

        otLog3 << "\n\nStop order -- "
               << (stopActivated_ ? "Already activated" : "Will activate")
               << " when price " << (stopActivated_ ? "was" : "reaches") << " "
               << (('<' == stopSign_) ? "LESS THAN" : "GREATER THAN") << ": "
               << stopPrice_ << ".\n";

        returnVal = 1;
    }
    else if (!strcmp("offer", xml->getNodeName())) {
        if (!Contract::LoadEncodedTextField(xml, marketOffer_)) {
            otErr << "Error in OTTrade::ProcessXMLNode: offer field without "
                     "value.\n";
            return (-1); // error condition
        }

        returnVal = 1;
    }

    return returnVal;
}

void OTTrade::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    m_xmlUnsigned.Concatenate("<?xml version=\"%s\"?>\n\n", "1.0");

    const String SERVER_ID(GetServerID()), USER_ID(GetSenderUserID()),
        ASSET_TYPE_ID(GetAssetID()), ASSET_ACCT_ID(GetSenderAcctID()),
        CURRENCY_TYPE_ID(GetCurrencyID()),
        CURRENCY_ACCT_ID(GetCurrencyAcctID());

    m_xmlUnsigned.Concatenate(
        "<trade\n version=\"%s\"\n"
        " hasActivated=\"%s\"\n"
        " serverID=\"%s\"\n"
        " assetTypeID=\"%s\"\n"
        " assetAcctID=\"%s\"\n"
        " currencyTypeID=\"%s\"\n"
        " currencyAcctID=\"%s\"\n"
        " userID=\"%s\"\n"
        " completedNoTrades=\"%d\"\n"
        " transactionNum=\"%" PRId64 "\"\n"
        " creationDate=\"%" PRId64 "\"\n"
        " validFrom=\"%" PRId64 "\"\n"
        " validTo=\"%" PRId64 "\""
        " >\n\n",
        m_strVersion.Get(), (hasTradeActivated_ ? "true" : "false"),
        SERVER_ID.Get(), ASSET_TYPE_ID.Get(), ASSET_ACCT_ID.Get(),
        CURRENCY_TYPE_ID.Get(), CURRENCY_ACCT_ID.Get(), USER_ID.Get(),
        tradesAlreadyDone_, m_lTransactionNum,
        OTTimeGetSecondsFromTime(GetCreationDate()),
        OTTimeGetSecondsFromTime(GetValidFrom()),
        OTTimeGetSecondsFromTime(GetValidTo()));

    // There are "closing" transaction numbers, used to CLOSE a transaction.
    // Often where Cron items are involved such as this payment plan, or in
    // baskets,
    // where many asset accounts are involved and require receipts to be closed
    // out.

    for (int32_t i = 0; i < GetCountClosingNumbers(); i++) {
        int64_t closingNumber = GetClosingTransactionNoAt(i);
        OT_ASSERT(closingNumber > 0);

        m_xmlUnsigned.Concatenate("<closingTransactionNumber value=\"%" PRId64
                                  "\"/>\n\n",
                                  closingNumber);
    }

    if (('<' == stopSign_) || ('>' == stopSign_)) {
        m_xmlUnsigned.Concatenate("<stopOrder\n"
                                  " hasActivated=\"%s\"\n"
                                  " sign=\"%c\"\n"
                                  " price=\"%" PRId64 "\""
                                  " />\n\n",
                                  (stopActivated_ ? "true" : "false"),
                                  stopSign_, stopPrice_);
    }

    if (marketOffer_.Exists()) {
        OTASCIIArmor ascOffer(marketOffer_);
        m_xmlUnsigned.Concatenate("<offer>\n%s</offer>\n\n", ascOffer.Get());
    }

    m_xmlUnsigned.Concatenate("</trade>\n");
}

// The trade stores a copy of the Offer in string form.
// This function verifies that offer against the trade,
// and also verifies the signature on the offer.
//
// The Nym's ID is compared to offer's SenderUserID, and then the Signature
// is checked
// on the offer.  It also compares the server ID, asset and currency IDs,
// transaction #, etc
// between this trade and the offer, in order to fully verify the offer's
// authenticity.
//
bool OTTrade::VerifyOffer(OTOffer& offer) const
{
    // At this point, I have a working, loaded, model of the Offer.
    // Let's verify the thing.

    if (GetTransactionNum() != offer.GetTransactionNum()) {
        otErr << "While verifying offer, failed matching transaction number.\n";
        return false;
    }
    else if (GetServerID() != offer.GetServerID()) {
        otErr << "While verifying offer, failed matching Server ID.\n";
        return false;
    }
    else if (GetAssetID() != offer.GetAssetID()) {
        otErr << "While verifying offer, failed matching asset type ID.\n";
        return false;
    }
    else if (GetCurrencyID() != offer.GetCurrencyID()) {
        otErr << "While verifying offer, failed matching currency type ID.\n";
        return false;
    }

    // the Offer validates properly for this Trade.
    //
    return true;
}

// Assuming the offer is ON the market, this will get the pointer to that offer.
// Otherwise it will try to add it to the market.
// Otherwise it will fail. (Perhaps it's a stop order, and not ready to activate
// yet.)
//
OTOffer* OTTrade::GetOffer(Identifier* offerMarketId, OTMarket** market)
{
    OT_ASSERT(GetCron() != nullptr);

    // See if the offer has already been instantiated onto a market...
    if (offer_ != nullptr) {
        offer_->SetTrade(*this); // Probably don't need this line. I'll remove
                                 // it someday while optimizing.
        // In fact since it should already be set, having this here would
        // basically
        // hide it from me if the memory was ever walked on from a bug
        // somewhere.

        // It loaded. Let's get the Market ID off of it so we can locate the
        // market.
        const Identifier OFFER_MARKET_ID(*offer_);

        if (market != nullptr) {
            OTMarket* pMarket = GetCron()->GetMarket(OFFER_MARKET_ID);

            // Sometimes the caller function would like a copy of this market
            // pointer, when available.
            // So I pass it back to him here, if he wants. That way he doesn't
            // have to do this work again
            // to look it up.
            if (pMarket != nullptr)
                *market = pMarket; // <=================
            else
                otErr << "OTTrade::" << __FUNCTION__
                      << ": offer_ already exists, yet unable to find the "
                         "market it's supposed to be on.\n";
        }

        if (offerMarketId != nullptr) {
            // Sometimes the caller function would like a copy of this ID. So I
            // give the option to pass in a pointer so I can give it here.
            offerMarketId->assign(OFFER_MARKET_ID.begin(),
                                  OFFER_MARKET_ID.end());
        }

        return offer_;
    } // if offer_ ALREADY EXISTS.

    // else (BELOW) offer_ IS nullptr, and thus it didn't exist yet...

    if (!marketOffer_.Exists()) {
        otErr << "OTTrade::GetOffer called with empty marketOffer_.\n";
        return nullptr;
    }

    OTOffer* offer = new OTOffer();
    OT_ASSERT(offer != nullptr);

    // Trying to load the offer from the trader's original signed request
    // (So I can use it to lookup the Market ID, so I can see the offer is
    // already there on the market.)
    if (!offer->LoadContractFromString(marketOffer_)) {
        otErr << "Error loading offer from string in OTTrade::GetOffer\n";
        delete offer;
        offer = nullptr;
        return nullptr;
    }

    // No need to do any additional security verification here on the Offer,
    // since the Offer is already heavily verified in
    // OTServer::NotarizeMarketOffer().
    // So as long as you feel safe about the Trade, then you can feel safe about
    // the Offer already, with no further checks.
    // *Also remember we saved a copy of the original in the cron folder.

    // It loaded. Let's get the Market ID off of it so we can locate the market.
    Identifier OFFER_MARKET_ID(*offer);

    if (offerMarketId != nullptr) {
        // Sometimes the caller function would like a copy of this ID. So I
        // give the option to pass in a pointer so I can give it here.
        offerMarketId->assign(OFFER_MARKET_ID.begin(), OFFER_MARKET_ID.end());
    }

    // Previously if a user tried to use a market that didn't exist, I'd just
    // return failure.
    // But now we will create any market that doesn't already exist.
    // (Remember, the server operator could just erase the market folder--it
    // wouldn't
    // affect anyone's balances!) Update: he probably couldn't just wipe the
    // markets folder,
    // actually, without making it impossible for certain Nyms to get rid of
    // certain issued #s.
    //
    //    OTMarket * pMarket = m_cron->GetMarket(OFFER_MARKET_ID);
    OTMarket* pMarket = GetCron()->GetOrCreateMarket(
        GetAssetID(), GetCurrencyID(), offer->GetScale());

    // Couldn't find (or create) the market.
    if (pMarket != nullptr) {
        otOut
            << "OTTrade::" << __FUNCTION__
            << ": Unable to find or create market within requested parameters.";
        delete offer;
        offer = nullptr;
        return nullptr;
    }

    // If the caller passed in the address of a market pointer (optional)
    if (market != nullptr) {
        // Sometimes the caller function would like a copy of this market
        // pointer, when available.
        // So I pass it back to him here, if he wants. That way he doesn't have
        // to do this work again
        // to look it up.
        *market = pMarket;
    }

    // At this point, I have heap-allocated the offer, used it to get the Market
    // ID, and successfully
    // used that to get a pointer to the market matching that ID.
    //
    // Let's see if the offer is ALREADY allocated and on this market!
    // If so, delete the one I just allocated. If not, add it to the market.
    OTOffer* marketOffer = pMarket->GetOffer(offer->GetTransactionNum());

    // The Offer is already on the Market.
    // NOTE: It may just start out this way, without ever being added.
    // How is that possible? Because maybe it was in the market file when we
    // first loaded up,
    // and had been added on some previous run of the software. So since we
    // started running,
    // the pMarket->AddOffer() code below has literally never run for that
    // offer. Instead we
    // first find it here, and thus return the pointer before getting any
    // farther.
    //
    // IN ALL CASES, we make sure to call offer_->SetTrade() so that it has a
    // pointer BACK to
    // this Trade object! (When actually processing the offer, the market will
    // need the account
    // numbers and Nym IDs... which are stored here on the trade.)
    if (marketOffer != nullptr) {
        offer_ = marketOffer;

        // Since the Offer already exists on the market, no need anymore for the
        // one we allocated above (to get the market ID.) So we delete it now.
        delete offer;
        offer = nullptr;

        offer_->SetTrade(*this);

        return offer_;
    }

    // Okay so the offer ISN'T already on the market. If it's not a stop order,
    // let's ADD the one we
    // allocated to the market now! (Stop orders are activated through their own
    // logic, which is below
    // this, in the else block.)
    //
    if (!IsStopOrder()) {
        if (hasTradeActivated_) {
            // Error -- how has the trade already activated, yet not on the
            // market and null in my pointer?
            otErr << "How has the trade already activated, yet not on the "
                     "market and null in my pointer?\n";
        }
        else if (!pMarket->AddOffer(this, *offer, true)) // Since we're
                                                           // actually adding
                                                           // an offer to the
                                                           // market (not just
        { // loading from disk) the we actually want to save the market.
            // bSaveFile=true.
            // Error adding the offer to the market!
            otErr << "Error adding the offer to the market! (Even though "
                     "supposedly the right market.)\n";
        }
        else {
            // SUCCESS!
            offer_ = offer;

            hasTradeActivated_ = true;

            // The Trade (stored on Cron) has a copy of the Original Offer, with
            // the User's signature on it.
            // A copy of that original Trade object (itself with the user's
            // signature) is already stored in
            // the cron folder (by transaction number.) This happens when the
            // Trade is FIRST added to cron,
            // so it's already safe before we even get here.
            //
            // So thus I am FREE to release the signatures on the offer, and
            // sign with the server instead.
            // The server-signed offer will be stored by the OTMarket.
            offer_->ReleaseSignatures();
            offer_->SignContract(*(GetCron()->GetServerNym()));
            offer_->SaveContract();

            pMarket->SaveMarket();

            // Now when the market loads next time, it can verify this offer
            // using the server's signature,
            // instead of having to load the user. Because the server has
            // verified it and added it, and now
            // signs it, vouching for it.

            // The Trade itself (all its other variables) are now allowed to
            // change, since its signatures
            // are also released and it is now server-signed. (With a copy
            // stored of the original.)

            offer_->SetTrade(*this);

            return offer_;
        }
    }

    // It's a stop order, and not activated yet.
    // Should we activate it now?
    //
    else if (IsStopOrder() && !stopActivated_) {
        int64_t relevantPrice = 0;

        // If the stop order is trying to sell something, then it cares about
        // the highest bidder.
        if (offer->IsAsk())
            relevantPrice = pMarket->GetHighestBidPrice();
        else // But if the stop order is trying to buy something, then it cares
             // about the lowest ask price.
            relevantPrice = pMarket->GetLowestAskPrice();

        // It's a stop order that hasn't activated yet. SHOULD IT ACTIVATE NOW?
        if ((IsGreaterThan() && (relevantPrice > GetStopPrice())) ||
            (IsLessThan() && (relevantPrice < GetStopPrice()))) {
            // Activate the stop order!
            if (!pMarket->AddOffer(this, *offer, true)) // Since we're actually
                                                        // adding an offer to
                                                        // the market (not just
            { // loading from disk) the we actually want to save the market.
                // Error adding the offer to the market!    // bSaveFile=true.
                otErr << "Error adding the stop order to the market! (Even "
                         "though supposedly the right market.)\n";
            }
            else {
                // SUCCESS!
                offer_ = offer;

                stopActivated_ = true;
                hasTradeActivated_ = true;

                // The Trade (stored on Cron) has a copy of the Original Offer,
                // with the User's signature on it.
                // A copy of that original Trade object (itself with the user's
                // signature) is already stored in
                // the cron folder (by transaction number.) This happens when
                // the Trade is FIRST added to cron,
                // so it's already safe before we even get here.
                //
                // So thus I am FREE to release the signatures on the offer, and
                // sign with the server instead.
                // The server-signed offer will be stored by the OTMarket.
                offer_->ReleaseSignatures();
                offer_->SignContract(*(GetCron()->GetServerNym()));
                offer_->SaveContract();

                pMarket->SaveMarket();

                // Now when the market loads next time, it can verify this offer
                // using the server's signature,
                // instead of having to load the user. Because the server has
                // verified it and added it, and now
                // signs it, vouching for it.

                // The Trade itself (all its other variables) are now allowed to
                // change, since its signatures
                // are also released and it is now server-signed. (With a copy
                // stored of the original.)

                offer_->SetTrade(*this);

                return offer_;
            }
        }
    }

    delete offer;
    offer = nullptr;

    return nullptr;
}

// Cron only removes an item when that item REQUESTS to be removed (by setting
// the flag.)
// Once this happens, Cron has full permission to remove it. Thus, this hook is
// forceful. It
// is cron saying, YOU ARE BEING REMOVED. Period. So cleanup whatever you have
// to clean up.
//
// In this case, it removes the corresponding offer from the market.
//
void OTTrade::onRemovalFromCron()
{
    OTCron* cron = GetCron();

    OT_ASSERT(cron != nullptr);

    // If I don't already have an offer on the market, then I will have trouble
    // figuring out
    // my SCALE, which is stored on the Offer. Therefore I will instantiate an
    // offer (since I
    // store the original internally) and I will look up the scale.
    //

    int64_t scale = 1; // todo stop hardcoding.
    int64_t transactionNum = 0;

    if (offer_ != nullptr) {
        if (!marketOffer_.Exists()) {
            otErr
                << "OTTrade::onRemovalFromCron called with nullptr offer_ and "
                   "empty marketOffer_.\n";
            return;
        }

        std::unique_ptr<OTOffer> offer(new OTOffer());

        // Trying to load the offer from the trader's original signed request
        // (So I can use it to lookup the Market ID, so I can see if the offer
        // is
        // already there on the market.)
        if (!offer->LoadContractFromString(marketOffer_)) {
            otErr << "Error loading offer from string in "
                     "OTTrade::onRemovalFromCron\n";
            return;
        }

        scale = offer->GetScale();
        transactionNum = offer->GetTransactionNum();
    }
    else {
        scale = offer_->GetScale();
        transactionNum = offer_->GetTransactionNum();
    }

    OTMarket* market =
        cron->GetOrCreateMarket(GetAssetID(), GetCurrencyID(), scale);

    // Couldn't find (or create) the market.
    //
    if (market == nullptr) {
        otErr << "Unable to find market within requested parameters in "
                 "OTTrade::onRemovalFromCron.\n";
        return;
    }

    //
    // Let's see if the offer is ALREADY allocated and on this market!
    //
    OTOffer* marketOffer = market->GetOffer(transactionNum);

    // The Offer is already on the Market.
    //
    if (marketOffer != nullptr) {
        offer_ = marketOffer;

        offer_->SetTrade(*this);
    }

    market->RemoveOffer(transactionNum);
}

//    GetSenderAcctID()    -- asset account.
//    GetCurrencyAcctID()    -- currency account.

int64_t OTTrade::GetClosingNumber(const Identifier& acctId) const
{
    if (acctId == GetSenderAcctID())
        return GetAssetAcctClosingNum();
    else if (acctId == GetCurrencyAcctID())
        return GetCurrencyAcctClosingNum();
    return 0;
}

int64_t OTTrade::GetAssetAcctClosingNum() const
{
    return (GetCountClosingNumbers() > 0) ? GetClosingTransactionNoAt(0)
                                          : 0; // todo stop hardcoding.
}

int64_t OTTrade::GetCurrencyAcctClosingNum() const
{
    return (GetCountClosingNumbers() > 1) ? GetClosingTransactionNoAt(1)
                                          : 0; // todo stop hardcoding.
}

/// See if nym has rights to remove this item from Cron.
///
bool OTTrade::CanRemoveItemFromCron(OTPseudonym& nym)
{
    // I don't call the parent class' version of this function, in the case of
    // OTTrade,
    // since it would just be redundant.

    // You don't just go willy-nilly and remove a cron item from a market unless
    // you check first
    // and make sure the Nym who requested it actually has said trans# (and 2
    // related closing #s)
    // signed out to him on his last receipt...
    //
    if (!nym.CompareID(GetSenderUserID())) {
        otLog5 << "OTTrade::CanRemoveItem: nym is not the originator of "
                  "this CronItem. "
                  "(He could be a recipient though, so this is normal.)\n";
        return false;
    }

    // By this point, that means nym is DEFINITELY the originator (sender)...
    else if (GetCountClosingNumbers() < 2) {
        otOut
            << "OTTrade::CanRemoveItem Weird: Sender tried to remove a market "
               "trade; expected at "
               "least 2 closing numbers to be available--that weren't. (Found "
            << GetCountClosingNumbers() << ").\n";
        return false;
    }

    const String serverID(GetServerID());

    if (!nym.VerifyIssuedNum(serverID, GetAssetAcctClosingNum())) {
        otOut << "OTTrade::CanRemoveItemFromCron: Closing number didn't verify "
                 "for asset account.\n";
        return false;
    }

    if (!nym.VerifyIssuedNum(serverID, GetCurrencyAcctClosingNum())) {
        otOut << "OTTrade::CanRemoveItemFromCron: Closing number didn't verify "
                 "for currency account.\n";
        return false;
    }

    // By this point, we KNOW nym is the sender, and we KNOW there are the
    // proper number of transaction
    // numbers available to close. We also know that this cron item really was
    // on the cron object, since
    // that is where it was looked up from, when this function got called! So
    // I'm pretty sure, at this point,
    // to authorize removal, as long as the transaction num is still issued to
    // nym (this check here.)
    //
    return nym.VerifyIssuedNum(serverID, GetOpeningNum());

    // Normally this will be all we need to check. The originator will have the
    // transaction
    // number signed-out to him still, if he is trying to close it. BUT--in some
    // cases, someone
    // who is NOT the originator can cancel. Like in a payment plan, the sender
    // is also the depositor,
    // who would normally be the person cancelling the plan. But technically,
    // the RECIPIENT should
    // also have the ability to cancel that payment plan.  BUT: the transaction
    // number isn't signed
    // out to the RECIPIENT... In THAT case, the below VerifyIssuedNum() won't
    // work! In those cases,
    // expect that the special code will be in the subclasses override of this
    // function. (OTPaymentPlan::CanRemoveItem() etc)

    // P.S. If you override this function, MAKE SURE to call the parent
    // (OTCronItem::CanRemoveItem) first,
    // for the VerifyIssuedNum call above. Only if that fails, do you need to
    // dig deeper...
}

// This is called by OTCronItem::HookRemovalFromCron
// (After calling this method, HookRemovalFromCron then calls
// onRemovalFromCron.)
//
void OTTrade::onFinalReceipt(OTCronItem& origCronItem,
                             const int64_t& newTransactionNumber,
                             OTPseudonym& originator, OTPseudonym* remover)
{
    const char* szFunc = "OTTrade::onFinalReceipt";

    OTCron* cron = GetCron();
    OT_ASSERT(cron != nullptr);

    OTPseudonym* serverNym = cron->GetServerNym();
    OT_ASSERT(serverNym != nullptr);

    // First, we are closing the transaction number ITSELF, of this cron item,
    // as an active issued number on the originating nym. (Changing it to
    // CLOSED.)
    //
    // Second, we're verifying the CLOSING number, and using it as the closing
    // number
    // on the FINAL RECEIPT (with that receipt being "InReferenceTo"
    // GetTransactionNum())
    //
    const int64_t openingNumber = origCronItem.GetTransactionNum();

    const int64_t closingAssetNumber =
        (origCronItem.GetCountClosingNumbers() > 0)
            ? origCronItem.GetClosingTransactionNoAt(0)
            : 0;
    const int64_t closingCurrencyNumber =
        (origCronItem.GetCountClosingNumbers() > 1)
            ? origCronItem.GetClosingTransactionNoAt(1)
            : 0;

    const String serverID(GetServerID());

    // The marketReceipt ITEM's NOTE contains the UPDATED TRADE.
    // And the **UPDATED OFFER** is stored on the ATTACHMENT on the **ITEM.**
    //
    // BUT!!! This is not a marketReceipt Item, is it? ***This is a finalReceipt
    // ITEM!***
    // I'm reversing note and attachment for finalReceipt, with the
    // intention of
    // eventually reversing them for marketReceipt as well. (Making them all in
    // line with paymentReceipt.)
    //
    // WHY?  Because I want a standard convention:
    //          1. ORIGINAL (user-signed) Cron Items are always stored "in
    // reference to" on cron receipts in the Inbox (an OTTransaction).
    //          2. The UPDATED VERSION of that same cron item (a trade or
    // payment plan) is stored in the ATTACHMENT on the OTItem member.
    //          3. ADDITIONAL INFORMATION is stored in the NOTE field of the
    // OTItem member.
    //
    // Unfortunately, marketReceipt doesn't adhere to this convention, as it
    // stores the Updated Cron Item (the trade) in
    // the note instead of the attachment, and it stores the updated Offer (the
    // additional info) in the attachment instead
    // of the note.
    // Perhaps this is for the best -- it will certainly kick out any accidental
    // confusions between marketReceipt and finalReceipt!
    // todo: switch marketReceipt over to be like finalReceipt as described in
    // this paragraph.
    //
    // Once everything is consistent on the above convention -- starting here
    // and now with finalReceipt -- then we will ALWAYS
    // be able to count on a Cron Item being in the Transaction Item's
    // Attachment! We can load it using the existing factory class,
    // without regard to type, KNOWING it's a cron item every time.
    // todo: convert marketReceipt to do the same.

    // The finalReceipt Item's ATTACHMENT contains the UPDATED Cron Item.
    // (With the SERVER's signature on it!)
    //
    String updatedCronItem(*this);
    String* attachment = &updatedCronItem; // the Updated TRADE.
    String updatedOffer;
    String* note = nullptr; // the updated Offer (if available.)

    if (offer_) {
        offer_->SaveContractRaw(updatedOffer);
        note = &updatedOffer;
    }

    const String strOrigCronItem(origCronItem);

    OTPseudonym theActualNym; // unused unless it's really not already loaded.
                              // (use actualNym.)

    // The OPENING transaction number must still be signed-out.
    // It is this act of placing the final receipt, which then finally closes
    // the opening number.
    // The closing number, by contrast, is not closed out until the final
    // Receipt is ACCEPTED
    // (which happens in a "process inbox" transaction.)
    //
    if ((openingNumber > 0) &&
        originator.VerifyIssuedNum(serverID, openingNumber)) {
        // The Nym (server side) stores a list of all opening and closing cron
        // #s.
        // So when the number is released from the Nym, we also take it off that
        // list.
        //
        std::set<int64_t>& idSet = originator.GetSetOpenCronItems();
        idSet.erase(openingNumber);

        originator.RemoveIssuedNum(*serverNym, serverID, openingNumber,
                                   false);        // bSave=false
        originator.SaveSignedNymfile(*serverNym); // forcing a save here,
                                                  // since multiple things
                                                  // have changed.

        const Identifier& actualNymId = GetSenderUserID();

        OTPseudonym* actualNym = nullptr; // use this. DON'T use theActualNym.
        if ((serverNym != nullptr) && serverNym->CompareID(actualNymId))
            actualNym = serverNym;
        else if (originator.CompareID(actualNymId))
            actualNym = &originator;
        else if ((remover != nullptr) && remover->CompareID(actualNymId))
            actualNym = remover;

        else // We couldn't find the Nym among those already loaded--so we have
             // to load
        {    // it ourselves (so we can update its NymboxHash value.)
            theActualNym.SetIdentifier(actualNymId);

            if (!theActualNym.LoadPublicKey()) // Note: this step may be
                                               // unnecessary since we
                                               // are only updating his
                                               // Nymfile, not his key.
            {
                String strNymID(actualNymId);
                otErr << szFunc
                      << ": Failure loading public key for Nym : " << strNymID
                      << ". "
                         "(To update his NymboxHash.) \n";
            }
            else if (theActualNym.VerifyPseudonym() && // this line may be
                                                         // unnecessary.
                       theActualNym.LoadSignedNymfile(
                           *serverNym)) // ServerNym here is not theActualNym's
                                        // identity, but merely the signer on
                                        // this file.
            {
                otLog3
                    << szFunc
                    << ": Loading actual Nym, since he wasn't already loaded. "
                       "(To update his NymboxHash.)\n";
                actualNym = &theActualNym; //  <=====
            }
            else {
                String strNymID(actualNymId);
                otErr
                    << szFunc
                    << ": Failure loading or verifying Actual Nym public key: "
                    << strNymID << ". "
                                   "(To update his NymboxHash.)\n";
            }
        }

        if (!DropFinalReceiptToNymbox(GetSenderUserID(), newTransactionNumber,
                                      strOrigCronItem, note, attachment,
                                      actualNym)) {
            otErr << szFunc << ": Failure dropping receipt into nymbox.\n";
        }
    }
    else {
        otErr << szFunc << ": Problem verifying Opening Number when calling "
                           "VerifyIssuedNum(openingNumber)\n";
    }

    // ASSET ACCT
    //
    if ((closingAssetNumber > 0) &&
        originator.VerifyIssuedNum(serverID, closingAssetNumber)) {
        DropFinalReceiptToInbox(
            GetSenderUserID(), GetSenderAcctID(), newTransactionNumber,
            closingAssetNumber, // The closing transaction number to put on the
                                // receipt.
            strOrigCronItem, note, attachment);
    }
    else {
        otErr << szFunc
              << ": Failed verifying "
                 "closingAssetNumber=origCronItem."
                 "GetClosingTransactionNoAt(0)>0 &&  "
                 "originator.VerifyTransactionNum(closingAssetNumber)\n";
    }

    // CURRENCY ACCT
    //
    if ((closingCurrencyNumber > 0) &&
        originator.VerifyIssuedNum(serverID, closingCurrencyNumber)) {
        DropFinalReceiptToInbox(
            GetSenderUserID(), GetCurrencyAcctID(), newTransactionNumber,
            closingCurrencyNumber, // closing transaction number for the
                                   // receipt.
            strOrigCronItem, note, attachment);
    }
    else {
        otErr << szFunc
              << ": Failed verifying "
                 "closingCurrencyNumber=origCronItem."
                 "GetClosingTransactionNoAt(1)>0 "
                 "&&  "
                 "originator.VerifyTransactionNum(closingCurrencyNumber)\n";
    }

    // the RemoveIssued call means the original transaction# (to find this cron
    // item on cron) is now CLOSED.
    // But the Transaction itself is still OPEN. How? Because the CLOSING number
    // is still signed out.
    // The closing number is also USED, since the NotarizePaymentPlan or
    // NotarizeMarketOffer call, but it
    // remains ISSUED, until the final receipt itself is accepted during a
    // process inbox.
    //
    //    if (bDroppedReceiptAssetAcct || bDroppedReceiptCurrencyAcct)  // ASSET
    // ACCOUNT and CURRENCY ACCOUNT
    //    {
    // This part below doesn't happen until you ACCEPT the finalReceipt (when
    // processing your inbox.)
    //
    //      if (bDroppedReceiptAssetAcct)
    //          originator.RemoveIssuedNum(serverID, closingAssetNumber,
    // true); //bSave=false
    //      else if (bDroppedReceiptCurrencyAcct)
    //          originator.RemoveIssuedNum(serverID,
    // closingCurrencyNumber, true); //bSave=false
    //    }
    //    else
    //    {
    //        otErr << "OTTrade::onFinalReceipt: Failure dropping receipt into
    // asset or currency inbox.\n";
    //    }

    // QUESTION: Won't there be Cron Items that have no asset account at all?
    // In which case, there'd be no need to drop a final receipt, but I don't
    // think
    // that's the case, since you have to use a transaction number to get onto
    // cron
    // in the first place.
}

// OTCron calls this regularly, which is my chance to expire, etc.
// Return True if I should stay on the Cron list for more processing.
// Return False if I should be removed and deleted.
bool OTTrade::ProcessCron()
{

    // Right now Cron is called 10 times per second.
    // I'm going to slow down all trades so they are once every
    // GetProcessInterval()
    if (GetLastProcessDate() > OT_TIME_ZERO) {
        // (Default ProcessInterval is 1 second, but Trades will use 10 seconds,
        // and Payment Plans will use an hour or day.)
        if (OTTimeGetTimeInterval(OTTimeGetCurrentTime(),
                                  GetLastProcessDate()) <= GetProcessInterval())
            return true;
    }

    // Keep a record of the last time this was processed.
    // (NOT saved to storage, only used while the software is running.)
    // (Thus no need to release signatures, sign contract, save contract, etc.)
    SetLastProcessDate(OTTimeGetCurrentTime());

    // PAST END DATE?
    // First call the parent's version (which this overrides) so it has
    // a chance to check its stuff. Currently it checks IsExpired().
    if (!ot_super::ProcessCron())
        return false; // It's expired or flagged for removal--remove it from
                      // Cron.

    // You might ask, why not check here if this trade is flagged for removal?
    // Supposedly the answer is, because it's only below that I have the market
    // pointer,
    // and am able to remove the corresponding trade from the market.
    // Therefore I am adding a hook for "onRemoval" so that Objects such as
    // OTTrade ALWAYS
    // have the opportunity to perform such cleanup, without having to juggle
    // such logic.

    // REACHED START DATE?
    // Okay, so it's not expired. But might not have reached START DATE yet...
    if (!VerifyCurrentDate())
        return true; // The Trade is not yet valid, so we return. BUT, we return
                     //  true, so it will stay on Cron until it BECOMES valid.

    // TRADE-specific stuff below.

    bool bStayOnMarket =
        true; // by default stay on the market (until some rule expires me.)

    Identifier OFFER_MARKET_ID;
    OTMarket* market = nullptr;

    // If the Offer is already active on a market, then I already have a pointer
    // to
    // it. This function returns that pointer. If nullptr, it tries to find the
    // offer on
    // the market and then sets the pointer and returns. If it can't find it, IT
    // TRIES
    // TO ADD IT TO THE MARKET and sets the pointer and returns it.
    OTOffer* offer = GetOffer(
        &OFFER_MARKET_ID, &market); // Both of these parameters are optional.

    // In this case, the offer is NOT on the market.
    // Perhaps it wasn't ready to activate yet.
    if (offer == nullptr) {
        // The offer SHOULD HAVE been on the market, since we're within the
        // valid range,
        // and GetOffer adds it when it's not already there.

        //        otErr << "OTTrade::ProcessCron: Offer SHOULD have been on
        // Market. I might ASSERT this.\n"; // comment this out

        // Actually! If it's a Stop Order, then it WOULD be within the valid
        // range, yet would
        // not yet have activated. So I don't want to log some big error every
        // time a stop order
        // checks its prices.
    }
    else if (market == nullptr) {
        // todo. (This will already leave a log above in GetOffer somewhere.)
        //        otErr << "OTTrade::ProcessCron: Market was nullptr.\n"; //
        // comment this out
    }
    else // If a valid pointer was returned, that means the offer is on the
           // market.
    {
        // Make sure it hasn't already been flagged by someone else...
        if (IsFlaggedForRemoval()) // This is checked above in
                                   // OTCronItem::ProcessCron().
            bStayOnMarket = false; // I'm leaving the check here in case the
                                   // flag was set since then.

        else // Process it!  <===================
        {
            otInfo << "Processing trade: " << GetTransactionNum() << ".\n";

            bStayOnMarket = market->ProcessTrade(*this, *offer);
            // No need to save the Trade or Offer, since they will
            // be saved inside this call if they are changed.
        }
    }

    // Return True if I should stay on the Cron list for more processing.
    // Return False if I should be removed and deleted.
    return bStayOnMarket; // defaults true, so if false, that means someone is
                          // removing it for a reason.
}

/*
X OTIdentifier    currencyTypeID_;    // GOLD (Asset) is trading for DOLLARS
(Currency).
X OTIdentifier    currencyAcctID_;    // My Dollar account, used for paying
for my Gold (say) trades.

X int64_t            stopPrice_;        // The price limit that activates the
STOP order.
X char            stopSign_;        // Value is 0, or '<', or '>'.

X time64_t        m_CREATION_DATE;    // The date, in seconds, when the trade
was authorized.
X int32_t            tradesAlreadyDone_;    // How many trades have already
processed through this order? We keep track.
*/

// This is called by the client side. First you call MakeOffer() to set up the
// Offer,
// then you call IssueTrade() and pass the Offer into it here.
bool OTTrade::IssueTrade(OTOffer& offer, char stopSign, int64_t stopPrice)
{
    // Make sure the Stop Sign is within parameters (0, '<', or '>')
    if ((stopSign == 0) || (stopSign == '<') || (stopSign == '>'))
        stopSign_ = stopSign;
    else {
        otErr << "Bad data in Stop Sign while issuing trade: " << stopSign
              << "\n";
        return false;
    }

    // Make sure, if this IS a Stop order, that the price is within parameters
    // and set.
    if ((stopSign_ == '<') || (stopSign_ == '>')) {
        if (0 >= stopPrice) {
            otErr << "Expected Stop Price for trade.\n";
            return false;
        }

        stopPrice_ = stopPrice;
    }

    tradesAlreadyDone_ = 0;

    SetCreationDate(
        OTTimeGetCurrentTime()); // This time is set to TODAY NOW  (OTCronItem)

    // Validate the Server ID, Asset Type ID, Currency Type ID, and Date Range.
    if ((GetServerID() != offer.GetServerID()) ||
        (GetCurrencyID() != offer.GetCurrencyID()) ||
        (GetAssetID() != offer.GetAssetID()) ||
        (offer.GetValidFrom() < OT_TIME_ZERO) ||
        (offer.GetValidTo() < offer.GetValidFrom())) {
        return false;
    }

    //    currencyTypeID_ // This is already set in the constructors of this
    // and the offer. (And compared.)
    //    currencyAcctID_ // This is already set in the constructor of this.

    // Set the (now validated) date range as per the Offer.
    SetValidFrom(offer.GetValidFrom());
    SetValidTo(offer.GetValidTo());

    // Get the transaction number from the Offer.
    SetTransactionNum(offer.GetTransactionNum());

    // Save a copy of the offer, in XML form, here on this Trade.
    String strOffer(offer);
    marketOffer_.Set(strOffer);

    return true;
}

OTTrade::OTTrade()
    : ot_super()
    , offer_(nullptr)
    , hasTradeActivated_(false)
    , stopPrice_(0)
    , stopSign_(0)
    , stopActivated_(false)
    , tradesAlreadyDone_(0)
{
    //    offer_            = nullptr;    // NOT responsible to clean this up.
    // Just keeping the pointer for convenience.
    // You might ask, "but what if it goes bad?" Actually only THIS object
    // should ever decide that.
    // Only the Trade object decides when to add or remove an offer from any
    // market.

    InitTrade();
}

OTTrade::OTTrade(const Identifier& serverId, const Identifier& assetId,
                 const Identifier& assetAcctId, const Identifier& userId,
                 const Identifier& currencyId, const Identifier& currencyAcctId)
    : ot_super(serverId, assetId, assetAcctId, userId)
    , offer_(nullptr)
    , hasTradeActivated_(false)
    , stopPrice_(0)
    , stopSign_(0)
    , stopActivated_(false)
    , tradesAlreadyDone_(0)
{
    //    offer_            = nullptr;    // NOT responsible to clean this up.
    // Just keeping the pointer for convenience.
    // You might ask, "but what if it goes bad?" Actually only THIS object
    // should ever decide that.
    // Only the Trade object decides when to add or remove an offer from any
    // market.

    InitTrade();

    SetCurrencyID(currencyId);
    SetCurrencyAcctID(currencyAcctId);
}

OTTrade::~OTTrade()
{
    Release_Trade();
}

// the framework will call this at the right time.
void OTTrade::Release_Trade()
{
    // If there were any dynamically allocated objects, clean them up here.
    currencyTypeID_.clear();
    currencyAcctID_.clear();

    marketOffer_.Release();
}

// the framework will call this at the right time.
void OTTrade::Release()
{
    Release_Trade();

    ot_super::Release();

    // Then I call this to re-initialize everything
    // (Only cause it's convenient...)
    InitTrade();
}

// This CAN have values that are reset
void OTTrade::InitTrade()
{
    // initialization here. Sometimes also called during cleanup to zero values.
    m_strContractType = "TRADE";

    SetProcessInterval(TradeProcessIntervalSeconds); // Trades default to
                                                     // processing every 10
                                                     // seconds.
    // (vs 1 second for Cron items and 1 hour for payment plans)

    tradesAlreadyDone_ = 0;

    stopSign_ = 0;          // IS THIS a STOP order? Value is 0, or '<', or '>'.
    stopPrice_ = 0;         // The price limit that activates the STOP order.
    stopActivated_ = false; // Once the Stop Order activates, it puts the
                            // order on the market.
    // I'll put a "HasOrderOnMarket()" bool method that answers this for u.
    hasTradeActivated_ = false; // I want to keep track of general activations
                                // as well, not just stop orders.
}

} // namespace opentxs
