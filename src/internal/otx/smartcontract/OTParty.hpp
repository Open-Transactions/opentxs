// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Account.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
class Notary;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
class Server;
}  // namespace context
}  // namespace otx

class Contract;
class NumList;
class OTAgent;
class OTPartyAccount;
class OTScript;
class OTScriptable;
class PasswordPrompt;
class Tag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
// Party is always either an Owner Nym, or an Owner Entity formed by Contract.
//
// Either way, the agents are there to represent the interests of the parties.
//
// This is meant in the sense of "actually" since the agent is not just a
// trusted friend of the party, but is either the party himself (if party is a
// Nym), OR is a voting group or employee that belongs to the party. (If party
// is an entity.)
// Either way, the point is that in this context, the agent is ACTUALLY
// authorized by the party by virtue of its existence, versus being a "separate
// but authorized" party in the legal sense. No need exists to "grant" the
// authority since the authority is already INHERENT.
//
// A party may also have multiple agents.
class OTParty
{
public:
    using mapOfAccounts = UnallocatedMap<UnallocatedCString, SharedAccount>;
    using mapOfAgents = UnallocatedMap<UnallocatedCString, OTAgent*>;
    using mapOfPartyAccounts =
        UnallocatedMap<UnallocatedCString, OTPartyAccount*>;

    void CleanupAgents();
    void CleanupAccounts();
    auto Compare(const OTParty& rhs) const -> bool;
    void Serialize(
        Tag& parent,
        bool bCalculatingID = false,
        bool bSpecifyInstrumentDefinitionID = false,
        bool bSpecifyParties = false) const;

    // Clears temp pointers when I'm done with them, so I don't get stuck
    // with bad addresses.
    //
    void ClearTemporaryPointers();
    // The party will use its authorizing agent.
    auto SignContract(Contract& theInput, const PasswordPrompt& reason) const
        -> bool;
    // See if a certain transaction number is present.
    // Checks opening number on party, and closing numbers on his accounts.
    auto HasTransactionNum(const std::int64_t& lInput) const -> bool;
    void GetAllTransactionNumbers(NumList& numlistOutput) const;
    // Set aside all the necessary transaction #s from the various Nyms.
    // (Assumes those Nym pointers are available inside their various agents.)
    //
    auto ReserveTransNumsForConfirm(otx::context::Server& context) -> bool;
    void HarvestAllTransactionNumbers(otx::context::Server& context);
    void HarvestOpeningNumber(const String& strNotaryID);
    void HarvestOpeningNumber(otx::context::Server& context);
    void CloseoutOpeningNumber(
        const String& strNotaryID,
        const PasswordPrompt& reason);
    void HarvestClosingNumbers(
        const String& strNotaryID,
        const PasswordPrompt& reason);
    void HarvestClosingNumbers(otx::context::Server& context);
    // Iterates through the agents.
    //
    auto DropFinalReceiptToNymboxes(
        const std::int64_t& lNewTransactionNumber,
        const String& strOrigCronItem,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory()) -> bool;
    // Iterates through the accounts.
    //
    auto DropFinalReceiptToInboxes(
        const String& strNotaryID,
        const std::int64_t& lNewTransactionNumber,
        const String& strOrigCronItem,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory()) -> bool;
    auto SendNoticeToParty(
        const api::Session& api,
        bool bSuccessMsg,
        const identity::Nym& theServerNym,
        const identifier::Notary& theNotaryID,
        const std::int64_t& lNewTransactionNumber,
        // const std::int64_t& lInReferenceTo,
        // We use GetOpenTransNo() now.
        const String& strReference,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory(),
        identity::Nym* pActualNym = nullptr) -> bool;
    // This pointer isn't owned -- just stored for convenience.
    //
    auto GetOwnerAgreement() -> OTScriptable* { return owner_agreement_; }
    void SetOwnerAgreement(OTScriptable& theOwner)
    {
        owner_agreement_ = &theOwner;
    }
    void SetMySignedCopy(const String& strMyCopy)
    {
        my_signed_copy_->Set(strMyCopy.Get());
    }
    auto GetMySignedCopy() -> const String& { return my_signed_copy_; }
    auto GetOpeningTransNo() const -> std::int64_t { return opening_trans_no_; }
    void SetOpeningTransNo(const std::int64_t& theNumber)
    {
        opening_trans_no_ = theNumber;
    }
    // There is one of these for each asset account on the party.
    // You need the acct name to look it up.
    //
    auto GetClosingTransNo(UnallocatedCString str_for_acct_name) const
        -> std::int64_t;
    // as used "IN THE SCRIPT."
    //
    auto GetPartyName(bool* pBoolSuccess = nullptr) const
        -> UnallocatedCString;  // "sales_director",
                                // "marketer",
                                // etc
    auto SetPartyName(const UnallocatedCString& str_party_name_input) -> bool;
    // ACTUAL PARTY OWNER (Only ONE of these can be true...)
    // Debating whether these two functions should be private. (Should it matter
    // to outsider?)
    //
    auto IsNym() const -> bool;     // If the party is a Nym. (The party is the
                                    // actual owner/beneficiary.)
    auto IsEntity() const -> bool;  // If the party is an Entity. (Either way,
                                    // the AGENT carries out all wishes.)
    // ACTUAL PARTY OWNER
    //
    // If the party is a Nym, this is the Nym's ID. Otherwise this is false.
    auto GetNymID(bool* pBoolSuccess = nullptr) const -> UnallocatedCString;

