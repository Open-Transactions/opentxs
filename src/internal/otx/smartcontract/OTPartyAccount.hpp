// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Account.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
class OTAgent;
class OTParty;
class OTScript;
class OTSmartContract;
class PasswordPrompt;
class Tag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
// Each party has a list of accounts. Just as the agent, depending on context,
// MAY
// have an unowned-but-useful pointer to its active Nym, similarly a
// partyaccount
// MAY have an unowned-but-useful point to its actual Account.
//
// (Normally, an OTPartyAccount is loaded with the party, as part of an
// OTScriptable-
// derived object, and it doesn't actually load a copy of the actual Nyms and
// Accounts
// at that time, it just stores their IDs. But times may also come when the
// actual pointer
// is passed in for use by the Parties, who are not otherwise expected to do
// anything
// with it, or to clean it up or rely on it for their normal data storage.)
//
// Represents an account (to a party.) This is because ACTUALLY loading the REAL
// account is a pain in the ass,
// and we just want to store its ID, etc.  This way, the party can OWN this
// information about this list of
// accounts, and store it, load it, destroy it, etc, often without involving the
// actual account file at all.
//
// (Even if a this isn't currently doing anything with any of the party's
// accounts, the contract still
// has to have a LIST of them, so it knows what to do in those cases when it
// DOES need to do something.)
//
// For example, this is where we store the CLOSING TRANSACTION # for that
// account (for this agreement.)
// That number can't be found in the account itself, but all smart contracts
// involving asset accounts must
// provide a closing number for each asset account.
//
class OTPartyAccount
{
public:
    void RegisterForExecution(OTScript& theScript);

    auto GetParty() const -> OTParty* { return for_party_; }
    void SetParty(OTParty& theOwnerParty);  // This happens when the
                                            // partyaccount
                                            // is added to the party. (so I have
                                            // a ptr back)

    auto GetName() const -> const String&
    {
        return name_;
    }  // account's name as used in a script.
    auto GetAgentName() const -> const String&
    {
        return agent_name_;
    }  // agent's name as used in a script.
    auto GetAcctID() const -> const String&
    {
        return acct_id_;
    }  // account's ID as used internal to OT.
    auto GetInstrumentDefinitionID() const -> const String&
    {
        return instrument_definition_id_;
    }  // instrument definition ID for the account.

    void SetAgentName(const String& strAgentName)
    {
        agent_name_ = strAgentName;
    }
    void SetAcctID(const String& strAccountID) { acct_id_ = strAccountID; }
    auto GetAuthorizedAgent() -> OTAgent*;
    auto LoadAccount() -> SharedAccount;
    auto IsAccount(const Account& theAccount) -> bool;
    auto IsAccountByID(const identifier::Account& theAcctID) const -> bool;
    auto VerifyOwnership() const -> bool;  // I have a ptr
                                           // to my
    // owner (party), as well as to the actual account. I will ask him to
    // verify whether he actually owns it.
    auto VerifyAgency() -> bool;  // I can get a ptr to my
                                  // agent, and I have one
                                  // to the actual account.
                                  // I will ask him to
                                  // verify whether he
                                  // actually has agency
                                  // over it.
    auto GetClosingTransNo() const -> std::int64_t { return closing_trans_no_; }
    void SetClosingTransNo(std::int64_t lTransNo)
    {
        closing_trans_no_ = lTransNo;
    }
    auto Compare(const OTPartyAccount& rhs) const -> bool;
    auto DropFinalReceiptToInbox(
        const String& strNotaryID,
        OTSmartContract& theSmartContract,
        const std::int64_t& lNewTransactionNumber,
        const String& strOrigCronItem,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory()) -> bool;
    void Serialize(
        Tag& parent,
        bool bCalculatingID = false,
        bool bSpecifyInstrumentDefinitionID = false) const;

    OTPartyAccount(
        const api::Session& api,
        const UnallocatedCString& dataFolder);
    OTPartyAccount(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& str_account_name,
        const String& strAgentName,
        Account& theAccount,
        std::int64_t lClosingTransNo);
    OTPartyAccount(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const String& strName,
        const String& strAgentName,
        const String& strAcctID,
        const String& strInstrumentDefinitionID,
        std::int64_t lClosingTransNo);
    OTPartyAccount() = delete;
    OTPartyAccount(const OTPartyAccount&) = delete;
    OTPartyAccount(OTPartyAccount&&) = delete;
    auto operator=(const OTPartyAccount&) -> OTPartyAccount& = delete;
    auto operator=(OTPartyAccount&&) -> OTPartyAccount& = delete;

    virtual ~OTPartyAccount();

private:
    const api::Session& api_;
    const UnallocatedCString data_folder_{""};
    OTParty* for_party_;  // When being added to a party, this pointer will be
                          // set.
    // NOTE: each party needs to have a list of partyaccounts, AND each account
    // on that list needs to have a CLOSING #!!! Ahh...
    std::int64_t closing_trans_no_;  // Any account that is party to an
                                     // agreement,
                                     // must have a closing transaction # for
                                     // finalReceipt.
    // account name (inside the script language, "gold_acct_A" could be used to
    // reference this acct.)
    //
    OTString name_;     // Name of the account (for use in scripts.)
    OTString acct_id_;  // The Account ID itself.
    OTString instrument_definition_id_;  // The instrument definition ID for
                                         // the account. Stored
    // because parties agree on this even before the
    // account ID is selected. Compare() uses this
    // even when the account ID is blank, and when
    // acct ID *is* added, its instrument definition must match
    // this.
    OTString agent_name_;  // The name of the agent who has rights to this
                           // account.
    // Entity, role, and Nym information are not stored here.
    // Entity is already known on the party who owns this account (and I should
    // have a ptr to him.)
    // Role is already known on the agent who is presumably on the party's list
    // of agents.
    // Nym is known on the party (for owner) and on the agent.

    // "GetOwnerID()" for a partyaccount (if it were to store NymID, EntityIsD,
    // and a bool to choose
    // between them) should be logically the same as
    // owner_party_->GetPartyID().
    //

    auto get_account() const -> SharedAccount;
};
}  // namespace opentxs
