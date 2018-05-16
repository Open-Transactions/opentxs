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

// OTAgreement is derived from OTCronItem.  It handles re-occuring billing.

#ifndef OPENTXS_CORE_OTAGREEMENT_HPP
#define OPENTXS_CORE_OTAGREEMENT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <deque>

namespace opentxs
{
// An Agreement occurs between TWO PEOPLE, and is for a CONSIDERATION.
// Thus, we add the RECIPIENT (already have SENDER from OTTrackable.)
//
// While other instruments are derived from OTTrackable (like OTCheque) in order
// to gain a transaction number and sender user/acct, Agreements are derived
// from
// a further subclass of trackable: OTCronItem.
//
// OTCronItems are allowed to be posted on the OTCron object, which performs
// regular
// processing on a timely basis to the items that are posted there. In this way,
// payment authorizations can be posted (and expire properly), and trades can be
// posted with valid date ranges, and payment plans can be instituted, and so
// on.
//
// OTAgreement is derived from OTCronItem because it allows people to post
// Agreements
// on OTCron until a certain expiration period, so that third parties can query
// the
// server and verify the agreements, and so that copies of the agreement,
// stamped
// with the server's signature, can be made available to the parties and to 3rd
// parties.
//
class OTAgreement : public OTCronItem
{
private:  // Private prevents erroneous use by other classes.
    typedef OTCronItem ot_super;

private:
    OTIdentifier m_RECIPIENT_ACCT_ID;
    OTIdentifier m_RECIPIENT_NYM_ID;

protected:
    String m_strConsideration;  // Presumably an agreement is in return for
                                // some consideration. Memo here.

    String m_strMerchantSignedCopy;  // The merchant sends it over, then the
                                     // payer confirms it, which adds
    // his own transaction numbers and signs it. This, unfortunately,
    // invalidates the merchant's version, so we store
    // a copy of the merchant's signed agreement INSIDE our own. The server can
    // do the hard work of comparing them, though
    // such will probably occur through a comparison function I'll have to add
    // right here in this class.

    void onFinalReceipt(
        OTCronItem& theOrigCronItem,
        const std::int64_t& lNewTransactionNumber,
        Nym& theOriginator,
        Nym* pRemover) override;
    void onRemovalFromCron() override;

    // Numbers used for CLOSING a transaction. (finalReceipt.)
    std::deque<TransactionNumber> m_dequeRecipientClosingNumbers;

public:
    originType GetOriginType() const override
    {
        return originType::origin_payment_plan;
    }

    void setCustomerNymId(const Identifier& NYM_ID);

    const String& GetConsideration() const { return m_strConsideration; }
    void SetMerchantSignedCopy(const String& strMerchantCopy)
    {
        m_strMerchantSignedCopy = strMerchantCopy;
    }
    const String& GetMerchantSignedCopy() const
    {
        return m_strMerchantSignedCopy;
    }

    // SetAgreement replaced with the 2 functions below. See notes even lower.
    //
    //    bool    SetAgreement(const std::int64_t& lTransactionNum,    const
    // OTString& strConsideration,
    //                       const time64_t& VALID_FROM=0,    const time64_t&
    // VALID_TO=0);

    EXPORT bool SetProposal(
        ServerContext& context,
        Account& MERCHANT_ACCT,
        const String& strConsideration,
        time64_t VALID_FROM = OT_TIME_ZERO,
        time64_t VALID_TO = OT_TIME_ZERO);

    // Merchant Nym is passed here so we can verify the signature before
    // confirming.
    EXPORT bool Confirm(
        ServerContext& context,
        Account& PAYER_ACCT,
        const Nym* pMERCHANT_NYM = nullptr,
        const Identifier* p_id_MERCHANT_NYM = nullptr);

    // What should be the process here?

