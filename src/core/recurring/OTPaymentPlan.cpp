// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/recurring/OTPaymentPlan.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/recurring/OTAgreement.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"

#include <irrxml/irrXML.hpp>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <memory>
#include <ostream>
#include <string>

#define OT_METHOD "opentxs::OTPaymentPlan"

// return -1 if error, 0 if nothing, and 1 if the node was processed.

namespace opentxs
{
OTPaymentPlan::OTPaymentPlan(const api::Core& core)
    : ot_super(core)
    , m_bProcessingInitialPayment(false)
    , m_bProcessingPaymentPlan(false)
{
    InitPaymentPlan();
}

OTPaymentPlan::OTPaymentPlan(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID)
    : ot_super(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_bProcessingInitialPayment(false)
    , m_bProcessingPaymentPlan(false)
{
    InitPaymentPlan();
}

OTPaymentPlan::OTPaymentPlan(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& SENDER_ACCT_ID,
    const Identifier& SENDER_NYM_ID,
    const Identifier& RECIPIENT_ACCT_ID,
    const Identifier& RECIPIENT_NYM_ID)
    : ot_super(
          core,
          NOTARY_ID,
          INSTRUMENT_DEFINITION_ID,
          SENDER_ACCT_ID,
          SENDER_NYM_ID,
          RECIPIENT_ACCT_ID,
          RECIPIENT_NYM_ID)
    , m_bProcessingInitialPayment(false)
    , m_bProcessingPaymentPlan(false)
{
    InitPaymentPlan();
}

std::int32_t OTPaymentPlan::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    std::int32_t nReturnVal = 0;

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    if (0 != (nReturnVal = ot_super::ProcessXMLNode(xml))) return nReturnVal;

    // Note: the closing transaction numbers are read in
    // OTCronItem::ProcessXMLNode,
    // and similarly, the closing Recipient transaction numbers are read in
    // OTAgreement::ProcessXMLNode.

    if (!strcmp("initialPayment", xml->getNodeName())) {
        // Yes, there IS apparently an initial payment. We can set the bool to
        // true.
        m_bInitialPayment = true;

        SetInitialPaymentDate(parseTimestamp(xml->getAttributeValue("date")));
        SetInitialPaymentCompletedDate(
            parseTimestamp(xml->getAttributeValue("dateCompleted")));
        SetLastFailedInitialPaymentDate(
            parseTimestamp(xml->getAttributeValue("dateOfLastAttempt")));

        SetNoInitialFailures(atoi(xml->getAttributeValue("numberOfAttempts")));
        SetInitialPaymentAmount(
            String::StringToLong(xml->getAttributeValue("amount")));

        auto strCompleted =
            String::Factory(xml->getAttributeValue("completed"));
        m_bInitialPaymentDone = strCompleted->Compare("true");

        LogDetail(OT_METHOD)(__FUNCTION__)(": Initial Payment. Amount: ")(
            m_lInitialPaymentAmount)(". Date: ")(GetInitialPaymentDate())(
            ". Completed Date: ")(GetInitialPaymentCompletedDate())(
            ". Number of failed attempts: ")(m_nNumberInitialFailures)(
            ". Date of last failed attempt: ")(
            GetLastFailedInitialPaymentDate())(". Payment ")(
            m_bInitialPaymentDone ? "COMPLETED" : "NOT completed")(".")
            .Flush();

        nReturnVal = 1;
    } else if (!strcmp("paymentPlan", xml->getNodeName())) {
        // Yes, there IS apparently a payment plan. We can set the bool to true.
        m_bPaymentPlan = true;

        SetPaymentPlanAmount(
            String::StringToLong(xml->getAttributeValue("amountPerPayment")));
        SetMaximumNoPayments(atoi(xml->getAttributeValue("maxNoPayments")));
        SetNoPaymentsDone(atoi(xml->getAttributeValue("completedNoPayments")));
        SetNoFailedPayments(atoi(xml->getAttributeValue("failedNoPayments")));

        const auto str_between =
            String::Factory(xml->getAttributeValue("timeBetweenPayments"));
        const auto str_start =
            String::Factory(xml->getAttributeValue("planStartDate"));
        const auto str_length =
            String::Factory(xml->getAttributeValue("planLength"));
        const auto str_last =
            String::Factory(xml->getAttributeValue("dateOfLastPayment"));
        const auto str_last_attempt =
            String::Factory(xml->getAttributeValue("dateOfLastFailedPayment"));

        std::int64_t tBetween = str_between->ToLong();
        std::int64_t tStart = parseTimestamp(str_start->Get());
        std::int64_t tLength = str_length->ToLong();
        std::int64_t tLast = parseTimestamp(str_last->Get());
        std::int64_t tLastAttempt = parseTimestamp(str_last_attempt->Get());

        SetTimeBetweenPayments(OTTimeGetTimeFromSeconds(tBetween));
        SetPaymentPlanStartDate(OTTimeGetTimeFromSeconds(tStart));
        SetPaymentPlanLength(OTTimeGetTimeFromSeconds(tLength));
        SetDateOfLastPayment(OTTimeGetTimeFromSeconds(tLast));
        SetDateOfLastFailedPayment(OTTimeGetTimeFromSeconds(tLastAttempt));

        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Payment Plan. Amount per payment: ")(m_lPaymentPlanAmount)(
            ". Time between payments: ")(tBetween)(
            ". Payment plan Start Date: ")(tStart)(". Length: ")(tLength)(
            ". Maximum No. of Payments: ")(m_nMaximumNoPayments)(
            ". Completed No. of Payments: ")(m_nNoPaymentsDone)(
            ". Failed No. of Payments: ")(m_nNoFailedPayments)(
            ". Date of last payment: ")(tLast)(
            ". Date of last failed payment: ")(tLastAttempt)(".")
            .Flush();

        nReturnVal = 1;
    }

    return nReturnVal;
}

