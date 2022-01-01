// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <vector>

#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
namespace identity
{
class Nym;
}  // namespace identity

class Identifier;
class Message;
class String;
}  // namespace opentxs

namespace opentxs
{
using CredentialIndexModeFlag = bool;
static const CredentialIndexModeFlag CREDENTIAL_INDEX_MODE_ONLY_IDS = true;
static const CredentialIndexModeFlag CREDENTIAL_INDEX_MODE_FULL_CREDS = false;

using CredentialModeFlag = bool;
static const CredentialModeFlag PRIVATE_VERSION = true;
static const CredentialModeFlag PUBLIC_VERSION = false;

using SerializationModeFlag = bool;
static const SerializationModeFlag AS_PRIVATE = true;
static const SerializationModeFlag AS_PUBLIC = false;

using SerializationSignatureFlag = bool;
static const SerializationSignatureFlag WITH_SIGNATURES = true;
static const SerializationSignatureFlag WITHOUT_SIGNATURES = false;

using ProtoValidationVerbosity = bool;
static const ProtoValidationVerbosity SILENT = true;
static const ProtoValidationVerbosity VERBOSE = false;

using BIP44Chain = bool;
static const BIP44Chain INTERNAL_CHAIN = true;
static const BIP44Chain EXTERNAL_CHAIN = false;

using BlockMode = bool;
static const BlockMode BLOCK_MODE = true;
static const BlockMode NOBLOCK_MODE = false;

enum class StringStyle : bool { Hex = true, Raw = false };

using GetPreimage = std::function<std::string()>;
using SimpleCallback = std::function<void()>;

using DhtResults = std::pmr::vector<std::shared_ptr<std::string>>;

using DhtDoneCallback = std::function<void(bool)>;
using DhtResultsCallback = std::function<bool(const DhtResults&)>;

using PeriodicTask = std::function<void()>;

/** C++11 representation of a claim. This version is more useful than the
 *  protobuf version, since it contains the claim ID.
 */
using Claim = std::tuple<
    std::string,                    // claim identifier
    std::uint32_t,                  // section
    std::uint32_t,                  // type
    std::string,                    // value
    std::int64_t,                   // start time
    std::int64_t,                   // end time
    std::pmr::set<std::uint32_t>>;  // attributes
using ClaimTuple = Claim;

/** C++11 representation of all contact data associated with a nym, aggregating
 *  each the nym's contact credentials in the event it has more than one.
 */
using ClaimSet = std::pmr::set<Claim>;

/** A list of object IDs and their associated aliases
 *  * string: id of the stored object
 *  * string: alias of the stored object
 */
using ObjectList = std::pmr::list<std::pair<std::string, std::string>>;

using RawData = std::pmr::vector<unsigned char>;

using Nym_p = std::shared_ptr<const identity::Nym>;

// local ID, remote ID
using ContextID = std::pair<std::string, std::string>;
using ContextLockCallback =
    std::function<std::recursive_mutex&(const ContextID&)>;
using SetID = std::function<void(const Identifier&)>;

using NetworkOperationStatus = std::int32_t;

using Lock = std::unique_lock<std::mutex>;
using rLock = std::unique_lock<std::recursive_mutex>;
using sLock = std::shared_lock<std::shared_mutex>;
using eLock = std::unique_lock<std::shared_mutex>;

enum class ClaimPolarity : std::uint8_t {
    NEUTRAL = 0,
    POSITIVE = 1,
    NEGATIVE = 2
};

enum class StorageBox : std::uint8_t {
    SENTPEERREQUEST = 0,
    INCOMINGPEERREQUEST = 1,
    SENTPEERREPLY = 2,
    INCOMINGPEERREPLY = 3,
    FINISHEDPEERREQUEST = 4,
    FINISHEDPEERREPLY = 5,
    PROCESSEDPEERREQUEST = 6,
    PROCESSEDPEERREPLY = 7,
    MAILINBOX = 8,
    MAILOUTBOX = 9,
    BLOCKCHAIN = 10,
    RESERVED_1 = 11,
    INCOMINGCHEQUE = 12,
    OUTGOINGCHEQUE = 13,
    OUTGOINGTRANSFER = 14,
    INCOMINGTRANSFER = 15,
    INTERNALTRANSFER = 16,
    PENDING_SEND = 253,
    DRAFT = 254,
    UNKNOWN = 255,
};

enum class EcdsaCurve : std::uint8_t {
    invalid = 0,
    secp256k1 = 1,
    ed25519 = 2,
};

enum class NymCapability : std::uint8_t {
    SIGN_MESSAGE = 0,
    ENCRYPT_MESSAGE = 1,
    AUTHENTICATE_CONNECTION = 2,
    SIGN_CHILDCRED = 3,
};

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
};

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
};  // If you add any types to this list, update the list of strings at the
// top of Ledger.cpp.

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
};

// a transaction can be blank (issued from server)
// or pending (in the inbox/outbox)
// or it can be a "process inbox" transaction
// might also be in the nymbox.
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
};  // If you add any types to this list, update the list of strings at the
// top of OTTransaction.cpp.

enum class SendResult : std::int8_t {
    TRANSACTION_NUMBERS = -3,
    INVALID_REPLY = -2,
    TIMEOUT = -1,
    Error = 0,
    UNNECESSARY = 1,
    VALID_REPLY = 2,
    SHUTDOWN = 3,
};

enum class ConnectionState : std::uint8_t {
    NOT_ESTABLISHED = 0,
    ACTIVE = 1,
    STALLED = 2
};

using Endpoint = std::tuple<
    int,            // address type
    int,            // protocol version
    std::string,    // hostname / address
    std::uint32_t,  // port
    VersionNumber>;
using NetworkReplyRaw = std::pair<SendResult, std::shared_ptr<std::string>>;
using NetworkReplyString = std::pair<SendResult, std::shared_ptr<String>>;
using NetworkReplyMessage = std::pair<SendResult, std::shared_ptr<Message>>;

using CommandResult =
    std::tuple<RequestNumber, TransactionNumber, NetworkReplyMessage>;

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
};

enum class ThreadStatus : std::uint8_t {
    Error = 0,
    RUNNING = 1,
    FINISHED_SUCCESS = 2,
    FINISHED_FAILED = 3,
    SHUTDOWN = 4,
};

enum class Messagability : std::int8_t {
    MISSING_CONTACT = -5,
    CONTACT_LACKS_NYM = -4,
    NO_SERVER_CLAIM = -3,
    INVALID_SENDER = -2,
    MISSING_SENDER = -1,
    READY = 0,
    MISSING_RECIPIENT = 1,
    UNREGISTERED = 2,
};

enum class Depositability : std::int8_t {
    ACCOUNT_NOT_SPECIFIED = -4,
    WRONG_ACCOUNT = -3,
    WRONG_RECIPIENT = -2,
    INVALID_INSTRUMENT = -1,
    READY = 0,
    NOT_REGISTERED = 1,
    NO_ACCOUNT = 2,
    UNKNOWN = 127,
};

enum class RemoteBoxType : std::int8_t {
    Error = -1,
    Nymbox = 0,
    Inbox = 1,
    Outbox = 2,
};

enum class AccountType : std::int8_t {
    Error = 0,
    Blockchain = 1,
    Custodial = 2,
};

enum class PaymentType : int {
    Error = 0,
    Cheque = 1,
    Voucher = 2,
    Transfer = 3,
    Blinded = 4,
};
}  // namespace opentxs
