// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "internal/otx/common/Contract.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class FactoryPrivate;
class Wallet;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace OTDB
{
class OfferListNym;
class TradeListMarket;
}  // namespace OTDB

class Account;
class Armored;
class OTCron;
class OTOffer;
class OTTrade;
class PasswordPrompt;

#define MAX_MARKET_QUERY_DEPTH                                                 \
    50  // todo add this to the ini file. (Now that we actually have one.)

// Multiple offers, mapped by price limit.
// Using multi-map since there will be more than one offer for each single
// price.
// (Map would only allow a single item on the map for each price.)
using mapOfOffers = UnallocatedMultimap<Amount, OTOffer*>;
// The same offers are also mapped (uniquely) to transaction number.
using mapOfOffersTrnsNum = UnallocatedMap<Amount, OTOffer*>;

// A market has a list of OTOffers for all the bids, and another list of
// OTOffers for all the asks.
// Presumably the server will have different markets for different instrument
// definitions.

class OTMarket : public Contract
{
public:
    auto ValidateOfferForMarket(OTOffer& theOffer) -> bool;

    auto GetOffer(const std::int64_t& lTransactionNum) -> OTOffer*;
    auto AddOffer(
        OTTrade* pTrade,
        OTOffer& theOffer,
        const PasswordPrompt& reason,
        const bool bSaveFile = true,
        const Time tDateAddedToMarket = {}) -> bool;
    auto RemoveOffer(
        const std::int64_t& lTransactionNum,
        const PasswordPrompt& reason) -> bool;
    // returns general information about offers on the market
    auto GetOfferList(
        Armored& ascOutput,
        std::int64_t lDepth,
        std::int32_t& nOfferCount) -> bool;
    auto GetRecentTradeList(Armored& ascOutput, std::int32_t& nTradeCount)
        -> bool;

    // Returns more detailed information about offers for a specific Nym.
    auto GetNym_OfferList(
        const identifier::Nym& NYM_ID,
        OTDB::OfferListNym& theOutputList,
        std::int32_t& nNymOfferCount) -> bool;

    // Assumes a few things: Offer is part of Trade, and both have been
    // proven already to be a part of this market.
    // Basically the Offer is looked up on the Market by the Trade, and
    // then both are passed in here.
    // --Returns True if Trade should stay on the Cron list for more processing.
    // --Returns False if it should be removed and deleted.
    void ProcessTrade(
        const api::session::Wallet& wallet,
        OTTrade& theTrade,
        OTOffer& theOffer,
        OTOffer& theOtherOffer,
        const PasswordPrompt& reason);
    auto ProcessTrade(
        const api::session::Wallet& wallet,
        OTTrade& theTrade,
        OTOffer& theOffer,
        const PasswordPrompt& reason) -> bool;

    auto GetHighestBidPrice() -> Amount;
    auto GetLowestAskPrice() -> Amount;

    auto GetBidCount() -> mapOfOffers::size_type { return bids_.size(); }
    auto GetAskCount() -> mapOfOffers::size_type { return asks_.size(); }
    void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    {
        instrument_definition_id_ = INSTRUMENT_DEFINITION_ID;
    }
    void SetCurrencyID(const identifier::UnitDefinition& CURRENCY_ID)
    {
        currency_type_id_ = CURRENCY_ID;
    }
    void SetNotaryID(const identifier::Notary& NOTARY_ID)
    {
        notary_id_ = NOTARY_ID;
    }

    inline auto GetInstrumentDefinitionID() const
        -> const identifier::UnitDefinition&
    {
        return instrument_definition_id_;
    }
    inline auto GetCurrencyID() const -> const identifier::UnitDefinition&
    {
        return currency_type_id_;
    }
    inline auto GetNotaryID() const -> const identifier::Notary&
    {
        return notary_id_;
    }

    inline auto GetScale() const -> const Amount& { return scale_; }
    inline void SetScale(const Amount& lScale)
    {
        scale_ = lScale;
        if (scale_ < 1) { scale_ = 1; }
    }

