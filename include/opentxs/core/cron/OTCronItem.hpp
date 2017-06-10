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

// Base class for OTTrade and OTAgreement.
// OTCron contains lists of these for regular processing.

#ifndef OPENTXS_CORE_CRON_OTCRONITEM_HPP
#define OPENTXS_CORE_CRON_OTCRONITEM_HPP

#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/OTTrackable.hpp"
#include "opentxs/core/Types.hpp"

#include <deque>

namespace opentxs
{
class ClientContext;
class Identifier;
class Nym;
class OTCron;
class ServerContext;
class String;
class Tag;

class OTCronItem : public OTTrackable
{
public:
    OTCronItem();

    virtual originType GetOriginType() const = 0;

private: // Private prevents erroneous use by other classes.
    typedef OTTrackable ot_super;

private:
    OTCron* m_pCron{nullptr};
    Nym* serverNym_{nullptr};
    Identifier* notaryID_{nullptr};
    time64_t m_CREATION_DATE{0};     // The date, in seconds, when the CronItem was
                                  // authorized.
    time64_t m_LAST_PROCESS_DATE{0}; // The last time this item was processed.
    int64_t m_PROCESS_INTERVAL{0};   // How often to Process Cron on this item.

protected:
    std::deque<int64_t> m_dequeClosingNumbers; // Numbers used for CLOSING a
                                               // transaction. (finalReceipt.)

protected:
    OTCronItem(const Identifier& NOTARY_ID,
               const Identifier& INSTRUMENT_DEFINITION_ID);
    OTCronItem(const Identifier& NOTARY_ID,
               const Identifier& INSTRUMENT_DEFINITION_ID,
               const Identifier& ACCT_ID, const Identifier& NYM_ID);

    Identifier* m_pCancelerNymID{nullptr};

    bool m_bCanceled{false}; // This defaults to false. But if someone cancels it
                      // (BEFORE it is ever activated, just to nip it in the bud
                      // and harvest the numbers, and send the notices, etc) --
                      // then we set this to true, and we also set the canceler
                      // Nym ID. (So we can see these values later and know
                      // whether it was canceled before activation, and if so,
                      // who did it.)

    bool m_bRemovalFlag{false}; // Set this to true and the cronitem will be removed
                         // from Cron on next process.
    // (And its offer will be removed from the Market as well, if appropriate.)
    virtual void onActivate()
    {
    } // called by HookActivationOnCron().

    virtual void onFinalReceipt(OTCronItem& theOrigCronItem,
                                const int64_t& lNewTransactionNumber,
                                Nym& theOriginator,
                                Nym* pRemover); // called by
                                                // HookRemovalFromCron().

    virtual void onRemovalFromCron()
    {
    } // called by HookRemovalFromCron().
    void ClearClosingNumbers();

public:
    // To force the Nym to close out the closing number on the receipt.
    bool DropFinalReceiptToInbox(
        const Identifier& NYM_ID,
        const Identifier& ACCOUNT_ID,
        const int64_t& lNewTransactionNumber,
        const int64_t& lClosingNumber,
        const String& strOrigCronItem,
        const originType theOriginType,
        String* pstrNote = nullptr,
        String* pstrAttachment = nullptr,
        Account* pActualAcct = nullptr);

    // Notify the Nym that the OPENING number is now closed, so he can remove it
    // from his issued list.
    bool DropFinalReceiptToNymbox(
        const Identifier& NYM_ID,
        const TransactionNumber& lNewTransactionNumber,
        const String& strOrigCronItem,
        const originType theOriginType,
        String* pstrNote = nullptr,
        String* pstrAttachment = nullptr,
        const Nym* pActualNym = nullptr);
    virtual bool CanRemoveItemFromCron(const ClientContext& context);
    virtual void HarvestOpeningNumber(ServerContext& context);
    virtual void HarvestClosingNumbers(ServerContext& context);
    // pActivator and pRemover are both "SOMETIMES nullptr"
    // I don't default the parameter, because I want to force the programmer to
    // choose.

    // Called in OTCron::AddCronItem.
    void HookActivationOnCron(Nym* pActivator,
                              bool bForTheFirstTime = false); // This calls
                                                              // onActivate,
                                                              // which is
                                                              // virtual.