void OTPaymentPlan::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    const auto NOTARY_ID = String::Factory(GetNotaryID()),
               INSTRUMENT_DEFINITION_ID =
                   String::Factory(GetInstrumentDefinitionID()),
               SENDER_ACCT_ID = String::Factory(GetSenderAcctID()),
               SENDER_NYM_ID = String::Factory(GetSenderNymID()),
               RECIPIENT_ACCT_ID = String::Factory(GetRecipientAcctID()),
               RECIPIENT_NYM_ID = String::Factory(GetRecipientNymID());

    OT_ASSERT(!m_pCancelerNymID->empty());

    auto strCanceler = String::Factory();

    if (m_bCanceled) m_pCancelerNymID->GetString((strCanceler));

    // OTAgreement
    Tag tag("agreement");

    tag.add_attribute("version", m_strVersion->Get());
    tag.add_attribute("notaryID", NOTARY_ID->Get());
    tag.add_attribute(
        "instrumentDefinitionID", INSTRUMENT_DEFINITION_ID->Get());
    tag.add_attribute("senderAcctID", SENDER_ACCT_ID->Get());
    tag.add_attribute("senderNymID", SENDER_NYM_ID->Get());
    tag.add_attribute("recipientAcctID", RECIPIENT_ACCT_ID->Get());
    tag.add_attribute("recipientNymID", RECIPIENT_NYM_ID->Get());
    tag.add_attribute("canceled", formatBool(m_bCanceled));
    tag.add_attribute("cancelerNymID", m_bCanceled ? strCanceler->Get() : "");
    tag.add_attribute("transactionNum", formatLong(m_lTransactionNum));
    tag.add_attribute("creationDate", formatTimestamp(GetCreationDate()));
    tag.add_attribute("validFrom", formatTimestamp(GetValidFrom()));
    tag.add_attribute("validTo", formatTimestamp(GetValidTo()));

    // There are "closing" transaction numbers, used to CLOSE a transaction.
    // Often where Cron items are involved such as this payment plan, or in
    // baskets, where many asset accounts are involved and require receipts
    // to be closed out.

    // OTCronItem
    for (std::int32_t i = 0; i < GetCountClosingNumbers(); i++) {
        std::int64_t lClosingNumber = GetClosingTransactionNoAt(i);
        OT_ASSERT(lClosingNumber > 0);

        TagPtr tagClosingNo(new Tag("closingTransactionNumber"));
        tagClosingNo->add_attribute("value", formatLong(lClosingNumber));
        tag.add_tag(tagClosingNo);
    }

    // OTAgreement
    // For the recipient, his OPENING *and* CLOSING transaction numbers go on
    // this list. (For sender, the "opening" number is the GetTransactionNum()
    // on this object, and the "closing" number is in the above list.)
    for (std::int32_t i = 0; i < GetRecipientCountClosingNumbers(); i++) {
        std::int64_t lClosingNumber = GetRecipientClosingTransactionNoAt(i);
        OT_ASSERT(lClosingNumber > 0);

        TagPtr tagClosingNo(new Tag("closingRecipientNumber"));
        tagClosingNo->add_attribute("value", formatLong(lClosingNumber));
        tag.add_tag(tagClosingNo);
    }

    // OTPaymentPlan
    if (HasInitialPayment()) {
        TagPtr tagInitial(new Tag("initialPayment"));

        tagInitial->add_attribute(
            "date", formatTimestamp(GetInitialPaymentDate()));
        tagInitial->add_attribute(
            "amount", formatLong(GetInitialPaymentAmount()));
        tagInitial->add_attribute(
            "numberOfAttempts", formatInt(GetNoInitialFailures()));
        tagInitial->add_attribute(
            "dateOfLastAttempt",
            formatTimestamp(GetLastFailedInitialPaymentDate()));
        tagInitial->add_attribute(
            "dateCompleted", formatTimestamp(GetInitialPaymentCompletedDate()));
        tagInitial->add_attribute(
            "completed", formatBool(IsInitialPaymentDone()));

        tag.add_tag(tagInitial);
    }

    // OTPaymentPlan
    if (HasPaymentPlan()) {
        const std::int64_t lTimeBetween =
            OTTimeGetSecondsFromTime(GetTimeBetweenPayments());
        const std::int64_t lPlanLength =
            OTTimeGetSecondsFromTime(GetPaymentPlanLength());

        TagPtr tagPlan(new Tag("paymentPlan"));

        tagPlan->add_attribute(
            "amountPerPayment", formatLong(GetPaymentPlanAmount()));
        tagPlan->add_attribute("timeBetweenPayments", formatLong(lTimeBetween));
        tagPlan->add_attribute(
            "planStartDate", formatTimestamp(GetPaymentPlanStartDate()));
        tagPlan->add_attribute("planLength", formatLong(lPlanLength));
        tagPlan->add_attribute(
            "maxNoPayments", formatInt(GetMaximumNoPayments()));
        tagPlan->add_attribute(
            "completedNoPayments", formatInt(GetNoPaymentsDone()));
        tagPlan->add_attribute(
            "failedNoPayments", formatInt(GetNoFailedPayments()));
        tagPlan->add_attribute(
            "dateOfLastPayment", formatTimestamp(GetDateOfLastPayment()));
        tagPlan->add_attribute(
            "dateOfLastFailedPayment",
            formatTimestamp(GetDateOfLastFailedPayment()));

        tag.add_tag(tagPlan);
    }

    // OTAgreement
    if (m_strConsideration->Exists()) {
        auto ascTemp = Armored::Factory(m_strConsideration);
        tag.add_tag("consideration", ascTemp->Get());
    }

    // OTAgreement
    if (m_strMerchantSignedCopy->Exists()) {
        auto ascTemp = Armored::Factory(m_strMerchantSignedCopy);
        tag.add_tag("merchantSignedCopy", ascTemp->Get());
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate("%s", str_result.c_str());
}

// *** Set Initial Payment ***  / Make sure to call SetAgreement() first.

bool OTPaymentPlan::SetInitialPayment(
    const std::int64_t& amount,
    time64_t tTimeUntilInitialPayment)
{
    m_bInitialPayment = true;       // There is now an initial payment.
    m_bInitialPaymentDone = false;  // It has not yet been paid.

    // The initial date passed it is measured relative to the creation date.
    // (Assumes SetAgreement() was already called...)
    const time64_t INITIAL_PAYMENT_DATE = OTTimeAddTimeInterval(
        GetCreationDate(), OTTimeGetSecondsFromTime(tTimeUntilInitialPayment));

    SetInitialPaymentDate(INITIAL_PAYMENT_DATE);

    SetInitialPaymentAmount(amount);

    return true;
}

bool OTPaymentPlan::CompareAgreement(const OTAgreement& rhs) const
{
    if (!ot_super::CompareAgreement(rhs)) return false;

    // Compare OTPaymentPlan specific info here.
    const OTPaymentPlan* pPlan = dynamic_cast<const OTPaymentPlan*>(&rhs);

    if ((nullptr != pPlan) &&
        (HasInitialPayment() == pPlan->HasInitialPayment()) &&
        (GetInitialPaymentDate() == pPlan->GetInitialPaymentDate()) &&
        (GetInitialPaymentAmount() == pPlan->GetInitialPaymentAmount()) &&
        (HasPaymentPlan() == pPlan->HasPaymentPlan()) &&
        (GetPaymentPlanAmount() == pPlan->GetPaymentPlanAmount()) &&
        (GetTimeBetweenPayments() == pPlan->GetTimeBetweenPayments()) &&
        (GetPaymentPlanStartDate() == pPlan->GetPaymentPlanStartDate()) &&
        (GetPaymentPlanLength() == pPlan->GetPaymentPlanLength()) &&
        (GetMaximumNoPayments() == pPlan->GetMaximumNoPayments()))
        return true;

    return false;
}

bool OTPaymentPlan::VerifyMerchantSignature(const Nym& RECIPIENT_NYM) const
{
    // Load up the merchant's copy.
    OTPaymentPlan theMerchantCopy{api_};
    if (!m_strMerchantSignedCopy->Exists() ||
        !theMerchantCopy.LoadContractFromString(m_strMerchantSignedCopy)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Expected Merchant's signed copy to be inside the "
            "payment plan, but unable to load.")
            .Flush();
        return false;
    }

    // Compare this against the merchant's copy using Compare function.
    if (!Compare(theMerchantCopy)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Merchant's copy of payment plan isn't equal to Customer's "
            "copy.")
            .Flush();
        return false;
    }

    // Verify recipient's signature on merchant's copy.
    //
    if (!theMerchantCopy.VerifySignature(RECIPIENT_NYM)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Merchant's signature failed to verify on "
            "internal merchant copy of agreement.")
            .Flush();
        return false;
    }

    return true;
}