    // If the party is an Entity, this is the Entity's ID. Otherwise this is
    // false.
    auto GetEntityID(bool* pBoolSuccess = nullptr) const -> UnallocatedCString;

    // If party is a Nym, this is the NymID. Else return EntityID().
    auto GetPartyID(bool* pBoolSuccess = nullptr) const -> UnallocatedCString;
    // Some agents are passive (voting groups) and cannot behave actively, and
    // so cannot do
    // certain things that only Nyms can do. But they can still act as an agent
    // in CERTAIN
    // respects, so they are still allowed to do so. However, likely many
    // functions will
    // require that HasActiveAgent() be true for a party to do various actions.
    // Attempts to
    // do those actions otherwise will fail.
    // It's almost a separate kind of party but not worthy of a separate class.
    //
    auto HasActiveAgent() const -> bool;
    auto AddAgent(OTAgent& theAgent) -> bool;
    auto GetAgentCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(agents_.size());
    }
    auto GetAgent(const UnallocatedCString& str_agent_name) const -> OTAgent*;
    auto GetAgentByIndex(std::int32_t nIndex) const -> OTAgent*;
    auto GetAuthorizingAgentName() const -> const UnallocatedCString&
    {
        return authorizing_agent_;
    }
    void SetAuthorizingAgentName(UnallocatedCString str_agent_name)
    {
        authorizing_agent_ = str_agent_name;
    }
    // If Nym is authorizing agent for Party, set agent's pointer to Nym and
    // return true.
    //
    auto HasAgent(
        const identity::Nym& theNym,
        OTAgent** ppAgent = nullptr) const -> bool;  // If Nym is agent for
                                                     // Party,
    // set agent's pointer to Nym
    // and return true.
    auto HasAgentByNymID(
        const identifier::Generic& theNymID,
        OTAgent** ppAgent = nullptr) const -> bool;
    auto HasAuthorizingAgent(
        const identity::Nym& theNym,
        OTAgent** ppAgent = nullptr) const -> bool;
    auto HasAuthorizingAgentByNymID(
        const identifier::Generic& theNymID,
        OTAgent** ppAgent = nullptr) const
        -> bool;  // ppAgent lets you get the agent
                  // ptr if it was there.
    // Load the authorizing agent from storage. Set agent's pointer to Nym.
    //
    auto LoadAuthorizingAgentNym(
        const identity::Nym& theSignerNym,
        OTAgent** ppAgent = nullptr) -> Nym_p;
    auto AddAccount(OTPartyAccount& thePartyAcct) -> bool;
    auto AddAccount(
        const String& strAgentName,
        const String& strName,
        const String& strAcctID,
        const String& strInstrumentDefinitionID,
        std::int64_t lClosingTransNo) -> bool;
    auto AddAccount(
        const String& strAgentName,
        const char* szAcctName,
        Account& theAccount,
        std::int64_t lClosingTransNo) -> bool;

    auto RemoveAccount(const UnallocatedCString str_Name) -> bool;

    auto GetAccountCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(party_accounts_.size());
    }  // returns total of all accounts owned by this party.
    auto GetAccountCount(UnallocatedCString str_agent_name) const
        -> std::int32_t;  // Only counts accounts authorized for str_agent_name.
    auto GetAccount(const UnallocatedCString& str_acct_name) const
        -> OTPartyAccount*;  // Get PartyAcct by name.
    auto GetAccountByIndex(std::int32_t nIndex) -> OTPartyAccount*;  // by index
    auto GetAccountByAgent(const UnallocatedCString& str_agent_name)
        -> OTPartyAccount*;  // by agent name
    auto GetAccountByID(const identifier::Generic& theAcctID) const
        -> OTPartyAccount*;  // by asset acct id
    // If account is present for Party, set account's pointer to theAccount and
    // return true.
    //
    auto HasAccount(
        const Account& theAccount,
        OTPartyAccount** ppPartyAccount = nullptr) const -> bool;
    auto HasAccountByID(
        const identifier::Generic& theAcctID,
        OTPartyAccount** ppPartyAccount = nullptr) const -> bool;
    auto VerifyOwnershipOfAccount(const Account& theAccount) const -> bool;
    auto VerifyAccountsWithTheirAgents(
        const String& strNotaryID,
        const PasswordPrompt& reason,
        bool bBurnTransNo = false) -> bool;
    auto CopyAcctsToConfirmingParty(OTParty& theParty) const
        -> bool;  // When confirming a party, a new version replaces the
                  // original. This is part of that process.
    void RegisterAccountsForExecution(OTScript& theScript);

    auto LoadAndVerifyAssetAccounts(
        const String& strNotaryID,
        mapOfAccounts& map_Accts_Already_Loaded,
        mapOfAccounts& map_NewlyLoaded) -> bool;

    // ------------- OPERATIONS -------------

    // Below this point, have all the actions that a party might do.
    //
    // (The party will internally call the appropriate agent according to its
    // own rules.
    // the script should not care how the party chooses its agents. At the most,
    // the script
    // only cares that the party has an active agent, but does not actually
    // speak directly
    // to said agent.)

    OTParty(const api::Session& api, const UnallocatedCString& dataFolder);
    OTParty(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const char* szName,
        bool bIsOwnerNym,
        const char* szOwnerID,
        const char* szAuthAgent,
        bool bCreateAgent = false);
    OTParty(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        UnallocatedCString str_PartyName,
        const identity::Nym& theNym,  // Nym is BOTH owner AND agent, when
                                      // using this constructor.
        UnallocatedCString str_agent_name,
        Account* pAccount = nullptr,
        const UnallocatedCString* pstr_account_name = nullptr,
        std::int64_t lClosingTransNo = 0);
    OTParty() = delete;
    OTParty(const OTParty&) = delete;
    auto operator=(const OTParty&) -> OTParty& = delete;

    virtual ~OTParty();

