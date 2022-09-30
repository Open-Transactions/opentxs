// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "internal/otx/common/trade/OTTrade.hpp"  // IWYU pragma: associated

#include <chrono>
#include <compare>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/cron/OTCron.hpp"
#include "internal/otx/common/cron/OTCronItem.hpp"
#include "internal/otx/common/trade/OTMarket.hpp"
#include "internal/otx/common/trade/OTOffer.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/consensus/Client.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
enum { TradeProcessIntervalSeconds = 10 };

OTTrade::OTTrade(const api::Session& api)
    : ot_super(api)
    , currency_type_id_()
    , currency_acct_id_()
    , offer_(nullptr)
    , has_trade_activated_(false)
    , stop_price_(0)
    , stop_sign_(0)
    , stop_activated_(false)
    , trades_already_done_(0)
    , market_offer_(String::Factory())
{
    InitTrade();
}

OTTrade::OTTrade(
    const api::Session& api,
    const identifier::Notary& notaryID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const identifier::Generic& assetAcctId,
    const identifier::Nym& nymID,
    const identifier::UnitDefinition& currencyId,
    const identifier::Generic& currencyAcctId)
    : ot_super(api, notaryID, instrumentDefinitionID, assetAcctId, nymID)
    , currency_type_id_(currencyId)
    , currency_acct_id_(currencyAcctId)
    , offer_(nullptr)
    , has_trade_activated_(false)
    , stop_price_(0)
    , stop_sign_(0)
    , stop_activated_(false)
    , trades_already_done_(0)
    , market_offer_(String::Factory())
{
    InitTrade();
}

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
auto OTTrade::VerifyNymAsAgent(const identity::Nym& nym, const identity::Nym&)
    const -> bool
{
    return VerifySignature(nym);
}

