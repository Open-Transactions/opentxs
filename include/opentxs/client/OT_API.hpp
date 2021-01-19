// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_OT_API_HPP
#define OPENTXS_CLIENT_OT_API_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/otx/consensus/Server.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Activity;
class Contacts;
class Workflow;

namespace implementation
{
class Manager;
}  // namespace implementation

namespace internal
{
struct Core;
}  // namespace internal
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal

namespace network
{
class ZMQ;
}  // namespace network
}  // namespace api

namespace identifier
{
class Nym;
class Server;
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
class Base;
class Server;
}  // namespace context
}  // namespace otx

namespace proto
{
class UnitDefinition;
}  // namespace proto

class Armored;
class Basket;
class Cheque;
class Identifier;
class Ledger;
class Message;
class OTClient;
class OTPaymentPlan;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
// The C++ high-level interface to the Open Transactions client-side.
class OT_API : Lockable
{
public:
    using ProcessInboxOnly =
        std::pair<std::unique_ptr<Ledger>, TransactionNumber>;

    OPENTXS_EXPORT OTClient* GetClient() const { return m_pClient.get(); }

    // This works by checking to see if the Nym has a request number for the
    // given server.
    // That's why it's important, when registering at a specific server, to
    // immediately do a
    // "get request number" since that's what locks in the clients ability to be
    // able to tell
    // that it's registered there.
    OPENTXS_EXPORT bool IsNym_RegisteredAtServer(
        const identifier::Nym& NYM_ID,
        const identifier::Server& NOTARY_ID) const;

    /// === Verify Account Receipt ===
    /// Returns bool. Verifies any asset account (intermediary files) against
    /// its own last signed receipt.
    /// Obviously this will fail for any new account that hasn't done any
    /// transactions yet, and thus has no receipts.
    OPENTXS_EXPORT bool VerifyAccountReceipt(
        const identifier::Server& NOTARY_ID,
        const identifier::Nym& NYM_ID,
        const Identifier& ACCOUNT_ID) const;

    // Returns an OTCheque pointer, or nullptr.
    // (Caller responsible to delete.)
    OPENTXS_EXPORT Cheque* WriteCheque(
        const identifier::Server& NOTARY_ID,
        const std::int64_t& CHEQUE_AMOUNT,
        const Time& VALID_FROM,
        const Time& VALID_TO,
        const Identifier& SENDER_accountID,
        const identifier::Nym& SENDER_NYM_ID,
        const String& CHEQUE_MEMO,
        const identifier::Nym& pRECIPIENT_NYM_ID) const;
    
#if OT_WITH_PAYMENT_PLANS
    // PROPOSE PAYMENT PLAN (called by Merchant)
    //
    // Returns an OTPaymentPlan pointer, or nullptr.
    // (Caller responsible to delete.)
    //
    // Payment Plan Delay, and Payment Plan Period, both default to 30 days (if
    // you pass 0), measured in seconds.
    //
    // Payment Plan Length, and Payment Plan Max Payments, both default to 0,
    // which means no maximum length and no maximum number of payments.
    OPENTXS_EXPORT OTPaymentPlan* ProposePaymentPlan(
        const identifier::Server& NOTARY_ID,
        const Time& VALID_FROM,  // 0 defaults to the current time in
                                 // seconds
                                 // since Jan 1970.
        const Time& VALID_TO,    // 0 defaults to "no expiry." Otherwise this
                                 // value is ADDED to VALID_FROM. (It's a
                                 // length.)
        const Identifier& pSENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const String& PLAN_CONSIDERATION,  // like a memo.
        const Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID,
        const std::int64_t& INITIAL_PAYMENT_AMOUNT,
        const std::chrono::seconds INITIAL_PAYMENT_DELAY,
        const std::int64_t& PAYMENT_PLAN_AMOUNT,
        const std::chrono::seconds PAYMENT_PLAN_DELAY,
        const std::chrono::seconds PAYMENT_PLAN_PERIOD,
        const std::chrono::seconds PAYMENT_PLAN_LENGTH = {},
        const std::int32_t PAYMENT_PLAN_MAX_PAYMENTS = 0) const;