    inline auto GetLastSalePrice() -> const Amount&
    {
        if (last_sale_price_ < 1) { last_sale_price_ = 1; }
        return last_sale_price_;
    }
    inline void SetLastSalePrice(const std::int64_t& lLastSalePrice)
    {
        last_sale_price_ = lLastSalePrice;
        if (last_sale_price_ < 1) { last_sale_price_ = 1; }
    }

    auto GetLastSaleDate() -> const UnallocatedCString&
    {
        return last_sale_date_;
    }
    auto GetTotalAvailableAssets() -> Amount;

    void GetIdentifier(identifier::Generic& theIdentifier) const override;

    inline void SetCronPointer(OTCron& theCron) { cron_ = &theCron; }
    inline auto GetCron() -> OTCron* { return cron_; }
    auto LoadMarket() -> bool;
    auto SaveMarket(const PasswordPrompt& reason) -> bool;

    void InitMarket();

    void Release() override;
    void Release_Market();

    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

    void UpdateContents(const PasswordPrompt& reason)
        override;  // Before transmission or
                   // serialization, this is where the
                   // ledger saves its contents

    OTMarket() = delete;
    OTMarket(const OTMarket&) = delete;
    OTMarket(OTMarket&&) = delete;
    auto operator=(const OTMarket&) -> OTMarket& = delete;
    auto operator=(OTMarket&&) -> OTMarket& = delete;

    ~OTMarket() override;

private:
    friend api::session::FactoryPrivate;

    using ot_super = Contract;

    OTCron* cron_{nullptr};  // The Cron object that owns this Market.

    OTDB::TradeListMarket* trade_list_{nullptr};

    mapOfOffers bids_;  // The buyers, ordered by price limit
    mapOfOffers asks_;  // The sellers, ordered by price limit

    mapOfOffersTrnsNum offers_;  // All of the offers on a single list,
                                 // ordered by transaction number.

    identifier::Notary notary_id_;  // Always store this in any object that's
                                    // associated with a specific server.

    // Every market involves a certain instrument definition being traded in a
    // certain
    // currency.
    identifier::UnitDefinition instrument_definition_id_;  // This is the GOLD
                                                           // market. (Say.) |
                                                           // (GOLD for
    identifier::UnitDefinition currency_type_id_;  // Gold is trading for
                                                   // DOLLARS.        |
                                                   // DOLLARS, for example.)

    // Each Offer on the market must have a minimum increment that this divides
    // equally into.
    // (There is a "gold for dollars, minimum 1 oz" market, a "gold for dollars,
    // min 500 oz" market, etc.)
    Amount scale_{0};

    Amount last_sale_price_{0};
    UnallocatedCString last_sale_date_;

    // The server stores a map of markets, one for each unique combination of
    // instrument definitions. That's what this market class represents: one
    // instrument definition being traded and priced in another. It could be
    // wheat for dollars, wheat for yen, or gold for dollars, or gold for wheat,
    // or gold for oil, or oil for wheat.  REALLY, THE TWO ARE JUST ARBITRARY
    // ASSET TYPES. But in order to keep terminology clear, I will refer to one
    // as the "instrument definition" and the other as the "currency type" so
    // that it stays VERY clear which instrument definition is up for sale, and
    // which instrument definition (currency type) it is being priced in. Other
    // than that, the two are technically interchangeable.

    OTMarket(const api::Session& api);
    OTMarket(const api::Session& api, const char* szFilename);
    OTMarket(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_TYPE_ID,
        const Amount& lScale);

    void rollback_four_accounts(
        Account& p1,
        bool b1,
        const Amount& a1,
        Account& p2,
        bool b2,
        const Amount& a2,
        Account& p3,
        bool b3,
        const Amount& a3,
        Account& p4,
        bool b4,
        const Amount& a4);
};
}  // namespace opentxs
