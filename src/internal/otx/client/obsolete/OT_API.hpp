// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

#include "internal/core/String.hpp"
#include "internal/otx/client/Types.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/Types.internal.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/otx/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Activity;
class ClientPrivate;
class Contacts;
class Workflow;
class ZeroMQ;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Generic;
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
class Message;
class OTClient;
class OTPaymentPlan;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
// The C++ high-level interface to the Open Transactions client-side.
class OT_API : Lockable
{
public:
    using ProcessInboxOnly =
        std::pair<std::unique_ptr<Ledger>, TransactionNumber>;

    auto GetClient() const -> OTClient* { return client_.get(); }

    // This works by checking to see if the Nym has a request number for the
    // given server.
    // That's why it's important, when registering at a specific server, to
    // immediately do a
    // "get request number" since that's what locks in the clients ability to be
    // able to tell
    // that it's registered there.
    auto IsNym_RegisteredAtServer(
        const identifier::Nym& NYM_ID,
        const identifier::Notary& NOTARY_ID) const -> bool;

    /// === Verify Account Receipt ===
    /// Returns bool. Verifies any asset account (intermediary files) against
    /// its own last signed receipt.
    /// Obviously this will fail for any new account that hasn't done any
    /// transactions yet, and thus has no receipts.
    auto VerifyAccountReceipt(
        const identifier::Notary& NOTARY_ID,
        const identifier::Nym& NYM_ID,
        const identifier::Account& account_ID) const -> bool;

    // Returns an OTCheque pointer, or nullptr.
    // (Caller responsible to delete.)
    auto WriteCheque(
        const identifier::Notary& NOTARY_ID,
        const Amount& chequeAmount,
        const Time& VALID_FROM,
        const Time& VALID_TO,
        const identifier::Account& SENDER_accountID,
        const identifier::Nym& SENDER_NYM_ID,
        const String& chequeMemo,
        const identifier::Nym& pRECIPIENT_NYM_ID) const -> Cheque*;

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
    auto ProposePaymentPlan(
        const identifier::Notary& NOTARY_ID,
        const Time& VALID_FROM,  // 0 defaults to the current time in
                                 // seconds
                                 // since Jan 1970.
        const Time& VALID_TO,    // 0 defaults to "no expiry." Otherwise this
                                 // value is ADDED to VALID_FROM. (It's a
                                 // length.)
        const identifier::Account& pSENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const String& PLAN_CONSIDERATION,  // like a memo.
        const identifier::Account& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID,
        const std::int64_t& INITIAL_PAYMENT_AMOUNT,
        const std::chrono::seconds INITIAL_PAYMENT_DELAY,
        const std::int64_t& PAYMENT_PLAN_AMOUNT,
        const std::chrono::seconds PAYMENT_PLAN_DELAY,
        const std::chrono::seconds PAYMENT_PLAN_PERIOD,
        const std::chrono::seconds PAYMENT_PLAN_LENGTH = {},
        const std::int32_t PAYMENT_PLAN_MAX_PAYMENTS = 0) const
        -> OTPaymentPlan*;

    // CONFIRM PAYMENT PLAN (called by Customer)
    auto ConfirmPaymentPlan(
        const identifier::Notary& NOTARY_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const identifier::Account& SENDER_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID,
        OTPaymentPlan& thePlan) const -> bool;
    auto IsBasketCurrency(
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID) const
        -> bool;

    auto GetBasketMinimumTransferAmount(
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID) const
        -> Amount;

    auto GetBasketMemberCount(
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID) const
        -> std::int32_t;

    auto GetBasketMemberType(
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
        std::int32_t nIndex,
        identifier::UnitDefinition& theOutputMemberType) const -> bool;

    auto GetBasketMemberMinimumTransferAmount(
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
        std::int32_t nIndex) const -> std::int64_t;
    auto LoadNymbox(
        const identifier::Notary& NOTARY_ID,
        const identifier::Nym& NYM_ID) const -> std::unique_ptr<Ledger>;

    auto CreateProcessInbox(
        const identifier::Account& accountID,
        otx::context::Server& context,
        Ledger& inbox) const -> ProcessInboxOnly;

    auto IncludeResponse(
        const identifier::Account& accountID,
        const bool accept,
        otx::context::Server& context,
        OTTransaction& source,
        Ledger& processInbox) const -> bool;

    auto FinalizeProcessInbox(
        const identifier::Account& accountID,
        otx::context::Server& context,
        Ledger& processInbox,
        Ledger& inbox,
        Ledger& outbox,
        const PasswordPrompt& reason) const -> bool;

    // These commands below send messages to the server:

    auto unregisterNym(otx::context::Server& context) const -> CommandResult;

    auto usageCredits(
        otx::context::Server& context,
        const identifier::Nym& NYM_ID_CHECK,
        std::int64_t lAdjustment = 0) const -> CommandResult;

    auto queryInstrumentDefinitions(
        otx::context::Server& context,
        const Armored& ENCODED_MAP) const -> CommandResult;

