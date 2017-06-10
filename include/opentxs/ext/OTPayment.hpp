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

#ifndef OPENTXS_EXT_OTPAYMENT_HPP
#define OPENTXS_EXT_OTPAYMENT_HPP

#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"

#include <stdint.h>

namespace opentxs
{

class Cheque;
class NumList;
class OTPaymentPlan;
class OTSmartContract;
class OTTrackable;
class OTTransaction;
class Purse;

/*
  The PAYMENT can be of types:
    - CHEQUE, INVOICE, VOUCHER (these are all forms of cheque)
    - PAYMENT PLAN, SMART CONTRACT (these are cron items)
    - PURSE (containing cash)

 FYI:

 Contract — Most other classes are derived from this one. Contains the actual
 XML contents,
  as well as various data values that were loaded from those contents, including
 public keys.
  Also contains a list of signatures.

 OTScriptable — Derived from Contract, but enables scriptable clauses. Also
 contains a list
  of parties (each with agents and asset accounts) as well as a list of bylaws
 (each with scripted
  clauses, internal state, hooks, callbacks, etc.)

 OTInstrument — Has a date range, a server ID, and an instrument definition id.
 Derived from
 OTScriptable.

 OTTrackable  — Has a transaction number, user ID, and an asset account ID.
 Derived from OTInstrument.

 OTCheque — A financial instrument. Derived from OTTrackable.

 OTCronItem — Derived from OTTrackable. OT has a central “Cron” object which
 runs recurring tasks, known as CronItems.

 OTAgreement — Derived from OTCronItem. It has a recipient and recipient asset
 account.

 OTPaymentPlan — Derived from OTAgreement, derived from OTCronItem. Allows
 merchants and customers
  to set up recurring payments. (Cancel anytime, with a receipt going to both
 inboxes.)

 OTSmartContract — Derived from OTCronItem. All CronItems are actually derived
 from OTScriptable already
  (through OTTrackable/OTInstrument). But OTSmartContract is the first/only Cron
 Item specifically designed
  to take full advantage of both the cron system AND the scriptable system in
 conjunction with each other.
  Currently OTSmartContract is the only actual server-side scripting on OT.
 */

class OTPayment : public Contract
{
private:  // Private prevents erroneous use by other classes.
    typedef Contract ot_super;

public:
    enum paymentType {
        // OTCheque is derived from OTTrackable, which is derived from
        // OTInstrument, which is
        // derived from OTScriptable, which is derived from Contract.
        CHEQUE,   // A cheque drawn on a user's account.
        VOUCHER,  // A cheque drawn on a server account (cashier's cheque aka
                  // banker's cheque)
        INVOICE,  // A cheque with a negative amount. (Depositing this causes a
                  // payment out, instead of a deposit in.)
        PAYMENT_PLAN,    // An OTCronItem-derived OTPaymentPlan, related to a
                         // recurring payment plan.
        SMART_CONTRACT,  // An OTCronItem-derived OTSmartContract, related to a
                         // smart contract.
        PURSE,   // An Contract-derived OTPurse containing a list of cash
                 // OTTokens.
        NOTICE,  // An OTTransaction containing a notice that a cron item was
                 // activated/canceled.
        // NOTE: Even though a notice isn't a "payment instrument" it can still
        // be found
        // in the Nym's record box, where all his received payments are moved
        // once they
        // are deposited. Interestingly though, I believe those are all
        // RECEIVED, except
        // for the notices, which are SENT. (Well, the notice was actually
        // received from
        // the server, BUT IN REFERENCE TO something that had been sent, and
        // thus the outgoing
        // payment is removed when the notice is received into the record box.
        ERROR_STATE
    };  // If you add any types to this list, update the list of strings at the
        // top of the .CPP file.

protected:
    void UpdateContents() override;  // Before transmission or serialization,
                                     // this
                                     // is where the object saves its contents
    String m_strPayment;  // Contains the cheque / payment plan / etc in string
                          // form.
    paymentType m_Type = ERROR_STATE;  // Default value is ERROR_STATE
    // Once the actual instrument is loaded up, we copy some temp values to
    // *this
    // object. Until then, this bool (m_bAreTempValuesSet) is set to false.
    //
    bool m_bAreTempValuesSet = false;

    // Here are the TEMP values:
    // (These are not serialized.)
    //
    bool m_bHasRecipient = false;  // For cheques mostly, and payment plans too.
    bool m_bHasRemitter =
        false;  // For vouchers (cashier's cheques), the Nym who bought
                // the voucher is the remitter, whereas the "sender" is
                // the server Nym whose account the voucher is drawn on.

    int64_t m_lAmount =
        0;  // Contains 0 by default. This is set by SetPayment()
            // along with other useful values.
    int64_t m_lTransactionNum = 0;  // Contains 0 by default. This is set by
    // SetPayment() along with other useful values.
    int64_t m_lTransNumDisplay = 0;  // Contains 0 by default. This is set by
    // SetPayment() along with other useful values.

    String m_strMemo;  // Memo, Consideration, Subject, etc.

    Identifier m_InstrumentDefinitionID;  // These are for convenience only, for
                                          // caching
                                          // once they happen to be loaded.
    Identifier m_NotaryID;     // These values are NOT serialized other than via
                               // the payment instrument itself
    Identifier m_SenderNymID;  // (where they are captured from, whenever it
                               // is instantiated.) Until m_bAreTempValuesSet
    Identifier m_SenderAcctID;     // is set to true, these values can NOT be
                                   // considered available. Use the accessing
                                   // methods
    Identifier m_RecipientNymID;   // below. These values are not ALL always
                                   // available, depending on the payment
                                   // instrument
    Identifier m_RecipientAcctID;  // type. Different payment instruments
                                   // support different temp values.
    Identifier m_RemitterNymID;    // A voucher (cashier's cheque) has the
                                   // "bank" as the sender. Whereas the Nym who
                                   // actually purchased the voucher is the
                                   // remitter.
    Identifier m_RemitterAcctID;   // A voucher (cashier's cheque) has the
                                   // "bank"s account as the sender acct.
                                   // Whereas the account that was originally
                                   // used to purchase the voucher is the
                                   // remitter account.
    time64_t m_VALID_FROM = 0;     // Temporary values. Not always available.
    time64_t m_VALID_TO = 0;       // Temporary values. Not always available.

