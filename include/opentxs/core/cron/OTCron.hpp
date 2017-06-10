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

#ifndef OPENTXS_CORE_CRON_OTCRON_HPP
#define OPENTXS_CORE_CRON_OTCRON_HPP

#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/StringUtils.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/Contract.hpp"

namespace opentxs
{

class OTCronItem;
class OTMarket;
class Nym;

/** mapOfCronItems:      Mapped (uniquely) to transaction number. */
typedef std::map<int64_t, OTCronItem*> mapOfCronItems;
/** multimapOfCronItems: Mapped to date the item was added to Cron. */
typedef std::multimap<time64_t, OTCronItem*> multimapOfCronItems;
/** Mapped (uniquely) to market ID. */
typedef std::map<std::string, OTMarket*> mapOfMarkets;
/** Cron stores a bunch of these on this list, which the server refreshes from
 * time to time. */
typedef std::list<int64_t> listOfLongNumbers;

/** OTCron has a list of OTCronItems. (Really subclasses of that such as OTTrade
 * and OTAgreement.) */
class OTCron : public Contract
{
private:
    typedef Contract ot_super;

private:
    // A list of all valid markets.
    mapOfMarkets m_mapMarkets;
    // Cron Items are found on both lists.
    mapOfCronItems m_mapCronItems;
    multimapOfCronItems m_multimapCronItems;
    // Always store this in any object that's associated with a specific server.
    Identifier m_NOTARY_ID;
    // I can't put receipts in people's inboxes without a supply of these.
    listOfLongNumbers m_listTransactionNumbers;
    // I don't want to start Cron processing until everything else is all loaded
    //  up and ready to go.
    bool m_bIsActivated{false};
    // I'll need this for later.
    Nym* m_pServerNym{nullptr};
    // Number of transaction numbers Cron  will grab for itself, when it gets
    // low, before each round.
    static int32_t __trans_refill_amount;
    // Number of milliseconds (ideally) between each "Cron Process" event.
    static int32_t __cron_ms_between_process;
    // Int. The maximum number of cron items any given Nym can have
    // active at the same time.
    static int32_t __cron_max_items_per_nym;

    static Timer tCron;

public:
    static int32_t GetCronMsBetweenProcess()
    {
        return __cron_ms_between_process;
    }
    static void SetCronMsBetweenProcess(int32_t lMS)
    {
        __cron_ms_between_process = lMS;
    }

    static int32_t GetCronRefillAmount() { return __trans_refill_amount; }
    static void SetCronRefillAmount(int32_t nAmount)
    {
        __trans_refill_amount = nAmount;
    }
    static int32_t GetCronMaxItemsPerNym() { return __cron_max_items_per_nym; }
    static void SetCronMaxItemsPerNym(int32_t nMax)
    {
        __cron_max_items_per_nym = nMax;
    }
    inline bool IsActivated() const { return m_bIsActivated; }
    inline bool ActivateCron()
    {
        if (!m_bIsActivated)
            return m_bIsActivated = true;
        else
            return false;
    }
    // RECURRING TRANSACTIONS
    EXPORT bool AddCronItem(
        OTCronItem& theItem,
        Nym* pActivator,
        bool bSaveReceipt,
        time64_t tDateAdded);  // Date it was FIRST added to Cron.
    /** if returns false, item wasn't found. */
    EXPORT bool RemoveCronItem(int64_t lTransactionNum, Nym& theRemover);
    EXPORT OTCronItem* GetItemByOfficialNum(int64_t lTransactionNum);
    EXPORT OTCronItem* GetItemByValidOpeningNum(int64_t lOpeningNum);
    EXPORT mapOfCronItems::iterator FindItemOnMap(int64_t lTransactionNum);
    EXPORT multimapOfCronItems::iterator FindItemOnMultimap(
        int64_t lTransactionNum);
    // MARKETS
    bool AddMarket(OTMarket& theMarket, bool bSaveMarketFile = true);
    bool RemoveMarket(const Identifier& MARKET_ID);  // if returns false,
                                                     // market wasn't found.

    EXPORT OTMarket* GetMarket(const Identifier& MARKET_ID);
    OTMarket* GetOrCreateMarket(
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& CURRENCY_ID,
        const int64_t& lScale);
    /** This is informational only. It returns OTStorage-type data objects,
     * packed in a string. */
    EXPORT bool GetMarketList(OTASCIIArmor& ascOutput, int32_t& nMarketCount);
    EXPORT bool GetNym_OfferList(
        OTASCIIArmor& ascOutput,
        const Identifier& NYM_ID,
        int32_t& nOfferCount);
    // TRANSACTION NUMBERS
    /**The server starts out putting a bunch of numbers in here so Cron can use
     * them. Then the internal trades and payment plans get numbers from here as
     * needed. Server MUST replenish from time-to-time, or Cron will stop
     * working. Part of using Cron properly is to call ProcessCron() regularly,
     * as well as to call AddTransactionNumber() regularly, in order to keep
     * GetTransactionCount() at some minimum threshold. */
    EXPORT void AddTransactionNumber(const int64_t& lTransactionNum);
    int64_t GetNextTransactionNumber();
    /** How many numbers do I currently have on the list? */
    EXPORT int32_t GetTransactionCount() const;
    /** Make sure every time you call this, you check the GetTransactionCount()
     * first and replenish it to whatever your minimum supply is. (The
     * transaction numbers in there must be enough to last for the entire
     * ProcessCronItems() call, and all the trades and payment plans within,
     * since it will not be replenished again at least until the call has
     * finished.) */
    EXPORT void ProcessCronItems();

    int64_t computeTimeout();

    inline void SetNotaryID(const Identifier& NOTARY_ID)
    {
        m_NOTARY_ID = NOTARY_ID;
    }
    inline const Identifier& GetNotaryID() const { return m_NOTARY_ID; }

    inline void SetServerNym(Nym* pServerNym)
    {
        OT_ASSERT(nullptr != pServerNym);
        m_pServerNym = pServerNym;
    }
    inline Nym* GetServerNym() const { return m_pServerNym; }

    EXPORT bool LoadCron();
    EXPORT bool SaveCron();

    EXPORT OTCron();
    explicit OTCron(const Identifier& NOTARY_ID);
    explicit OTCron(const char* szFilename);

    EXPORT virtual ~OTCron();

    void InitCron();

    void Release() override;
    void Release_Cron();

    /** return -1 if error, 0 if nothing, and 1 if the node was processed. */
    int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;
    /** Before transmission or serialization, this is where the ledger saves its
     * contents */
    void UpdateContents() override;
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRON_OTCRON_HPP