bool OTPaymentPlan::VerifyCustomerSignature(const Nym& SENDER_NYM) const
{
    if (!VerifySignature(SENDER_NYM)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Customer's signature failed to verify.")
            .Flush();
        return false;
    }

    return true;
}

// This function assumes that it is the customer's copy, with the customer's
// transaction numbers, and that the merchant's copy is attached within. The
// function tries to verify they are the same, and properly signed.
bool OTPaymentPlan::VerifyAgreement(
    const ClientContext& recipient,
    const ClientContext& sender) const
{
    if (!VerifyMerchantSignature(recipient.RemoteNym())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Merchant's signature failed to verify.")
            .Flush();

        return false;
    }

    if (!VerifyCustomerSignature(sender.RemoteNym())) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Customer's signature failed to verify.")
            .Flush();

        return false;
    }

    // Verify Transaction Num and Closing Nums against SENDER's issued list
    if ((GetCountClosingNumbers() < 1) ||
        !sender.VerifyIssuedNumber(GetTransactionNum())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Transaction number ")(
            GetTransactionNum())(" isn't on sender's issued list, "
                                 "OR there weren't enough closing numbers.")
            .Flush();

        return false;
    }

    for (std::int32_t i = 0; i < GetCountClosingNumbers(); i++)
        if (!sender.VerifyIssuedNumber(GetClosingTransactionNoAt(i))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Closing transaction number ")(
                GetClosingTransactionNoAt(i))(" isn't on sender's issued list.")
                .Flush();

            return false;
        }

    // Verify Recipient closing numbers against RECIPIENT's issued list.
    if (GetRecipientCountClosingNumbers() < 2) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed verifying: "
            "Expected opening and closing transaction numbers for "
            "recipient. (2 total).")
            .Flush();

        return false;
    }

    for (std::int32_t i = 0; i < GetRecipientCountClosingNumbers(); i++)
        if (!recipient.VerifyIssuedNumber(
                GetRecipientClosingTransactionNoAt(i))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Recipient's Closing transaction number ")(
                GetRecipientClosingTransactionNoAt(i))(
                " isn't on recipient's issued list.")
                .Flush();

            return false;
        }

    return true;  // Success!
}

// *** Set Payment Plan *** / Make sure to call SetAgreement() first.
// default: 1st payment in 30 days
bool OTPaymentPlan::SetPaymentPlan(
    const std::int64_t& lPaymentAmount,
    time64_t tTimeUntilPlanStart,
    time64_t tBetweenPayments,  // Default: 30
                                // days.
    time64_t tPlanLength,
    std::int32_t nMaxPayments)
{

    if (lPaymentAmount <= 0) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Payment Amount less than zero.")
            .Flush();
        return false;
    }

    SetPaymentPlanAmount(lPaymentAmount);

    // Note: this is just a safety mechanism. And while it should be turned back
    // ON at some point,
    //       I need it off for testing at the moment. So if you're reading this
    // now, go ahead and uncomment the below code so it is functional again.

    //    if (tBetweenPayments < OT_TIME_DAY_IN_SECONDS) // If the time between
    //    payments is set
    //                                                   // to LESS than a
    //                                                   24-hour day, then
    //        tBetweenPayments = OT_TIME_DAY_IN_SECONDS; // set the minimum time
    //        to 24 hours.
    //                                                   // this is a safety
    //                                                   mechanism.
    //    if (tBetweenPayments < 10) // Every 10 seconds (still need some kind
    // of standard, even while testing.)
    //        tBetweenPayments = 10; // TODO: Remove this and uncomment the
    //        above code.
    //
    // TODO possible make it possible to configure this in the currency contract
    // itself.Rece

    SetTimeBetweenPayments(tBetweenPayments);

    // Assuming no need to check a time64_t for <0 since it's probably
    // unsigned...
    const time64_t PAYMENT_PLAN_START = OTTimeAddTimeInterval(
        GetCreationDate(), OTTimeGetSecondsFromTime(tTimeUntilPlanStart));

    SetPaymentPlanStartDate(PAYMENT_PLAN_START);

    // Is this even a problem? todo: verify that time64_t is unisigned.
    if (OT_TIME_ZERO > tPlanLength)  // if it's a negative number...
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Attempt to use negative number for plan length.")
            .Flush();
        return false;
    }

    SetPaymentPlanLength(tPlanLength);  // any zero (no expiry) or above-zero
                                        // value will do.

    if (0 > nMaxPayments)  // if it's a negative number...
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Attempt to use negative number for plan max payments.")
            .Flush();
        return false;
    }

    SetMaximumNoPayments(nMaxPayments);  // any zero (no expiry) or above-zero
                                         // value will do.

    // Set these to zero, they will be incremented later at the right times.
    m_tDateOfLastPayment = OT_TIME_ZERO;
    m_nNoPaymentsDone = 0;

    // Okay, we're a payment plan! (Still need to add the object to OTCron...
    // But it's ready...)
    m_bPaymentPlan = true;

    return true;
}

bool OTPaymentPlan::SetInitialPaymentDone()
{
    if (m_bInitialPaymentDone)  // if done already.
        return false;

    m_bInitialPaymentDone = true;
    // We store the bool that it's done (above), and we also store the date when
    // it was done:
    SetInitialPaymentCompletedDate(OTTimeGetCurrentTime());

    return true;
}

// TODO: Run a scanner on the code for memory leaks and buffer overflows.

