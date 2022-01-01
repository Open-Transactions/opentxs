// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <map>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
namespace imp
{
class Factory;
}  // namespace imp

class Wallet;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace OTDB
{
class OfferListNym;
class TradeListMarket;
}  // namespace OTDB

class Account;
class Armored;
class Identifier;
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
using mapOfOffers = std::pmr::multimap<Amount, OTOffer*>;
// The same offers are also mapped (uniquely) to transaction number.
using mapOfOffersTrnsNum = std::pmr::map<Amount, OTOffer*>;

// A market has a list of OTOffers for all the bids, and another list of
// OTOffers for all the asks.
// Presumably the server will have different markets for different instrument
// definitions.

class OPENTXS_EXPORT OTMarket : public Contract
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

    auto GetBidCount() -> mapOfOffers::size_type { return m_mapBids.size(); }
    auto GetAskCount() -> mapOfOffers::size_type { return m_mapAsks.size(); }
    void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    {
        m_INSTRUMENT_DEFINITION_ID = INSTRUMENT_DEFINITION_ID;
    }
    void SetCurrencyID(const identifier::UnitDefinition& CURRENCY_ID)
    {
        m_CURRENCY_TYPE_ID = CURRENCY_ID;
    }
    void SetNotaryID(const identifier::Server& NOTARY_ID)
    {
        m_NOTARY_ID = NOTARY_ID;
    }

    inline auto GetInstrumentDefinitionID() const
        -> const identifier::UnitDefinition&
    {
        return m_INSTRUMENT_DEFINITION_ID;
    }
    inline auto GetCurrencyID() const -> const identifier::UnitDefinition&
    {
        return m_CURRENCY_TYPE_ID;
    }
    inline auto GetNotaryID() const -> const identifier::Server&
    {
        return m_NOTARY_ID;
    }

    inline auto GetScale() const -> const Amount& { return m_lScale; }
    inline void SetScale(const Amount& lScale)
    {
        m_lScale = lScale;
        if (m_lScale < 1) m_lScale = 1;
    }

    inline auto GetLastSalePrice() -> const Amount&
    {
        if (m_lLastSalePrice < 1) m_lLastSalePrice = 1;
        return m_lLastSalePrice;
    }
    inline void SetLastSalePrice(const std::int64_t& lLastSalePrice)
    {
        m_lLastSalePrice = lLastSalePrice;
        if (m_lLastSalePrice < 1) m_lLastSalePrice = 1;
    }

    auto GetLastSaleDate() -> const std::string& { return m_strLastSaleDate; }
    auto GetTotalAvailableAssets() -> Amount;

    void GetIdentifier(Identifier& theIdentifier) const override;

    inline void SetCronPointer(OTCron& theCron) { m_pCron = &theCron; }
    inline auto GetCron() -> OTCron* { return m_pCron; }
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

    ~OTMarket() override;

private:
    friend api::session::imp::Factory;

    using ot_super = Contract;

    OTCron* m_pCron{nullptr};  // The Cron object that owns this Market.

    OTDB::TradeListMarket* m_pTradeList{nullptr};

    mapOfOffers m_mapBids;  // The buyers, ordered by price limit
    mapOfOffers m_mapAsks;  // The sellers, ordered by price limit

    mapOfOffersTrnsNum m_mapOffers;  // All of the offers on a single list,
                                     // ordered by transaction number.

    OTServerID m_NOTARY_ID;  // Always store this in any object that's
                             // associated with a specific server.

    // Every market involves a certain instrument definition being traded in a
    // certain
    // currency.
    OTUnitID m_INSTRUMENT_DEFINITION_ID;  // This is the GOLD market. (Say.)
                                          // | (GOLD
                                          // for
    OTUnitID m_CURRENCY_TYPE_ID;  // Gold is trading for DOLLARS.        |
                                  // DOLLARS, for example.)

    // Each Offer on the market must have a minimum increment that this divides
    // equally into.
    // (There is a "gold for dollars, minimum 1 oz" market, a "gold for dollars,
    // min 500 oz" market, etc.)
    Amount m_lScale{0};

    Amount m_lLastSalePrice{0};
    std::string m_strLastSaleDate;

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
        const identifier::Server& NOTARY_ID,
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

    OTMarket() = delete;
    OTMarket(const OTMarket&) = delete;
    OTMarket(OTMarket&&) = delete;
    auto operator=(const OTMarket&) -> OTMarket& = delete;
    auto operator=(OTMarket&&) -> OTMarket& = delete;
};
}  // namespace opentxs
