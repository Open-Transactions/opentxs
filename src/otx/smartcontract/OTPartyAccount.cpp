// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/smartcontract/OTPartyAccount.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/otx/smartcontract/OTAgent.hpp"
#include "internal/otx/smartcontract/OTParty.hpp"
#include "internal/otx/smartcontract/OTScript.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// IDEA: Put a Nym in the Nyms folder for each entity. While it may
// not have a public key in the pubkey folder, or embedded within it,
// it can still have information about the entity or role related to it,
// which becomes accessible when that Nym is loaded based on the Entity ID.
// This also makes sure that Nyms and Entities don't ever share IDs, so the
// IDs become more and more interchangeable.

namespace opentxs
{
OTPartyAccount::OTPartyAccount(
    const api::Session& api,
    const UnallocatedCString& dataFolder)
    : api_(api)
    , data_folder_{dataFolder}
    , for_party_(nullptr)
    , closing_trans_no_(0)
    , name_(String::Factory())
    , acct_id_(String::Factory())
    , instrument_definition_id_(String::Factory())
    , agent_name_(String::Factory())

{
}

// For an account to be party to an agreement, there must be a closing
// transaction # provided, for the finalReceipt for that account.
OTPartyAccount::OTPartyAccount(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& str_account_name,
    const String& strAgentName,
    Account& theAccount,
    std::int64_t lClosingTransNo)
    : api_(api)
    , data_folder_{dataFolder}
    , for_party_(nullptr)
    // This gets set when this partyaccount is added to its party.
    , closing_trans_no_(lClosingTransNo)
    , name_(String::Factory(str_account_name.c_str()))
    , acct_id_(String::Factory(theAccount.GetRealAccountID()))
    , instrument_definition_id_(
          String::Factory(theAccount.GetInstrumentDefinitionID()))
    , agent_name_(strAgentName)
{
}

OTPartyAccount::OTPartyAccount(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const String& strName,
    const String& strAgentName,
    const String& strAcctID,
    const String& strInstrumentDefinitionID,
    std::int64_t lClosingTransNo)
    : api_(api)
    , data_folder_{dataFolder}
    , for_party_(nullptr)
    // This gets set when this partyaccount is added to its party.
    , closing_trans_no_(lClosingTransNo)
    , name_(strName)
    , acct_id_(strAcctID)
    , instrument_definition_id_(strInstrumentDefinitionID)
    , agent_name_(strAgentName)
{
}

auto OTPartyAccount::get_account() const -> SharedAccount
{
    if (!acct_id_->Exists()) { return {}; }

    return api_.Wallet().Internal().Account(
        api_.Factory().IdentifierFromBase58(acct_id_->Bytes()));
}

// Every partyaccount has its own authorized agent's name.
// Use that name to look up the agent ON THE PARTY (I already
// have a pointer to my owner party.)
//
auto OTPartyAccount::GetAuthorizedAgent() -> OTAgent*
{
    OT_ASSERT(nullptr != for_party_);

    if (!agent_name_->Exists()) {
        LogError()(OT_PRETTY_CLASS())("Error: Authorized agent "
                                      "name (for this account) is blank!")
            .Flush();
        return nullptr;
    }

    const UnallocatedCString str_agent_name = agent_name_->Get();

    OTAgent* pAgent = for_party_->GetAgent(str_agent_name);

    return pAgent;
}

// This happens when the partyaccount is added to the party.
//
void OTPartyAccount::SetParty(OTParty& theOwnerParty)
{
    for_party_ = &theOwnerParty;
}

auto OTPartyAccount::IsAccountByID(const identifier::Generic& theAcctID) const
    -> bool
{
    if (!acct_id_->Exists()) { return false; }

    if (!instrument_definition_id_->Exists()) { return false; }

    const auto theMemberAcctID =
        api_.Factory().IdentifierFromBase58(acct_id_->Bytes());
    if (!(theAcctID == theMemberAcctID)) {
        LogTrace()(OT_PRETTY_CLASS())("Account IDs don't match: ")(
            acct_id_.get())(" / ")(theAcctID)
            .Flush();

        return false;
    }

    // They  match!

    return true;
}

auto OTPartyAccount::IsAccount(const Account& theAccount) -> bool
{
    if (!acct_id_->Exists()) {
        LogError()(OT_PRETTY_CLASS())("Error: Empty acct_id_.").Flush();
        return false;
    }

    bool bCheckAssetId = true;
    if (!instrument_definition_id_->Exists()) {
        LogError()(OT_PRETTY_CLASS())(
            "FYI, Asset ID is blank in this smart contract, for this account.")
            .Flush();
        bCheckAssetId = false;
    }

    const auto theAcctID =
        api_.Factory().IdentifierFromBase58(acct_id_->Bytes());
    if (!(theAccount.GetRealAccountID() == theAcctID)) {
        LogTrace()(OT_PRETTY_CLASS())("Account IDs don't match: ")(
            acct_id_.get())(" / ")(theAccount.GetRealAccountID())
            .Flush();

        return false;
    }

    if (bCheckAssetId) {
        const auto theInstrumentDefinitionID =
            api_.Factory().UnitIDFromBase58(instrument_definition_id_->Bytes());
        if (!(theAccount.GetInstrumentDefinitionID() ==
              theInstrumentDefinitionID)) {
            auto strRHS =
                String::Factory(theAccount.GetInstrumentDefinitionID());
            {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Instrument Definition IDs don't "
                    "match ( ")(instrument_definition_id_.get())(" / ")(
                    strRHS.get())(" ) for Acct ID: ")(acct_id_.get())(".")
                    .Flush();
            }
            return false;
        }
    }