// This can be called by either the initial payment code, or by the payment plan
// code. true == success, false == failure.
bool OTPaymentPlan::ProcessPayment(
    const api::Wallet& wallet,
    const Amount& amount)
{
    const OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    auto pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    bool bSuccess = false;  // The return value.

    const auto NOTARY_ID = Identifier::Factory(pCron->GetNotaryID());
    const auto NOTARY_NYM_ID = Identifier::Factory(*pServerNym);

    const Identifier& SOURCE_ACCT_ID = GetSenderAcctID();
    const Identifier& SENDER_NYM_ID = GetSenderNymID();

    const Identifier& RECIPIENT_ACCT_ID = GetRecipientAcctID();
    const Identifier& RECIPIENT_NYM_ID = GetRecipientNymID();

    auto strSenderNymID = String::Factory(SENDER_NYM_ID),
         strRecipientNymID = String::Factory(RECIPIENT_NYM_ID),
         strSourceAcctID = String::Factory(SOURCE_ACCT_ID),
         strRecipientAcctID = String::Factory(RECIPIENT_ACCT_ID);

    // Make sure they're not the same Account IDs ...
    // Otherwise we would have to take care not to load them twice, like with
    // the Nyms below.
    // (Instead I just disallow it entirely.)
    if (SOURCE_ACCT_ID == RECIPIENT_ACCT_ID) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed to process payment: both account IDs were identical.")
            .Flush();
        FlagForRemoval();  // Remove from Cron
        return false;      // TODO: should have a "Verify Payment Plan" function
                           // that
                           // weeds this crap out before we even get here.
    }
    // When the accounts are actually loaded up, then we should also compare
    // the instrument definitions to make sure they were what we expected them
    // to be.

    // Need to load up the ORIGINAL PAYMENT PLAN (with BOTH users' original
    // SIGNATURES on it!)
    // Will need to verify those signatures as well as attach a copy of it to
    // the receipt.

    // OTCronItem::LoadCronReceipt loads the original version with the user's
    // signature.
    // (Updated versions, as processing occurs, are signed by the server.)
    std::unique_ptr<OTCronItem> pOrigCronItem{
        OTCronItem::LoadCronReceipt(api_, GetTransactionNum())};

    OT_ASSERT(false != bool(pOrigCronItem));  // How am I processing it now if
                                              // the receipt wasn't saved in the
                                              // first place??
    // TODO: Decide global policy for handling situations where the hard drive
    // stops working, etc.

    // strOrigPlan is a String copy (a PGP-signed XML file, in string form) of
    // the original Payment Plan request...
    auto strOrigPlan =
        String::Factory(*pOrigCronItem);  // <====== Farther down in the code, I
                                          // attach this string to the receipts.

    // -------------- Make sure have both nyms loaded and checked out.
    // --------------------------------------------------
    // WARNING: 1 or both of the Nyms could be also the Server Nym. They could
    // also be the same Nym, but NOT the Server.
    // In all of those different cases, I don't want to load the same file twice
    // and overwrite it with itself, losing
    // half of all my changes. I have to check all three IDs carefully and set
    // the pointers accordingly, and then operate
    // using the pointers from there.

    // We MIGHT use ONE, OR BOTH, of these, or none. (But probably both.)

    // Find out if either Nym is actually also the server.
    bool bSenderNymIsServerNym =
        ((SENDER_NYM_ID == NOTARY_NYM_ID) ? true : false);
    bool bRecipientNymIsServerNym =
        ((RECIPIENT_NYM_ID == NOTARY_NYM_ID) ? true : false);

    // We also see, after all that is done, whether both pointers go to the same
    // entity.
    // (We'll want to know that later.)
    bool bUsersAreSameNym =
        ((SENDER_NYM_ID == RECIPIENT_NYM_ID) ? true : false);

    ConstNym pSenderNym = nullptr;
    ConstNym pRecipientNym = nullptr;

    // Figure out if Sender Nym is also Server Nym.
    if (bSenderNymIsServerNym) {
        // If the First Nym is the server, then just point to that.
        pSenderNym = pServerNym;
    } else  // Else load the First Nym from storage.
    {
        pSenderNym = api_.Wallet().Nym(SENDER_NYM_ID);
        if (nullptr == pSenderNym) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failure loading Sender Nym in: ")(SENDER_NYM_ID)(".")
                .Flush();
            FlagForRemoval();  // Remove it from future Cron processing, please.
            return false;
        }
    }

    // Next, we also find out if Recipient Nym is Server Nym...
    if (bRecipientNymIsServerNym) {
        // If the Recipient Nym is the server, then just point to that.
        pRecipientNym = pServerNym;
    } else if (bUsersAreSameNym)  // Else if the participants are the same Nym,
                                  // point to the one we already loaded.
    {
        pRecipientNym = pSenderNym;  // theSenderNym is pSenderNym
    } else  // Otherwise load the Other Nym from Disk and point to that.
    {
        pRecipientNym = api_.Wallet().Nym(RECIPIENT_NYM_ID);
        if (nullptr == pRecipientNym) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failure loading Recipient Nym in: ")(RECIPIENT_NYM_ID)(".")
                .Flush();
            FlagForRemoval();  // Remove it from future Cron processing, please.
            return false;
        }
    }

    // Now that I have the original Payment Plan loaded, and all the Nyms
    // ready to go, let's make sure that BOTH the nyms in question have
    // SIGNED the original request. FYI, their signatures wouldn't be on the
    // updated version in Cron--the server signs that one--which is *this.

    OTPaymentPlan* pPlan = dynamic_cast<OTPaymentPlan*>(pOrigCronItem.get());

    if ((nullptr == pPlan) || !pPlan->VerifyMerchantSignature(*pRecipientNym) ||
        !pPlan->VerifyCustomerSignature(*pSenderNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed verifying original signatures on Payment plan (while "
            "attempting to process...) "
            "Flagging for removal.")
            .Flush();
        FlagForRemoval();  // Remove it from Cron.
        return false;
    }

    // AT THIS POINT, I have pServerNym, pSenderNym, and pRecipientNym. ALL
    // are loaded from disk (where necessary.) AND... ALL are valid
    // pointers, (even if they sometimes point to the same object,) AND none
    // are capable of overwriting the storage of the other (by accidentally
    // loading the same storage twice.) We also have boolean variables at
    // this point to tell us exactly which are which, (in case some of those
    // pointers do go to the same object.) They are: bSenderNymIsServerNym,
    // bRecipientNymIsServerNym, and bUsersAreSameNym.
    //
    // I also have pOrigCronItem, which is a dynamically-allocated copy of
    // the original Cron Receipt for this Payment Plan. (And I don't need to
    // worry about deleting it, either.) I know for a fact they have both
    // signed pOrigCronItem...

    auto sourceAccount = wallet.mutable_Account(SOURCE_ACCT_ID);

    if (false == bool(sourceAccount)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR verifying existence of source account during attempted "
            "payment plan processing.")
            .Flush();
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    }

    auto recipientAccount = wallet.mutable_Account(RECIPIENT_ACCT_ID);

    if (false == bool(recipientAccount)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR verifying existence of recipient account during "
            "attempted payment plan processing.")
            .Flush();
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    }

    // BY THIS POINT, both accounts are successfully loaded, and I don't have to
    // worry about cleaning either one of them up, either. But I can now use
    // sourceAccount and recipientAccount...

    // A few verification if/elses...

    // Are both accounts of the same Asset Type?
    if (sourceAccount.get().GetInstrumentDefinitionID() !=
        recipientAccount.get().GetInstrumentDefinitionID()) {  // We already
                                                               // know the
                                                               // SUPPOSED
        // Instrument Definition Ids of these accounts...
        // But only once
        // the accounts THEMSELVES have been loaded can we VERIFY this to be
        // true.
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR - attempted payment between accounts of different "
            "instrument definitions.")
            .Flush();
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    }

    // Make sure all accounts are signed by the server and have the owner they
    // are expected to have.

    // I call VerifySignature here since VerifyContractID was already called in
    // LoadExistingAccount().
    else if (
        !sourceAccount.get().VerifyOwner(*pSenderNym) ||
        !sourceAccount.get().VerifySignature(*pServerNym)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR verifying ownership or signature on source account.")
            .Flush();

        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    } else if (
        !recipientAccount.get().VerifyOwner(*pRecipientNym) ||
        !recipientAccount.get().VerifySignature(*pServerNym)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR verifying ownership or signature on "
            "recipient account.")
            .Flush();
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    }

    // By this point, I know I have both accounts loaded, and I know that they
    // have the right instrument definitions,
    // and I know they have the right owners and they were all signed by the
    // server.
    // I also know that their account IDs in their internal records matched the
    // account filename for each acct.
    // I also have pointers to the Nyms who own these accounts.
    else {
        // Okay then, everything checks out. Let's add a receipt to the sender's
        // outbox and the recipient's inbox.
        // IF they can be loaded up from file, or generated, that is.

        // Load the inbox/outbox in case they already exist
        auto theSenderInbox{
            api_.Factory().Ledger(SENDER_NYM_ID, SOURCE_ACCT_ID, NOTARY_ID)};
        auto theRecipientInbox{api_.Factory().Ledger(
            RECIPIENT_NYM_ID, RECIPIENT_ACCT_ID, NOTARY_ID)};

        // ALL inboxes -- no outboxes. All will receive notification of
        // something ALREADY DONE.
        bool bSuccessLoadingSenderInbox = theSenderInbox->LoadInbox();
        bool bSuccessLoadingRecipientInbox = theRecipientInbox->LoadInbox();

        // ...or generate them otherwise...
        //
        if (true == bSuccessLoadingSenderInbox)
            bSuccessLoadingSenderInbox =
                theSenderInbox->VerifyAccount(*pServerNym);
        else
            bSuccessLoadingSenderInbox = theSenderInbox->GenerateLedger(
                SOURCE_ACCT_ID,
                NOTARY_ID,
                ledgerType::inbox,
                true);  // bGenerateFile=true

        if (true == bSuccessLoadingRecipientInbox)
            bSuccessLoadingRecipientInbox =
                theRecipientInbox->VerifyAccount(*pServerNym);
        else
            bSuccessLoadingRecipientInbox = theRecipientInbox->GenerateLedger(
                RECIPIENT_ACCT_ID,
                NOTARY_ID,
                ledgerType::inbox,
                true);  // bGenerateFile=true

        if ((false == bSuccessLoadingSenderInbox) ||
            (false == bSuccessLoadingRecipientInbox)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": ERROR loading or generating inbox ledger.")
                .Flush();
        } else {
            // Generate new transaction numbers for these new transactions
            std::int64_t lNewTransactionNumber =
                GetCron()->GetNextTransactionNumber();

            //            OT_ASSERT(lNewTransactionNumber > 0); // this can be
            // my reminder.
            if (0 == lNewTransactionNumber) {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": WARNING: Payment plan is unable to process because "
                    "there are no more transaction numbers available.")
                    .Flush();
                // (Here I do NOT flag for removal.)
                return false;
            }

            auto pTransSend{api_.Factory().Transaction(
                *theSenderInbox,
                transactionType::paymentReceipt,
                originType::origin_payment_plan,
                lNewTransactionNumber)};

            OT_ASSERT(false != bool(pTransSend));

            auto pTransRecip{api_.Factory().Transaction(
                *theRecipientInbox,
                transactionType::paymentReceipt,
                originType::origin_payment_plan,
                lNewTransactionNumber)};

            OT_ASSERT(false != bool(pTransRecip));

            // Both inboxes will get receipts with the same (new) transaction ID
            // on them.
            // They will have a "In reference to" field containing the original
            // payment plan
            // (with user's signature).

            // set up the transaction items (each transaction may have multiple
            // items... but not in this case.)
            auto pItemSend{api_.Factory().Item(
                *pTransSend, itemType::paymentReceipt, Identifier::Factory())};
            auto pItemRecip{api_.Factory().Item(
                *pTransRecip, itemType::paymentReceipt, Identifier::Factory())};

            OT_ASSERT(false != bool(pItemSend));
            OT_ASSERT(false != bool(pItemRecip));

            pItemSend->SetStatus(Item::rejection);   // the default.
            pItemRecip->SetStatus(Item::rejection);  // the default.

            // Here I make sure that each receipt (each inbox notice) references
            // the original
            // transaction number that was used to set the payment plan into
            // place...
            // This number is used to track all cron items. (All Cron items
            // require a transaction
            // number from the user in order to add them to Cron in the first
            // place.)
            //
            // The number is also used to uniquely identify all other
            // transactions, as you
            // might guess from its name.

            //          pTransSend->SetReferenceToNum(GetTransactionNum());
            //          pTransRecip->SetReferenceToNum(GetTransactionNum());
            pTransSend->SetReferenceToNum(GetOpeningNumber(SENDER_NYM_ID));
            pTransRecip->SetReferenceToNum(GetOpeningNumber(RECIPIENT_NYM_ID));

            // The TRANSACTION (a receipt in my inbox) will be sent with "In
            // Reference To" information
            // containing the ORIGINAL SIGNED PLAN. (With both parties' original
            // signatures on it.)
            //
            // Whereas the TRANSACTION ITEM will include an "attachment"
            // containing the UPDATED
            // PLAN (this time with the SERVER's signature on it.)
            //
            // Here's the original one going onto the transaction:
            //
            pTransSend->SetReferenceString(strOrigPlan);
            pTransRecip->SetReferenceString(strOrigPlan);

            // MOVE THE DIGITAL ASSETS FROM ONE ACCOUNT TO ANOTHER...

            // Calculate the amount and debit/ credit the accounts
            // Make sure each Account can afford it, and roll back in case of
            // failure.

            // Make sure he can actually afford it...
            if (sourceAccount.get().GetBalance() >= amount) {
                // Debit the source account.
                bool bMoveSender =
                    sourceAccount.get().Debit(amount);  // <====== DEBIT FUNDS
                bool bMoveRecipient = false;

                // IF success, credit the recipient.
                if (bMoveSender) {
                    bMoveRecipient = recipientAccount.get().Credit(
                        amount);  // <=== CREDIT FUNDS

                    // Okay, we already took it from the source account.
                    // But if we FAIL to credit the recipient, then we need to
                    // PUT IT BACK in the source acct.
                    // (EVEN THOUGH we'll just "NOT SAVE" after any failure, so
                    // it's really superfluous.)
                    //
                    if (!bMoveRecipient)
                        sourceAccount.get().Credit(amount);  // put the money
                                                             // back
                    else
                        bSuccess = true;
                }

                // If ANY of these failed, then roll them all back and break.
                if (!bMoveSender || !bMoveRecipient) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Very strange! Funds were available but debit or "
                        "credit failed while performing payment.")
                        .Flush();
                    // We won't save the files anyway, if this failed.
                    bSuccess = false;
                }
            }
            // --------------------------------------------------------------------------
            // DO NOT SAVE ACCOUNTS if bSuccess is false.
            // We only save these accounts if bSuccess == true.
            // (But we do save the inboxes either way, since payment failures
            // always merit an inbox notice.)

            if (true == bSuccess)  // The payment succeeded.
            {
                // Both accounts involved need to get a receipt of this trade in
                // their inboxes...
                pItemSend->SetStatus(Item::acknowledgement);  // sourceAccount
                pItemRecip->SetStatus(
                    Item::acknowledgement);  // recipientAccount

                pItemSend->SetAmount(amount * (-1));  // "paymentReceipt" is
                                                      // otherwise ambigious
                                                      // about whether you are
                                                      // paying or being paid.
                pItemRecip->SetAmount(amount);  // So, I decided for payment
                                                // and market receipts, to use
                                                // negative and positive
                                                // amounts.
                // I will probably do the same for cheques, since they can be
                // negative as well (invoices).

                if (m_bProcessingInitialPayment)  // if this is a success for an
                                                  // initial payment
                {
                    SetInitialPaymentDone();
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": Initial payment performed.")
                        .Flush();
                } else if (m_bProcessingPaymentPlan)  // if this is a success
                                                      // for
                                                      // payment plan payment.
                {
                    IncrementNoPaymentsDone();
                    SetDateOfLastPayment(OTTimeGetCurrentTime());
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": Recurring payment performed.")
                        .Flush();
                }

                // (I do NOT save m_pCron here, since that already occurs after
                // this function is called.)
            } else  // bSuccess = false.  The payment failed.
            {
                pItemSend->SetStatus(Item::rejection);   // sourceAccount
                                                         // // These are
                                                         // already
                                                         // initialized to
                                                         // false.
                pItemRecip->SetStatus(Item::rejection);  // recipientAccount
                                                         // // (But just
                                                         // making sure...)

                pItemSend->SetAmount(0);   // No money changed hands. Just being
                                           // explicit.
                pItemRecip->SetAmount(0);  // No money changed hands. Just being
                                           // explicit.

                if (m_bProcessingInitialPayment) {
                    IncrementNoInitialFailures();
                    SetLastFailedInitialPaymentDate(OTTimeGetCurrentTime());
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": Initial payment failed.")
                        .Flush();
                } else if (m_bProcessingPaymentPlan) {
                    IncrementNoFailedPayments();
                    SetDateOfLastFailedPayment(OTTimeGetCurrentTime());
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": Recurring payment failed.")
                        .Flush();
                }
            }

            // Everytime a payment processes, a receipt is put in the user's
            // inbox, containing a
            // CURRENT copy of the payment plan (which took just money from the
            // user's acct, or not,
            // and either way thus updated its status -- so its internal data
            // has changed.)
            //
            // It will also contain a copy of the user's ORIGINAL signed payment
            // plan, where the data
            // has NOT changed, (so the user's original signature is still
            // good.)
            //
            // In order for it to export the RIGHT VERSION of the CURRENT plan,
            // which has just changed
            // (above), then I need to re-sign it and save it first. (The
            // original version I'll load from
            // a separate file using
            // OTCronItem::LoadCronReceipt(lTransactionNum). It has both
            // original
            // signatures on it. Nice, eh?)

            ReleaseSignatures();
            SignContract(*pServerNym);
            SaveContract();

            // No need to save Cron here, since both caller functions call
            // SaveCron() EVERY time anyway,
            // success or failure, rain or shine.
            // m_pCron->SaveCron(); // Cron is where I am serialized, so if
            // Cron's not saved, I'm not saved.

            //
            // EVERYTHING BELOW is just about notifying the users, by dropping
            // the receipt in their
            // inboxes. The rest is done.  The accounts and inboxes will all be
            // saved at the same time.
            //
            // The Payment Plan is entirely updated and saved by this point, and
            // Cron will
            // also be saved in the calling function once we return (no matter
            // what.)
            //

            // Basically I load up both INBOXES, which are actually LEDGERS, and
            // then I create
            // a new transaction, with a new transaction item, for each of the
            // ledgers.
            // (That's where the receipt information goes.)
            //

            // The TRANSACTION will be sent with "In Reference To" information
            // containing the
            // ORIGINAL SIGNED PLAN. (With both of the users' original
            // signatures on it.)
            //
            // Whereas the TRANSACTION ITEM will include an "attachment"
            // containing the UPDATED
            // PLAN (this time with the SERVER's signature on it.)

            // (Lucky I just signed and saved the updated plan (above), or it
            // would still have
            // have the old data in it.)

            // I also already loaded the original plan. Remember this from
            // above,
            // near the top of the function:
            //  OTCronItem * pOrigCronItem    = nullptr;
            //     OTString strOrigPlan(*pOrigCronItem); // <====== Farther down
            // in the code, I attach this string to the receipts.
            //  ... then lower down...
            // pTransSend->SetReferenceString(strOrigPlan);
            // pTransRecip->SetReferenceString(strOrigPlan);
            //
            // So the original plan is already loaded and copied to the
            // Transaction as the "In Reference To"
            // Field. Now let's add the UPDATED plan as an ATTACHMENT on the
            // Transaction ITEM:
            //
            auto strUpdatedPlan = String::Factory(*this);

            // Set the updated plan as the attachment on the transaction item.
            // (With the SERVER's signature on it!)
            // (As a receipt for each trader, so they can see their offer
            // updating.)
            pItemSend->SetAttachment(strUpdatedPlan);
            pItemRecip->SetAttachment(strUpdatedPlan);

            // Success OR failure, either way I want a receipt in both inboxes.
            // But if FAILURE, I do NOT want to save the Accounts, JUST the
            // inboxes.
            // So the inboxes happen either way, but the accounts are saved only
            // on success.

            // sign the item
            pItemSend->SignContract(*pServerNym);
            pItemRecip->SignContract(*pServerNym);

            pItemSend->SaveContract();
            pItemRecip->SaveContract();

            std::shared_ptr<Item> itemSend{pItemSend.release()};
            std::shared_ptr<Item> itemRecip{pItemRecip.release()};
            pTransSend->AddItem(itemSend);
            pTransRecip->AddItem(itemRecip);

            pTransSend->SignContract(*pServerNym);
            pTransRecip->SignContract(*pServerNym);

            pTransSend->SaveContract();
            pTransRecip->SaveContract();

            // Here, the transactions we just created are actually added to the
            // ledgers.
            // This happens either way, success or fail.

            std::shared_ptr<OTTransaction> transSend{pTransSend.release()};
            std::shared_ptr<OTTransaction> transRecip{pTransRecip.release()};
            theSenderInbox->AddTransaction(transSend);
            theRecipientInbox->AddTransaction(transRecip);

            // Release any signatures that were there before (They won't
            // verify anymore anyway, since the content has changed.)
            theSenderInbox->ReleaseSignatures();
            theRecipientInbox->ReleaseSignatures();

            // Sign both of them.
            theSenderInbox->SignContract(*pServerNym);
            theRecipientInbox->SignContract(*pServerNym);

            // Save both of them internally
            theSenderInbox->SaveContract();
            theRecipientInbox->SaveContract();

            // Save both inboxes to storage. (File, DB, wherever it goes.)
            sourceAccount.get().SaveInbox(
                *theSenderInbox, Identifier::Factory());
            recipientAccount.get().SaveInbox(
                *theRecipientInbox, Identifier::Factory());

            // These correspond to the AddTransaction() calls just above. These
            // are stored in separate files now.
            //
            transSend->SaveBoxReceipt(*theSenderInbox);
            transRecip->SaveBoxReceipt(*theRecipientInbox);

            // If success, save the accounts with new balance. (Save inboxes
            // with receipts either way,
            // and the receipts will contain a rejection or acknowledgment
            // stamped by the Server Nym.)
            if (true == bSuccess) {
                sourceAccount.Release();
                recipientAccount.Release();
            } else {
                sourceAccount.Abort();
                recipientAccount.Abort();
            }
        }  // both inboxes were successfully loaded or generated.
    }  // By the time we enter this block, accounts and nyms are already loaded.
       // As we begin, inboxes are instantiated.

    return bSuccess;
}