    void lowLevelSetTempValuesFromPaymentPlan(const OTPaymentPlan& theInput);
    void lowLevelSetTempValuesFromSmartContract(
        const OTSmartContract& theInput);

public:
    EXPORT bool SetPayment(const String& strPayment);

    EXPORT bool IsCheque() const { return (CHEQUE == m_Type); }
    EXPORT bool IsVoucher() const { return (VOUCHER == m_Type); }
    EXPORT bool IsInvoice() const { return (INVOICE == m_Type); }
    EXPORT bool IsPaymentPlan() const { return (PAYMENT_PLAN == m_Type); }
    EXPORT bool IsSmartContract() const { return (SMART_CONTRACT == m_Type); }
    EXPORT bool IsPurse() const { return (PURSE == m_Type); }
    EXPORT bool IsNotice() const { return (NOTICE == m_Type); }
    EXPORT bool IsValid() const { return (ERROR_STATE != m_Type); }

    EXPORT paymentType GetType() const { return m_Type; }
    EXPORT OTTrackable* Instantiate() const;
    EXPORT OTTrackable* Instantiate(const String& strPayment);

    EXPORT Purse* InstantiatePurse() const;
    EXPORT Purse* InstantiatePurse(const String& strPayment);

    EXPORT OTTransaction* InstantiateNotice() const;
    EXPORT OTTransaction* InstantiateNotice(const String& strNotice);

    EXPORT bool GetPaymentContents(String& strOutput) const
    {
        strOutput = m_strPayment;
        return true;
    }

    // Since the temp values are not available until at least ONE instantiating
    // has occured,
    // this function forces that very scenario (cleanly) so you don't have to
    // instantiate-and-
    // then-delete a payment instrument. Instead, just call this, and then the
    // temp values will
    // be available thereafter.
    //
    EXPORT bool SetTempValues();

    EXPORT bool SetTempValuesFromCheque(const Cheque& theInput);
    EXPORT bool SetTempValuesFromPaymentPlan(const OTPaymentPlan& theInput);
    EXPORT bool SetTempValuesFromSmartContract(const OTSmartContract& theInput);
    EXPORT bool SetTempValuesFromPurse(const Purse& theInput);
    EXPORT bool SetTempValuesFromNotice(const OTTransaction& theInput);
    // Once you "Instantiate" the first time, then these values are
    // set, if available, and can be queried thereafter from *this.
    // Otherwise, these functions will return false.
    //
    EXPORT bool GetAmount(int64_t& lOutput) const;

    EXPORT bool GetTransactionNum(int64_t& lOutput) const;
    EXPORT bool GetTransNumDisplay(int64_t& lOutput) const;

    // Only works for payment plans and smart contracts. Gets the
    // opening transaction number for a given Nym, if applicable.
    // (Or closing number for a given asset account.)
    EXPORT bool GetOpeningNum(int64_t& lOutput, const Identifier& theNymID)
        const;
    EXPORT bool GetClosingNum(int64_t& lOutput, const Identifier& theAcctID)
        const;
    EXPORT bool GetAllTransactionNumbers(NumList& numlistOutput) const;
    EXPORT bool HasTransactionNum(const int64_t& lInput) const;
    EXPORT bool GetMemo(String& strOutput) const;
    EXPORT bool GetInstrumentDefinitionID(Identifier& theOutput) const;
    EXPORT bool GetNotaryID(Identifier& theOutput) const;
    EXPORT bool GetSenderNymID(Identifier& theOutput) const;
    EXPORT bool GetSenderAcctID(Identifier& theOutput) const;
    EXPORT bool GetRecipientNymID(Identifier& theOutput) const;
    EXPORT bool GetRecipientAcctID(Identifier& theOutput) const;
    EXPORT bool GetRemitterNymID(Identifier& theOutput) const;
    EXPORT bool GetRemitterAcctID(Identifier& theOutput) const;
    EXPORT bool GetSenderNymIDForDisplay(Identifier& theOutput) const;
    EXPORT bool GetSenderAcctIDForDisplay(Identifier& theOutput) const;
    EXPORT bool GetValidFrom(time64_t& tOutput) const;
    EXPORT bool GetValidTo(time64_t& tOutput) const;
    EXPORT bool VerifyCurrentDate(bool& bVerified);  // Verify whether the
                                                     // CURRENT date is WITHIN
                                                     // the VALID FROM / TO
                                                     // dates.
    EXPORT bool IsExpired(bool& bExpired);  // Verify whether the CURRENT date
                                            // is
                                            // AFTER the the "VALID TO" date.
    EXPORT OTPayment();
    EXPORT OTPayment(const String& strPayment);
    EXPORT virtual ~OTPayment();
    EXPORT void InitPayment();
    EXPORT void Release() override;
    EXPORT void Release_Payment();

    EXPORT int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;
    EXPORT static const char* _GetTypeString(paymentType theType);
    EXPORT const char* GetTypeString() const { return _GetTypeString(m_Type); }
    EXPORT static paymentType GetTypeFromString(const String& strType);

    EXPORT const String& Payment() const { return m_strPayment; }
};

}  // namespace opentxs

#endif  // OPENTXS_EXT_OTPAYMENT_HPP
