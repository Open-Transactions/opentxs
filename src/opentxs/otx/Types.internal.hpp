// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ConsensusType
// IWYU pragma: no_forward_declare opentxs::proto::OTXPushType
// IWYU pragma: no_include "opentxs/otx/PushType.hpp"

#pragma once

#include <ConsensusEnums.pb.h>
#include <OTXEnums.pb.h>
#include <cstdint>

#include "opentxs/otx/Types.hpp"

namespace opentxs::otx
{
constexpr auto OTX_PUSH_VERSION = 1;

enum class itemType : std::uint8_t {
    // TRANSFER
    transfer,    // this item is an outgoing transfer, probably part of an
                 // outoing transaction.
    atTransfer,  // Server reply.

    // NYMBOX RESOLUTION
    acceptTransaction,  // this item is a client-side acceptance of a
                        // transaction number (a blank) in my Nymbox
    atAcceptTransaction,
    acceptMessage,  // this item is a client-side acceptance of a message in
                    // my Nymbox
    atAcceptMessage,
    acceptNotice,  // this item is a client-side acceptance of a server
                   // notification in my Nymbox
    atAcceptNotice,

    // INBOX RESOLUTION
    acceptPending,  // this item is a client-side acceptance of a pending
                    // transfer
    atAcceptPending,
    rejectPending,  // this item is a client-side rejection of a pending
                    // transfer
    atRejectPending,

    // RECEIPT ACKNOWLEDGMENT / DISPUTE
    acceptCronReceipt,     // this item is a client-side acceptance of a cron
                           // receipt in his inbox.
    atAcceptCronReceipt,   // this item is a server reply to that acceptance.
    acceptItemReceipt,     // this item is a client-side acceptance of an item
                           // receipt in his inbox.
    atAcceptItemReceipt,   // this item is a server reply to that acceptance.
    disputeCronReceipt,    // this item is a client dispute of a cron receipt
                           // in his inbox.
    atDisputeCronReceipt,  // Server reply to dispute message.
    disputeItemReceipt,    // this item is a client dispute of an item receipt
                           // in his inbox.
    atDisputeItemReceipt,  // Server reply to dispute message.

    // Sometimes the attachment will be an Item, and sometimes it will be
    // an OTPaymentPlan or OTTrade.  These different types above help the
    // code to differentiate.
    acceptFinalReceipt,     // this item is a client-side acceptance of a final
                            // receipt in his inbox. (All related receipts must
                            // also be closed!)
    atAcceptFinalReceipt,   // server reply
    acceptBasketReceipt,    // this item is a client-side acceptance of a
                            // basket receipt in his inbox.
    atAcceptBasketReceipt,  // server reply
    disputeFinalReceipt,    // this item is a client-side rejection of a final
    // receipt in his inbox. (All related receipts must
    // also be closed!)
    atDisputeFinalReceipt,   // server reply
    disputeBasketReceipt,    // this item is a client-side rejection of a
                             // basket receipt in his inbox.
    atDisputeBasketReceipt,  // server reply