    auto deleteAssetAccount(
        otx::context::Server& context,
        const identifier::Account& account_ID) const -> CommandResult;

    auto AddBasketCreationItem(
        proto::UnitDefinition& basketTemplate,
        const String& currencyID,
        const std::uint64_t weight) const -> bool;

    auto issueBasket(
        otx::context::Server& context,
        const proto::UnitDefinition& basket,
        const UnallocatedCString& label) const -> CommandResult;

    auto GenerateBasketExchange(
        const identifier::Notary& NOTARY_ID,
        const identifier::Nym& NYM_ID,
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
        const identifier::Account& BASKET_ASSET_ACCT_ID,
        std::int32_t TRANSFER_MULTIPLE) const -> Basket*;  // 1            2 3
    // 5=2,3,4  OR  10=4,6,8  OR 15=6,9,12

    auto AddBasketExchangeItem(
        const identifier::Notary& NOTARY_ID,
        const identifier::Nym& NYM_ID,
        Basket& theBasket,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::Account& ASSET_ACCT_ID) const -> bool;

    auto exchangeBasket(
        otx::context::Server& context,
        const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
        const String& BASKET_INFO,
        bool bExchangeInOrOut) const -> CommandResult;

    auto getTransactionNumbers(otx::context::Server& context) const
        -> std::unique_ptr<Message>;

    auto withdrawVoucher(
        otx::context::Server& context,
        const identifier::Account& ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID,
        const String& chequeMemo,
        const Amount amount) const -> CommandResult;

    auto payDividend(
        otx::context::Server& context,
        const identifier::Account& DIVIDEND_FROM_ACCT_ID,  // if dollars paid
                                                           // for pepsi shares,
                                                           // then this is the
                                                           // issuer's dollars
                                                           // account.
        const identifier::UnitDefinition&
            SHARES_INSTRUMENT_DEFINITION_ID,  // if dollars paid
                                              // for pepsi
                                              // shares,
        // then this is the pepsi shares
        // instrument definition id.
        const String& DIVIDEND_MEMO,  // user-configurable note that's added to
                                      // the
                                      // payout request message.
        const Amount& AMOUNT_PER_SHARE) const
        -> CommandResult;  // number of dollars to be paid
                           // out
    // PER SHARE (multiplied by total
    // number of shares issued.)

    auto triggerClause(
        otx::context::Server& context,
        const TransactionNumber& lTransactionNum,
        const String& strClauseName,
        const String& pStrParam = String::Factory()) const -> CommandResult;

    auto Create_SmartContract(
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
        String& strOutput) const -> bool;

    auto SmartContract_SetDates(
        const String& THE_CONTRACT,  // The contract, about to have the dates
                                     // changed on it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        Time VALID_FROM,                       // Default (0 or nullptr) == NOW
        Time VALID_TO,  // Default (0 or nullptr) == no expiry / cancel
                        // anytime.
        String& strOutput) const -> bool;

    auto Smart_ArePartiesSpecified(const String& THE_CONTRACT) const -> bool;

    auto Smart_AreAssetTypesSpecified(const String& THE_CONTRACT) const -> bool;