// Assumes we're due for this payment. Execution oriented.
// NOTE: there used to be more to this function, but it ended up like this. Que
// sera sera.
void OTPaymentPlan::ProcessInitialPayment(const api::Wallet& wallet)
{
    OT_ASSERT(nullptr != GetCron());

    m_bProcessingInitialPayment = true;
    ProcessPayment(wallet, GetInitialPaymentAmount());
    m_bProcessingInitialPayment = false;

    // No need to save the Payment Plan itself since it's already
    // saved inside the ProcessPayment() call as part of constructing the
    // receipt.

    // Since this' data file is actually a blob as part of Cron,
    // then we need to save Cron as well. Only then are these changes truly
    // saved.
    // I'm actually lucky to even be able to save cron here, since I know for a
    // fact
    // that I have to save no matter what. In cases where I don't know for sure,
    // the
    // save call has to go a level deeper, since only there would the code know
    // whether
    // or not something has actually changed.
    // HMM Todo: Look into adding a "DIRTY" field to OT objects.
    // Double-hmm, todo: I could make a series of macros for serializable member
    // variables,
    // that would deal with all the xml read/write very compactly, and
    // automatically track
    // whether the variable was dirty. Then anywhere in the code I can just ask
    // an object
    // if it is dirty, or instruct it to update itself if it is.  Anyway, let's
    // save Cron...

    GetCron()->SaveCron();

    // Todo: put the actual Cron items in separate files, so I don't have to
    // update
    // the entire cron file every time an item changes. That way the main cron
    // file
    // can just keep a record of the active items, and will change only when an
    // item
    // is added or removed.
    //
    // Until then, the entire Cron file will have to save EVERY time this call
    // happens.
    //
    // (I'll add something to Cron so it can't save more than once per second or
    // something.)
    // (DONE: Payment Plans process hourly and Trades process every 10 seconds.)
}