    // FEEs
    serverfee,  // this item is a fee from the transaction server (per
                // contract)
    atServerfee,
    issuerfee,  // this item is a fee from the issuer (per contract)
    atIssuerfee,
    // INFO (BALANCE, HASH, etc) these are still all messages with replies.
    balanceStatement,  // this item is a statement of balance. (For asset
                       // account.)
    atBalanceStatement,
    transactionStatement,  // this item is a transaction statement. (For Nym
                           // -- which numbers are assigned to him.)
    atTransactionStatement,
    // CASH WITHDRAWAL / DEPOSIT
    withdrawal,  // this item is a cash withdrawal (of chaumian blinded
                 // tokens)
    atWithdrawal,
    deposit,  // this item is a cash deposit (of a purse containing blinded
              // tokens.)
    atDeposit,
    // CHEQUES AND VOUCHERS
    withdrawVoucher,  // this item is a request to purchase a voucher (a
                      // cashier's cheque)
    atWithdrawVoucher,
    depositCheque,    // this item is a request to deposit a cheque
    atDepositCheque,  // this item is a server response to that request.
    // PAYING DIVIDEND ON SHARES OF STOCK
    payDividend,    // this item is a request to pay a dividend.
    atPayDividend,  // the server reply to that request.
    // TRADING ON MARKETS
    marketOffer,    // this item is an offer to be put on a market.
    atMarketOffer,  // server reply or updated notification regarding a
                    // market offer.
    // PAYMENT PLANS
    paymentPlan,    // this item is a new payment plan
    atPaymentPlan,  // server reply or updated notification regarding a
                    // payment plan.
    // SMART CONTRACTS
    smartContract,    // this item is a new smart contract
    atSmartContract,  // server reply or updated notification regarding a
                      // smart contract.
    // CANCELLING: Market Offers and Payment Plans.
    cancelCronItem,    // this item is intended to cancel a market offer or
                       // payment plan.
    atCancelCronItem,  // reply from the server regarding said cancellation.
    // EXCHANGE IN/OUT OF A BASKET CURRENCY
    exchangeBasket,    // this item is an exchange in/out of a basket
                       // currency.
    atExchangeBasket,  // reply from the server regarding said exchange.
    // Now these three receipts have a dual use:  as the receipts in the
    // inbox, and also
    // as the representation for transactions in the inbox report (for
    // balance statements.)
    // Actually chequeReceipt is ONLY used for inbox report, and otherwise
    // is not actually
    // needed for real cheque receipts.  marketReceipt and paymentReceipt
    // are used as real
    // receipts, and also in inbox reports to represent transaction items in
    // an inbox.
    chequeReceipt,  // Currently don't create an Item for cheque receipt
                    // in
                    // inbox. Not needed.
    // I also don't create one for the transfer receipt, currently.
    // (Although near the top, I do have item types to go in a processInbox
    // message and
    // clear those transaction types out of my inbox.)
    voucherReceipt,   // Newest addition. This is so users can close a
                      // transaction number used on a voucher.
    marketReceipt,    // server receipt dropped into inbox as result of market
                      // trading.
    paymentReceipt,   // server receipt dropped into an inbox as result of
                      // payment occuring.
    transferReceipt,  // server receipt dropped into an inbox as result of
                      // transfer being accepted.
    finalReceipt,     // server receipt dropped into inbox / nymbox as result
                      // of
                      // cron item expiring or being canceled.
    basketReceipt,    // server receipt dropped into inbox as result of a
                      // basket exchange.
    replyNotice,      // server notice of a reply that nym should have already
                      // received as a response to a request.
    // (Some are so important, a copy of the server reply is dropped to your
    // nymbox, to make SURE you got it and processed it.)
    successNotice,  // server notice dropped into nymbox as result of a
                    // transaction# being successfully signed out.
    notice,         // server notice dropped into nymbox as result of a smart
                    // contract processing.
    // Also could be used for ballots / elections, corporate meetings /
    // minutes, etc.
    // finalReceipt is also basically a notice (in the Nymbox, anyway) but
    // it still is
    // information that you have to act on as soon as you receive it,
    // whereas THIS kind
    // of notice isn't so hardcore. It's more laid-back.
    error_state  // error state versus error status
};               // IWYU pragma: export

enum class ledgerType : std::uint8_t {
    nymbox,   // the nymbox is per user account (versus per asset account)
              // and is used to receive new transaction numbers (and
              // messages.)
    inbox,    // each asset account has an inbox, with pending transfers as
              // well as receipts inside.
    outbox,   // if you SEND a pending transfer, it sits in your outbox until
              // it's accepted, rejected, or canceled.
    message,  // used in OTMessages, to send various lists of transactions
              // back and forth.
    paymentInbox,  // Used for client-side-only storage of incoming cheques,
                   // invoices, payment plan requests, etc. (Coming in from
                   // the Nymbox.)
    recordBox,     // Used for client-side-only storage of completed items from
                   // the inbox, and the paymentInbox.
    expiredBox,    // Used for client-side-only storage of expired items from
                   // the paymentInbox.
    error_state
};  // IWYU pragma: export
// WARNING If you add any types to this list, update the list of strings at
// thetop of Ledger.cpp.

enum class MessageType : std::uint8_t {
    badID = 0,
    // Your public key is sent along with this message so the server can
    // reply to you even without your being a registered user. Other than
    // these top two commands, all other commands can only be executed by
    // registered users.
    //
    // The server ID is a hash of the server contract. The signature on the
    // contract can be verified by a public key that appears in a standard
    // section of any server contract. The URL/port information is also
    // derived from the contract.
    //
    // Simply by importing the server contract into your wallet, you are
    // able to connect to it and encrypt all of your communications to it.
    //
    // Thus, the check server ID command really just confirms what you
    // should already know... Your wallet still wants to see that the server
    // agrees with the server ID, and that the server is able to read
    // messages that were encrypted to the public key in the contract, and
    // that the server is able to sign all of its future correspondence with
    // the same public key.
    //
    // It is the server operator's responsibility to secure the domain name
    // and web host that users will connect to when they import the
    // contract, as well as the private key that matches the public key from
    // the contract.
    pingNotary = 1,
    pingNotaryResponse = 2,
    // register user account on a specific server, with public key. Nym ID
    // will be hash of said public key.
    registerNym = 3,
    registerNymResponse = 4,
    // Delete user account from a specific server.
    unregisterNym = 5,
    unregisterNymResponse = 6,
    // Get the next request number from the server (for this user). Most
    // requests must be accompanied by a request number, which increments
    // for each Nym with each request.
    getRequestNumber = 7,
    getRequestNumberResponse = 8,
    // Every transaction requires a transaction number. If your wallet
    // doesn't have one, then here it can request the server to send one
    // over. (Or several.)
    getTransactionNumbers = 9,
    getTransactionNumbersResponse = 10,
    // Used by AcceptEntireNymbox() as it's setting everything up.
    processNymbox = 11,
    processNymboxResponse = 12,
    checkNym = 13,
    checkNymResponse = 14,
    sendNymMessage = 15,
    sendNymMessageResponse = 16,
    sendNymInstrument = 17,
    // sendNymInstrumentResponse = 18,
    unregisterAccount = 19,
    unregisterAccountResponse = 20,
    registerAccount = 21,
    registerAccountResponse = 22,
    registerInstrumentDefinition = 23,
    registerInstrumentDefinitionResponse = 24,
    issueBasket = 25,
    issueBasketResponse = 26,
    notarizeTransaction = 27,
    notarizeTransactionResponse = 28,
    getNymbox = 29,
    getNymboxResponse = 30,
    getBoxReceipt = 31,
    getBoxReceiptResponse = 32,
    getAccountData = 33,
    getAccountDataResponse = 34,
    processInbox = 35,
    processInboxResponse = 36,
    queryInstrumentDefinitions = 37,
    queryInstrumentDefinitionsResponse = 38,
    getInstrumentDefinition = 39,
    getInstrumentDefinitionResponse = 40,
    getMint = 41,
    getMintResponse = 42,
    getMarketList = 43,
    getMarketListResponse = 44,
    getMarketOffers = 45,
    getMarketOffersResponse = 46,
    getMarketRecentTrades = 47,
    getMarketRecentTradesResponse = 48,
    getNymMarketOffers = 49,
    getNymMarketOffersResponse = 50,
    triggerClause = 51,
    triggerClauseResponse = 52,
    usageCredits = 53,
    usageCreditsResponse = 54,
    registerContract = 55,
    registerContractResponse = 56,
    requestAdmin = 57,
    requestAdminResponse = 58,
    addClaim = 59,
    addClaimResponse = 60,
    outmail = 61,
};  // IWYU pragma: export

// originType is DISPLAY ONLY. Used in OTTransaction and Item.
// sometimes an Item is used to represent an OTTransaction.
// (for example, processInbox transaction has a processInbox item that
// contains a list of sub-items that represent the receipts aka
// OTTransactions in the inbox.)
//
// This is used for for finalReceipts and for paymentReceipts,
// so the GUI can sort them properly without having to load up the
// original transaction and see its type.
// This won't affect the actual operation of OT itself, which ignores
// this value. It's just here to help the GUI to sort receipts that
// have already been closed, with less work necessary to do so.
//
// NOTE: I'll also use this for paymentReceipts, so I can distinguish
// smart contract receipts from payment plan receipts. In the case of
// marketReceipts, it's not that important, since we already know it's
// for a market trade. But with paymentReceipts, it's useful. (And
// finalReceipts.) Maybe I should create a "contractReceipt" to fix
// that ambiguity.
//
enum class originType : std::int8_t {
    not_applicable,
    origin_market_offer,    // finalReceipt
    origin_payment_plan,    // finalReceipt, paymentReceipt
    origin_smart_contract,  // finalReceipt, paymentReceipt
    origin_pay_dividend,    // SOME voucherReceipts are from a payDividend.
    origin_error_state
};  // IWYU pragma: export

// a transaction can be blank (issued from server) or pending (in the
// inbox/outbox) or it can be a "process inbox" transaction might also be in the
// nymbox.
enum class transactionType : std::int8_t {
    // ***** INBOX / OUTBOX / NYMBOX

