// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <chrono>
#include <cstdint>

#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/recurring/OTAgreement.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
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

class Wallet;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
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
}  // namespace context
}  // namespace otx

class Identifier;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
/*
 OTPaymentPlan

 This instrument is signed by two parties or more (the first one, I think...)

 While processing payment, BOTH parties to a payment plan will be loaded up and
 their signatures will be checked against the original plan, which is saved as a
 cron receipt.

 There is also a "current version" of the payment plan, which contains updated
 info
 from processing, and is signed by the server.

 BOTH the original version, and the updated version, are sent to EACH user
 whenever
 a payment is processed, as his receipt. This way you have the user's signature
 on
 the terms, and the server's signature whenever it carries out the terms. A
 receipt
 with both is placed in the inbox of both users after any action.

 As with cheques, the server can use the receipts in the inboxes, plus the last
 agreed
 balance, to prove the current balance of any account. The user removes the
 receipt from
 his inbox by accepting it and, in the process, performing a new balance
 agreement.

 THIS MEANS that the OT server can carry out the terms of contracts!  So far, at
 least,
 cheques, trades, payment plans... as long as everything is signed off, we're
 free and
 clear under the same triple-signed system that account transfer uses. (The
 Users cannot
 repudiate their own signatures later, and the server can prove all balances
 with the
 user's own signature.)

 Of course, either side is free to CANCEL a payment plan, or to leave their
 account bereft
 of funds and prone to failed payments. But if they cancel, their signature will
 appear
 on the cancellation request, and the recipient gets a copy of it in his inbox.
 And if
 the funds are insufficient, the plan will keep trying to charge, leaving
 failure notices
 in both inboxes when such things occur.

 You could even have the server manage an issuer account, backed in payment plan
 revenue,
 that would form a new instrument definition that can then be traded on markets.
 (The same
 as you can
 have the server manage the issuer account for a basket currency now, which is
 backed with
 reserve accounts managed by the server, and you can then trade the basket
 currency on markets.)
 */
class OTPaymentPlan : public OTAgreement
{
public:
    // *********** Methods for generating a payment plan: ****************
    // From parent:  (This must be called first, before the other two methods
    // below can be called.)
    //
    //  bool        SetAgreement(const std::int64_t& lTransactionNum, const
    //  OTString&
    //  strConsideration,
    //                           const Time VALID_FROM=0,   const Time
    //                           VALID_TO=0);

    // Then call one (or both) of these:

    auto SetInitialPayment(
        const Amount lAmount,
        const std::chrono::seconds tTimeUntilInitialPayment = {})
        -> bool;  // default:
                  // now.

    // These two methods (above and below) can be called independent of each
    // other.
    //
    // Meaning: You can have an initial payment AND/OR a payment plan.

    auto SetPaymentPlan(
        const Amount& lPaymentAmount,
        const std::chrono::seconds tTimeUntilPlanStart =
            std::chrono::hours{24 * 30},
        const std::chrono::seconds tBetweenPayments =
            std::chrono::hours{24 * 30},  // Default: 30
                                          // days.
        const std::chrono::seconds tPlanLength = {},
        const std::int32_t nMaxPayments = 0) -> bool;

    // VerifyAgreement()
    // This function verifies both Nyms and both signatures. Due to the
    // peculiar nature of how OTAgreement/OTPaymentPlan works, there are two
    // signed copies stored. The merchant signs first, adding his
    // transaction numbers (2), and then he sends it to the customer, who
    // also adds two numbers and signs. (Also resetting the creation date.)
    // The problem is, adding the additional transaction numbers invalidates
    // the first (merchant's) signature. The solution is, when the customer
    // confirms the agreement, he stores an internal copy of the merchant's
    // signed version.  This way later, in VERIFY AGREEMENT, the internal
    // copy can be loaded, and BOTH Nyms can be checked to verify that BOTH
    // transaction numbers are valid for each. The two versions of the
    // contract can also be compared to each other, to make sure that none
    // of the vital terms, values, clauses, etc are different between the two.
    //
    auto VerifyAgreement(
        const otx::context::Client& recipient,
        const otx::context::Client& sender) const -> bool override;
    auto CompareAgreement(const OTAgreement& rh) const -> bool override;