    // CONFIRM PAYMENT PLAN (called by Customer)
    OPENTXS_EXPORT bool ConfirmPaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const Identifier& SENDER_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID,
        OTPaymentPlan& thePlan) const;
#endif

#if OT_WITH_BASKETS
    OPENTXS_EXPORT bool IsBasketCurrency(
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID)
        const;

    OPENTXS_EXPORT std::int64_t GetBasketMinimumTransferAmount(
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID)
        const;

    OPENTXS_EXPORT std::int32_t GetBasketMemberCount(
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID)
        const;

    OPENTXS_EXPORT bool GetBasketMemberType(
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
        std::int32_t nIndex,
        identifier::UnitDefinition& theOutputMemberType) const;

    OPENTXS_EXPORT std::int64_t GetBasketMemberMinimumTransferAmount(
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
        std::int32_t nIndex) const;
#endif

    OPENTXS_EXPORT std::unique_ptr<Ledger> LoadNymbox(
        const identifier::Server& NOTARY_ID,
        const identifier::Nym& NYM_ID) const;

    OPENTXS_EXPORT ProcessInboxOnly CreateProcessInbox(
        const Identifier& accountID,
        otx::context::Server& context,
        Ledger& inbox) const;

    OPENTXS_EXPORT bool IncludeResponse(
        const Identifier& accountID,
        const bool accept,
        otx::context::Server& context,
        OTTransaction& source,
        Ledger& processInbox) const;

    OPENTXS_EXPORT bool FinalizeProcessInbox(
        const Identifier& accountID,
        otx::context::Server& context,
        Ledger& processInbox,
        Ledger& inbox,
        Ledger& outbox,
        const PasswordPrompt& reason) const;

    // These commands below send messages to the server:

    OPENTXS_EXPORT CommandResult
    unregisterNym(otx::context::Server& context) const;

    OPENTXS_EXPORT CommandResult usageCredits(
        otx::context::Server& context,
        const identifier::Nym& NYM_ID_CHECK,
        std::int64_t lAdjustment = 0) const;

    OPENTXS_EXPORT CommandResult queryInstrumentDefinitions(
        otx::context::Server& context,
        const Armored& ENCODED_MAP) const;

    OPENTXS_EXPORT CommandResult deleteAssetAccount(
        otx::context::Server& context,
        const Identifier& ACCOUNT_ID) const;

#if OT_WITH_BASKETS
    OPENTXS_EXPORT bool AddBasketCreationItem(
        proto::UnitDefinition& basketTemplate,
        const String& currencyID,
        const std::uint64_t weight) const;

    OPENTXS_EXPORT CommandResult issueBasket(
        otx::context::Server& context,
        const proto::UnitDefinition& basket,
        const std::string& label) const;

    OPENTXS_EXPORT Basket* GenerateBasketExchange(
        const identifier::Server& NOTARY_ID,
        const identifier::Nym& NYM_ID,
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
        const Identifier& BASKET_ASSET_ACCT_ID,
        std::int32_t TRANSFER_MULTIPLE) const;  // 1            2             3
    // 5=2,3,4  OR  10=4,6,8  OR 15=6,9,12

    OPENTXS_EXPORT bool AddBasketExchangeItem(
        const identifier::Server& NOTARY_ID,
        const identifier::Nym& NYM_ID,
        Basket& theBasket,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const Identifier& ASSET_ACCT_ID) const;

    OPENTXS_EXPORT CommandResult exchangeBasket(
        otx::context::Server& context,
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
        const String& BASKET_INFO,
        bool bExchangeInOrOut) const;
#endif
    
    OPENTXS_EXPORT std::unique_ptr<Message> getTransactionNumbers(
        otx::context::Server& context) const;

    OPENTXS_EXPORT CommandResult withdrawVoucher(
        otx::context::Server& context,
        const Identifier& ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID,
        const String& CHEQUE_MEMO,
        const Amount amount) const;

