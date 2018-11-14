// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/script/OTPartyAccount.hpp"

#include "opentxs/api/Native.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/script/OTAgent.hpp"
#include "opentxs/core/script/OTParty.hpp"
#include "opentxs/core/script/OTScript.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>

// IDEA: Put a Nym in the Nyms folder for each entity. While it may
// not have a public key in the pubkey folder, or embedded within it,
// it can still have information about the entity or role related to it,
// which becomes accessible when that Nym is loaded based on the Entity ID.
// This also makes sure that Nyms and Entities don't ever share IDs, so the
// IDs become more and more interchangeable.

#define OT_METHOD "opentxs::OTPartyAccount::"

namespace opentxs
{
OTPartyAccount::OTPartyAccount(
    const api::Wallet& wallet,
    const std::string& dataFolder)
    : wallet_(wallet)
    , data_folder_{dataFolder}
    , m_pForParty(nullptr)
    , m_lClosingTransNo(0)
    , m_strName(String::Factory())
    , m_strAcctID(String::Factory())
    , m_strInstrumentDefinitionID(String::Factory())
    , m_strAgentName(String::Factory())

{
}

// For an account to be party to an agreement, there must be a closing
// transaction # provided, for the finalReceipt for that account.
OTPartyAccount::OTPartyAccount(
    const api::Wallet& wallet,
    const std::string& dataFolder,
    const std::string& str_account_name,
    const String& strAgentName,
    Account& theAccount,
    std::int64_t lClosingTransNo)
    : wallet_(wallet)
    , data_folder_{dataFolder}
    , m_pForParty(nullptr)
    // This gets set when this partyaccount is added to its party.
    , m_lClosingTransNo(lClosingTransNo)
    , m_strName(String::Factory(str_account_name.c_str()))
    , m_strAcctID(String::Factory(theAccount.GetRealAccountID()))
    , m_strInstrumentDefinitionID(
          String::Factory(theAccount.GetInstrumentDefinitionID()))
    , m_strAgentName(strAgentName)
{
}

OTPartyAccount::OTPartyAccount(
    const api::Wallet& wallet,
    const std::string& dataFolder,
    const String& strName,
    const String& strAgentName,
    const String& strAcctID,
    const String& strInstrumentDefinitionID,
    std::int64_t lClosingTransNo)
    : wallet_(wallet)
    , data_folder_{dataFolder}
    , m_pForParty(nullptr)
    // This gets set when this partyaccount is added to its party.
    , m_lClosingTransNo(lClosingTransNo)
    , m_strName(strName)
    , m_strAcctID(strAcctID)
    , m_strInstrumentDefinitionID(strInstrumentDefinitionID)
    , m_strAgentName(strAgentName)
{
}

SharedAccount OTPartyAccount::get_account() const
{
    if (!m_strAcctID->Exists()) { return {}; }

    return wallet_.Account(Identifier::Factory(m_strAcctID));
}

// Every partyaccount has its own authorized agent's name.
// Use that name to look up the agent ON THE PARTY (I already
// have a pointer to my owner party.)
//
OTAgent* OTPartyAccount::GetAuthorizedAgent()
{
    OT_ASSERT(nullptr != m_pForParty);

    if (!m_strAgentName->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: Authorized agent "
                                           "name (for this account) is blank!")
            .Flush();
        return nullptr;
    }

    const std::string str_agent_name = m_strAgentName->Get();

    OTAgent* pAgent = m_pForParty->GetAgent(str_agent_name);

    return pAgent;
}

// This happens when the partyaccount is added to the party.
//
void OTPartyAccount::SetParty(OTParty& theOwnerParty)
{
    m_pForParty = &theOwnerParty;
}

bool OTPartyAccount::IsAccountByID(const Identifier& theAcctID) const
{
    if (!m_strAcctID->Exists()) { return false; }

    if (!m_strInstrumentDefinitionID->Exists()) { return false; }

    const auto theMemberAcctID = Identifier::Factory(m_strAcctID);
    if (!(theAcctID == theMemberAcctID)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Account IDs don't match: ")(
            m_strAcctID)(" / ")(theAcctID)
            .Flush();

        return false;
    }

    // They  match!

    return true;
}