    return true;
}

// I have a ptr to my owner (party), as well as to the actual account.
// I will ask him to verify whether he actually owns it.
auto OTPartyAccount::VerifyOwnership() const -> bool
{
    if (nullptr == for_party_) {
        LogError()(OT_PRETTY_CLASS())("Error: nullptr pointer to owner party.")
            .Flush();
        return false;
    }

    auto account = get_account();

    if (false == bool(account)) {
        LogError()(OT_PRETTY_CLASS())(
            "Error: nullptr pointer to account. (This function expects account "
            "to already be loaded).")
            .Flush();
        return false;
    }  // todo maybe turn the above into OT_ASSERT()s.

    if (!for_party_->VerifyOwnershipOfAccount(account.get())) {
        {
            LogConsole()(OT_PRETTY_CLASS())(
                "Party doesn't verify as the ACTUAL owner of account: ")(
                name_.get())(".")
                .Flush();
        }
        return false;
    }

    return true;
}

// I can get a ptr to my agent, and I have one to the actual account.
// I will ask him to verify whether he actually has agency over it.
auto OTPartyAccount::VerifyAgency() -> bool
{
    auto account = get_account();

    if (false == bool(account)) {
        LogError()(OT_PRETTY_CLASS())(
            "Error: nullptr pointer to account. (This function expects account "
            "to already be loaded).")
            .Flush();
        return false;
    }  // todo maybe turn the above into OT_ASSERT()s.

    OTAgent* pAgent = GetAuthorizedAgent();

    if (nullptr == pAgent) {
        {
            LogConsole()(OT_PRETTY_CLASS())(
                "Unable to find authorized agent (")(GetAgentName())(
                ") for this account: ")(GetName())(".")
                .Flush();
        }
        return false;
    }

    if (!pAgent->VerifyAgencyOfAccount(account.get())) {
        {
            LogConsole()(OT_PRETTY_CLASS())("Agent ")(GetAgentName())(
                " doesn't verify as ACTUALLY having rights over account ")(
                GetName())(" with ID: ")(GetAcctID())(".")
                .Flush();
        }
        return false;
    }

    return true;
}

auto OTPartyAccount::DropFinalReceiptToInbox(
    const String& strNotaryID,
    OTSmartContract& theSmartContract,
    const std::int64_t& lNewTransactionNumber,
    const String& strOrigCronItem,
    const PasswordPrompt& reason,
    OTString pstrNote,
    OTString pstrAttachment) -> bool
{
    if (nullptr == for_party_) {
        LogError()(OT_PRETTY_CLASS())("nullptr for_party_.").Flush();
        return false;
    } else if (!acct_id_->Exists()) {
        LogError()(OT_PRETTY_CLASS())("Empty Acct ID.").Flush();
        return false;
    } else if (!agent_name_->Exists()) {
        LogError()(OT_PRETTY_CLASS())("No agent named for this account.")
            .Flush();
        return false;
    }

    // TODO: When entites and roles are added, this function may change a bit to
    // accommodate them.

    const UnallocatedCString str_agent_name(agent_name_->Get());

    OTAgent* pAgent = for_party_->GetAgent(str_agent_name);

    if (nullptr == pAgent) {
        LogError()(OT_PRETTY_CLASS())("Named agent wasn't found on party.")
            .Flush();
    } else {
        const auto theAccountID =
            api_.Factory().IdentifierFromBase58(acct_id_->Bytes());

        return pAgent->DropFinalReceiptToInbox(
            strNotaryID,
            theSmartContract,
            theAccountID,  // acct ID from this.
            lNewTransactionNumber,
            closing_trans_no_,  // closing_no from this.
            strOrigCronItem,
            reason,
            pstrNote,
            pstrAttachment);
    }

    return false;
}

