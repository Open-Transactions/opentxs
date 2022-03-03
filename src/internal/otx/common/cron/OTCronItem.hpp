// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Base class for OTTrade and OTAgreement.
// OTCron contains lists of these for regular processing.

#pragma once

#include <irrxml/irrXML.hpp>
#include <chrono>
#include <cstdint>
#include <memory>

#include "internal/otx/Types.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/OTTrackable.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
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
class Wallet;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
class Client;
class Server;
}  // namespace context
}  // namespace otx

class NumList;
class OTCron;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OTCronItem : public OTTrackable
{
public:
    virtual auto GetOriginType() const -> originType = 0;

    // To force the Nym to close out the closing number on the receipt.
    auto DropFinalReceiptToInbox(
        const identifier::Nym& NYM_ID,
        const Identifier& ACCOUNT_ID,
        const std::int64_t& lNewTransactionNumber,
        const std::int64_t& lClosingNumber,
        const String& strOrigCronItem,
        const originType theOriginType,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory()) -> bool;

    // Notify the Nym that the OPENING number is now closed, so he can remove it
    // from his issued list.
    auto DropFinalReceiptToNymbox(
        const identifier::Nym& NYM_ID,
        const TransactionNumber& lNewTransactionNumber,
        const String& strOrigCronItem,
        const originType theOriginType,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory()) -> bool;
    virtual auto CanRemoveItemFromCron(const otx::context::Client& context)
        -> bool;
    virtual void HarvestOpeningNumber(otx::context::Server& context);
    virtual void HarvestClosingNumbers(otx::context::Server& context);
    // pActivator and pRemover are both "SOMETIMES nullptr"
    // I don't default the parameter, because I want to force the programmer to
    // choose.

    // Called in OTCron::AddCronItem.
    void HookActivationOnCron(
        const PasswordPrompt& reason,
        bool bForTheFirstTime = false);  // This calls
                                         // onActivate,
                                         // which is
                                         // virtual.

    // Called in OTCron::RemoveCronItem as well as OTCron::ProcessCron.
    // This calls onFinalReceipt, then onRemovalFromCron. Both are virtual.
    void HookRemovalFromCron(
        const api::session::Wallet& wallet,
        Nym_p pRemover,
        std::int64_t newTransactionNo,
        const PasswordPrompt& reason);

    inline auto IsFlaggedForRemoval() const -> bool { return m_bRemovalFlag; }
    inline void FlagForRemoval() { m_bRemovalFlag = true; }
    inline void SetCronPointer(OTCron& theCron) { m_pCron = &theCron; }

    static auto LoadCronReceipt(
        const api::Session& api,
        const TransactionNumber& lTransactionNum)
        -> std::unique_ptr<OTCronItem>;  // Server-side only.
    static auto LoadActiveCronReceipt(
        const api::Session& api,
        const TransactionNumber& lTransactionNum,
        const identifier::Notary& notaryID)
        -> std::unique_ptr<OTCronItem>;  // Client-side only.
    static auto EraseActiveCronReceipt(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const TransactionNumber& lTransactionNum,
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID) -> bool;  // Client-side only.
    static auto GetActiveCronTransNums(
        const api::Session& api,
        NumList& output,  // Client-side
                          // only.
        const UnallocatedCString& dataFolder,
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID) -> bool;
    inline void SetCreationDate(const Time CREATION_DATE)
    {
        m_CREATION_DATE = CREATION_DATE;
    }
    inline auto GetCreationDate() const -> const Time
    {
        return m_CREATION_DATE;
    }

    auto SetDateRange(
        const Time VALID_FROM = Time{},
        const Time VALID_TO = Time{}) -> bool;
    inline void SetLastProcessDate(const Time THE_DATE)
    {
        m_LAST_PROCESS_DATE = THE_DATE;
    }

    inline auto GetLastProcessDate() const -> const Time
    {
        return m_LAST_PROCESS_DATE;
    }

    inline void SetProcessInterval(const std::chrono::seconds interval)
    {
        m_PROCESS_INTERVAL = interval;
    }
    inline auto GetProcessInterval() const -> const std::chrono::seconds
    {
        return m_PROCESS_INTERVAL;
    }