    // NYMBOX
    blank,          // freshly issued transaction number, not used yet
                    // (the server drops these into the nymbox.)
    message,        // A message from one user to another, also in the nymbox.
    notice,         // A notice from the server. Used in Nymbox.
    replyNotice,    // A copy of a server reply to a previous request you
                    // sent.
                    // (To make SURE you get the reply.)
    successNotice,  // A transaction # has successfully been signed out.

    // INBOX / OUTBOX (pending transfer)
    pending,  // Server puts this in your outbox (when sending) and
              // recipient's inbox.

    // INBOX / receipts
    transferReceipt,  // the server drops this into your inbox, when someone
                      // accepts your transfer.
    chequeReceipt,    // the server drops this into your inbox, when someone
                      // deposits your cheque.
    voucherReceipt,   // the server drops this into your inbox, when someone
                      // deposits your voucher.
    marketReceipt,    // server periodically drops this into your inbox if an
                      // offer is live.
    paymentReceipt,   // the server drops this into people's inboxes, every
                      // time a payment processes. (from a payment plan or a
                      // smart contract)
    finalReceipt,     // the server drops this into your in/nym box(es), when a
                      // CronItem expires or is canceled.
    basketReceipt,    // the server drops this into your inboxes, when a
                      // basket
                      // exchange is processed.