    auto SmartContract_AddBylaw(
        const String& THE_CONTRACT,  // The contract, about to have the bylaw
                                     // added to it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,  // The Bylaw's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const -> bool;

    auto SmartContract_RemoveBylaw(
        const String& THE_CONTRACT,  // The contract, about to have the bylaw
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,  // The Bylaw's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const -> bool;

    auto SmartContract_AddClause(
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
        String& strOutput) const -> bool;

    auto SmartContract_UpdateClause(
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
        String& strOutput) const -> bool;

    auto SmartContract_RemoveClause(
        const String& THE_CONTRACT,  // The contract, about to have the clause
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,   // Should already be on the contract. (This
                                    // way we can find it.)
        const String& CLAUSE_NAME,  // The Clause's name as referenced in the
                                    // smart contract. (And the scripts...)
        String& strOutput) const -> bool;

    auto SmartContract_AddVariable(
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
        String& strOutput) const -> bool;

    auto SmartContract_RemoveVariable(
        const String& THE_CONTRACT,  // The contract, about to have the variable
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& VAR_NAME,    // The Variable's name as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const -> bool;

    auto SmartContract_AddCallback(
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
        String& strOutput) const -> bool;

    auto SmartContract_RemoveCallback(
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
        String& strOutput) const -> bool;

    auto SmartContract_AddHook(
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
        String& strOutput) const -> bool;

    auto SmartContract_RemoveHook(
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
        String& strOutput) const -> bool;

    auto SmartContract_AddParty(
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
        String& strOutput) const -> bool;

    auto SmartContract_RemoveParty(
        const String& THE_CONTRACT,  // The contract, about to have the party
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& PARTY_NAME,  // The Party's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const -> bool;

    auto SmartContract_AddAccount(
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
        String& strOutput) const -> bool;

    auto SmartContract_RemoveAccount(
        const String& THE_CONTRACT,  // The contract, about to have the account
                                     // removed from it.
        const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                               // (The signing at this point is
                                               // only to cause a save.)
        const String& PARTY_NAME,  // The Party's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        const String& ACCT_NAME,   // The Account's name as referenced in the
                                   // smart contract
        String& strOutput) const -> bool;

    auto SmartContract_CountNumsNeeded(
        const String& THE_CONTRACT,  // The contract, about to have the bylaw
                                     // added to it.
        const String& AGENT_NAME) const
        -> std::int32_t;  // An AGENT will be added by default
                          // for
                          // this party. Need Agent NAME.

    auto SmartContract_ConfirmAccount(
        const String& THE_CONTRACT,
        const identifier::Nym& SIGNER_NYM_ID,
        const String& PARTY_NAME,
        const String& ACCT_NAME,
        const String& AGENT_NAME,
        const String& ACCT_ID,
        String& strOutput) const -> bool;

    auto SmartContract_ConfirmParty(
        const String& THE_CONTRACT,  // The smart contract, about to be changed
                                     // by this function.
        const String& PARTY_NAME,    // Should already be on the contract. This
                                     // way we can find it.
        const identifier::Nym& NYM_ID,  // Nym ID for the party, the actual
                                        // owner,
        const identifier::Notary& NOTARY_ID,
        String& strOutput) const
        -> bool;  // ===> AS WELL AS for the default AGENT of
                  // that
                  // party. (For now, until I code entities)
    auto activateSmartContract(
        otx::context::Server& context,
        const String& THE_SMART_CONTRACT) const -> CommandResult;

    auto depositPaymentPlan(
        otx::context::Server& context,
        const String& THE_PAYMENT_PLAN) const -> CommandResult;
    auto issueMarketOffer(
        otx::context::Server& context,
        const identifier::Account& ASSET_ACCT_ID,
        const identifier::Account& CURRENCY_ACCT_ID,
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
        const Amount ACTIVATION_PRICE = 0) const
        -> CommandResult;  // For stop orders, set the
                           // threshold price here.
    auto getMarketList(otx::context::Server& context) const -> CommandResult;
    auto getMarketOffers(
        otx::context::Server& context,
        const identifier::Generic& MARKET_ID,
        const std::int64_t& lDepth) const -> CommandResult;
    auto getMarketRecentTrades(
        otx::context::Server& context,
        const identifier::Generic& MARKET_ID) const -> CommandResult;

    auto getNymMarketOffers(otx::context::Server& context) const
        -> CommandResult;

    // For cancelling market offers and payment plans.
    auto cancelCronItem(
        otx::context::Server& context,
        const identifier::Account& ASSET_ACCT_ID,
        const TransactionNumber& lTransactionNum) const -> CommandResult;

    OT_API() = delete;
    OT_API(const OT_API&) = delete;
    OT_API(OT_API&&) = delete;
    auto operator=(const OT_API&) -> OT_API = delete;
    auto operator=(OT_API&&) -> OT_API = delete;

    ~OT_API() override;  // calls Cleanup();

private:
    friend api::session::ClientPrivate;

    const api::Session& api_;
    const api::session::Workflow& workflow_;
    bool default_store_;
    OTString data_path_;
    OTString config_filename_;
    OTString config_file_path_;
    std::unique_ptr<OTClient> client_;
    ContextLockCallback lock_callback_;

    void AddHashesToTransaction(
        OTTransaction& transaction,
        const otx::context::Base& context,
        const Account& account,
        const PasswordPrompt& reason) const;
    void AddHashesToTransaction(
        OTTransaction& transaction,
        const otx::context::Base& context,
        const identifier::Account& accountid,
        const PasswordPrompt& reason) const;

    auto add_accept_item(
        const otx::itemType type,
        const TransactionNumber originNumber,
        const TransactionNumber referenceNumber,
        const String& note,
        const identity::Nym& nym,
        const Amount amount,
        const String& inRefTo,
        OTTransaction& processInbox) const -> bool;
    auto find_cron(
        const otx::context::Server& context,
        const Item& item,
        OTTransaction& processInbox,
        OTTransaction& serverTransaction,
        Ledger& inbox,
        Amount& amount,
        UnallocatedSet<TransactionNumber>& closing) const -> bool;
    auto find_standard(
        const otx::context::Server& context,
        const Item& item,
        const TransactionNumber number,
        OTTransaction& serverTransaction,
        Ledger& inbox,
        Amount& amount,
        UnallocatedSet<TransactionNumber>& closing) const -> bool;
    auto get_or_create_process_inbox(
        const identifier::Account& accountID,
        otx::context::Server& context,
        Ledger& response) const -> OTTransaction*;
    auto get_origin(
        const identifier::Notary& notaryID,
        const OTTransaction& source,
        String& note) const -> TransactionNumber;
    auto GetTime() const -> Time;
    auto response_type(
        const otx::transactionType sourceType,
        const bool success) const -> otx::itemType;

    auto Cleanup() -> bool;
    auto Init() -> bool;
    auto LoadConfigFile() -> bool;

    OT_API(
        const api::Session& api,
        const api::session::Workflow& workflow,
        ContextLockCallback lockCallback);
};
}  // namespace opentxs