    auto VerifyMerchantSignature(const identity::Nym& RECIPIENT_NYM) const
        -> bool;
    auto VerifyCustomerSignature(const identity::Nym& SENDER_NYM) const -> bool;

    // ************ "INITIAL PAYMENT" public GET METHODS **************
    inline auto HasInitialPayment() const -> bool { return m_bInitialPayment; }
    inline auto GetInitialPaymentDate() const -> const Time
    {
        return m_tInitialPaymentDate;
    }
    inline auto GetInitialPaymentAmount() const -> const Amount&
    {
        return m_lInitialPaymentAmount;
    }
    inline auto IsInitialPaymentDone() const -> bool
    {
        return m_bInitialPaymentDone;
    }

    inline auto GetInitialPaymentCompletedDate() const -> const Time
    {
        return m_tInitialPaymentCompletedDate;
    }
    inline auto GetLastFailedInitialPaymentDate() const -> const Time
    {
        return m_tFailedInitialPaymentDate;
    }
    inline auto GetNoInitialFailures() const -> std::int32_t
    {
        return m_nNumberInitialFailures;
    }

    // ************ "PAYMENT PLAN" public GET METHODS ****************
    inline auto HasPaymentPlan() const -> bool { return m_bPaymentPlan; }
    inline auto GetPaymentPlanAmount() const -> const Amount&
    {
        return m_lPaymentPlanAmount;
    }
    inline auto GetTimeBetweenPayments() const -> const std::chrono::seconds
    {
        return m_tTimeBetweenPayments;
    }
    inline auto GetPaymentPlanStartDate() const -> const Time
    {
        return m_tPaymentPlanStartDate;
    }
    inline auto GetPaymentPlanLength() const -> const std::chrono::seconds
    {
        return m_tPaymentPlanLength;
    }
    inline auto GetMaximumNoPayments() const -> std::int32_t
    {
        return m_nMaximumNoPayments;
    }

    inline auto GetDateOfLastPayment() const -> const Time
    {
        return m_tDateOfLastPayment;
    }
    inline auto GetDateOfLastFailedPayment() const -> const Time
    {
        return m_tDateOfLastFailedPayment;
    }

    inline auto GetNoPaymentsDone() const -> std::int32_t
    {
        return m_nNoPaymentsDone;
    }
    inline auto GetNoFailedPayments() const -> std::int32_t
    {
        return m_nNoFailedPayments;
    }

    // Return True if should stay on OTCron's list for more processing.
    // Return False if expired or otherwise should be removed.
    auto ProcessCron(const PasswordPrompt& reason)
        -> bool override;  // OTCron calls
                           // this regularly,
                           // which is my
                           // chance to
                           // expire, etc.
    void InitPaymentPlan();
    void Release() override;
    void Release_PaymentPlan();
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;
    void UpdateContents(const PasswordPrompt& reason)
        override;  // Before transmission or serialization,
                   // this
                   // is where the ledger saves its contents

    ~OTPaymentPlan() override;

private:
    friend api::session::imp::Factory;

    OTPaymentPlan(const api::Session& api);
    OTPaymentPlan(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);
    OTPaymentPlan(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const Identifier& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID);

protected:
    // "INITIAL PAYMENT" protected SET METHODS
    inline void SetInitialPaymentDate(const Time tInitialPaymentDate)
    {
        m_tInitialPaymentDate = tInitialPaymentDate;
    }
    inline void SetInitialPaymentAmount(const Amount& lAmount)
    {
        m_lInitialPaymentAmount = lAmount;
    }

    // Sets the bool that officially the initial payment has been done. (Checks
    // first to make sure not already done.)
    auto SetInitialPaymentDone() -> bool;

    inline void SetInitialPaymentCompletedDate(const Time tInitialPaymentDate)
    {
        m_tInitialPaymentCompletedDate = tInitialPaymentDate;
    }
    inline void SetLastFailedInitialPaymentDate(
        const Time tFailedInitialPaymentDate)
    {
        m_tFailedInitialPaymentDate = tFailedInitialPaymentDate;
    }