// This is an override. See note above.
//
auto OTTrade::VerifyNymAsAgentForAccount(
    const identity::Nym& nym,
    const Account& account) const -> bool
{
    return account.VerifyOwner(nym);
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto OTTrade::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    std::int32_t returnVal = 0;

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    //
    if (0 != (returnVal = ot_super::ProcessXMLNode(xml))) { return returnVal; }

    if (!strcmp("trade", xml->getNodeName())) {
        version_ = String::Factory(xml->getAttributeValue("version"));
        trades_already_done_ =
            atoi(xml->getAttributeValue("completedNoTrades"));

        SetTransactionNum(
            String::StringToLong(xml->getAttributeValue("transactionNum")));

        const UnallocatedCString creationStr =
            xml->getAttributeValue("creationDate");
        const UnallocatedCString validFromStr =
            xml->getAttributeValue("validFrom");
        const UnallocatedCString validToStr = xml->getAttributeValue("validTo");

        const auto creation = parseTimestamp(creationStr);
        const auto validFrom = parseTimestamp(validFromStr);
        const auto validTo = parseTimestamp(validToStr);

        SetCreationDate(creation);
        SetValidFrom(validFrom);
        SetValidTo(validTo);

        auto activated =
            String::Factory(xml->getAttributeValue("hasActivated"));

        if (activated->Compare("true")) {
            has_trade_activated_ = true;
        } else {
            has_trade_activated_ = false;
        }

        const auto notaryID =
                       String::Factory(xml->getAttributeValue("notaryID")),
                   nymID = String::Factory(xml->getAttributeValue("nymID")),
                   instrumentDefinitionID = String::Factory(
                       xml->getAttributeValue("instrumentDefinitionID")),
                   assetAcctID =
                       String::Factory(xml->getAttributeValue("assetAcctID")),
                   currencyTypeID = String::Factory(
                       xml->getAttributeValue("currencyTypeID")),
                   currencyAcctID = String::Factory(
                       xml->getAttributeValue("currencyAcctID"));

        const auto NOTARY_ID =
            api_.Factory().NotaryIDFromBase58(notaryID->Bytes());
        const auto INSTRUMENT_DEFINITION_ID = api_.Factory().UnitIDFromBase58(
                       instrumentDefinitionID->Bytes()),
                   CURRENCY_TYPE_ID =
                       api_.Factory().UnitIDFromBase58(currencyTypeID->Bytes());
        const auto ASSET_ACCT_ID = api_.Factory().IdentifierFromBase58(
                       assetAcctID->Bytes()),
                   CURRENCY_ACCT_ID = api_.Factory().IdentifierFromBase58(
                       currencyAcctID->Bytes());
        const auto NYM_ID = api_.Factory().NymIDFromBase58(nymID->Bytes());

        SetNotaryID(NOTARY_ID);
        SetSenderNymID(NYM_ID);
        SetInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
        SetSenderAcctID(ASSET_ACCT_ID);
        SetCurrencyID(CURRENCY_TYPE_ID);
        SetCurrencyAcctID(CURRENCY_ACCT_ID);

        LogDebug()(OT_PRETTY_CLASS())("Trade. Transaction Number: ")(
            transaction_num_)("Completed # of Trades: ")(trades_already_done_)
            .Flush();

        LogDetail()(OT_PRETTY_CLASS())("Creation Date: ")(
            creation)(". Valid From: ")(validFrom)(". Valid To: ")(
            validTo)(". assetTypeID: ")(instrumentDefinitionID.get())(
            ". assetAccountID: ")(assetAcctID.get())(". NotaryID: ")(
            notaryID.get())(". NymID: ")(nymID.get())(". currencyTypeID: ")(
            currencyTypeID.get())(". currencyAccountID: ")(
            currencyAcctID.get())(".")
            .Flush();

        returnVal = 1;
    }

    if (!strcmp("stopOrder", xml->getNodeName())) {
        auto sign = String::Factory(xml->getAttributeValue("sign"));

        if (sign->Compare("0")) {
            stop_sign_ = 0;  // Zero means it isn't a stop order. So why is the
                             // tag in the file?
            LogError()(OT_PRETTY_CLASS())(
                "Strange: Stop order tag found in trade, but sign "
                "character set to 0. "
                "(Zero means: NOT a stop order).")
                .Flush();
            return (-1);
        } else if (sign->Compare("<")) {
            stop_sign_ = '<';
        } else if (sign->Compare(">")) {
            stop_sign_ = '>';
        } else {
            stop_sign_ = 0;
            LogError()(OT_PRETTY_CLASS())(
                "Unexpected or nonexistent value in stop order sign: ")(
                sign.get())(".")
                .Flush();
            return (-1);
        }

        // Now we know the sign is properly formed, let's grab the price value.

        stop_price_ = String::StringToLong(xml->getAttributeValue("price"));

        auto activated =
            String::Factory(xml->getAttributeValue("hasActivated"));

        if (activated->Compare("true")) {
            stop_activated_ = true;
        } else {
            stop_activated_ = false;
        }

        const auto unittype = api_.Wallet().CurrencyTypeBasedOnUnitType(
            GetInstrumentDefinitionID());
        LogDebug()(OT_PRETTY_CLASS())("Stop order --")(
            stop_activated_ ? "Already activated" : "Will activate")(
            " when price ")(stop_activated_ ? "was" : "reaches")(
            ('<' == stop_sign_) ? "LESS THAN"
                                : "GREATER THAN")(stop_price_, unittype)
            .Flush();

        returnVal = 1;
    } else if (!strcmp("offer", xml->getNodeName())) {
        if (!LoadEncodedTextField(xml, market_offer_)) {

            LogError()(OT_PRETTY_CLASS())("Error: Offer field without "
                                          "value.")
                .Flush();
            return (-1);  // error condition
        }

        returnVal = 1;
    }

    return returnVal;
}