    /*
        FIRST: (Construction)

     OTAgreement(const Identifier& NOTARY_ID,
                 const Identifier& INSTRUMENT_DEFINITION_ID);
       OR:
     OTAgreement(const Identifier& NOTARY_ID, const Identifier&
    INSTRUMENT_DEFINITION_ID,
                 const Identifier& SENDER_ACCT_ID, const Identifier&
    SENDER_NYM_ID,
                 const Identifier& RECIPIENT_ACCT_ID, const Identifier&
    RECIPIENT_NYM_ID);
       OR:
     OTPaymentPlan * pPlan = new OTPaymentPlan(pAccount->GetRealNotaryID(),
                                    pAccount->GetInstrumentDefinitionID(),
                                    pAccount->GetRealAccountID(),
                                    pAccount->GetNymID(),
                                    RECIPIENT_ACCT_ID, RECIPIENT_NYM_ID);
     THEN:  (Agreement)

     bool bSuccessSetAgreement = pPlan->SetAgreement(lTransactionNumber,
                                                    PLAN_CONSIDERATION,
    VALID_FROM, VALID_TO);

     THEN, (OTPaymentPlan) adds TWO OPTIONS (additional and independent of each
    other):

     bool        SetInitialPayment(const std::int64_t& lAmount, time64_t
                    tTimeUntilInitialPayment=0); // default: now.
     bool        SetPaymentPlan(const std::int64_t& lPaymentAmount, time64_t
                                tTimeUntilPlanStart=OT_TIME_MONTH_IN_SECONDS,
                                time64_t
                                tBetweenPayments=OT_TIME_MONTH_IN_SECONDS, //
    Default: 30 days.
                                time64_t tPlanLength=0, std::int32_t
    nMaxPayments=0);


     The new process is the same, but it adds some additional transaction
    numbers...

     HERE IS THE WAY I ENVISION IT BEING CALLED:

     ---- The MERCHANT does these steps: -----

     Step one, though it says PaymentPlan, is basically the OTAgreement
    constructor.
     Its primary concern is with determining the server, payer, payee, accounts,
    etc.

     1) OTPaymentPlan * pPlan =
        new OTPaymentPlan(pAccount->GetRealNotaryID(),
            pAccount->GetInstrumentDefinitionID(),
            pAccount->GetRealAccountID(),
            pAccount->GetNymID(),
            RECIPIENT_ACCT_ID, RECIPIENT_NYM_ID);

    STILL, this is the MERCHANT. Step two is concerned with the specific terms
    of the offer.

     2) bool bOffer =
            pPlan->SetProposal(MERCHANT_NYM,
                        PLAN_CONSIDERATION, VALID_FROM, VALID_TO);
      (lMerchantTransactionNumber, lMerchantClosingNumber are set internally
    using the MERCHANT_NYM.)

     ==> Optionally, the merchant also calls SetInitialPayment and/or
    SetPaymentPlan at this time.
     ==> Next, the merchant signs it, and sends to the recipient.

     THE RECIPIENT:

     3) bool bConfirmation =  pPlan->Confirm(OTPseudonym& PAYER_NYM,
                                             OTPseudonym *
    pMERCHANT_NYM=nullptr,
                                             OTIdentifier *
    p_id_MERCHANT_NYM=nullptr);

     (Transaction number and closing number are retrieved from Nym at this
    time.)

     NO NEED TO SIGN ANYTHING AFTER THIS POINT, and the Payment Plan should
    store a copy of itself at this time.
    (That is, STORE A COPY of the Merchant's signed version, since the above
    call to Confirm will change the plan
     and sign it again. The server is left with the chore of comparing the two
    against each other, which I will
     probably have to code right here in this class!  TOdo.)

     */