/*
// This is called by OTCronItem::HookRemovalFromCron
// (After calling this method, HookRemovalFromCron then calls
onRemovalFromCron.)
//
void OTPaymentPlan::onFinalReceipt()
{

    OTAgreement::onFinalReceipt();



    // Not much done in this one.





}


// This is called by OTCronItem::HookRemovalFromCron
// (Before calling this method, HookRemovalFromCron first calls onFinalReceipt.)
//
void OTPaymentPlan::onRemovalFromCron()
{

    OTAgreement::onRemovalFromCron();



    // Not much done in this one.



}
*/

// Assumes we're due for a payment. Execution oriented.
// NOTE: There used to be more to this function, but it ended up like this. Que
// sera sera.
void OTPaymentPlan::ProcessPaymentPlan(const api::Wallet& wallet)
{
    OT_ASSERT(nullptr != GetCron());

    // This way the ProcessPayment() function knows what kind of payment it's
    // processing.
    // Basically there's just one little spot in there where it needs to know.
    // :-(
    // But the member could be useful in the future anyway.
    m_bProcessingPaymentPlan = true;
    ProcessPayment(wallet, GetPaymentPlanAmount());
    m_bProcessingPaymentPlan = false;

    // No need to save the Payment Plan itself since it's already
    // saved inside the ProcessPayment() call as part of constructing the
    // receipt.

    // Either way, Cron should save, since it just updated.
    // (The above function call WILL change this payment plan
    // and re-sign it and save it, no matter what. So I just
    // call this here to keep it simple:

    GetCron()->SaveCron();
}