private:
    const api::Session& api_;
    const UnallocatedCString data_folder_;
    UnallocatedCString* party_name_;
    // true, is "nym". false, is "entity".
    bool party_is_nym_;
    UnallocatedCString owner_id_;           // Nym ID or Entity ID.
    UnallocatedCString authorizing_agent_;  // Contains the name of the
                                            // authorizing
                                            // agent (the one who supplied
                                            // the opening Trans#)

    mapOfAgents agents_;                 // These are owned.
    mapOfPartyAccounts party_accounts_;  // These are owned. Each contains a
                                         // Closing Transaction#.

    // Each party (to a smart contract anyway) must provide an opening
    // transaction #.
    TransactionNumber opening_trans_no_;
    OTString my_signed_copy_;  // One party confirms it and sends it over.
                               // Then another confirms it,
    // which adds his own transaction numbers and signs it. This, unfortunately,
    // invalidates the original version,
    // (since the digital signature ceases to verify, once you change the
    // contents.) So... we store a copy of each
    // signed agreement INSIDE each party. The server can do the hard work of
    // comparing them all, though such will
    // probably occur through a comparison function I'll have to add right here
    // in this class.

    // This Party is owned by an agreement (OTScriptable-derived.) Convenience
    // pointer
    OTScriptable* owner_agreement_{nullptr};

    void recover_closing_numbers(
        OTAgent& theAgent,
        otx::context::Server& context) const;
    void recover_opening_number(
        OTAgent& theAgent,
        otx::context::Server& context) const;
};
}  // namespace opentxs