    inline void SetNoInitialFailures(const std::int32_t& nNoFailures)
    {
        m_nNumberInitialFailures = nNoFailures;
    }
    inline void IncrementNoInitialFailures() { m_nNumberInitialFailures++; }

    // "PAYMENT PLAN" protected SET METHODS
    inline void SetPaymentPlanAmount(const Amount& lAmount)
    {
        m_lPaymentPlanAmount = lAmount;
    }
    inline void SetTimeBetweenPayments(const std::chrono::seconds tTimeBetween)
    {
        m_tTimeBetweenPayments = tTimeBetween;
    }
    inline void SetPaymentPlanStartDate(const Time tPlanStartDate)
    {
        m_tPaymentPlanStartDate = tPlanStartDate;
    }
    inline void SetPaymentPlanLength(const std::chrono::seconds tPlanLength)
    {
        m_tPaymentPlanLength = tPlanLength;
    }
    inline void SetMaximumNoPayments(std::int32_t nMaxNoPayments)
    {
        m_nMaximumNoPayments = nMaxNoPayments;
    }

    inline void SetDateOfLastPayment(const Time tDateOfLast)
    {
        m_tDateOfLastPayment = tDateOfLast;
    }
    inline void SetDateOfLastFailedPayment(const Time tDateOfLast)
    {
        m_tDateOfLastFailedPayment = tDateOfLast;
    }

    inline void SetNoPaymentsDone(std::int32_t nNoPaymentsDone)
    {
        m_nNoPaymentsDone = nNoPaymentsDone;
    }
    inline void SetNoFailedPayments(std::int32_t nNoFailed)
    {
        m_nNoFailedPayments = nNoFailed;
    }

    inline void IncrementNoPaymentsDone() { m_nNoPaymentsDone++; }
    inline void IncrementNoFailedPayments() { m_nNoFailedPayments++; }

    auto ProcessPayment(
        const api::session::Wallet& wallet,
        const Amount& amount,
        const PasswordPrompt& reason) -> bool;
    void ProcessInitialPayment(
        const api::session::Wallet& wallet,
        const PasswordPrompt& reason);
    void ProcessPaymentPlan(
        const api::session::Wallet& wallet,
        const PasswordPrompt& reason);

private:
    using ot_super = OTAgreement;

    // "INITIAL PAYMENT" private MEMBERS
    bool m_bInitialPayment;      // Will there be an initial payment?
    Time m_tInitialPaymentDate;  // Date of the initial payment, measured
                                 // seconds after creation.
    Time m_tInitialPaymentCompletedDate;  // Date the initial payment was
                                          // finally transacted.
    Time m_tFailedInitialPaymentDate;     // Date of the last failed
                                          // payment, measured seconds after
                                          // creation.
    Amount m_lInitialPaymentAmount;       // Amount of the
                                          // initial payment.
    bool m_bInitialPaymentDone;           // Has the initial payment been made?
    std::int32_t m_nNumberInitialFailures;  // If we've tried to process this
                                            // multiple times, we'll know.
    // "PAYMENT PLAN" private MEMBERS
    bool m_bPaymentPlan;          // Will there be a payment plan?
    Amount m_lPaymentPlanAmount;  // Amount of each
                                  // payment.
    std::chrono::seconds m_tTimeBetweenPayments;  // How much time between
                                                  // each payment?
    Time m_tPaymentPlanStartDate;  // Date for the first payment plan
                                   // payment.
    std::chrono::seconds m_tPaymentPlanLength;  // Optional. Plan length
                                                // measured in seconds since
                                                // plan start.
    std::int32_t m_nMaximumNoPayments;          // Optional. The most number of
                                                // payments that are authorized.

    Time m_tDateOfLastPayment;         // Recording of date of the last payment.
    Time m_tDateOfLastFailedPayment;   // Recording of date of the last
                                       // failed payment.
    std::int32_t m_nNoPaymentsDone;    // Recording of the number of payments
                                       // already processed.
    std::int32_t m_nNoFailedPayments;  // Every time a payment fails, we
                                       // record that here.
    // These are NOT stored as part of the payment plan. They are merely used
    // during execution.
    bool m_bProcessingInitialPayment;
    bool m_bProcessingPaymentPlan;

    OTPaymentPlan() = delete;
};
}  // namespace opentxs