    inline auto GetCron() const -> OTCron* { return m_pCron; }
    void setServerNym(Nym_p serverNym) { serverNym_ = serverNym; }
    void setNotaryID(const identifier::Notary& notaryID);
    // When first adding anything to Cron, a copy needs to be saved in a
    // folder somewhere.
    auto SaveCronReceipt() -> bool;  // server side only
    auto SaveActiveCronReceipt(const identifier::Nym& theNymID)
        -> bool;  // client
                  // side
                  // only

    // Return True if should stay on OTCron's list for more processing.
    // Return False if expired or otherwise should be removed.
    virtual auto ProcessCron(const PasswordPrompt& reason)
        -> bool;  // OTCron calls this
                  // regularly, which is my
                  // chance to expire, etc.
                  // From OTTrackable
                  // (parent class of this)
    ~OTCronItem() override;

    void InitCronItem();

    void Release() override;
    void Release_CronItem();
    auto GetCancelerID(identifier::Nym& theOutput) const -> bool;
    auto IsCanceled() const -> bool { return m_bCanceled; }

    // When canceling a cron item before it
    // has been activated, use this.
    auto CancelBeforeActivation(
        const identity::Nym& theCancelerNym,
        const PasswordPrompt& reason) -> bool;

    // These are for     UnallocatedDeque<std::int64_t> m_dequeClosingNumbers;
    // They are numbers used for CLOSING a transaction. (finalReceipt.)
    auto GetClosingTransactionNoAt(std::uint32_t nIndex) const -> std::int64_t;
    auto GetCountClosingNumbers() const -> std::int32_t;

    void AddClosingTransactionNo(const std::int64_t& lClosingTransactionNo);

    // HIGHER LEVEL ABSTRACTIONS:
    auto GetOpeningNum() const -> std::int64_t;
    auto GetClosingNum() const -> std::int64_t;
    virtual auto IsValidOpeningNumber(const std::int64_t& lOpeningNum) const
        -> bool;

    virtual auto GetOpeningNumber(const identifier::Nym& theNymID) const
        -> std::int64_t;
    virtual auto GetClosingNumber(const Identifier& theAcctID) const
        -> std::int64_t;
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

protected:
    UnallocatedDeque<std::int64_t> m_dequeClosingNumbers;  // Numbers used for
                                                           // CLOSING a
                                                           // transaction.
                                                           // (finalReceipt.)
    OTNymID m_pCancelerNymID;

    bool m_bCanceled{false};  // This defaults to false. But if someone
                              // cancels it (BEFORE it is ever activated,
                              // just to nip it in the bud and harvest the
                              // numbers, and send the notices, etc) -- then
                              // we set this to true, and we also set the
                              // canceler Nym ID. (So we can see these
                              // values later and know whether it was
                              // canceled before activation, and if so, who
                              // did it.)

    bool m_bRemovalFlag{false};  // Set this to true and the cronitem will
                                 // be removed from Cron on next process.
    // (And its offer will be removed from the Market as well, if
    // appropriate.)
    virtual void onActivate([[maybe_unused]] const PasswordPrompt& reason) {
    }  // called by HookActivationOnCron().

    virtual void onFinalReceipt(
        OTCronItem& theOrigCronItem,
        const std::int64_t& lNewTransactionNumber,
        Nym_p theOriginator,
        Nym_p pRemover,
        const PasswordPrompt& reason) = 0;  // called by
                                            // HookRemovalFromCron().

    virtual void onRemovalFromCron(
        [[maybe_unused]] const PasswordPrompt& reason)
    {
    }  // called by HookRemovalFromCron().
    void ClearClosingNumbers();

    OTCronItem(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);
    OTCronItem(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const Identifier& ACCT_ID,
        const identifier::Nym& NYM_ID);
    OTCronItem(const api::Session& api);

private:
    using ot_super = OTTrackable;

    OTCron* m_pCron;
    Nym_p serverNym_;
    OTIdentifier notaryID_;
    Time m_CREATION_DATE;      // The date, in seconds, when the CronItem was
                               // authorized.
    Time m_LAST_PROCESS_DATE;  // The last time this item was processed.
    std::chrono::seconds m_PROCESS_INTERVAL;  // How often to Process Cron
                                              // on this item.

    OTCronItem() = delete;
    OTCronItem(const OTCronItem&) = delete;
    OTCronItem(OTCronItem&&) = delete;
    auto operator=(const OTCronItem&) -> OTCronItem& = delete;
    auto operator=(OTCronItem&&) -> OTCronItem& = delete;
};
}  // namespace opentxs