// OTCron calls this regularly, which is my chance to expire, etc.
// Return True if I should stay on the Cron list for more processing.
// Return False if I should be removed and deleted.
bool OTPaymentPlan::ProcessCron()
{
    OT_ASSERT(nullptr != GetCron());

    // Right now Cron is called 10 times per second.
    // I'm going to slow down all trades so they are once every
    // GetProcessInterval()
    if (GetLastProcessDate() > OT_TIME_ZERO) {
        // (Default ProcessInternal is 1 second, but Trades will use 10 seconds,
        // and Payment Plans will use an hour or day.)
        if (OTTimeGetTimeInterval(
                OTTimeGetCurrentTime(), GetLastProcessDate()) <=
            GetProcessInterval())
            return true;
    }
    // Keep a record of the last time this was processed.
    // (NOT saved to storage, only used while the software is running.)
    // (Thus no need to release signatures, sign contract, save contract, etc.)
    SetLastProcessDate(OTTimeGetCurrentTime());

    // END DATE --------------------------------
    // First call the parent's version (which this overrides) so it has
    // a chance to check its stuff.
    // Currently it calls OTCronItem::ProcessCron, which checks IsExpired().
    //
    if (!ot_super::ProcessCron()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Cron job has expired.").Flush();
        return false;  // It's expired or flagged for removal--remove it from
                       // Cron.
    }

    // START DATE --------------------------------
    // Okay, so it's not expired. But might not have reached START DATE yet...
    if (!VerifyCurrentDate())
        return true;  // The Payment Plan is not yet valid, so we return. BUT,
                      // we
                      // also
    // return TRUE, so it will STAY on Cron until it BECOMES valid.

    if (GetCron()->GetTransactionCount() < 1) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed to process payment: Out of transaction numbers!")
            .Flush();
        return true;  // If there aren't enough transaction numbers, this won't
                      // log
        // 10 times per second, but instead every hour or every day,
    }  // since plans don't process any more often than that anyway.

    // First process the initial payment...

    if (HasInitialPayment() &&      // If I have an initial payment...
        !IsInitialPaymentDone() &&  // and I have not yet processed it...
        (OTTimeGetCurrentTime() >
         GetInitialPaymentDate()) &&  // and we're past the initial payment due
                                      // date...
        ((0 == GetLastFailedInitialPaymentDate()) ||  // And we've not yet got a
                                                      // failed date, OR
         (OTTimeGetTimeInterval(
              OTTimeGetCurrentTime(),
              GetLastFailedInitialPaymentDate()) >  // it's been more than a day
          OTTimeGetSecondsFromTime(OT_TIME_DAY_IN_SECONDS)))  // since I last
                                                              // failed
                                                              // attmpting
                                                              // this...
    ) {  // THEN we're due for the initial payment! Process it!

        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Cron: Processing initial payment...")
            .Flush();

        ProcessInitialPayment(api_.Wallet());
    }
    // ----------------------------------------------
    // Next, process the payment plan...
    //    otOut << "(payment plan): Flagged/Removal: "
    //           << (IsFlaggedForRemoval() ? "TRUE" : "FALSE")
    //           << " Has Plan: " << (HasPaymentPlan() ? "TRUE" : "FALSE")
    //           << " Current time: "
    //           << OTTimeGetSecondsFromTime(OTTimeGetCurrentTime())
    //           << " Start Date: "
    //           << OTTimeGetSecondsFromTime(GetPaymentPlanStartDate()) << "\n";

    // This object COULD have gotten flagged for removal
    // during the ProcessInitialPayment() call just above.
    // Therefore, I am sure to check that it's NOT IsFlaggedForRemoval()
    // before calling this block of code.
    //
    if (!IsFlaggedForRemoval() && HasPaymentPlan() &&
        (OTTimeGetCurrentTime() > GetPaymentPlanStartDate())) {
        //      otErr << "DEBUG: Payment Plan -------------\n";

        // First I'll calculate whether the next payment would be due, based on
        // start date,
        // time between payments, and date of last payment.

        const std::int64_t DURATION_SINCE_START = OTTimeGetTimeInterval(
            OTTimeGetCurrentTime(), GetPaymentPlanStartDate());

        // Let's say the plan charges every week, and it's been 16 DAYS DURATION
        // since starting.
        // The first charge would have been on the 1st day, 16 days ago.
        // Then the second charge would have been on the 8th day, (7 days later)
        // Then the third charge would have been on the 15th day, (7 days later
        // again)
        // That means the next charge isn't due until the 22nd.

        // Right now in this example, DURATION_SINCE_START is: (16 *
        // OT_TIME_DAY_IN_SECONDS).
        // I must calculate from that, that three charges have already happened,
        // and that the
        // next one is not yet due.
        //
        // I also know that GetTimeBetweenPayments() is set to
        // (OT_TIME_DAY_IN_SECONDS * 7)
        //
        // Duration / timebetween == 16/7 == 2 with 2 remainder. (+1 to get 3:
        // THREE should have already happened.)
        // if it was the 14th, 14/7 == 2 with 0 remainder.       (+1 to get 3:
        // THREE should have happened by the 14th)
        // If it was the 22nd, 22/7 == 3 with 1 remainder.       (+1 to get 4:
        // FOUR payments should have already happened.)
        //
        // Can also just add the TimeBetweenPayments to the DateOfLastPayment...
        //
        const std::int64_t nNoPaymentsThatShouldHaveHappenedByNow =
            DURATION_SINCE_START /
                OTTimeGetSecondsFromTime(GetTimeBetweenPayments()) +
            1;
        // The +1 is because it charges on the 1st day of the plan. So 14 days,
        // which is 7 times 2, equals *3* payments, not 2.

        // It's expired, remove it. (I check >0 because this one is an optional
        // field.)
        if ((GetMaximumNoPayments() > 0) &&
            (GetNoPaymentsDone() >= GetMaximumNoPayments())) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                "Payment plan has expired by reaching max number of "
                "payments allowed.")
                .Flush();
            return false;  // This payment plan will be removed from Cron by
                           // returning false.
        }
        // Again, I check >0 because the plan length is optional and might just
        // be 0.
        else if (
            (GetPaymentPlanLength() > OT_TIME_ZERO) &&
            (OTTimeGetCurrentTime() >=
             OTTimeAddTimeInterval(
                 GetPaymentPlanStartDate(),
                 OTTimeGetSecondsFromTime(GetPaymentPlanLength())))) {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Payment plan has expired by reaching its maximum length "
                "of time.")
                .Flush();
            return false;  // This payment plan will be removed from Cron by
                           // returning false.
        } else if (
            GetNoPaymentsDone() >=
            nNoPaymentsThatShouldHaveHappenedByNow)  // if enough payments have
                                                     // happened for now...
        {
            //            otLog3 << "DEBUG: Enough payments have already been
            //            made at the current time.\n";
        } else if (
            OTTimeGetTimeInterval(
                OTTimeGetCurrentTime(), GetDateOfLastPayment()) <
            OTTimeGetSecondsFromTime(GetTimeBetweenPayments()))  // and the time
                                                                 // since last
        // payment is more than the payment period...
        {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": DEBUG: Not enough time has elapsed.")
                .Flush();
        } else if (
            (0 != GetDateOfLastFailedPayment()) &&
            (OTTimeGetTimeInterval(
                 OTTimeGetCurrentTime(), GetDateOfLastFailedPayment()) <
             OTTimeGetSecondsFromTime(OT_TIME_DAY_IN_SECONDS)))  // and it
                                                                 // hasn't been
                                                                 // at least 24
                                                                 // hrs since
                                                                 // the last
                                                                 // failed
                                                                 // payment...
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Cron (processing payment plan): Not enough time since "
                "last failed payment. (Returning--for now).")
                .Flush();
        }
        // Okay -- PROCESS IT!
        else  // The above 3 end-comments have opposite logic from their if(),
              // since they used to be here.
        {  // I reversed the operators so they could be failures, resulting in
            // this else block for success.

            LogNormal(OT_METHOD)(__FUNCTION__)(": Cron: Processing payment...")
                .Flush();

            // This function assumes the payment is due, and it only fails in
            // the case of the payer's account having insufficient funds.
            ProcessPaymentPlan(api_.Wallet());
        }
    }

    // Notice something: Markets are very concerned whether a trade failed,
    // or if the account was short of funds. They track that, and remove any
    // trades when they have this problem. So in OTTrade right now, you
    // would be checking if it was flagged for removal, and returning false
    // in that case.
    //
    // But with a PAYMENT PLAN, if a payment fails, you don't want to cancel
    // the plan!!! You want it to keep trying until it gets in more
    // payments, and ONLY cancel in the case where the user REQUESTS it, or
    // when one of the legitimate terms above expires naturally.
    // Insufficient funds? NO PROBLEM: you can stay on your payment plan as
    // long as you want! :-)
    //
    // There ARE however funny cases where you WOULD want the plan removed.
    // For example:
    //
    if (IsFlaggedForRemoval() ||
        (HasInitialPayment() && IsInitialPaymentDone() && !HasPaymentPlan())) {
        LogDebug(OT_METHOD)(__FUNCTION__)(
            ": Removing payment plan from cron processing...")
            .Flush();
        return false;  // if there's no plan, and initial payment is done,
                       // nothing left to do. Remove!
    }

    return true;
}