void OTTrade::UpdateContents(const PasswordPrompt& reason)
{
    // I release this because I'm about to repopulate it.
    xml_unsigned_->Release();

    const auto NOTARY_ID = String::Factory(GetNotaryID()),
               NYM_ID = String::Factory(GetSenderNymID()),
               INSTRUMENT_DEFINITION_ID =
                   String::Factory(GetInstrumentDefinitionID()),
               ASSET_ACCT_ID = String::Factory(GetSenderAcctID()),
               CURRENCY_TYPE_ID = String::Factory(GetCurrencyID()),
               CURRENCY_ACCT_ID = String::Factory(GetCurrencyAcctID());

    Tag tag("trade");

    tag.add_attribute("version", version_->Get());
    tag.add_attribute("hasActivated", formatBool(has_trade_activated_));
    tag.add_attribute("notaryID", NOTARY_ID->Get());
    tag.add_attribute(
        "instrumentDefinitionID", INSTRUMENT_DEFINITION_ID->Get());
    tag.add_attribute("assetAcctID", ASSET_ACCT_ID->Get());
    tag.add_attribute("currencyTypeID", CURRENCY_TYPE_ID->Get());
    tag.add_attribute("currencyAcctID", CURRENCY_ACCT_ID->Get());
    tag.add_attribute("nymID", NYM_ID->Get());
    tag.add_attribute(
        "completedNoTrades", std::to_string(trades_already_done_));
    tag.add_attribute("transactionNum", std::to_string(transaction_num_));
    tag.add_attribute("creationDate", formatTimestamp(GetCreationDate()));
    tag.add_attribute("validFrom", formatTimestamp(GetValidFrom()));
    tag.add_attribute("validTo", formatTimestamp(GetValidTo()));

    // There are "closing" transaction numbers, used to CLOSE a transaction.
    // Often where Cron items are involved such as this payment plan, or in
    // baskets,
    // where many asset accounts are involved and require receipts to be closed
    // out.

    for (std::int32_t i = 0; i < GetCountClosingNumbers(); i++) {
        std::int64_t closingNumber = GetClosingTransactionNoAt(i);
        OT_ASSERT(closingNumber > 0);
        TagPtr tagClosing(new Tag("closingTransactionNumber"));
        tagClosing->add_attribute("value", std::to_string(closingNumber));
        tag.add_tag(tagClosing);
    }

    if (('<' == stop_sign_) || ('>' == stop_sign_)) {
        TagPtr tagStopOrder(new Tag("stopOrder"));
        tagStopOrder->add_attribute(
            "hasActivated", formatBool(stop_activated_));
        tagStopOrder->add_attribute("sign", std::to_string(stop_sign_));
        tagStopOrder->add_attribute("price", [&] {
            auto buf = UnallocatedCString{};
            stop_price_.Serialize(writer(buf));
            return buf;
        }());
        tag.add_tag(tagStopOrder);
    }

    if (market_offer_->Exists()) {
        auto ascOffer = Armored::Factory(market_offer_);
        tag.add_tag("offer", ascOffer->Get());
    }

    UnallocatedCString str_result;
    tag.output(str_result);

    xml_unsigned_->Concatenate(String::Factory(str_result));
}