bool OTPartyAccount::IsAccount(const Account& theAccount)
{
    if (!m_strAcctID->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: Empty m_strAcctID.")
            .Flush();
        return false;
    }

    bool bCheckAssetId = true;
    if (!m_strInstrumentDefinitionID->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": FYI, Asset ID is blank in this smart contract, for this "
            "account.")
            .Flush();
        bCheckAssetId = false;
    }

    const auto theAcctID = Identifier::Factory(m_strAcctID);
    if (!(theAccount.GetRealAccountID() == theAcctID)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Account IDs don't match: ")(
            m_strAcctID)(" / ")(theAccount.GetRealAccountID())
            .Flush();

        return false;
    }

    if (bCheckAssetId) {
        const auto theInstrumentDefinitionID =
            Identifier::Factory(m_strInstrumentDefinitionID);
        if (!(theAccount.GetInstrumentDefinitionID() ==
              theInstrumentDefinitionID)) {
            auto strRHS =
                String::Factory(theAccount.GetInstrumentDefinitionID());
            {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Instrument Definition IDs don't "
                    "match ( ")(m_strInstrumentDefinitionID)(" / ")(strRHS)(
                    " ) for Acct ID: ")(m_strAcctID)(".")
                    .Flush();
            }
            return false;
        }
    }

    return true;
}

// I have a ptr to my owner (party), as well as to the actual account.
// I will ask him to verify whether he actually owns it.
bool OTPartyAccount::VerifyOwnership() const
{
    if (nullptr == m_pForParty) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: nullptr pointer to "
                                           "owner party.")
            .Flush();
        return false;
    }

    auto account = get_account();

    if (false == bool(account)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: nullptr pointer to "
            "account. (This function expects account to already be "
            "loaded).")
            .Flush();
        return false;
    }  // todo maybe turn the above into OT_ASSERT()s.

    if (!m_pForParty->VerifyOwnershipOfAccount(account.get())) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Party %s doesn't verify as "
                "the ACTUAL owner of account: ")(m_strName)(".")
                .Flush();
        }
        return false;
    }

    return true;
}

// I can get a ptr to my agent, and I have one to the actual account.
// I will ask him to verify whether he actually has agency over it.
bool OTPartyAccount::VerifyAgency()
{
    auto account = get_account();

    if (false == bool(account)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: nullptr pointer to "
            "account. (This function expects account to already be "
            "loaded).")
            .Flush();
        return false;
    }  // todo maybe turn the above into OT_ASSERT()s.

    OTAgent* pAgent = GetAuthorizedAgent();

    if (nullptr == pAgent) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Unable to find authorized agent (")(GetAgentName())(
                ") for this account: ")(GetName())(".")
                .Flush();
        }
        return false;
    }

    if (!pAgent->VerifyAgencyOfAccount(account.get())) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Agent ")(GetAgentName())(
                " doesn't verify as ACTUALLY having rights over account ")(
                GetName())(" with ID: ")(GetAcctID())(".")
                .Flush();
        }
        return false;
    }

    return true;
}

bool OTPartyAccount::DropFinalReceiptToInbox(
    const String& strNotaryID,
    OTSmartContract& theSmartContract,
    const std::int64_t& lNewTransactionNumber,
    const String& strOrigCronItem,
    OTString pstrNote,
    OTString pstrAttachment)
{
    if (nullptr == m_pForParty) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr m_pForParty.").Flush();
        return false;
    } else if (!m_strAcctID->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Empty Acct ID.").Flush();
        return false;
    } else if (!m_strAgentName->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No agent named for this account.")
            .Flush();
        return false;
    }

    // TODO: When entites and roles are added, this function may change a bit to
    // accommodate them.

    const std::string str_agent_name(m_strAgentName->Get());

    OTAgent* pAgent = m_pForParty->GetAgent(str_agent_name);

    if (nullptr == pAgent)
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Named agent wasn't found on party.")
            .Flush();
    else {
        const auto theAccountID = Identifier::Factory(m_strAcctID);

        return pAgent->DropFinalReceiptToInbox(
            strNotaryID,
            theSmartContract,
            theAccountID,  // acct ID from this.
            lNewTransactionNumber,
            m_lClosingTransNo,  // closing_no from this.
            strOrigCronItem,
            pstrNote,
            pstrAttachment);
    }

    return false;
}