    // This function verifies both Nyms and both signatures.
    // Due to the peculiar nature of how OTAgreement/OTPaymentPlan works, there
    // are two signed
    // copies stored. The merchant signs first, adding his transaction numbers
    // (2), and then he
    // sends it to the customer, who also adds two numbers and signs. (Also
    // resetting the creation date.)
    // The problem is, adding the additional transaction numbers invalidates the
    // first (merchant's)
    // signature.
    // The solution is, when the customer confirms the agreement, he stores an
    // internal copy of the
    // merchant's signed version.  This way later, in VERIFY AGREEMENT, the
    // internal copy can be loaded,
    // and BOTH Nyms can be checked to verify that BOTH transaction numbers are
    // valid for each.
    // The two versions of the contract can also be compared to each other, to
    // make sure that none of
    // the vital terms, values, clauses, etc are different between the two.
    //
    virtual bool VerifyAgreement(
        const ClientContext& recipient,
        const ClientContext& sender) const = 0;

    virtual bool CompareAgreement(const OTAgreement& rhs) const;

    inline const Identifier& GetRecipientAcctID() const
    {
        return m_RECIPIENT_ACCT_ID;
    }
    inline const Identifier& GetRecipientNymID() const
    {
        return m_RECIPIENT_NYM_ID;
    }
    inline void SetRecipientAcctID(const Identifier& ACCT_ID)
    {
        m_RECIPIENT_ACCT_ID = ACCT_ID;
    }
    inline void SetRecipientNymID(const Identifier& NYM_ID)
    {
        m_RECIPIENT_NYM_ID = NYM_ID;
    }

    // The recipient must also provide an opening and closing transaction
    // number(s).
    //
    EXPORT std::int64_t GetRecipientClosingTransactionNoAt(
        std::uint32_t nIndex) const;
    EXPORT std::int32_t GetRecipientCountClosingNumbers() const;

    void AddRecipientClosingTransactionNo(
        const std::int64_t& lClosingTransactionNo);

    // This is a higher-level than the above functions. It calls them.
    // Below is the abstraction, above is the implementation.

    EXPORT TransactionNumber GetRecipientOpeningNum() const;
    EXPORT TransactionNumber GetRecipientClosingNum() const;

    // From OTCronItem (parent class of this)
    /*
     inline void SetCronPointer(OTCron& theCron) { m_pCron = &theCron; }

     inline void SetCreationDate(const time64_t& CREATION_DATE) {
     m_CREATION_DATE = CREATION_DATE; }
     inline const time64_t& GetCreationDate() const { return m_CREATION_DATE; }

     // These are for:
     // std::deque<std::int64_t> m_dequeClosingNumbers;
     //
     // They are numbers used for CLOSING a transaction. (finalReceipt, someday
     more.)

     std::int64_t    GetClosingTransactionNoAt(std::int32_t nIndex) const;
     std::int32_t     GetCountClosingNumbers() const;

     void    AddClosingTransactionNo(const std::int64_t& lClosingTransactionNo);
     */
    bool CanRemoveItemFromCron(const ClientContext& context) override;

    EXPORT void HarvestOpeningNumber(ServerContext& context) override;
    EXPORT void HarvestClosingNumbers(ServerContext& context) override;

    // Return True if should stay on OTCron's list for more processing.
    // Return False if expired or otherwise should be removed.
    bool ProcessCron() override;  // OTCron calls this regularly, which is my
                                  // chance to expire, etc.

    // From OTTrackable (parent class of OTCronItem, parent class of this)
    /*
     inline std::int64_t GetTransactionNum() const { return m_lTransactionNum; }
     inline void SetTransactionNum(std::int64_t lTransactionNum) {
     m_lTransactionNum
     = lTransactionNum; }

     inline const Identifier&    GetSenderAcctID()        { return
     m_SENDER_ACCT_ID; }
     inline const Identifier&    GetSenderNymID()        { return
     m_SENDER_NYM_ID; }
     inline void            SetSenderAcctID(const Identifier& ACCT_ID)
     { m_SENDER_ACCT_ID = ACCT_ID; }
     inline void            SetSenderNymID(const Identifier& NYM_ID)
     { m_SENDER_NYM_ID = NYM_ID; }
     */

