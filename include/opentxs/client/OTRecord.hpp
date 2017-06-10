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

#ifndef OPENTXS_CLIENT_OTRECORD_HPP
#define OPENTXS_CLIENT_OTRECORD_HPP

#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Types.hpp"

#include <stdint.h>
#include <string>

namespace opentxs
{

class OTRecordList;

class OTRecord
{
public:
    enum OTRecordType { Mail = 0, Transfer, Receipt, Instrument, Notice, ErrorState };

private:
    OTRecordList & backlink_;
    int32_t m_nBoxIndex{-1};
    time64_t m_ValidFrom;
    time64_t m_ValidTo;
    const std::string& m_str_notary_id;
    const std::string& m_str_instrument_definition_id;
    const std::string& m_str_currency_tla;
    const std::string& m_str_nym_id;
    const std::string& m_str_account_id;
    std::string m_str_other_nym_id;
    std::string m_str_other_account_id;
    std::string m_str_name;
    std::string m_str_date;
    std::string m_str_amount;
    std::string m_str_type;
    std::string m_str_memo;
    // Contains payment instrument or mail message (or nothing, if not
    // applicable.)
    //
    std::string m_str_contents;
    // SPECIAL MAIL
    //
    // Let's say this OTRecord is a Mail, but it's not via OT. (Perhaps it's
    // from Bitmessage or Freenet.) In that case it's a "special mail." In that
    // case, there are a few additional pieces of data we store here.
    // For example, the method ID (if one is available.) If you use Moneychanger
    // to send or receive a message through Bitmessage, the sender or recipient
    // will have a Method ID which references the connection string used to send
    // or receive. There will also be the message type "bitmessage" with the
    // type display string "Bitmessage", and the sender and recipient addresses.
    //
    bool m_bIsSpecialMail{false}; // Meaning a bitmessage vs an OT message.
    int32_t m_nMethodID{0};       // A Nym in Moneychanger might have 2 Bitmessage
                                  // addresses, each used on different BM nodes with
                                  // different connection strings. The Method ID is used
                                  // to lookup the connection string.
    std::string m_str_my_address;    // My Bitmessage address.
    std::string m_str_other_address; // The sender or recipient's Bitmessage
                                     // address.
    std::string m_str_msg_id;   // If you want to delete a Bitmessage, you must
                                // know the message ID.
    std::string m_str_msg_type; // "bitmessage"
    std::string m_str_msg_type_display; // "Bitmessage"
    // Contains transaction number of actual receipt in inbox,
    // or payment inbox, or record box. (If outpayment, contains
    // transaction number on outgoing instrument.)
    //
    int64_t m_lTransactionNum{0};
    int64_t m_lTransNumForDisplay{0};
    int64_t m_lClosingNum{0};  // Only used for finalReceipts.

    bool m_bIsPending;
    bool m_bIsOutgoing;
    bool m_bIsRecord;  // record box (closed, finished, historical only.)
    bool m_bIsReceipt; // It's a receipt, not a payment.
    bool m_bIsPaymentPlan{false};
    bool m_bIsSmartContract{false};
    bool m_bIsVoucher{false};
    bool m_bIsCheque{false};
    bool m_bIsInvoice{false};
    bool m_bIsCash{false};
    bool m_bIsNotice{false};
    bool m_bIsExpired{false};
    bool m_bIsCanceled{false};
    bool m_bIsFinalReceipt{false};
    bool m_bHasOriginType{false};
    originType   m_originType{originType::not_applicable};
    OTRecordType m_RecordType{ErrorState};

    bool m_bHasSuccess{false};  // Does it even HAVE a "success" state?
                                // (Incoming cheque, for example, does NOT.)
    bool m_bIsSuccess {false};  // If it DOES have a "success" state, then
                                // is it set to a success or a failure?
    bool AcceptIncomingTransferOrReceipt() const;

public:
    void SetOriginType(originType theOriginType);

    // Only a few receipts (finalReceipts and paymentReceipts currently)
    // even have an origin type set. (Usually origin is already known and
    // obvious.)
    // But in a few cases, we need to know. For example if the finalReceipt
    // came from a market offer, a payment plan, or a smart contract. Which
    // is it? The origin type tells you this.
    //
    // Make SURE you check HasOriginType first, since the answer given by
    // the other functions may otherwise seem wrong.
    //
    EXPORT bool HasOriginType() const;
    EXPORT bool IsOriginTypeMarketOffer() const;
    EXPORT bool IsOriginTypePaymentPlan() const;
    EXPORT bool IsOriginTypeSmartContract() const;

