// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/Instrument.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
namespace imp
{
class Factory;
}  // namespace imp
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Notary;
}  // namespace identifier

class OTTrade;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

// Each instance of OTOffer represents a Bid or Ask. (A Market has a list of bid
// offers and a list of ask offers.)

/*
 OTOffer

 Offer MUST STORE:

 X 1) Transaction ID (MUST be linked to a trade, so it can expire, and so it can
 be paid for.)
 X 2) ASSET TYPE ID of whatever I’m trying to BUY or SELL. (Is this the Gold
 market?)
 X 7) CURRENCY TYPE ID of whatever I’m trying to buy or sell it WITH. (Is it
 dollars? Euro? Yen?)
 X 8) BUY OR SELL? (BOOL)
 X 9) Bid/Ask price (limit / per minimum increment.)

 X 4) Total number of assets available for sale or purchase. (4 ounces of gold?
 12 ounces of gold?)
 X 5) Number of assets already traded, against the above total.
 X 6) Minimum increment for sale or purchase (if matches “total number of assets
 for sale”, effectively becomes a FILL OR KILL order. MUST be 1 or greater.
 CANNOT be zero.)
*/
namespace opentxs
{
class OTOffer : public Instrument
{
public:
    auto MakeOffer(
        bool bBuyingOrSelling,            // True == SELLING, False == BUYING
        const Amount& lPriceLimit,        // Per Scale...
        const Amount& lTotalAssetsOffer,  // Total assets available for
                                          // sale or
                                          // purchase.
        const Amount& lMinimumIncrement,  // The minimum increment that
                                          // must be
        // bought or sold for each transaction
        const std::int64_t& lTransactionNum,  // The transaction number
                                              // authorizing
                                              // this trade.
        const Time VALID_FROM = {},           // defaults to RIGHT NOW
        const Time VALID_TO = {}) -> bool;    // defaults to 24 hours (a
                                              // "Day Order")
    inline void IncrementFinishedSoFar(const Amount& lFinishedSoFar)
    {
        finished_so_far_ += lFinishedSoFar;
    }

    inline auto GetAmountAvailable() const -> Amount
    {
        return GetTotalAssetsOnOffer() - GetFinishedSoFar();
    }
    inline auto GetTransactionNum() const -> const std::int64_t&
    {
        return transaction_num_;
    }

    inline auto GetPriceLimit() const -> const Amount& { return price_limit_; }
    inline auto GetTotalAssetsOnOffer() const -> const Amount&
    {
        return total_assets_offer_;
    }
    inline auto GetFinishedSoFar() const -> const Amount&
    {
        return finished_so_far_;
    }
    inline auto GetMinimumIncrement() -> const Amount&
    {
        if (minimum_increment_ < 1) { minimum_increment_ = 1; }
        return minimum_increment_;
    }
    inline auto GetScale() const -> const Amount& { return scale_; }

    inline auto GetCurrencyID() const -> const identifier::Generic&
    {
        return currency_type_id_;
    }
    inline void SetCurrencyID(const identifier::UnitDefinition& CURRENCY_ID)
    {
        currency_type_id_ = CURRENCY_ID;
    }

    // Buying or selling?
    inline auto IsBid() -> bool { return !selling_; }
    inline auto IsAsk() -> bool { return selling_; }

    auto IsMarketOrder() const -> bool;
    auto IsLimitOrder() const -> bool;

    // Stores a pointer to theTrade for later use. (Not responsible to clean up,
    // just convenient.)
    inline auto GetTrade() -> OTTrade* { return trade_; }
    inline void SetTrade(const OTTrade& theTrade)
    {
        trade_ = &(const_cast<OTTrade&>(theTrade));
    }
    // Note: date_added_to_market_ is not saved in the Offer Contract, but
    // OTMarket sets/saves/loads it.
    //
    auto GetDateAddedToMarket() const -> Time;    // Used in
                                                  // OTMarket::GetOfferList
                                                  // and GetNymOfferList.
    void SetDateAddedToMarket(const Time tDate);  // Used in OTCron when
                                                  // adding/loading
                                                  // offers.
    // Overridden from Contract.
    void GetIdentifier(identifier::Generic& theIdentifier) const override;

    void InitOffer();

    void Release() override;
    void Release_Offer();

    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

    void UpdateContents(const PasswordPrompt& reason)
        override;  // Before transmission or
                   // serialization, this is where the
                   // ledger saves its contents

    OTOffer() = delete;
    OTOffer(const OTOffer&) = delete;
    OTOffer(OTOffer&&) = delete;
    auto operator=(const OTOffer&) -> OTOffer& = delete;
    auto operator=(OTOffer&&) -> OTOffer& = delete;

    ~OTOffer() override;

protected:
    // If this offer is actually connected to a trade, it will have a pointer.
    OTTrade* trade_{nullptr};
    // GOLD (Asset) is trading for DOLLARS (Currency).
    identifier::Generic currency_type_id_;
    bool selling_{false};  // true = ask. false = bid.
    // If a bid, this is the most I will pay. If an ask, this is the least I
    // will sell for. My limit.
    // (Normally the price I get is whatever is the best one on the market right
    // now.)
    Amount price_limit_{0};  // Denominated in CURRENCY TYPE, and priced
                             // per SCALE. 1oz market price limit might
                             // be 1,300
    // 100oz market price limit might be 130,000 (or 127,987 or whatever)

    std::int64_t transaction_num_{0};  // Matches to an OTTrade stored in
                                       // OTCron.
    Amount total_assets_offer_{0};     // Total amount of ASSET TYPE trying
                                       // to BUY or SELL, this trade.
    Amount finished_so_far_{0};        // Number of ASSETs bought or sold
                                       // already against the above total.

    Amount scale_{0};              // 1oz market? 100oz market? 10,000oz market?
                                   // This determines size and granularity.
    Amount minimum_increment_{0};  // Each sale or purchase against the
                                   // above total must be in minimum
                                   // increments.
    // Minimum Increment must be evenly divisible by scale_.
    // (This effectively becomes a "FILL OR KILL" order if set to the same value
    // as total_assets_offer_. Also, MUST be 1
    // or great. CANNOT be zero. Enforce this at class level. You cannot sell
    // something in minimum increments of 0.)
    inline void SetTransactionNum(const std::int64_t& lTransactionNum)
    {
        transaction_num_ = lTransactionNum;
    }
    inline void SetPriceLimit(const Amount& lPriceLimit)
    {
        price_limit_ = lPriceLimit;
    }
    inline void SetTotalAssetsOnOffer(const Amount& lTotalAssets)
    {
        total_assets_offer_ = lTotalAssets;
    }
    inline void SetFinishedSoFar(const std::int64_t& lFinishedSoFar)
    {
        finished_so_far_ = lFinishedSoFar;
    }
    inline void SetMinimumIncrement(const Amount& lMinIncrement)
    {
        minimum_increment_ = lMinIncrement;
        if (minimum_increment_ < 1) { minimum_increment_ = 1; }
    }
    inline void SetScale(const Amount& lScale)
    {
        scale_ = lScale;
        if (scale_ < 1) { scale_ = 1; }
    }

private:
    friend api::session::imp::Factory;

    using ot_super = Instrument;

    Time date_added_to_market_;

    auto isPowerOfTen(const std::int64_t& x) -> bool;

    OTOffer(const api::Session& api);  // The constructor contains
                                       // the 3 variables needed to
                                       // identify any market.
    OTOffer(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const Amount& MARKET_SCALE);
};
}  // namespace opentxs