// CALLER IS RESPONSIBLE TO DELETE.
// This is very low-level. (It's better to use OTPartyAccount through it's
// interface, than to just load up its account directly.) But this is here
// because it is appropriate in certain cases.
SharedAccount OTPartyAccount::LoadAccount()
{
    if (!m_strAcctID->Exists()) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Bad: Acct ID is blank for account: ")(m_strName)(".")
                .Flush();
        }

        return {};
    }

    auto account = wallet_.Account(Identifier::Factory(m_strAcctID));

    if (false == bool(account)) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to load account: ")(m_strName)(
                ", with AcctID: ")(m_strAcctID)(".")
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

    pTag->add_attribute("name", m_strName->Get());
    pTag->add_attribute("acctID", bCalculatingID ? "" : m_strAcctID->Get());
    pTag->add_attribute(
        "instrumentDefinitionID",
        (bCalculatingID && !bSpecifyInstrumentDefinitionID)
            ? ""
            : m_strInstrumentDefinitionID->Get());
    pTag->add_attribute(
        "agentName", bCalculatingID ? "" : m_strAgentName->Get());
    pTag->add_attribute(
        "closingTransNo", formatLong(bCalculatingID ? 0 : m_lClosingTransNo));

    parent.add_tag(pTag);
}

void OTPartyAccount::RegisterForExecution(OTScript& theScript)
{
    const std::string str_acct_name = m_strName->Get();
    theScript.AddAccount(str_acct_name, *this);
}

// Done
bool OTPartyAccount::Compare(const OTPartyAccount& rhs) const
{
    if (!(GetName().Compare(rhs.GetName()))) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Names don't match: ")(
                GetName())(" / ")(rhs.GetName())(".")
                .Flush();
        }
        return false;
    }

    if ((GetClosingTransNo() > 0) && (rhs.GetClosingTransNo() > 0) &&
        (GetClosingTransNo() != rhs.GetClosingTransNo())) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Closing transaction numbers don't match: ")(GetName())(".")
                .Flush();
        }
        return false;
    }

    if ((GetAcctID().Exists()) && (rhs.GetAcctID().Exists()) &&
        (!GetAcctID().Compare(rhs.GetAcctID()))) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Asset account numbers don't match "
                "for party account ")(GetName())(".")
                .Flush();

            LogNormal(OT_METHOD)(__FUNCTION__)(": ( ")(GetAcctID())("  /  ")(
                rhs.GetAcctID())(").")
                .Flush();
        }
        return false;
    }

    if ((GetAgentName().Exists()) && (rhs.GetAgentName().Exists()) &&
        (!GetAgentName().Compare(rhs.GetAgentName()))) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Agent Names don't match for party "
                "account ")(GetName())(".")
                .Flush();

            LogNormal(OT_METHOD)(__FUNCTION__)(": ( ")(GetAgentName())("  /  ")(
                rhs.GetAgentName())(").")
                .Flush();
        }
        return false;
    }

    if ((GetInstrumentDefinitionID().Exists() &&
         rhs.GetInstrumentDefinitionID().Exists()) &&
        !GetInstrumentDefinitionID().Compare(rhs.GetInstrumentDefinitionID())) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Instrument Definition IDs don't "
                "exist, or don't match (")(GetInstrumentDefinitionID())(" / ")(
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
    // m_pForParty NOT cleaned up here. pointer is only for convenience.
    m_pForParty = nullptr;
}
}  // namespace opentxs