    EXPORT void SetSuccess(const bool bIsSuccess);
    EXPORT bool HasSuccess(bool & bIsSuccess) const;

    EXPORT void SetClosingNum(const int64_t lClosingNum);
    EXPORT bool GetClosingNum(int64_t & lClosingNum) const;

    EXPORT void SetSpecialMail(bool bIsSpecial = true);
    EXPORT bool IsSpecialMail() const;
    EXPORT bool IsPending() const;
    EXPORT bool IsOutgoing() const;
    EXPORT bool IsRecord() const;
    EXPORT bool IsReceipt() const;
    EXPORT bool IsFinalReceipt() const;
    EXPORT bool IsMail() const;
    EXPORT bool IsTransfer() const;
    EXPORT bool IsCheque() const;
    EXPORT bool IsInvoice() const;
    EXPORT bool IsVoucher() const;
    EXPORT bool IsContract() const;
    EXPORT bool IsPaymentPlan() const;
    EXPORT bool IsCash() const;
    EXPORT bool IsNotice() const;
    EXPORT bool HasContents() const;
    EXPORT bool HasMemo() const;
    EXPORT bool IsExpired() const;
    EXPORT bool IsCanceled() const;
    EXPORT void SetExpired();
    EXPORT void SetCanceled();
    EXPORT void SetFinalReceipt(bool bValue=true);
    EXPORT time64_t GetValidFrom() const;
    EXPORT time64_t GetValidTo() const;
    EXPORT void SetDateRange(time64_t tValidFrom,
                             time64_t tValidTo);
    EXPORT bool CanDeleteRecord() const; // For completed records (not pending.)
    EXPORT bool CanAcceptIncoming() const;  // For incoming, pending
                                            // (not-yet-accepted) instruments.
    EXPORT bool CanDiscardIncoming() const; // For INcoming, pending
                                            // (not-yet-accepted) instruments.
    EXPORT bool CanCancelOutgoing() const;  // For OUTgoing, pending
                                            // (not-yet-accepted) instruments.
    EXPORT bool CanDiscardOutgoingCash() const; // For OUTgoing cash. (No way to
                                                // see if it's been accepted, so
                                                // this lets you erase the
                                                // record of sending it.)
    EXPORT bool CancelOutgoing(std::string str_via_acct) const; // For outgoing, pending
                                                                // (not-yet-accepted) instruments.
                                                                // NOTE: str_via_acct can be blank if a
                                                                // cheque. But if voucher, smart
                                                                // contract, payment plan, you must
                                                                // provide.
    EXPORT bool AcceptIncomingInstrument(
        const std::string& str_into_acct) const; // For incoming, pending
                                                 // (not-yet-accepted) instruments.
    EXPORT bool AcceptIncomingTransfer() const;  // For incoming, pending
                                                 // (not-yet-accepted) transfers.
    EXPORT bool AcceptIncomingReceipt() const;   // For incoming,
                                                 // (not-yet-accepted)
                                                 // receipts.
    EXPORT bool DiscardIncoming() const;         // For incoming, pending
                                                 // (not-yet-accepted)
                                                 // instruments.
    EXPORT bool DeleteRecord() const; // For completed records (not pending.)
    EXPORT bool DiscardOutgoingCash() const; // For OUTgoing cash. (No way to
                                             // see if it's been accepted, so this lets you
                                             // erase the record of sending it.)
    EXPORT int32_t GetBoxIndex() const; // If this is set to 3, for example, for
                                        // a payment in the payments inbox, then
                                        // index 3 in that same box refers to
                                        // the payment corresponding to this
                                        // record.
    EXPORT void SetBoxIndex(int32_t nBoxIndex);
    EXPORT int32_t GetMethodID() const; // Used by "special mail."
    EXPORT void SetMethodID(int32_t nMethodID);
    EXPORT const std::string& GetMsgID() const; // Used by "special mail."
    EXPORT void SetMsgID(const std::string& str_id);
    EXPORT const std::string& GetMsgType() const; // Used by "special mail."
    EXPORT void SetMsgType(const std::string& str_type);
    EXPORT const std::string& GetMsgTypeDisplay() const; // Used by "special
                                                         // mail."
    EXPORT void SetMsgTypeDisplay(const std::string& str_type);
    EXPORT int64_t GetTransactionNum() const; // Trans Num of receipt in the
                                              // box. (Unless outpayment,
                                              // contains number for
                                              // instrument.)
    EXPORT void SetTransactionNum(int64_t lTransNum);
    EXPORT int64_t GetTransNumForDisplay() const; // Trans Num of the cheque
                                                  // inside the receipt in the
                                                  // box.
    EXPORT void SetTransNumForDisplay(int64_t lTransNum);
    EXPORT OTRecordType GetRecordType() const;
    EXPORT const std::string& GetNotaryID() const;
    EXPORT const std::string& GetInstrumentDefinitionID() const;
    EXPORT const std::string& GetCurrencyTLA() const; // BTC, USD, etc.
    EXPORT const std::string& GetNymID() const;
    EXPORT const std::string& GetAccountID() const;
    EXPORT const std::string& GetAddress() const;    // Used by "special mail"
    EXPORT const std::string& GetOtherNymID() const; // Could be sender OR
                                                     // recipient depending on
                                                     // whether
                                                     // incoming/outgoing.
    EXPORT const std::string& GetOtherAccountID() const; // Could be sender OR
                                                         // recipient depending
                                                         // on whether
                                                         // incoming/outgoing.
    EXPORT const std::string& GetOtherAddress() const; // Used by "special mail"
    EXPORT const std::string& GetName() const;
    EXPORT const std::string& GetDate() const;
    EXPORT const std::string& GetAmount() const;
    EXPORT const std::string& GetInstrumentType() const;
    EXPORT const std::string& GetMemo() const;
    EXPORT const std::string& GetContents() const;
    EXPORT void SetOtherNymID(const std::string& str_ID);
    EXPORT void SetOtherAccountID(const std::string& str_ID);
    EXPORT void SetAddress(const std::string& str_Address); // Used by "special
                                                            // mail"
    EXPORT void SetOtherAddress(const std::string& str_Address); // Used by
                                                                 // "special
                                                                 // mail"
    EXPORT void SetMemo(const std::string& str_memo);
    EXPORT void SetContents(const std::string& str_contents);
    // These don't work unless the record is for a pending
    // payment plan.
    //
    EXPORT bool HasInitialPayment() const;
    EXPORT bool HasPaymentPlan() const;