// CALLER IS RESPONSIBLE TO DELETE.
// This is very low-level. (It's better to use OTPartyAccount through it's
// interface, than to just load up its account directly.) But this is here
// because it is appropriate in certain cases.
auto OTPartyAccount::LoadAccount() -> SharedAccount
{
    if (!acct_id_->Exists()) {
        {
            LogConsole()(OT_PRETTY_CLASS())(
                "Bad: Acct ID is blank for account: ")(name_.get())(".")
                .Flush();
        }

        return {};
    }

    auto account = api_.Wallet().Internal().Account(
        api_.Factory().IdentifierFromBase58(acct_id_->Bytes()));

    if (false == bool(account)) {
        {
            LogConsole()(OT_PRETTY_CLASS())("Failed trying to load account: ")(
                name_.get())(", with AcctID: ")(acct_id_.get())(".")
                .Flush();
        }

        return {};
    }

    // This compares instrument definition ID, AND account ID on the actual
    // loaded account, to what is expected.
    else if (!IsAccount(account.get())) {

        return {};
    }

    return account;
}

void OTPartyAccount::Serialize(
    Tag& parent,
    bool bCalculatingID,
    bool bSpecifyInstrumentDefinitionID) const
{
    TagPtr pTag(new Tag("assetAccount"));

    pTag->add_attribute("name", name_->Get());
    pTag->add_attribute("acctID", bCalculatingID ? "" : acct_id_->Get());
    pTag->add_attribute(
        "instrumentDefinitionID",
        (bCalculatingID && !bSpecifyInstrumentDefinitionID)
            ? ""
            : instrument_definition_id_->Get());
    pTag->add_attribute("agentName", bCalculatingID ? "" : agent_name_->Get());
    pTag->add_attribute(
        "closingTransNo",
        std::to_string(bCalculatingID ? 0 : closing_trans_no_));

    parent.add_tag(pTag);
}

void OTPartyAccount::RegisterForExecution(OTScript& theScript)
{
    const UnallocatedCString str_acct_name = name_->Get();
    theScript.AddAccount(str_acct_name, *this);
}

// Done
auto OTPartyAccount::Compare(const OTPartyAccount& rhs) const -> bool
{
    if (!(GetName().Compare(rhs.GetName()))) {
        {
            LogConsole()(OT_PRETTY_CLASS())("Names don't match: ")(GetName())(
                " / ")(rhs.GetName())(".")
                .Flush();
        }
        return false;
    }

    if ((GetClosingTransNo() > 0) && (rhs.GetClosingTransNo() > 0) &&
        (GetClosingTransNo() != rhs.GetClosingTransNo())) {
        {
            LogConsole()(OT_PRETTY_CLASS())(
                "Closing transaction numbers don't match: ")(GetName())(".")
                .Flush();
        }
        return false;
    }

    if ((GetAcctID().Exists()) && (rhs.GetAcctID().Exists()) &&
        (!GetAcctID().Compare(rhs.GetAcctID()))) {
        {
            LogConsole()(OT_PRETTY_CLASS())(
                "Asset account numbers don't match for party account ")(
                GetName())(".")
                .Flush();

            LogConsole()(OT_PRETTY_CLASS())("( ")(GetAcctID())("  /  ")(
                rhs.GetAcctID())(").")
                .Flush();
        }
        return false;
    }

    if ((GetAgentName().Exists()) && (rhs.GetAgentName().Exists()) &&
        (!GetAgentName().Compare(rhs.GetAgentName()))) {
        {
            LogConsole()(OT_PRETTY_CLASS())("Agent Names don't match for party "
                                            "account ")(GetName())(".")
                .Flush();

            LogConsole()(OT_PRETTY_CLASS())("( ")(GetAgentName())("  /  ")(
                rhs.GetAgentName())(").")
                .Flush();
        }
        return false;
    }

    if ((GetInstrumentDefinitionID().Exists() &&
         rhs.GetInstrumentDefinitionID().Exists()) &&
        !GetInstrumentDefinitionID().Compare(rhs.GetInstrumentDefinitionID())) {
        {
            LogConsole()(OT_PRETTY_CLASS())(
                "Instrument Definition IDs don't exist, or don't match (")(
                GetInstrumentDefinitionID())(" / ")(
                rhs.GetInstrumentDefinitionID())(") for party's account: ")(
                GetName())(".")
                .Flush();
        }
        return false;
    }

    return true;
}

OTPartyAccount::~OTPartyAccount()
{
    // for_party_ NOT cleaned up here. pointer is only for convenience.
    for_party_ = nullptr;
}
}  // namespace opentxs