    // PAYMENT INBOX / PAYMENT OUTBOX / RECORD BOX
    instrumentNotice,     // Receive these in paymentInbox (by way of Nymbox),
                          // and send in Outpayments (like Outmail).) (When
                          // done, they go to recordBox or expiredBox to await
                          // deletion.)
    instrumentRejection,  // When someone rejects your invoice from his
                          // paymentInbox, you get one of these in YOUR
                          // paymentInbox.

    // **** MESSAGES ****
    processNymbox,    // process nymbox transaction    // comes from client
    atProcessNymbox,  // process nymbox reply          // comes from server
    processInbox,     // process inbox transaction     // comes from client
    atProcessInbox,   // process inbox reply           // comes from server
    transfer,      // or "spend". This transaction is a request to transfer from
                   // one account to another
    atTransfer,    // reply from the server regarding a transfer request
    deposit,       // this transaction is a deposit (cash or cheque)
    atDeposit,     // reply from the server regarding a deposit request
    withdrawal,    // this transaction is a withdrawal (cash or voucher)
    atWithdrawal,  // reply from the server regarding a withdrawal request
    marketOffer,   // this transaction is a market offer
    atMarketOffer,     // reply from the server regarding a market offer
    paymentPlan,       // this transaction is a payment plan
    atPaymentPlan,     // reply from the server regarding a payment plan
    smartContract,     // this transaction is a smart contract
    atSmartContract,   // reply from the server regarding a smart contract
    cancelCronItem,    // this transaction is intended to cancel a market
                       // offer
                       // or payment plan.
    atCancelCronItem,  // reply from the server regarding said cancellation.
    exchangeBasket,    // this transaction is an exchange in/out of a basket
                       // currency.
    atExchangeBasket,  // reply from the server regarding said exchange.
    payDividend,       // this transaction is dividend payment (to all
                       // shareholders...)
    atPayDividend,     // reply from the server regarding said dividend
                       // payment.
    incomingCash,
    error_state
};  // IWYU pragma: export
// WARNING If you add any types to this list, update the list of strings at the
// top of OTTransaction.cpp.

auto translate(ConsensusType in) noexcept -> proto::ConsensusType;
auto translate(LastReplyStatus in) noexcept -> proto::LastReplyStatus;
auto translate(PushType in) noexcept -> proto::OTXPushType;
auto translate(ServerReplyType in) noexcept -> proto::ServerReplyType;
auto translate(ServerRequestType in) noexcept -> proto::ServerRequestType;
auto translate(const proto::ConsensusType in) noexcept -> ConsensusType;
auto translate(const proto::LastReplyStatus in) noexcept -> LastReplyStatus;
auto translate(const proto::OTXPushType in) noexcept -> PushType;
auto translate(const proto::ServerReplyType in) noexcept -> ServerReplyType;
auto translate(const proto::ServerRequestType in) noexcept -> ServerRequestType;
}  // namespace opentxs::otx
