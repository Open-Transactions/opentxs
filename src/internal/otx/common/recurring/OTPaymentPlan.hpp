// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <chrono>
#include <cstdint>

#include "internal/otx/common/recurring/OTAgreement.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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

class PasswordPrompt;
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
    inline auto HasInitialPayment() const -> bool { return initial_payment_; }
    inline auto GetInitialPaymentDate() const -> const Time
    {
        return initial_payment_date_;
    }
    inline auto GetInitialPaymentAmount() const -> const Amount&
    {
        return initial_payment_amount_;
    }
    inline auto IsInitialPaymentDone() const -> bool
    {
        return initial_payment_done_;
    }

    inline auto GetInitialPaymentCompletedDate() const -> const Time
    {
        return initial_payment_completed_date_;
    }
    inline auto GetLastFailedInitialPaymentDate() const -> const Time
    {
        return failed_initial_payment_date_;
    }
    inline auto GetNoInitialFailures() const -> std::int32_t
    {
        return number_initial_failures_;
    }

    // ************ "PAYMENT PLAN" public GET METHODS ****************
    inline auto HasPaymentPlan() const -> bool { return payment_plan_; }
    inline auto GetPaymentPlanAmount() const -> const Amount&
    {
        return payment_plan_amount_;
    }
    inline auto GetTimeBetweenPayments() const -> const std::chrono::seconds
    {
        return time_between_payments_;
    }
    inline auto GetPaymentPlanStartDate() const -> const Time
    {
        return payment_plan_start_date_;
    }
    inline auto GetPaymentPlanLength() const -> const std::chrono::seconds
    {
        return payment_plan_length_;
    }
    inline auto GetMaximumNoPayments() const -> std::int32_t
    {
        return maximum_no_payments_;
    }

    inline auto GetDateOfLastPayment() const -> const Time
    {
        return date_of_last_payment_;
    }
    inline auto GetDateOfLastFailedPayment() const -> const Time
    {
        return date_of_last_failed_payment_;
    }

    inline auto GetNoPaymentsDone() const -> std::int32_t
    {
        return no_payments_done_;
    }
    inline auto GetNoFailedPayments() const -> std::int32_t
    {
        return no_failed_payments_;
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

    OTPaymentPlan() = delete;

    ~OTPaymentPlan() override;

private:
    friend api::session::FactoryPrivate;

    OTPaymentPlan(const api::Session& api);
    OTPaymentPlan(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);
    OTPaymentPlan(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::Account& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const identifier::Account& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID);

protected:
    // "INITIAL PAYMENT" protected SET METHODS
    inline void SetInitialPaymentDate(const Time tInitialPaymentDate)
    {
        initial_payment_date_ = tInitialPaymentDate;
    }
    inline void SetInitialPaymentAmount(const Amount& lAmount)
    {
        initial_payment_amount_ = lAmount;
    }

    // Sets the bool that officially the initial payment has been done. (Checks
    // first to make sure not already done.)
    auto SetInitialPaymentDone() -> bool;

    inline void SetInitialPaymentCompletedDate(const Time tInitialPaymentDate)
    {
        initial_payment_completed_date_ = tInitialPaymentDate;
    }
    inline void SetLastFailedInitialPaymentDate(
        const Time tFailedInitialPaymentDate)
    {
        failed_initial_payment_date_ = tFailedInitialPaymentDate;
    }

    inline void SetNoInitialFailures(const std::int32_t& nNoFailures)
    {
        number_initial_failures_ = nNoFailures;
    }
    inline void IncrementNoInitialFailures() { number_initial_failures_++; }

    // "PAYMENT PLAN" protected SET METHODS
    inline void SetPaymentPlanAmount(const Amount& lAmount)
    {
        payment_plan_amount_ = lAmount;
    }
    inline void SetTimeBetweenPayments(const std::chrono::seconds tTimeBetween)
    {
        time_between_payments_ = tTimeBetween;
    }
    inline void SetPaymentPlanStartDate(const Time tPlanStartDate)
    {
        payment_plan_start_date_ = tPlanStartDate;
    }
    inline void SetPaymentPlanLength(const std::chrono::seconds tPlanLength)
    {
        payment_plan_length_ = tPlanLength;
    }
    inline void SetMaximumNoPayments(std::int32_t nMaxNoPayments)
    {
        maximum_no_payments_ = nMaxNoPayments;
    }

    inline void SetDateOfLastPayment(const Time tDateOfLast)
    {
        date_of_last_payment_ = tDateOfLast;
    }
    inline void SetDateOfLastFailedPayment(const Time tDateOfLast)
    {
        date_of_last_failed_payment_ = tDateOfLast;
    }

    inline void SetNoPaymentsDone(std::int32_t nNoPaymentsDone)
    {
        no_payments_done_ = nNoPaymentsDone;
    }
    inline void SetNoFailedPayments(std::int32_t nNoFailed)
    {
        no_failed_payments_ = nNoFailed;
    }

    inline void IncrementNoPaymentsDone() { no_payments_done_++; }
    inline void IncrementNoFailedPayments() { no_failed_payments_++; }

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
    bool initial_payment_;       // Will there be an initial payment?
    Time initial_payment_date_;  // Date of the initial payment, measured
                                 // seconds after creation.
    Time initial_payment_completed_date_;  // Date the initial payment was
                                           // finally transacted.
    Time failed_initial_payment_date_;     // Date of the last failed
                                           // payment, measured seconds after
                                           // creation.
    Amount initial_payment_amount_;        // Amount of the
                                           // initial payment.
    bool initial_payment_done_;            // Has the initial payment been made?
    std::int32_t number_initial_failures_;  // If we've tried to process this
                                            // multiple times, we'll know.
    // "PAYMENT PLAN" private MEMBERS
    bool payment_plan_;           // Will there be a payment plan?
    Amount payment_plan_amount_;  // Amount of each
                                  // payment.
    std::chrono::seconds time_between_payments_;  // How much time between
                                                  // each payment?
    Time payment_plan_start_date_;  // Date for the first payment plan
                                    // payment.
    std::chrono::seconds payment_plan_length_;  // Optional. Plan length
                                                // measured in seconds since
                                                // plan start.
    std::int32_t maximum_no_payments_;          // Optional. The most number of
                                                // payments that are authorized.

    Time date_of_last_payment_;  // Recording of date of the last payment.
    Time date_of_last_failed_payment_;  // Recording of date of the last
                                        // failed payment.
    std::int32_t no_payments_done_;     // Recording of the number of payments
                                        // already processed.
    std::int32_t no_failed_payments_;   // Every time a payment fails, we
                                        // record that here.
    // These are NOT stored as part of the payment plan. They are merely used
    // during execution.
    bool processing_initial_payment_;
    bool processing_payment_plan_;
};
}  // namespace opentxs