    bool HasTransactionNum(const std::int64_t& lInput) const override;
    void GetAllTransactionNumbers(NumList& numlistOutput) const override;

    // From OTInstrument (parent class of OTTrackable, parent class of
    // OTCronItem, parent class of this)
    /*
     OTInstrument(const Identifier& NOTARY_ID, const Identifier&
     INSTRUMENT_DEFINITION_ID)
     : Contract()

     inline const Identifier& GetInstrumentDefinitionID() const { return
     m_InstrumentDefinitionID; }
     inline const Identifier& GetNotaryID() const { return m_NotaryID; }

     inline void SetInstrumentDefinitionID(const Identifier&
     INSTRUMENT_DEFINITION_ID)  {
     m_InstrumentDefinitionID    =
     INSTRUMENT_DEFINITION_ID; }
     inline void SetNotaryID(const Identifier& NOTARY_ID) { m_NotaryID    =
     NOTARY_ID; }

     inline time64_t GetValidFrom()    const { return m_VALID_FROM; }
     inline time64_t GetValidTo()        const { return m_VALID_TO; }

     inline void SetValidFrom(time64_t TIME_FROM)    { m_VALID_FROM    =
     TIME_FROM; }
     inline void SetValidTo(time64_t TIME_TO)        { m_VALID_TO    = TIME_TO;
     }

     bool VerifyCurrentDate(); // Verify the current date against the VALID FROM
     / TO dates.
     */

    // From OTScriptable, we override this function. OTScriptable now does fancy
    // stuff like checking to see
    // if the Nym is an agent working on behalf of a party to the contract.
    // That's how all OTScriptable-derived
    // objects work by default.  But OTAgreement (payment plan) and OTTrade do
    // it the old way: they just check to
    // see if theNym has signed *this.
    //
    bool VerifyNymAsAgent(
        const Nym& theNym,
        const Nym& theSignerNym,
        mapOfConstNyms* pmap_ALREADY_LOADED = nullptr) const override;

    bool VerifyNymAsAgentForAccount(const Nym& theNym, Account& theAccount)
        const override;

    /*
     From Contract, I have:

     virtual bool SignContract (const OTPseudonym& theNym);

     */
    EXPORT bool SendNoticeToAllParties(
        bool bSuccessMsg,
        Nym& theServerNym,
        const Identifier& theNotaryID,
        const std::int64_t& lNewTransactionNumber,
        // const std::int64_t& lInReferenceTo, //
        // each party has its own opening trans #.
        const String& strReference,
        String* pstrNote = nullptr,
        String* pstrAttachment = nullptr,
        Nym* pActualNym = nullptr) const;

    // Nym receives an OTItem::acknowledgment or OTItem::rejection.
    EXPORT static bool DropServerNoticeToNymbox(
        bool bSuccessMsg,
        const Nym& theServerNym,
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const TransactionNumber& lNewTransactionNumber,
        const TransactionNumber& lInReferenceTo,
        const String& strReference,
        originType theOriginType,
        String* pstrNote = nullptr,
        String* pstrAttachment = nullptr,
        const Nym* pActualNym = nullptr);

    OTAgreement();
    OTAgreement(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID);
    OTAgreement(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& SENDER_ACCT_ID,
        const Identifier& SENDER_NYM_ID,
        const Identifier& RECIPIENT_ACCT_ID,
        const Identifier& RECIPIENT_NYM_ID);
    virtual ~OTAgreement();

    void InitAgreement();

    void Release() override;
    void Release_Agreement();
    bool IsValidOpeningNumber(const std::int64_t& lOpeningNum) const override;
    EXPORT std::int64_t GetOpeningNumber(
        const Identifier& theNymID) const override;
    std::int64_t GetClosingNumber(const Identifier& theAcctID) const override;
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;
    void UpdateContents() override;  // Before transmission or serialization,
                                     // this
                                     // is where the ledger saves its contents
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_OTAGREEMENT_HPP