    // Called in OTCron::RemoveCronItem as well as OTCron::ProcessCron.
    // This calls onFinalReceipt, then onRemovalFromCron. Both are virtual.
    void HookRemovalFromCron(Nym* pRemover, int64_t newTransactionNo);

    inline bool IsFlaggedForRemoval() const
    {
        return m_bRemovalFlag;
    }
    inline void FlagForRemoval()
    {
        m_bRemovalFlag = true;
    }
    inline void SetCronPointer(OTCron& theCron)
    {
        m_pCron = &theCron;
    }

    EXPORT static OTCronItem* NewCronItem(const String& strCronItem);
    EXPORT static OTCronItem* LoadCronReceipt(
        const int64_t& lTransactionNum); // Server-side only.
    EXPORT static OTCronItem* LoadActiveCronReceipt(
        const int64_t& lTransactionNum,
        const Identifier& notaryID); // Client-side only.
    EXPORT static bool EraseActiveCronReceipt(
        const int64_t& lTransactionNum, const Identifier& nymID,
        const Identifier& notaryID); // Client-side only.
    EXPORT static bool GetActiveCronTransNums(NumList& output, // Client-side
                                                               // only.
                                              const Identifier& nymID,
                                              const Identifier& notaryID);
    inline void SetCreationDate(const time64_t& CREATION_DATE)
    {
        m_CREATION_DATE = CREATION_DATE;
    }
    inline const time64_t& GetCreationDate() const
    {
        return m_CREATION_DATE;
    }

    EXPORT bool SetDateRange(time64_t VALID_FROM = OT_TIME_ZERO,
                             time64_t VALID_TO = OT_TIME_ZERO);
    inline void SetLastProcessDate(const time64_t& THE_DATE)
    {
        m_LAST_PROCESS_DATE = THE_DATE;
    }
    inline const time64_t& GetLastProcessDate() const
    {
        return m_LAST_PROCESS_DATE;
    }

    inline void SetProcessInterval(const int64_t& THE_DATE)
    {
        m_PROCESS_INTERVAL = THE_DATE;
    }
    inline const int64_t& GetProcessInterval() const
    {
        return m_PROCESS_INTERVAL;
    }

    inline OTCron* GetCron() const
    {
        return m_pCron;
    }
    void setServerNym(Nym* serverNym)
    {
        serverNym_ = serverNym;
    }
    void setNotaryID(Identifier* notaryID)
    {
        notaryID_ = notaryID;
    }
    // When first adding anything to Cron, a copy needs to be saved in a folder
    // somewhere.
    EXPORT bool SaveCronReceipt(); // server side only
    EXPORT bool SaveActiveCronReceipt(const Identifier& theNymID); // client
                                                                   // side
                                                                   // only

    // Return True if should stay on OTCron's list for more processing.
    // Return False if expired or otherwise should be removed.
    virtual bool ProcessCron(); // OTCron calls this regularly, which is my
                                // chance to expire, etc.
                                // From OTTrackable (parent class of this)
    virtual ~OTCronItem();

    void InitCronItem();

    void Release() override;
    void Release_CronItem();
    EXPORT bool GetCancelerID(Identifier& theOutput) const;
    EXPORT bool IsCanceled() const
    {
        return m_bCanceled;
    }

    // When canceling a cron item before it
    // has been activated, use this.
    EXPORT bool CancelBeforeActivation(Nym& theCancelerNym);

    // These are for     std::deque<int64_t> m_dequeClosingNumbers;
    // They are numbers used for CLOSING a transaction. (finalReceipt.)
    EXPORT int64_t GetClosingTransactionNoAt(uint32_t nIndex) const;
    EXPORT int32_t GetCountClosingNumbers() const;

    EXPORT void AddClosingTransactionNo(const int64_t& lClosingTransactionNo);

    // HIGHER LEVEL ABSTRACTIONS:
    EXPORT int64_t GetOpeningNum() const;
    EXPORT int64_t GetClosingNum() const;
    virtual bool IsValidOpeningNumber(const int64_t& lOpeningNum) const;

    virtual int64_t GetOpeningNumber(const Identifier& theNymID) const;
    virtual int64_t GetClosingNumber(const Identifier& theAcctID) const;
    int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;
    //  virtual void UpdateContents();
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRON_OTCRONITEM_HPP