// The trade stores a copy of the Offer in string form.
// This function verifies that offer against the trade,
// and also verifies the signature on the offer.
//
// The Nym's ID is compared to offer's SenderNymID, and then the Signature
// is checked
// on the offer.  It also compares the server ID, asset and currency IDs,
// transaction #, etc
// between this trade and the offer, in order to fully verify the offer's
// authenticity.
//
auto OTTrade::VerifyOffer(OTOffer& offer) const -> bool
{
    // At this point, I have a working, loaded, model of the Offer.
    // Let's verify the thing.

    if (GetTransactionNum() != offer.GetTransactionNum()) {
        LogError()(OT_PRETTY_CLASS())(
            "While verifying offer, failed matching transaction number.")
            .Flush();
        return false;
    } else if (GetNotaryID() != offer.GetNotaryID()) {
        LogError()(OT_PRETTY_CLASS())(
            "While verifying offer, failed matching Notary ID.")
            .Flush();
        return false;
    } else if (
        GetInstrumentDefinitionID() != offer.GetInstrumentDefinitionID()) {
        LogError()(OT_PRETTY_CLASS())(
            "While verifying offer, failed matching instrument definition "
            "ID.")
            .Flush();
        return false;
    } else if (GetCurrencyID() != offer.GetCurrencyID()) {
        LogError()(OT_PRETTY_CLASS())(
            "While verifying offer, failed matching currency type ID.")
            .Flush();
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

auto OTTrade::GetOffer(const PasswordPrompt& reason, OTMarket** market)
    -> OTOffer*
{
    auto id = identifier::Generic{};

    return GetOffer(id, reason, market);
}

auto OTTrade::GetOffer(
    identifier::Generic& offerMarketId,
    const PasswordPrompt& reason,
    OTMarket** market) -> OTOffer*
{
    OT_ASSERT(GetCron() != nullptr);

    // See if the offer has already been instantiated onto a market...
    if (offer_ != nullptr) {
        offer_->SetTrade(*this);  // Probably don't need this line. I'll remove
                                  // it someday while optimizing.
        // In fact since it should already be set, having this here would
        // basically
        // hide it from me if the memory was ever walked on from a bug
        // somewhere.

        // It loaded. Let's get the Market ID off of it so we can locate the
        // market.
        const auto OFFER_MARKET_ID =
            api_.Factory().Internal().Identifier(*offer_);

        if (market != nullptr) {
            auto pMarket = GetCron()->GetMarket(OFFER_MARKET_ID);

            // Sometimes the caller function would like a copy of this market
            // pointer, when available.
            // So I pass it back to him here, if he wants. That way he doesn't
            // have to do this work again
            // to look it up.
            if (false != bool(pMarket)) {
                *market = pMarket.get();  // <=================
            } else {
                LogError()(OT_PRETTY_CLASS())(
                    "Offer_ already exists, yet unable to find the "
                    "market it's supposed to be on.")
                    .Flush();
            }
        }

        offerMarketId.Assign(OFFER_MARKET_ID);

        return offer_;
    }  // if offer_ ALREADY EXISTS.

    // else (BELOW) offer_ IS nullptr, and thus it didn't exist yet...

    if (!market_offer_->Exists()) {
        LogError()(OT_PRETTY_CLASS())("Error: Called with empty market_offer_.")
            .Flush();
        return nullptr;
    }

    auto offer{api_.Factory().InternalSession().Offer()};
    OT_ASSERT(false != bool(offer));

    // Trying to load the offer from the trader's original signed request
    // (So I can use it to lookup the Market ID, so I can see the offer is
    // already there on the market.)
    if (!offer->LoadContractFromString(market_offer_)) {
        LogError()(OT_PRETTY_CLASS())("Error loading offer from string.")
            .Flush();
        return nullptr;
    }

    // No need to do any additional security verification here on the Offer,
    // since the Offer is already heavily verified in
    // Server::NotarizeMarketOffer().
    // So as long as you feel safe about the Trade, then you can feel safe about
    // the Offer already, with no further checks.
    // *Also remember we saved a copy of the original in the cron folder.

    // It loaded. Let's get the Market ID off of it so we can locate the market.
    const auto OFFER_MARKET_ID = api_.Factory().Internal().Identifier(*offer);
    offerMarketId.Assign(OFFER_MARKET_ID);

    // Previously if a user tried to use a market that didn't exist, I'd just
    // return failure.
    // But now we will create any market that doesn't already exist.
    // (Remember, the server operator could just erase the market folder--it
    // wouldn't
    // affect anyone's balances!) Update: he probably couldn't just wipe the
    // markets folder,
    // actually, without making it impossible for certain Nyms to get rid of
    // certain issued #s.
    auto pMarket = GetCron()->GetOrCreateMarket(
        GetInstrumentDefinitionID(), GetCurrencyID(), offer->GetScale());

    // Couldn't find (or create) the market.
    if (false == bool(pMarket)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Unable to find or create market within requested parameters.")
            .Flush();
        return nullptr;
    }

    // If the caller passed in the address of a market pointer (optional)
    if (market != nullptr) {
        // Sometimes the caller function would like a copy of this market
        // pointer, when available.
        // So I pass it back to him here, if he wants. That way he doesn't have
        // to do this work again
        // to look it up.
        *market = pMarket.get();
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
        if (has_trade_activated_) {
            // Error -- how has the trade already activated, yet not on the
            // market and null in my pointer?
            LogError()(OT_PRETTY_CLASS())(
                "How has the trade already activated, yet not on the "
                "market and null in my pointer?")
                .Flush();
        } else if (!pMarket->AddOffer(
                       this, *offer, reason, true))  // Since
                                                     // we're actually
                                                     // adding an offer
                                                     // to the market
        {  // (not just loading from disk) then we actually want to save the
            // market.
            // bSaveFile=true.
            // Error adding the offer to the market!
            LogError()(OT_PRETTY_CLASS())(
                "Error adding the offer to the market! (Even though "
                "supposedly the right market).")
                .Flush();
        } else {
            // SUCCESS!
            offer_ = offer.release();

            has_trade_activated_ = true;

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
            offer_->SignContract(*(GetCron()->GetServerNym()), reason);
            offer_->SaveContract();

            pMarket->SaveMarket(reason);

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
    else if (IsStopOrder() && !stop_activated_) {
        Amount relevantPrice = 0;

        // If the stop order is trying to sell something, then it cares about
        // the highest bidder.
        if (offer->IsAsk()) {
            relevantPrice = pMarket->GetHighestBidPrice();
        } else {  // But if the stop order is trying to buy something, then it
                  // cares
                  // about the lowest ask price.
            relevantPrice = pMarket->GetLowestAskPrice();
        }

        // It's a stop order that hasn't activated yet. SHOULD IT ACTIVATE NOW?
        if ((IsGreaterThan() && (relevantPrice > GetStopPrice())) ||
            (IsLessThan() && (relevantPrice < GetStopPrice()))) {
            // Activate the stop order!
            if (!pMarket->AddOffer(
                    this, *offer, reason, true))  // Since we're
                                                  // adding an offer to
                                                  // the market (not just
            {  // loading from disk) the we actually want to save the market.
                // Error adding the offer to the market!    // bSaveFile=true.
                LogError()(OT_PRETTY_CLASS())(
                    "Error adding the stop order to the market! (Even "
                    "though supposedly the right market).")
                    .Flush();
            } else {
                // SUCCESS!
                offer_ = offer.release();

                stop_activated_ = true;
                has_trade_activated_ = true;

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
                offer_->SignContract(*(GetCron()->GetServerNym()), reason);
                offer_->SaveContract();

                pMarket->SaveMarket(reason);

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
void OTTrade::onRemovalFromCron(const PasswordPrompt& reason)
{
    OTCron* cron = GetCron();
    OT_ASSERT(cron != nullptr);

    // If I don't already have an offer on the market, then I will have trouble
    // figuring out
    // my SCALE, which is stored on the Offer. Therefore I will instantiate an
    // offer (since I
    // store the original internally) and I will look up the scale.
    //

    Amount scale = 1;  // todo stop hardcoding.
    std::int64_t transactionNum = 0;

    if (offer_ == nullptr) {
        if (!market_offer_->Exists()) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Called with nullptr offer_ and "
                "empty market_offer_.")
                .Flush();
            return;
        }

        auto offer{api_.Factory().InternalSession().Offer()};

        OT_ASSERT(false != bool(offer));

        // Trying to load the offer from the trader's original signed request
        // (So I can use it to lookup the Market ID, so I can see if the offer
        // is
        // already there on the market.)
        if (!offer->LoadContractFromString(market_offer_)) {
            LogError()(OT_PRETTY_CLASS())("Error loading offer from string.")
                .Flush();
            return;
        }

        scale = offer->GetScale();
        transactionNum = offer->GetTransactionNum();
    } else {
        scale = offer_->GetScale();
        transactionNum = offer_->GetTransactionNum();
    }

    auto market = cron->GetOrCreateMarket(
        GetInstrumentDefinitionID(), GetCurrencyID(), scale);

    // Couldn't find (or create) the market.
    //
    if (false == bool(market)) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to find market within requested parameters.")
            .Flush();
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

    market->RemoveOffer(transactionNum, reason);
}

//    GetSenderAcctID()    -- asset account.
//    GetCurrencyAcctID()    -- currency account.

auto OTTrade::GetClosingNumber(const identifier::Generic& acctId) const
    -> std::int64_t
{
    if (acctId == GetSenderAcctID()) {
        return GetAssetAcctClosingNum();
    } else if (acctId == GetCurrencyAcctID()) {
        return GetCurrencyAcctClosingNum();
    }
    return 0;
}

auto OTTrade::GetAssetAcctClosingNum() const -> std::int64_t
{
    return (GetCountClosingNumbers() > 0) ? GetClosingTransactionNoAt(0)
                                          : 0;  // todo stop hardcoding.
}

auto OTTrade::GetCurrencyAcctClosingNum() const -> std::int64_t
{
    return (GetCountClosingNumbers() > 1) ? GetClosingTransactionNoAt(1)
                                          : 0;  // todo stop hardcoding.
}

/// See if nym has rights to remove this item from Cron.
auto OTTrade::CanRemoveItemFromCron(const otx::context::Client& context) -> bool
{
    // I don't call the parent class' version of this function, in the case of
    // OTTrade, since it would just be redundant.

    // You don't just go willy-nilly and remove a cron item from a market unless
    // you check first and make sure the Nym who requested it actually has said
    // trans# (and 2 related closing #s) signed out to him on his last receipt.
    if (!context.Nym()->CompareID(GetSenderNymID())) {
        LogInsane()(OT_PRETTY_CLASS())(
            "nym is not the originator of this CronItem. (He could be a "
            "recipient though, so this is normal).")
            .Flush();

        return false;
    }
    // By this point, that means nym is DEFINITELY the originator (sender)...
    else if (GetCountClosingNumbers() < 2) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Weird: Sender tried to remove a market "
            "trade; expected at "
            "least 2 closing numbers to be available--that weren't. (Found ")(
            GetCountClosingNumbers())(").")
            .Flush();

        return false;
    }

    const auto notaryID = String::Factory(GetNotaryID());

    if (!context.VerifyIssuedNumber(GetAssetAcctClosingNum())) {
        LogConsole()(OT_PRETTY_CLASS())("Closing number didn't verify "
                                        "for asset account.")
            .Flush();

        return false;
    }

    if (!context.VerifyIssuedNumber(GetCurrencyAcctClosingNum())) {
        LogConsole()(OT_PRETTY_CLASS())("Closing number didn't verify "
                                        "for currency account.")
            .Flush();

        return false;
    }

    // By this point, we KNOW nym is the sender, and we KNOW there are the
    // proper number of transaction numbers available to close. We also know
    // that this cron item really was on the cron object, since that is where it
    // was looked up from, when this function got called! So I'm pretty sure, at
    // this point, to authorize removal, as long as the transaction num is still
    // issued to nym (this check here.)

    return context.VerifyIssuedNumber(GetOpeningNum());

    // Normally this will be all we need to check. The originator will have the
    // transaction number signed-out to him still, if he is trying to close it.
    // BUT--in some cases, someone who is NOT the originator can cancel. Like in
    // a payment plan, the sender is also the depositor, who would normally be
    // the person cancelling the plan. But technically, the RECIPIENT should
    // also have the ability to cancel that payment plan.  BUT: the transaction
    // number isn't signed out to the RECIPIENT... In THAT case, the below
    // VerifyIssuedNum() won't work! In those cases, expect that the special
    // code will be in the subclasses override of this function.
    // (OTPaymentPlan::CanRemoveItem() etc)

    // P.S. If you override this function, MAKE SURE to call the parent
    // (OTCronItem::CanRemoveItem) first, for the VerifyIssuedNum call above.
    // Only if that fails, do you need to dig deeper...
}

// This is called by OTCronItem::HookRemovalFromCron
// (After calling this method, HookRemovalFromCron then calls
// onRemovalFromCron.)
//
void OTTrade::onFinalReceipt(
    OTCronItem& origCronItem,
    const std::int64_t& newTransactionNumber,
    Nym_p originator,
    Nym_p remover,
    const PasswordPrompt& reason)
{

    OTCron* cron = GetCron();
    OT_ASSERT(cron != nullptr);

    auto serverNym = cron->GetServerNym();
    OT_ASSERT(serverNym);

    auto context = api_.Wallet().Internal().mutable_ClientContext(
        originator->ID(), reason);

    // First, we are closing the transaction number ITSELF, of this cron item,
    // as an active issued number on the originating nym. (Changing it to
    // CLOSED.)
    //
    // Second, we're verifying the CLOSING number, and using it as the closing
    // number
    // on the FINAL RECEIPT (with that receipt being "InReferenceTo"
    // GetTransactionNum())
    //
    const TransactionNumber openingNumber = origCronItem.GetTransactionNum();
    const TransactionNumber closingAssetNumber =
        (origCronItem.GetCountClosingNumbers() > 0)
            ? origCronItem.GetClosingTransactionNoAt(0)
            : 0;
    const TransactionNumber closingCurrencyNumber =
        (origCronItem.GetCountClosingNumbers() > 1)
            ? origCronItem.GetClosingTransactionNoAt(1)
            : 0;
    const auto notaryID = String::Factory(GetNotaryID());

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
    // payment plan) is stored in the ATTACHMENT on the Item member.
    //          3. ADDITIONAL INFORMATION is stored in the NOTE field of the
    // Item member.
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
    auto updatedCronItem = String::Factory(*this);
    OTString attachment = updatedCronItem;  // the Updated TRADE.
    auto updatedOffer = String::Factory();
    OTString note = String::Factory();  // the updated Offer (if available.)

    if (offer_) {
        offer_->SaveContractRaw(updatedOffer);
        note = updatedOffer;
    }

    const auto strOrigCronItem = String::Factory(origCronItem);

    // The OPENING transaction number must still be signed-out. It is this act
    // of placing the final receipt, which then finally closes the opening
    // number. The closing number, by contrast, is not closed out until the
    // final Receipt is ACCEPTED (which happens in a "process inbox"
    // transaction.)
    if ((openingNumber > 0) &&
        context.get().VerifyIssuedNumber(openingNumber)) {
        // The Nym (server side) stores a list of all opening and closing cron
        // #s. So when the number is released from the Nym, we also take it off
        // that list.
        context.get().CloseCronItem(openingNumber);
        context.get().ConsumeIssued(openingNumber);

        if (!DropFinalReceiptToNymbox(
                GetSenderNymID(),
                newTransactionNumber,
                strOrigCronItem,
                GetOriginType(),
                reason,
                note,
                attachment)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failure dropping receipt into nymbox.")
                .Flush();
        }
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Problem verifying Opening Number when calling "
            "VerifyIssuedNum(openingNumber).")
            .Flush();
    }

    // ASSET ACCT
    //
    if ((closingAssetNumber > 0) &&
        context.get().VerifyIssuedNumber(closingAssetNumber)) {
        DropFinalReceiptToInbox(
            GetSenderNymID(),
            GetSenderAcctID(),
            newTransactionNumber,
            closingAssetNumber,  // The closing transaction number to put on the
                                 // receipt.
            strOrigCronItem,
            GetOriginType(),
            reason,
            note,
            attachment);
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Failed verifying "
            "closingAssetNumber=origCronItem. "
            "GetClosingTransactionNoAt(0)>0 &&  "
            "originator. VerifyTransactionNum(closingAssetNumber).")
            .Flush();
    }

    // CURRENCY ACCT
    if ((closingCurrencyNumber > 0) &&
        context.get().VerifyIssuedNumber(closingCurrencyNumber)) {
        DropFinalReceiptToInbox(
            GetSenderNymID(),
            GetCurrencyAcctID(),
            newTransactionNumber,
            closingCurrencyNumber,  // closing transaction number for the
                                    // receipt.
            strOrigCronItem,
            GetOriginType(),
            reason,
            note,
            attachment);
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Failed verifying closingCurrencyNumber=origCronItem. "
            "GetClosingTransactionNoAt(1)>0  && "
            "originator. VerifyTransactionNum(closingCurrencyNumber).")
            .Flush();
    }

    // the RemoveIssued call means the original transaction# (to find this cron
    // item on cron) is now CLOSED. But the Transaction itself is still OPEN.
    // How? Because the CLOSING number is still signed out. The closing number
    // is also USED, since the NotarizePaymentPlan or NotarizeMarketOffer call,
    // but it remains ISSUED, until the final receipt itself is accepted during
    // a process inbox.

    // QUESTION: Won't there be Cron Items that have no asset account at all?
    // In which case, there'd be no need to drop a final receipt, but I don't
    // think that's the case, since you have to use a transaction number to get
    // onto cron in the first place.
}

// OTCron calls this regularly, which is my chance to expire, etc.
// Return True if I should stay on the Cron list for more processing.
// Return False if I should be removed and deleted.
auto OTTrade::ProcessCron(const PasswordPrompt& reason) -> bool
{
    // Right now Cron is called 10 times per second.
    // I'm going to slow down all trades so they are once every
    // GetProcessInterval()
    if (GetLastProcessDate() > Time{}) {
        // (Default ProcessInterval is 1 second, but Trades will use 10 seconds,
        // and Payment Plans will use an hour or day.)
        if ((Clock::now() - GetLastProcessDate()) <= GetProcessInterval()) {

            return true;
        }
    }

    // Keep a record of the last time this was processed.
    // (NOT saved to storage, only used while the software is running.)
    // (Thus no need to release signatures, sign contract, save contract, etc.)
    SetLastProcessDate(Clock::now());

    // PAST END DATE?
    // First call the parent's version (which this overrides) so it has
    // a chance to check its stuff. Currently it checks IsExpired().
    if (!ot_super::ProcessCron(reason)) {
        return false;  // It's expired or flagged for removal--remove it from
    }
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
    if (!VerifyCurrentDate()) {
        return true;  // The Trade is not yet valid, so we return. BUT, we
    }
    // return
    //  true, so it will stay on Cron until it BECOMES valid.

    // TRADE-specific stuff below.

    bool bStayOnMarket =
        true;  // by default stay on the market (until some rule expires me.)

    auto OFFER_MARKET_ID = identifier::Generic{};
    OTMarket* market = nullptr;

    // If the Offer is already active on a market, then I already have a pointer
    // to it. This function returns that pointer. If nullptr, it tries to find
    // the offer on the market and then sets the pointer and returns. If it
    // can't find it, IT TRIES TO ADD IT TO THE MARKET and sets the pointer and
    // returns it.
    auto* offer = GetOffer(OFFER_MARKET_ID, reason, &market);

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
    } else if (market == nullptr) {

        // todo. (This will already leave a log above in GetOffer somewhere.)
        //        otErr << "OTTrade::ProcessCron: Market was nullptr.\n"; //
        // comment this out
    } else  // If a valid pointer was returned, that means the offer is on the
            // market.
    {
        // Make sure it hasn't already been flagged by someone else...
        if (IsFlaggedForRemoval()) {  // This is checked above in
                                      // OTCronItem::ProcessCron().
            bStayOnMarket = false;    // I'm leaving the check here in case the
                                      // flag was set since then.

        } else  // Process it!  <===================
        {
            LogVerbose()("Processing trade: ")(GetTransactionNum()).Flush();

            bStayOnMarket =
                market->ProcessTrade(api_.Wallet(), *this, *offer, reason);
            // No need to save the Trade or Offer, since they will
            // be saved inside this call if they are changed.
        }
    }

    // Return True if I should stay on the Cron list for more processing.
    // Return False if I should be removed and deleted.
    return bStayOnMarket;  // defaults true, so if false, that means someone is
                           // removing it for a reason.
}