#if OT_WITH_DIVIDENDS
    OPENTXS_EXPORT CommandResult payDividend(
        otx::context::Server& context,
        const Identifier& DIVIDEND_FROM_ACCT_ID,  // if dollars paid for pepsi
                                                  // shares, then this is the
                                                  // issuer's dollars account.
        const identifier::UnitDefinition&
            SHARES_INSTRUMENT_DEFINITION_ID,  // if dollars paid
                                              // for pepsi
                                              // shares,
        // then this is the pepsi shares
        // instrument definition id.
        const String& DIVIDEND_MEMO,  // user-configurable note that's added to
                                      // the
                                      // payout request message.
        const Amount& AMOUNT_PER_SHARE) const;  // number of dollars to be paid
                                                // out
    // PER SHARE (multiplied by total
    // number of shares issued.)
#endif
    
#if OT_WITH_SMART_CONTRACTS
    OPENTXS_EXPORT CommandResult triggerClause(
        otx::context::Server& context,
        const TransactionNumber& lTransactionNum,
        const String& strClauseName,
        const String& pStrParam = String::Factory()) const;

    OPENTXS_EXPORT bool Create_SmartContract(
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        Time VALID_FROM,                       // Default (0 or nullptr) == NOW
        Time VALID_TO,         // Default (0 or nullptr) == no expiry / cancel
                               // anytime
        bool SPECIFY_ASSETS,   // This means asset type IDs must be provided for
                               // every named account.
        bool SPECIFY_PARTIES,  // This means Nym IDs must be provided for every
                               // party.
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_SetDates(
        const String& THE_CONTRACT,  // The contract, about to have the dates
                                     // changed on it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        Time VALID_FROM,                       // Default (0 or nullptr) == NOW
        Time VALID_TO,  // Default (0 or nullptr) == no expiry / cancel
                        // anytime.
        String& strOutput) const;

    OPENTXS_EXPORT bool Smart_ArePartiesSpecified(
        const String& THE_CONTRACT) const;

    OPENTXS_EXPORT bool Smart_AreAssetTypesSpecified(
        const String& THE_CONTRACT) const;

    OPENTXS_EXPORT bool SmartContract_AddBylaw(
        const String& THE_CONTRACT,  // The contract, about to have the bylaw
                                     // added to it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,  // The Bylaw's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_RemoveBylaw(
        const String& THE_CONTRACT,  // The contract, about to have the bylaw
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,  // The Bylaw's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_AddClause(
        const String& THE_CONTRACT,  // The contract, about to have the clause
                                     // added to it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,   // Should already be on the contract. (This
                                    // way we can find it.)
        const String& CLAUSE_NAME,  // The Clause's name as referenced in the
                                    // smart contract. (And the scripts...)
        const String& SOURCE_CODE,  // The actual source code for the clause.
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_UpdateClause(
        const String& THE_CONTRACT,  // The contract, about to have the clause
                                     // updated on it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,   // Should already be on the contract. (This
                                    // way we can find it.)
        const String& CLAUSE_NAME,  // The Clause's name as referenced in the
                                    // smart contract. (And the scripts...)
        const String& SOURCE_CODE,  // The actual source code for the clause.
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_RemoveClause(
        const String& THE_CONTRACT,  // The contract, about to have the clause
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,   // Should already be on the contract. (This
                                    // way we can find it.)
        const String& CLAUSE_NAME,  // The Clause's name as referenced in the
                                    // smart contract. (And the scripts...)
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_AddVariable(
        const String& THE_CONTRACT,  // The contract, about to have the variable
                                     // added to it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& VAR_NAME,    // The Variable's name as referenced in the
                                   // smart contract. (And the scripts...)
        const String& VAR_ACCESS,  // "constant", "persistent", or "important".
        const String& VAR_TYPE,    // "string", "std::int64_t", or "bool"
        const String& VAR_VALUE,  // Contains a string. If type is std::int64_t,
                                  // atol() will be used to convert value to a
                                  // std::int64_t. If type is bool, the strings
                                  // "true" or "false" are expected here in
                                  // order to convert to a bool.
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_RemoveVariable(
        const String& THE_CONTRACT,  // The contract, about to have the variable
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& VAR_NAME,    // The Variable's name as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_AddCallback(
        const String& THE_CONTRACT,  // The contract, about to have the callback
                                     // added to it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& CALLBACK_NAME,  // The Callback's name as referenced in
                                      // the smart contract. (And the
                                      // scripts...)
        const String& CLAUSE_NAME,  // The actual clause that will be triggered
                                    // by the callback. (Must exist.)
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_RemoveCallback(
        const String& THE_CONTRACT,  // The contract, about to have the callback
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& CALLBACK_NAME,  // The Callback's name as referenced in
                                      // the smart contract. (And the
                                      // scripts...)
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_AddHook(
        const String& THE_CONTRACT,  // The contract, about to have the hook
                                     // added to it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& HOOK_NAME,   // The Hook's name as referenced in the smart
                                   // contract. (And the scripts...)
        const String& CLAUSE_NAME,  // The actual clause that will be triggered
                                    // by the hook. (You can call this multiple
                                    // times, and have multiple clauses trigger
                                    // on the same hook.)
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_RemoveHook(
        const String& THE_CONTRACT,  // The contract, about to have the hook
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& HOOK_NAME,   // The Hook's name as referenced in the smart
                                   // contract. (And the scripts...)
        const String& CLAUSE_NAME,  // The actual clause that will be triggered
                                    // by the hook. (You can call this multiple
                                    // times, and have multiple clauses trigger
                                    // on the same hook.)
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_AddParty(
        const String& THE_CONTRACT,  // The contract, about to have the party
                                     // added to it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& PARTY_NYM_ID,  // Optional. Some smart contracts require
                                     // the party's Nym to be specified in
                                     // advance.
        const String& PARTY_NAME,    // The Party's NAME as referenced in the
                                     // smart contract. (And the scripts...)
        const String& AGENT_NAME,    // An AGENT will be added by default for
                                     // this party. Need Agent NAME.
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_RemoveParty(
        const String& THE_CONTRACT,  // The contract, about to have the party
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& PARTY_NAME,  // The Party's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_AddAccount(
        const String& THE_CONTRACT,  // The contract, about to have the account
                                     // added to it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& PARTY_NAME,  // The Party's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        const String& ACCT_NAME,   // The Account's name as referenced in the
                                   // smart contract
        const String& INSTRUMENT_DEFINITION_ID,  // Instrument Definition ID for
                                                 // the
                                                 // Account.
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_RemoveAccount(
        const String& THE_CONTRACT,  // The contract, about to have the account
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& PARTY_NAME,  // The Party's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        const String& ACCT_NAME,   // The Account's name as referenced in the
                                   // smart contract
        String& strOutput) const;

    OPENTXS_EXPORT std::int32_t SmartContract_CountNumsNeeded(
        const String& THE_CONTRACT,  // The contract, about to have the bylaw
                                     // added to it.
        const String& AGENT_NAME) const;  // An AGENT will be added by default
                                          // for
                                          // this party. Need Agent NAME.

    OPENTXS_EXPORT bool SmartContract_ConfirmAccount(
        const String& THE_CONTRACT,
        const identifier::Nym& SIGNER_NYM_ID,
        const String& PARTY_NAME,
        const String& ACCT_NAME,
        const String& AGENT_NAME,
        const String& ACCT_ID,
        String& strOutput) const;

    OPENTXS_EXPORT bool SmartContract_ConfirmParty(
        const String& THE_CONTRACT,  // The smart contract, about to be changed
                                     // by this function.
        const String& PARTY_NAME,    // Should already be on the contract. This
                                     // way we can find it.
        const identifier::Nym& NYM_ID,  // Nym ID for the party, the actual
                                        // owner,
        const identifier::Server& NOTARY_ID,
        String& strOutput) const;  // ===> AS WELL AS for the default AGENT of
                                   // that
                                   // party. (For now, until I code entities)
    OPENTXS_EXPORT CommandResult activateSmartContract(
        otx::context::Server& context,
        const String& THE_SMART_CONTRACT) const;
#endif
    
#if OT_WITH_PAYMENT_PLANS
    OPENTXS_EXPORT CommandResult depositPaymentPlan(
        otx::context::Server& context,
        const String& THE_PAYMENT_PLAN) const;
#endif
    
#if OT_WITH_MARKETS
    OPENTXS_EXPORT CommandResult issueMarketOffer(
        otx::context::Server& context,
        const Identifier& ASSET_ACCT_ID,
        const Identifier& CURRENCY_ACCT_ID,
        const std::int64_t& MARKET_SCALE,  // Defaults to minimum of 1. Market
                                           // granularity.
        const std::int64_t& MINIMUM_INCREMENT,  // This will be multiplied by
                                                // the
                                                // Scale. Min 1.
        const std::int64_t& TOTAL_ASSETS_ON_OFFER,  // Total assets available
                                                    // for
                                                    // sale
        // or purchase. Will be multiplied
        // by minimum increment.
        const Amount PRICE_LIMIT,     // Per Minimum Increment...
        const bool bBuyingOrSelling,  // BUYING == false, SELLING == true.
        const std::chrono::seconds tLifespanInSeconds = std::chrono::hours{24},
        const char STOP_SIGN = 0,  // For stop orders, set to '<' or '>'
        const Amount ACTIVATION_PRICE = 0) const;  // For stop orders, set the
                                                   // threshold price here.
    OPENTXS_EXPORT CommandResult
    getMarketList(otx::context::Server& context) const;
    OPENTXS_EXPORT CommandResult getMarketOffers(
        otx::context::Server& context,
        const Identifier& MARKET_ID,
        const std::int64_t& lDepth) const;
    OPENTXS_EXPORT CommandResult getMarketRecentTrades(
        otx::context::Server& context,
        const Identifier& MARKET_ID) const;

    OPENTXS_EXPORT CommandResult
    getNymMarketOffers(otx::context::Server& context) const;
#endif
    
    // For cancelling market offers and payment plans.
    OPENTXS_EXPORT CommandResult cancelCronItem(
        otx::context::Server& context,
        const Identifier& ASSET_ACCT_ID,
        const TransactionNumber& lTransactionNum) const;

    ~OT_API() override;  // calls Cleanup();

private:
    friend api::client::implementation::Manager;

    const api::internal::Core& api_;
    const api::client::Activity& activity_;
    const api::client::Contacts& contacts_;
    const api::client::Workflow& workflow_;
    const api::network::ZMQ& zmq_;
    bool m_bDefaultStore{false};
    OTString m_strDataPath;
    OTString m_strConfigFilename;
    OTString m_strConfigFilePath;
    std::unique_ptr<OTClient> m_pClient;
    ContextLockCallback lock_callback_;

    void AddHashesToTransaction(
        OTTransaction& transaction,
        const otx::context::Base& context,
        const Account& account,
        const PasswordPrompt& reason) const;
    void AddHashesToTransaction(
        OTTransaction& transaction,
        const otx::context::Base& context,
        const Identifier& accountid,
        const PasswordPrompt& reason) const;

    bool add_accept_item(
        const itemType type,
        const TransactionNumber originNumber,
        const TransactionNumber referenceNumber,
        const String& note,
        const identity::Nym& nym,
        const Amount amount,
        const String& inRefTo,
        OTTransaction& processInbox) const;
    bool find_cron(
        const otx::context::Server& context,
        const Item& item,
        OTTransaction& processInbox,
        OTTransaction& serverTransaction,
        Ledger& inbox,
        Amount& amount,
        std::set<TransactionNumber>& closing) const;
    bool find_standard(
        const otx::context::Server& context,
        const Item& item,
        const TransactionNumber number,
        OTTransaction& serverTransaction,
        Ledger& inbox,
        Amount& amount,
        std::set<TransactionNumber>& closing) const;
    OTTransaction* get_or_create_process_inbox(
        const Identifier& accountID,
        otx::context::Server& context,
        Ledger& response) const;
    TransactionNumber get_origin(
        const identifier::Server& notaryID,
        const OTTransaction& source,
        String& note) const;
    Time GetTime() const;
    itemType response_type(const transactionType sourceType, const bool success)
        const;

    bool Cleanup();
    bool Init();
    bool LoadConfigFile();

    OT_API(
        const api::internal::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contacts,
        const api::client::Workflow& workflow,
        const api::network::ZMQ& zmq,
        const ContextLockCallback& lockCallback);
    OT_API() = delete;
    OT_API(const OT_API&) = delete;
    OT_API(OT_API&&) = delete;
    OT_API operator=(const OT_API&) = delete;
    OT_API operator=(OT_API&&) = delete;
};
}  // namespace opentxs
#endif