void OTPaymentPlan::InitPaymentPlan()
{
    m_strContractType = String::Factory("PAYMENT PLAN");

    SetProcessInterval(OTTimeGetSecondsFromTime(
        PLAN_PROCESS_INTERVAL));  // Payment plans currently process every hour.
                                  // (Could be reduced even more, to every day.)

    // Initial Payment...
    m_bInitialPayment = false;             // Will there be an initial payment?
    m_tInitialPaymentDate = OT_TIME_ZERO;  // Date of the initial payment,
                                           // measured seconds after creation
                                           // date.
    m_tInitialPaymentCompletedDate =
        OT_TIME_ZERO;  // Date the initial payment was finally completed.
    m_lInitialPaymentAmount = 0;    // Amount of the initial payment.
    m_bInitialPaymentDone = false;  // Has the initial payment been made?
    m_nNumberInitialFailures = 0;  // Number of times we failed to process this.
    m_tFailedInitialPaymentDate =
        OT_TIME_ZERO;  // Date of the last failed initial payment.

    // Payment Plan...
    m_bPaymentPlan = false;    // Will there be a payment plan?
    m_lPaymentPlanAmount = 0;  // Amount of each payment.
    m_tTimeBetweenPayments = OT_TIME_MONTH_IN_SECONDS;  // How std::int64_t
                                                        // between each payment?
                                                        // (Default:
    // 30 days) // TODO don't hardcode.
    m_tPaymentPlanStartDate = OT_TIME_ZERO;  // Date for the first payment plan
                                             // payment. Measured seconds after
                                             // creation.

    m_tPaymentPlanLength = OT_TIME_ZERO;  // Optional. Plan length measured in
                                          // seconds since plan start.
    m_nMaximumNoPayments =
        0;  // Optional. The most number of payments that are authorized.

    m_tDateOfLastPayment =
        OT_TIME_ZERO;  // Recording of date of the last payment.
    m_nNoPaymentsDone =
        0;  // Recording of the number of payments already processed.

    m_tDateOfLastFailedPayment =
        OT_TIME_ZERO;  // Recording of date of the last failed payment.
    m_nNoFailedPayments =
        0;  // Every time a payment fails, we record that here.
}

void OTPaymentPlan::Release_PaymentPlan()
{
    // If there were any dynamically allocated objects, clean them up here.
}

void OTPaymentPlan::Release()
{
    // If there were any dynamically allocated objects, clean them up here.
    Release_PaymentPlan();

    ot_super::Release();  // since I've overridden the base class, I call it
                          // now...

    // Then I call this to re-initialize everything
    InitPaymentPlan();
}

OTPaymentPlan::~OTPaymentPlan() { Release_PaymentPlan(); }
}  // namespace opentxs