/*
X identifier::Generic    currency_type_id_;    // GOLD (Asset) is trading for
DOLLARS (Currency). X identifier::Generic    currency_acct_id_;    // My Dollar
account, used for paying for my Gold (say) trades.

X std::int64_t            stop_price_;        // The price limit that activates
the
STOP order.
X char            stop_sign_;        // Value is 0, or '<', or '>'.

X time64_t        creation_date_;    // The date, in seconds, when the trade
was authorized.
X std::int32_t            trades_already_done_;    // How many trades have
already processed through this order? We keep track.
*/

// This is called by the client side. First you call MakeOffer() to set up the
// Offer,
// then you call IssueTrade() and pass the Offer into it here.
auto OTTrade::IssueTrade(OTOffer& offer, char stopSign, const Amount& stopPrice)
    -> bool
{
    // Make sure the Stop Sign is within parameters (0, '<', or '>')
    if ((stopSign == 0) || (stopSign == '<') || (stopSign == '>')) {
        stop_sign_ = stopSign;
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Bad data in Stop Sign while issuing trade: ")(stopSign)(".")
            .Flush();
        return false;
    }

    // Make sure, if this IS a Stop order, that the price is within parameters
    // and set.
    if ((stop_sign_ == '<') || (stop_sign_ == '>')) {
        if (0 >= stopPrice) {
            LogError()(OT_PRETTY_CLASS())("Expected Stop Price for trade.")
                .Flush();
            return false;
        }

        stop_price_ = stopPrice;
    }

    trades_already_done_ = 0;

    SetCreationDate(Clock::now());  // This time is set to TODAY NOW
                                    // (OTCronItem)

    // Validate the Notary ID, Instrument Definition ID, Currency Type ID, and
    // Date Range.
    if ((GetNotaryID() != offer.GetNotaryID()) ||
        (GetCurrencyID() != offer.GetCurrencyID()) ||
        (GetInstrumentDefinitionID() != offer.GetInstrumentDefinitionID()) ||
        (offer.GetValidFrom() < Time{}) ||
        (offer.GetValidTo() < offer.GetValidFrom())) {
        return false;
    }

    //    currency_type_id_ // This is already set in the constructors of this
    // and the offer. (And compared.)
    //    currency_acct_id_ // This is already set in the constructor of this.

    // Set the (now validated) date range as per the Offer.
    SetValidFrom(offer.GetValidFrom());
    SetValidTo(offer.GetValidTo());

    // Get the transaction number from the Offer.
    SetTransactionNum(offer.GetTransactionNum());

    // Save a copy of the offer, in XML form, here on this Trade.
    auto strOffer = String::Factory(offer);
    market_offer_->Set(strOffer);

    return true;
}

// the framework will call this at the right time.
void OTTrade::Release_Trade()
{
    // If there were any dynamically allocated objects, clean them up here.
    currency_type_id_.clear();
    currency_acct_id_.clear();

    market_offer_->Release();
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
    contract_type_ = String::Factory("TRADE");

    // Trades default to processing every 10 seconds. (vs 1 second for Cron
    // items and 1 hour for payment plans)
    SetProcessInterval(std::chrono::seconds{TradeProcessIntervalSeconds});

    trades_already_done_ = 0;

    stop_sign_ = 0;   // IS THIS a STOP order? Value is 0, or '<', or '>'.
    stop_price_ = 0;  // The price limit that activates the STOP order.
    stop_activated_ = false;  // Once the Stop Order activates, it puts the
                              // order on the market.
    // I'll put a "HasOrderOnMarket()" bool method that answers this for u.
    has_trade_activated_ = false;  // I want to keep track of general
                                   // activations as well, not just stop orders.
}

OTTrade::~OTTrade() { Release_Trade(); }
}  // namespace opentxs