    EXPORT time64_t GetInitialPaymentDate() const;
    EXPORT time64_t GetPaymentPlanStartDate() const;
    EXPORT time64_t GetTimeBetweenPayments() const;

    EXPORT int64_t GetInitialPaymentAmount() const;
    EXPORT int64_t GetPaymentPlanAmount() const;

    EXPORT int32_t GetMaximumNoPayments() const;
    EXPORT bool FormatAmount(std::string& str_output) const;
    EXPORT bool FormatAmountWithoutSymbol(std::string& str_output);
    EXPORT bool FormatAmountLocale(std::string& str_output,
                                   const std::string& str_thousands,
                                   const std::string& str_decimal) const;
    EXPORT bool FormatAmountWithoutSymbolLocale(
        std::string& str_output, const std::string& str_thousands,
        const std::string& str_decimal);
    EXPORT bool FormatDescription(std::string& str_output) const;
    EXPORT bool FormatShortMailDescription(std::string& str_output) const;
    EXPORT bool FormatMailSubject(std::string& str_output) const;
    bool operator<(const OTRecord& rhs);
    OTRecord(OTRecordList & backlink,
             const std::string& str_notary_id,
             const std::string& str_instrument_definition_id,
             const std::string& str_currency_tla, const std::string& str_nym_id,
             const std::string& str_account_id, const std::string& str_name,
             const std::string& str_date, const std::string& str_amount,
             const std::string& str_type, bool bIsPending, bool bIsOutgoing,
             bool bIsRecord, bool bIsReceipt, OTRecordType eRecordType);
};

} // namespace opentxs

#endif // OPENTXS_CLIENT_OTRECORD_HPP
