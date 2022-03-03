// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "internal/otx/smartcontract/OTParty.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <utility>

#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/script/OTScriptable.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/otx/smartcontract/OTAgent.hpp"
#include "internal/otx/smartcontract/OTPartyAccount.hpp"
#include "internal/otx/smartcontract/OTSmartContract.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Shared.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs
{
OTParty::OTParty(
    const api::session::Wallet& wallet,
    const UnallocatedCString& dataFolder)
    : wallet_{wallet}
    , data_folder_{dataFolder}
    , m_pstr_party_name(nullptr)
    , m_bPartyIsNym(false)
    , m_str_owner_id()
    , m_str_authorizing_agent()
    , m_mapAgents()
    , m_mapPartyAccounts()
    , m_lOpeningTransNo(0)
    , m_strMySignedCopy(String::Factory())
    , m_pOwnerAgreement(nullptr)
{
}

OTParty::OTParty(
    const api::session::Wallet& wallet,
    const UnallocatedCString& dataFolder,
    const char* szName,
    bool bIsOwnerNym,
    const char* szOwnerID,
    const char* szAuthAgent,
    bool bCreateAgent)
    : wallet_{wallet}
    , data_folder_{dataFolder}
    , m_pstr_party_name(nullptr)
    , m_bPartyIsNym(bIsOwnerNym)
    , m_str_owner_id(szOwnerID != nullptr ? szOwnerID : "")
    , m_str_authorizing_agent(szAuthAgent != nullptr ? szAuthAgent : "")
    , m_mapAgents()
    , m_mapPartyAccounts()
    , m_lOpeningTransNo(0)
    , m_strMySignedCopy(String::Factory())
    , m_pOwnerAgreement(nullptr)
{
    m_pstr_party_name = new UnallocatedCString(szName != nullptr ? szName : "");

    if (bCreateAgent) {
        const auto strName = String::Factory(m_str_authorizing_agent.c_str()),
                   strNymID = String::Factory(""),
                   strRoleID = String::Factory(""),
                   strGroupName = String::Factory("");

        auto* pAgent = new OTAgent(
            wallet_,
            true /*bNymRepresentsSelf*/,
            true /*bIsAnIndividual*/,
            strName,
            strNymID,
            strRoleID,
            strGroupName);
        OT_ASSERT(nullptr != pAgent);

        if (!AddAgent(*pAgent)) {
            LogError()(OT_PRETTY_CLASS())("*** Failed *** while adding default "
                                          "agent in CONSTRUCTOR! 2.")
                .Flush();
            delete pAgent;
            pAgent = nullptr;
        }
    }
}

OTParty::OTParty(
    const api::session::Wallet& wallet,
    const UnallocatedCString& dataFolder,
    UnallocatedCString str_PartyName,
    const identity::Nym& theNym,  // Nym is BOTH owner AND agent, when using
                                  // this constructor.
    const UnallocatedCString str_agent_name,
    Account* pAccount,
    const UnallocatedCString* pstr_account_name,
    std::int64_t lClosingTransNo)
    : wallet_{wallet}
    , data_folder_{dataFolder}
    , m_pstr_party_name(new UnallocatedCString(str_PartyName))
    , m_bPartyIsNym(true)
    , m_str_owner_id()
    , m_str_authorizing_agent()
    , m_mapAgents()
    , m_mapPartyAccounts()
    , m_lOpeningTransNo(0)
    , m_strMySignedCopy(String::Factory())
    , m_pOwnerAgreement(nullptr)
{
    //  m_pstr_party_name = new UnallocatedCString(str_PartyName);
    OT_ASSERT(nullptr != m_pstr_party_name);

    // theNym is owner, therefore save his ID information, and create the agent
    // for this Nym automatically (that's why it was passed in.)
    // This code won't compile until you do.  :-)

    auto strNymID = String::Factory();
    theNym.GetIdentifier(strNymID);
    m_str_owner_id = strNymID->Get();

    auto* pAgent = new OTAgent(
        wallet_, str_agent_name, theNym);  // (The third arg, bRepresentsSelf,
                                           // defaults here to true.)
    OT_ASSERT(nullptr != pAgent);

    if (!AddAgent(*pAgent)) {
        LogError()(OT_PRETTY_CLASS())(
            "*** Failed *** while adding default agent "
            "in CONSTRUCTOR!")
            .Flush();
        delete pAgent;
        pAgent = nullptr;
    } else
        m_str_authorizing_agent = str_agent_name;

    // if pAccount is NOT nullptr, then an account was passed in, so
    // let's also create a default partyaccount for it.
    //
    if (nullptr != pAccount) {
        OT_ASSERT(nullptr != pstr_account_name);  // If passing an account, then
                                                  // you MUST pass an account
                                                  // name also.

        bool bAdded = AddAccount(
            String::Factory(str_agent_name.c_str()),
            pstr_account_name->c_str(),
            *pAccount,
            lClosingTransNo);

        if (!bAdded)
            LogError()(OT_PRETTY_CLASS())("*** Failed *** while adding default "
                                          "account in CONSTRUCTOR!")
                .Flush();
    }
}

// Checks opening number on party, and closing numbers on his accounts.
auto OTParty::HasTransactionNum(const std::int64_t& lInput) const -> bool
{
    if (lInput == m_lOpeningTransNo) return true;

    for (const auto& it : m_mapPartyAccounts) {
        const OTPartyAccount* pAcct = it.second;
        OT_ASSERT_MSG(
            nullptr != pAcct,
            "Unexpected nullptr partyaccount pointer in party map.");

        if (lInput == pAcct->GetClosingTransNo()) return true;
    }

    return false;
}

void OTParty::GetAllTransactionNumbers(NumList& numlistOutput) const
{
    if (m_lOpeningTransNo > 0) numlistOutput.Add(m_lOpeningTransNo);

    for (const auto& it : m_mapPartyAccounts) {
        const OTPartyAccount* pAcct = it.second;
        OT_ASSERT_MSG(
            nullptr != pAcct,
            "Unexpected nullptr partyaccount pointer in party map.");

        const std::int64_t lTemp = pAcct->GetClosingTransNo();
        if (lTemp > 0) numlistOutput.Add(lTemp);
    }
}

// Only counts accounts authorized for str_agent_name.
//
auto OTParty::GetAccountCount(UnallocatedCString str_agent_name) const
    -> std::int32_t
{
    std::int32_t nCount = 0;

    for (const auto& it : m_mapPartyAccounts) {
        const OTPartyAccount* pAcct = it.second;
        OT_ASSERT_MSG(
            nullptr != pAcct,
            "Unexpected nullptr partyaccount pointer in party map.");

        const String& strAgentName = pAcct->GetAgentName();

        if (strAgentName.Compare(str_agent_name.c_str())) nCount++;
    }

    return nCount;
}

auto OTParty::AddAgent(OTAgent& theAgent) -> bool
{
    const UnallocatedCString str_agent_name = theAgent.GetName().Get();

    if (!OTScriptable::ValidateName(str_agent_name)) {
        LogError()(OT_PRETTY_CLASS())("Failed validating Agent name.").Flush();
        return false;
    }

    auto it = m_mapAgents.find(str_agent_name);

    if (m_mapAgents.end() == it)  // If it wasn't already there...
    {
        // TODO: Validate here that the same agent isn't already on this party
        // under a different name.
        // Server either has to validate this as well, or be smart enough to
        // juggle the Nyms inside the
        // agents so that they aren't loaded twice.

        // Then insert it...
        m_mapAgents.insert(
            std::pair<UnallocatedCString, OTAgent*>(str_agent_name, &theAgent));

        // Make sure it has a pointer back to me.
        theAgent.SetParty(*this);

        return true;
    } else
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed -- Agent was already there named ")(str_agent_name)(".")
            .Flush();

    return false;
}

auto OTParty::AddAccount(
    const String& strAgentName,
    const String& strName,
    const String& strAcctID,
    const String& strInstrumentDefinitionID,
    std::int64_t lClosingTransNo) -> bool
{
    auto* pPartyAccount = new OTPartyAccount(
        wallet_,
        data_folder_,
        strName,
        strAgentName,
        strAcctID,
        strInstrumentDefinitionID,
        lClosingTransNo);
    OT_ASSERT(nullptr != pPartyAccount);

    if (!AddAccount(*pPartyAccount)) {
        delete pPartyAccount;
        return false;
    }

    return true;
}

auto OTParty::AddAccount(
    const String& strAgentName,
    const char* szAcctName,
    Account& theAccount,
    std::int64_t lClosingTransNo) -> bool
{
    auto* pPartyAccount = new OTPartyAccount(
        wallet_,
        data_folder_,
        szAcctName,
        strAgentName,
        theAccount,
        lClosingTransNo);
    OT_ASSERT(nullptr != pPartyAccount);

    if (!AddAccount(*pPartyAccount)) {
        delete pPartyAccount;
        return false;
    }

    return true;
}

auto OTParty::RemoveAccount(const UnallocatedCString str_Name) -> bool
{
    for (auto it = m_mapPartyAccounts.begin(); it != m_mapPartyAccounts.end();
         ++it) {
        OTPartyAccount* pAcct = it->second;
        OT_ASSERT(nullptr != pAcct);

        const UnallocatedCString str_acct_name = pAcct->GetName().Get();

        if (0 == str_acct_name.compare(str_Name)) {
            m_mapPartyAccounts.erase(it);
            delete pAcct;
            pAcct = nullptr;
            return true;
        }
    }

    return false;
}

auto OTParty::AddAccount(OTPartyAccount& thePartyAcct) -> bool
{
    const UnallocatedCString str_acct_name = thePartyAcct.GetName().Get();

    if (!OTScriptable::ValidateName(str_acct_name)) {
        LogError()(OT_PRETTY_CLASS())("Failed validating Account name.")
            .Flush();
        return false;
    }

    auto it = m_mapPartyAccounts.find(str_acct_name);

    if (m_mapPartyAccounts.end() == it)  // If it wasn't already there...
    {
        // Todo:  Validate here that there isn't another account already on the
        // same party
        // that, while it has a different "account name" actually has the same
        // Account ID.
        // We do not want the same Account ID on multiple accounts. (Unless the
        // script
        // interpreter is going to be smart enough to make them available that
        // way without
        // ever loading the same account twice.) We can't otherwise take the
        // risk, si server
        //  will have to validate this unless it juggles the accounts like that.
        //

        // Then insert it...
        m_mapPartyAccounts.insert(
            std::pair<UnallocatedCString, OTPartyAccount*>(
                str_acct_name, &thePartyAcct));

        // Make sure it has a pointer back to me.
        thePartyAcct.SetParty(*this);

        return true;
    } else
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed -- Account was already on party "
            "named ")(str_acct_name)(".")
            .Flush();

    return false;
}

auto OTParty::GetClosingTransNo(UnallocatedCString str_for_acct_name) const
    -> std::int64_t
{
    auto it = m_mapPartyAccounts.find(str_for_acct_name);

    if (m_mapPartyAccounts.end() == it)  // If it wasn't already there...
    {
        LogConsole()(OT_PRETTY_CLASS())("Failed -- Account wasn't found: ")(
            str_for_acct_name)(".")
            .Flush();
        return 0;
    }

    OTPartyAccount* pPartyAccount = it->second;
    OT_ASSERT(nullptr != pPartyAccount);

    return pPartyAccount->GetClosingTransNo();
}

void OTParty::CleanupAgents()
{

    while (!m_mapAgents.empty()) {
        OTAgent* pTemp = m_mapAgents.begin()->second;
        OT_ASSERT(nullptr != pTemp);
        delete pTemp;
        pTemp = nullptr;
        m_mapAgents.erase(m_mapAgents.begin());
    }
}

void OTParty::CleanupAccounts()
{

    while (!m_mapPartyAccounts.empty()) {
        OTPartyAccount* pTemp = m_mapPartyAccounts.begin()->second;
        OT_ASSERT(nullptr != pTemp);
        delete pTemp;
        pTemp = nullptr;
        m_mapPartyAccounts.erase(m_mapPartyAccounts.begin());
    }
}

void OTParty::ClearTemporaryPointers()
{
    for (auto& it : m_mapAgents) {
        OTAgent* pAgent = it.second;
        OT_ASSERT_MSG(
            nullptr != pAgent,
            "Unexpected nullptr agent pointer in party map.");
    }
}

// as used "IN THE SCRIPT."
//
auto OTParty::GetPartyName(bool* pBoolSuccess) const -> UnallocatedCString
{
    UnallocatedCString retVal("");

    // "sales_director", "marketer", etc
    if (nullptr == m_pstr_party_name) {
        if (nullptr != pBoolSuccess) *pBoolSuccess = false;

        return retVal;
    }

    if (nullptr != pBoolSuccess) *pBoolSuccess = true;

    retVal = *m_pstr_party_name;

    return retVal;
}

auto OTParty::SetPartyName(const UnallocatedCString& str_party_name_input)
    -> bool
{
    if (!OTScriptable::ValidateName(str_party_name_input)) {
        LogError()(OT_PRETTY_CLASS())("Failed: Invalid name was passed in.")
            .Flush();
        return false;
    }

    if (nullptr == m_pstr_party_name)
        OT_ASSERT(nullptr != (m_pstr_party_name = new UnallocatedCString));

    *m_pstr_party_name = str_party_name_input;

    return true;
}

// ACTUAL PARTY OWNER (Only ONE of these can be true...)
// Debating whether these two functions should be private. (Should it matter to
// outsider?)
//
auto OTParty::IsNym() const -> bool
{
    // If the party is a Nym. (The party is the actual owner/beneficiary.)
    return m_bPartyIsNym;
}

auto OTParty::IsEntity() const -> bool
{
    // If the party is an Entity. (Either way, the AGENT carries out all
    // wishes.)
    return !m_bPartyIsNym;
}

// ACTUAL PARTY OWNER
//
auto OTParty::GetNymID(bool* pBoolSuccess) const -> UnallocatedCString
{
    if (IsNym() && (m_str_owner_id.size() > 0)) {
        if (nullptr != pBoolSuccess) *pBoolSuccess = true;

        return m_str_owner_id;
    }

    if (nullptr != pBoolSuccess) *pBoolSuccess = false;

    UnallocatedCString retVal("");

    return retVal;  // empty ID on failure.
}

auto OTParty::GetEntityID(bool* pBoolSuccess) const -> UnallocatedCString
{
    if (IsEntity() && (m_str_owner_id.size() > 0)) {
        if (nullptr != pBoolSuccess) *pBoolSuccess = true;

        return m_str_owner_id;
    }

    if (nullptr != pBoolSuccess) *pBoolSuccess = false;

    UnallocatedCString retVal("");

    return retVal;
}

auto OTParty::GetPartyID(bool* pBoolSuccess) const -> UnallocatedCString
{
    // If party is a Nym, this is the NymID. Else return EntityID().
    // if error, return false.

    if (IsNym()) return GetNymID(pBoolSuccess);

    return GetEntityID(pBoolSuccess);
}

// Some agents are passive (voting groups) and cannot behave actively, and so
// cannot do
// certain things that only Nyms can do. But they can still act as an agent in
// CERTAIN
// respects, so they are still allowed to do so. However, likely many functions
// will
// require that HasActiveAgent() be true for a party to do various actions.
// Attempts to
// do those actions otherwise will fail.
// It's almost a separate kind of party but not worthy of a separate class.
//
auto OTParty::HasActiveAgent() const -> bool
{
    // Loop through all agents and call IsAnIndividual() on each.
    // then return true if any is true. else return false;
    //
    for (const auto& it : m_mapAgents) {
        OTAgent* pAgent = it.second;
        OT_ASSERT(nullptr != pAgent);

        if (pAgent->IsAnIndividual()) return true;
    }

    return false;
}

/// Get Agent pointer by Name. Returns nullptr on failure.
///
auto OTParty::GetAgent(const UnallocatedCString& str_agent_name) const
    -> OTAgent*
{
    if (OTScriptable::ValidateName(str_agent_name)) {
        auto it = m_mapAgents.find(str_agent_name);

        if (m_mapAgents.end() != it)  // If we found something...
        {
            OTAgent* pAgent = it->second;
            OT_ASSERT(nullptr != pAgent);

            return pAgent;
        }
    } else
        LogError()(OT_PRETTY_CLASS())("Failed: str_agent_name is invalid...")
            .Flush();

    return nullptr;
}

/// Get Agent pointer by Index. Returns nullptr on failure.
///
auto OTParty::GetAgentByIndex(std::int32_t nIndex) const -> OTAgent*
{
    if (!((nIndex >= 0) &&
          (nIndex < static_cast<std::int64_t>(m_mapAgents.size())))) {
        LogError()(OT_PRETTY_CLASS())("Index out of bounds: ")(nIndex)(".")
            .Flush();
    } else {
        std::int32_t nLoopIndex = -1;

        for (auto& it : m_mapAgents) {
            OTAgent* pAgent = it.second;
            OT_ASSERT(nullptr != pAgent);

            ++nLoopIndex;  // 0 on first iteration.

            if (nLoopIndex == nIndex) return pAgent;
        }
    }
    return nullptr;
}

// Get PartyAccount pointer by Name. Returns nullptr on failure.
//
auto OTParty::GetAccount(const UnallocatedCString& str_acct_name) const
    -> OTPartyAccount*
{
    //    otErr << "DEBUGGING OTParty::GetAccount: above find. str_acct_name: %s
    // \n", str_acct_name.c_str());

    if (OTScriptable::ValidateName(str_acct_name)) {
        auto it = m_mapPartyAccounts.find(str_acct_name);

        if (m_mapPartyAccounts.end() != it)  // If we found something...
        {
            OTPartyAccount* pAcct = it->second;
            OT_ASSERT(nullptr != pAcct);

            return pAcct;
        }
    } else
        LogError()(OT_PRETTY_CLASS())("Failed: str_acct_name is invalid.")
            .Flush();

    return nullptr;
}

/// Get OTPartyAccount pointer by Index. Returns nullptr on failure.
///
auto OTParty::GetAccountByIndex(std::int32_t nIndex) -> OTPartyAccount*
{
    if (!((nIndex >= 0) &&
          (nIndex < static_cast<std::int64_t>(m_mapPartyAccounts.size())))) {
        LogError()(OT_PRETTY_CLASS())("Index out of bounds: ")(nIndex)(".")
            .Flush();
    } else {
        std::int32_t nLoopIndex = -1;

        for (auto& it : m_mapPartyAccounts) {
            OTPartyAccount* pAcct = it.second;
            OT_ASSERT(nullptr != pAcct);

            ++nLoopIndex;  // 0 on first iteration.

            if (nLoopIndex == nIndex) return pAcct;
        }
    }
    return nullptr;
}

// Get PartyAccount pointer by Agent Name. (It just grabs the first one.)
//
// Returns nullptr on failure.
auto OTParty::GetAccountByAgent(const UnallocatedCString& str_agent_name)
    -> OTPartyAccount*
{
    if (OTScriptable::ValidateName(str_agent_name)) {
        for (auto& it : m_mapPartyAccounts) {
            OTPartyAccount* pAcct = it.second;
            OT_ASSERT(nullptr != pAcct);

            if (pAcct->GetAgentName().Compare(str_agent_name.c_str()))
                return pAcct;
        }
    } else
        LogError()(OT_PRETTY_CLASS())("Failed: str_agent_name is invalid.")
            .Flush();

    return nullptr;
}

// Get PartyAccount pointer by Acct ID.
//
// Returns nullptr on failure.
auto OTParty::GetAccountByID(const Identifier& theAcctID) const
    -> OTPartyAccount*
{
    for (const auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT(nullptr != pAcct);

        if (pAcct->IsAccountByID(theAcctID)) return pAcct;
    }

    return nullptr;
}

// bool OTPartyAccount::IsAccountByID(const Identifier& theAcctID) const

// If account is present for Party, return true.
auto OTParty::HasAccountByID(
    const Identifier& theAcctID,
    OTPartyAccount** ppPartyAccount) const -> bool
{
    for (const auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT(nullptr != pAcct);

        if (pAcct->IsAccountByID(theAcctID)) {
            if (nullptr != ppPartyAccount) *ppPartyAccount = pAcct;

            return true;
        }
    }

    return false;
}

// If account is present for Party, set account's pointer to theAccount and
// return true.
auto OTParty::HasAccount(
    const Account& theAccount,
    OTPartyAccount** ppPartyAccount) const -> bool
{
    for (const auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT(nullptr != pAcct);

        if (pAcct->IsAccount(theAccount)) {
            if (nullptr != ppPartyAccount) *ppPartyAccount = pAcct;

            return true;
        }
    }

    return false;
}

// Find out if theNym is an agent for Party.
// If so, make sure that agent has a pointer to theNym and return true.
// else return false.
//
auto OTParty::HasAgent(const identity::Nym& theNym, OTAgent** ppAgent) const
    -> bool
{
    for (const auto& it : m_mapAgents) {
        OTAgent* pAgent = it.second;
        OT_ASSERT(nullptr != pAgent);

        if (pAgent->IsValidSigner(theNym)) {
            if (nullptr != ppAgent) *ppAgent = pAgent;

            return true;
        }
    }

    return false;
}

auto OTParty::HasAgentByNymID(const Identifier& theNymID, OTAgent** ppAgent)
    const -> bool
{
    for (const auto& it : m_mapAgents) {
        OTAgent* pAgent = it.second;
        OT_ASSERT(nullptr != pAgent);

        if (pAgent->IsValidSignerID(theNymID)) {
            if (nullptr != ppAgent) *ppAgent = pAgent;

            return true;
        }
    }

    return false;
}

// Find out if theNym is authorizing agent for Party. (Supplied opening
// transaction #) If so, make sure that agent has a pointer to theNym and return
// true. else return false.
auto OTParty::HasAuthorizingAgent(
    const identity::Nym& theNym,
    OTAgent** ppAgent) const -> bool  // ppAgent lets you get the agent ptr if
                                      // it was there.
{
    if (OTScriptable::ValidateName(m_str_authorizing_agent)) {
        auto it = m_mapAgents.find(m_str_authorizing_agent);

        if (m_mapAgents.end() != it)  // If we found something...
        {
            OTAgent* pAgent = it->second;
            OT_ASSERT(nullptr != pAgent);

            if (pAgent->IsValidSigner(theNym)) {
                // Optionally can pass in a pointer-to-pointer-to-Agent, in
                // order to get the Agent pointer back.
                if (nullptr != ppAgent) *ppAgent = pAgent;

                return true;
            }
        } else  // found nothing.
            LogError()(OT_PRETTY_CLASS())("Named agent wasn't found "
                                          "on list.")
                .Flush();
    }

    return false;
}

auto OTParty::HasAuthorizingAgentByNymID(
    const Identifier& theNymID,
    OTAgent** ppAgent) const -> bool  // ppAgent
                                      // lets you
                                      // get the
                                      // agent ptr
                                      // if it was
                                      // there.
{
    if (OTScriptable::ValidateName(m_str_authorizing_agent)) {
        auto it = m_mapAgents.find(m_str_authorizing_agent);

        if (m_mapAgents.end() != it)  // If we found something...
        {
            OTAgent* pAgent = it->second;
            OT_ASSERT(nullptr != pAgent);

            if (pAgent->IsValidSignerID(theNymID))  // if theNym is valid signer
                                                    // for pAgent.
            {
                // Optionally can pass in a pointer-to-pointer-to-Agent, in
                // order to get the Agent pointer back.
                if (nullptr != ppAgent) *ppAgent = pAgent;

                return true;
            }
        } else  // found nothing.
            LogError()(OT_PRETTY_CLASS())("Named agent wasn't "
                                          "found on list.")
                .Flush();
    }

    return false;
}

// Load up the Nym that authorized the agreement for this party
// (the nym who supplied the opening trans# to sign it.)
//
// This function ASSUMES that you ALREADY called HasAuthorizingAgentNym(), for
// example
// to verify that the serverNym isn't the one you were looking for.
// This is a low-level function.

// ppAgent lets you get the agent ptr if it was there.
auto OTParty::LoadAuthorizingAgentNym(
    const identity::Nym& theSignerNym,
    OTAgent** ppAgent) -> Nym_p
{
    if (OTScriptable::ValidateName(m_str_authorizing_agent)) {
        auto it = m_mapAgents.find(m_str_authorizing_agent);

        if (m_mapAgents.end() != it)  // If we found something...
        {
            OTAgent* pAgent = it->second;
            OT_ASSERT(nullptr != pAgent);

            Nym_p pNym = nullptr;

            if (!pAgent->IsAnIndividual())
                LogError()(OT_PRETTY_CLASS())(
                    "This agent is not "
                    "an individual--there's no Nym to load.")
                    .Flush();
            else if (nullptr == (pNym = pAgent->LoadNym()))
                LogError()(OT_PRETTY_CLASS())("Failed loading "
                                              "Nym.")
                    .Flush();
            else {
                if (nullptr != ppAgent)  // Pass the agent back, too, if it was
                                         // requested.
                    *ppAgent = pAgent;

                return pNym;  // Success
            }
        } else  // found nothing.
            LogError()(OT_PRETTY_CLASS())("Named agent wasn't "
                                          "found on list.")
                .Flush();
    }

    return nullptr;
}

auto OTParty::VerifyOwnershipOfAccount(const Account& theAccount) const -> bool
{
    if (IsNym())  // For those cases where the party is actually just a
                  // solitary Nym (not an entity.)
    {
        bool bNymID = false;
        UnallocatedCString str_nym_id =
            GetNymID(&bNymID);  // If the party is a Nym, this is the Nym's
                                // ID. Otherwise this is false.

        if (!bNymID || (str_nym_id.size() <= 0)) {
            LogError()(OT_PRETTY_CLASS())("Although party is a "
                                          "Nym, unable to retrieve NymID!")
                .Flush();
            return false;
        }

        const auto thePartyNymID = identifier::Nym::Factory(str_nym_id);

        return theAccount.VerifyOwnerByID(thePartyNymID);
    } else if (IsEntity())
        LogError()(OT_PRETTY_CLASS())("Error: Entities have not "
                                      "been implemented yet, "
                                      "but somehow this party is an entity.")
            .Flush();
    else
        LogError()(OT_PRETTY_CLASS())("Error: Unknown party "
                                      "type.")
            .Flush();

    return false;
}

// This is only for SmartContracts, NOT all scriptables.
//
auto OTParty::DropFinalReceiptToInboxes(
    const String& strNotaryID,
    const std::int64_t& lNewTransactionNumber,
    const String& strOrigCronItem,
    const PasswordPrompt& reason,
    OTString pstrNote,
    OTString pstrAttachment) -> bool
{
    bool bSuccess = true;  // Success is defined as "all inboxes were notified"

    OTSmartContract* pSmartContract = nullptr;

    if (nullptr == m_pOwnerAgreement) {
        LogError()(OT_PRETTY_CLASS())("Missing pointer to owner agreement.")
            .Flush();
        return false;
    } else if (
        nullptr ==
        (pSmartContract = dynamic_cast<OTSmartContract*>(m_pOwnerAgreement))) {
        LogError()(OT_PRETTY_CLASS())(
            "Can only drop finalReceipts for smart contracts.")
            .Flush();
        return false;
    }

    // By this point, we know pSmartContract is a good pointer.

    for (auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT_MSG(
            nullptr != pAcct,
            "Unexpected nullptr partyaccount pointer in party map.");

        if (false == pAcct->DropFinalReceiptToInbox(
                         strNotaryID,
                         *pSmartContract,
                         lNewTransactionNumber,
                         strOrigCronItem,
                         reason,
                         pstrNote,
                         pstrAttachment)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed dropping final Receipt to agent's Inbox.")
                .Flush();
            bSuccess = false;  // Notice: no break. We still try to notify them
                               // all, even if one fails.
        }
    }

    return bSuccess;
}

// This is only for SmartContracts, NOT all scriptables.
//
auto OTParty::DropFinalReceiptToNymboxes(
    const std::int64_t& lNewTransactionNumber,
    const String& strOrigCronItem,
    const PasswordPrompt& reason,
    OTString pstrNote,
    OTString pstrAttachment) -> bool
{
    bool bSuccess =
        false;  // Success is defined as "at least one agent was notified"

    OTSmartContract* pSmartContract = nullptr;

    if (nullptr == m_pOwnerAgreement) {
        LogError()(OT_PRETTY_CLASS())("Missing pointer to "
                                      "owner agreement.")
            .Flush();
        return false;
    } else if (
        nullptr ==
        (pSmartContract = dynamic_cast<OTSmartContract*>(m_pOwnerAgreement))) {
        LogError()(OT_PRETTY_CLASS())("Can only drop "
                                      "finalReceipts for smart contracts.")
            .Flush();
        return false;
    }

    // By this point, we know pSmartContract is a good pointer.

    for (auto& it : m_mapAgents) {
        OTAgent* pAgent = it.second;
        OT_ASSERT_MSG(
            nullptr != pAgent,
            "Unexpected nullptr agent pointer in party map.");

        if (false == pAgent->DropFinalReceiptToNymbox(
                         *pSmartContract,
                         lNewTransactionNumber,
                         strOrigCronItem,
                         reason,
                         pstrNote,
                         pstrAttachment))
            LogError()(OT_PRETTY_CLASS())("Failed dropping "
                                          "final Receipt to agent's Nymbox.")
                .Flush();
        else
            bSuccess = true;
    }

    return bSuccess;
}

auto OTParty::SendNoticeToParty(
    const api::Session& api,
    bool bSuccessMsg,
    const identity::Nym& theServerNym,
    const identifier::Notary& theNotaryID,
    const std::int64_t& lNewTransactionNumber,
    const String& strReference,
    const PasswordPrompt& reason,
    OTString pstrNote,
    OTString pstrAttachment,
    identity::Nym* pActualNym) -> bool
{
    bool bSuccess =
        false;  // Success is defined as "at least one agent was notified"

    if (nullptr == m_pOwnerAgreement) {
        LogError()(OT_PRETTY_CLASS())("Missing pointer to owner agreement.")
            .Flush();
        return false;
    }

    const std::int64_t lOpeningTransNo = GetOpeningTransNo();

    if (lOpeningTransNo > 0) {
        for (auto& it : m_mapAgents) {
            OTAgent* pAgent = it.second;
            OT_ASSERT_MSG(
                nullptr != pAgent,
                "Unexpected nullptr agent pointer in party map.");

            if (false == pAgent->DropServerNoticeToNymbox(
                             api,
                             bSuccessMsg,
                             theServerNym,
                             theNotaryID,
                             lNewTransactionNumber,
                             lOpeningTransNo,  // lInReferenceTo
                             strReference,
                             reason,
                             pstrNote,
                             pstrAttachment,
                             pActualNym))
                LogError()(OT_PRETTY_CLASS())(
                    "Failed dropping server notice to agent's Nymbox.")
                    .Flush();
            else
                bSuccess = true;
        }
    }
    return bSuccess;
}

auto OTParty::LoadAndVerifyAssetAccounts(
    const String& strNotaryID,
    mapOfAccounts& map_Accts_Already_Loaded,
    mapOfAccounts& map_NewlyLoaded) -> bool
{
    UnallocatedSet<UnallocatedCString> theAcctIDSet;  // Make sure all the acct
                                                      // IDs are unique.

    for (auto& it_acct : m_mapPartyAccounts) {
        const UnallocatedCString str_acct_name = it_acct.first;
        OTPartyAccount* pPartyAcct = it_acct.second;
        OT_ASSERT(pPartyAcct != nullptr);

        bool bHadToLoadtheAcctMyself = true;
        SharedAccount account;
        const String& strAcctID = pPartyAcct->GetAcctID();

        if (!strAcctID.Exists()) {
            LogConsole()(OT_PRETTY_CLASS())("Bad: Acct ID is "
                                            "blank for account: ")(
                str_acct_name)(", on party: ")(GetPartyName())(".")
                .Flush();
            return false;
        }

        // Disallow duplicate Acct IDs.
        // (Only can use an acct once inside a smart contract.)
        //
        auto it_acct_id = theAcctIDSet.find(strAcctID.Get());

        if (theAcctIDSet.end() == it_acct_id)  // It's not already there (good).
        {
            theAcctIDSet.insert(strAcctID.Get());  // Save a copy so we can make
                                                   // sure there's no duplicate
                                                   // acct IDs. (Not allowed.)
        } else {
            LogConsole()(OT_PRETTY_CLASS())("Failure: Found a "
                                            "duplicate Acct ID (")(
                strAcctID)("), on acct: ")(str_acct_name)(".")
                .Flush();
            return false;
        }

        // If it's there, it's mapped by Acct ID, so we can look it up.
        auto it = map_Accts_Already_Loaded.find(strAcctID.Get());

        if (map_Accts_Already_Loaded.end() != it) {
            account = it->second;

            OT_ASSERT(account);

            // Now we KNOW the Account is "already loaded" and we KNOW the
            // partyaccount has a POINTER to that Acct:
            //
            const bool bIsPartyAcct = pPartyAcct->IsAccount(account.get());
            OT_ASSERT_MSG(
                bIsPartyAcct,
                "OTParty::LoadAndVerifyAssetAccounts: Failed call: "
                "pPartyAcct->IsAccount(*account); \n");  // assert because the
                                                         // Acct was already
                                                         // mapped by ID, so it
                                                         // should already have
                                                         // been validated.
            if (!bIsPartyAcct)
                LogError()(OT_PRETTY_CLASS())(
                    "Failed call: "
                    "pPartyAcct->IsAccount(*account).")
                    .Flush();

            bHadToLoadtheAcctMyself = false;  // Whew. The Acct was already
                                              // loaded. Found it. (And the ptr
                                              // is now set.)
        }

        // Looks like the Acct wasn't already loaded....
        // Let's load it up...
        //
        if (bHadToLoadtheAcctMyself == true) {
            if ((account = pPartyAcct->LoadAccount())) {
                LogConsole()(OT_PRETTY_CLASS())("Failed loading "
                                                "Account with name: ")(
                    str_acct_name)(" and ID: ")(strAcctID)(".")
                    .Flush();
                return false;
            }
            // Successfully loaded the Acct! We add to this map so it gets
            // cleaned-up properly later.
            map_NewlyLoaded.emplace(strAcctID.Get(), std::move(account));
        }
    }

    return true;
}

// This is only meant to be used in OTSmartContract::VerifySmartContract() RIGHT
// AFTER the call
// to VerifyPartyAuthorization(). It ASSUMES the nyms and asset accounts are all
// loaded up, with
// internal pointers to them available.
//
auto OTParty::VerifyAccountsWithTheirAgents(
    const String& strNotaryID,
    const PasswordPrompt& reason,
    bool bBurnTransNo) -> bool
{
    OT_ASSERT(nullptr != m_pOwnerAgreement);

    bool bAllSuccessful = true;

    // By this time this function is called, ALL the Nyms and Asset Accounts
    // should ALREADY
    // be loaded up in memory!
    //
    for (auto& it : m_mapPartyAccounts) {
        const UnallocatedCString str_acct_name = it.first;
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT_MSG(
            nullptr != pAcct,
            "Unexpected nullptr partyaccount pointer in party map.");

        const bool bVerified = m_pOwnerAgreement->VerifyPartyAcctAuthorization(
            reason,
            *pAcct,  // The party is assumed to have been verified already via
                     // VerifyPartyAuthorization()
            strNotaryID,    // For verifying issued num, need the notaryID the #
                            // goes with.
            bBurnTransNo);  // bBurnTransNo=false ) // In
                            // Server::VerifySmartContract(), it not only
                            // wants to verify the closing # is properly issued,
                            // but it additionally wants to see that it hasn't
                            // been USED yet -- AND it wants to burn it, so it
                            // can't be used again!  This bool allows you to
                            // tell the function whether or not to do that.
        if (!bVerified)     // This mechanism is here because we still want
                            // to let them ALL verify, before returning
                            // false.
        {
            bAllSuccessful = false;  // That is, we don't want to return at the
                                     // first failure, but let them all go
                                     // through. (This is in order to keep the
                                     // output consistent.)
            LogConsole()(OT_PRETTY_CLASS())(
                "Ownership, agency, or potentially "
                "closing transaction # failed to verify on account: ")(
                str_acct_name)(".")
                .Flush();
        }
    }

    return bAllSuccessful;
}

// Done
// The party will use its authorizing agent.
//
auto OTParty::SignContract(Contract& theInput, const PasswordPrompt& reason)
    const -> bool
{
    if (GetAuthorizingAgentName().size() <= 0) {
        LogError()(OT_PRETTY_CLASS())("Error: Authorizing agent name is blank.")
            .Flush();
        return false;
    }

    OTAgent* pAgent = GetAgent(GetAuthorizingAgentName());

    if (nullptr == pAgent) {
        LogError()(OT_PRETTY_CLASS())(
            "Error: Unable to find Authorizing agent (")(
            GetAuthorizingAgentName())(") for party: ")(GetPartyName())(".")
            .Flush();
        return false;
    }

    return pAgent->SignContract(theInput, reason);
}

// for whichever partyaccounts have agents that happen to be loaded, this will
// harvest the closing trans#s.
// Calls OTAgent::HarvestTransactionNumber
void OTParty::HarvestClosingNumbers(
    const String& strNotaryID,
    const PasswordPrompt& reason)
{
    for (auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pAcct = it.second;

        OT_ASSERT_MSG(
            nullptr != pAcct,
            "OTParty::HarvestClosingNumbers: "
            "Unexpected nullptr partyaccount pointer in "
            "party map.");

        if (pAcct->GetClosingTransNo() <= 0) { continue; }

        const UnallocatedCString str_agent_name(pAcct->GetAgentName().Get());

        if (str_agent_name.size() <= 0) {
            LogError()(OT_PRETTY_CLASS())(
                "Missing agent name on party account: ")(pAcct->GetName())(".")
                .Flush();

            continue;
        }

        OTAgent* pAgent = GetAgent(str_agent_name);

        if (nullptr == pAgent) {
            LogError()(OT_PRETTY_CLASS())("Couldn't find agent (")(
                str_agent_name)(") for asset account: ")(pAcct->GetName())(".")
                .Flush();
        } else {
            pAgent->RecoverTransactionNumber(
                pAcct->GetClosingTransNo(), strNotaryID, reason);
        }
    }
}

// Done
// Calls OTAgent::HarvestTransactionNumber
//
void OTParty::recover_closing_numbers(
    OTAgent& theAgent,
    otx::context::Server& context) const
{
    for (auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pAcct = it.second;

        OT_ASSERT_MSG(
            nullptr != pAcct,
            "OTParty::HarvestClosingNumbers: "
            "Unexpected nullptr partyaccount pointer in "
            "partyaccount map.");

        if (pAcct->GetClosingTransNo() <= 0) { continue; }

        const UnallocatedCString str_agent_name(pAcct->GetAgentName().Get());

        if (str_agent_name.size() <= 0) { continue; }

        if (theAgent.GetName().Compare(str_agent_name.c_str())) {
            theAgent.RecoverTransactionNumber(
                pAcct->GetClosingTransNo(), context);
        }
        // We don't break here, on success, because this agent might represent
        // multiple accounts.
        // else nothing...
    }
}

// Done.
// IF theNym is one of my agents, then grab his numbers back for him.
// If he is NOT one of my agents, then do nothing.
void OTParty::HarvestClosingNumbers(otx::context::Server& context)
{
    OTAgent* pAgent = nullptr;

    if (HasAgent(*context.Nym(), &pAgent)) {

        OT_ASSERT(nullptr != pAgent);

        recover_closing_numbers(*pAgent, context);
    }
    // else nothing...
}

// Done
// IF theNym is one of my agents, then grab his opening number back for him.
// If he is NOT one of my agents, then do nothing.
void OTParty::HarvestOpeningNumber(otx::context::Server& context)
{
    OTAgent* pAgent = nullptr;

    if (HasAuthorizingAgent(*context.Nym(), &pAgent)) {
        OT_ASSERT(nullptr != pAgent);
        recover_opening_number(*pAgent, context);
    }
    // else no error, since many nyms could get passed in here (in a loop)
}  // The function above me, calls the one below.

void OTParty::recover_opening_number(
    OTAgent& theAgent,
    otx::context::Server& context) const
{
    if (!(GetAuthorizingAgentName().compare(theAgent.GetName().Get()) == 0)) {
        LogError()(OT_PRETTY_CLASS())("Error: Agent name doesn't match: ")(
            GetAuthorizingAgentName())(" / ")(theAgent.GetName())(".")
            .Flush();
    } else if (GetOpeningTransNo() > 0) {
        theAgent.RecoverTransactionNumber(GetOpeningTransNo(), context);
    } else {
        LogConsole()(OT_PRETTY_CLASS())(
            "Nothing to harvest, it was already 0 for party: ")(GetPartyName())(
            ".")
            .Flush();
    }
}

void OTParty::HarvestAllTransactionNumbers(otx::context::Server& context)
{
    HarvestOpeningNumber(context);
    HarvestClosingNumbers(context);
}

// Calls OTAgent::RemoveIssuedNumber (above)
void OTParty::CloseoutOpeningNumber(
    const String& strNotaryID,
    const PasswordPrompt& reason)
{
    if (GetAuthorizingAgentName().size() <= 0) {
        LogError()(OT_PRETTY_CLASS())("Error: Authorizing agent name is blank.")
            .Flush();
        return;
    }

    OTAgent* pAgent = GetAgent(GetAuthorizingAgentName());

    if (nullptr == pAgent) {
        LogError()(OT_PRETTY_CLASS())(
            "Error: Unable to find Authorizing agent (")(
            GetAuthorizingAgentName())(") for party: ")(GetPartyName())(".")
            .Flush();
    } else if (GetOpeningTransNo() > 0) {
        pAgent->RemoveIssuedNumber(GetOpeningTransNo(), strNotaryID, reason);
    } else {
        LogConsole()(OT_PRETTY_CLASS())(
            "Nothing to closeout, it was already 0 for party: ")(
            GetPartyName())(".")
            .Flush();
    }
}

// Done
// This function ASSUMES that the internal Nym pointer (on the authorizing
// agent) is set,
// and also that the Nym pointer is set on the authorized agent for each asset
// account as well.
//
// The party is getting ready to CONFIRM the smartcontract, so he will have to
// provide
// the appropriate transaction #s to do so.  This is the function where he tries
// to reserve
// those. Client-side.
//
auto OTParty::ReserveTransNumsForConfirm(otx::context::Server& context) -> bool
{
    // RESERVE THE OPENING TRANSACTION NUMBER, LOCATED ON THE AUTHORIZING AGENT
    // FOR THIS PARTY.

    if (GetAuthorizingAgentName().size() <= 0) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failure: Authorizing "
            "agent's name is empty on this party: ")(GetPartyName())(".")
            .Flush();
        return false;
    }

    OTAgent* pMainAgent = GetAgent(GetAuthorizingAgentName());

    if (nullptr == pMainAgent) {
        LogConsole()(OT_PRETTY_CLASS())("Failure: Authorizing "
                                        "agent (")(GetPartyName())(
            ") not found on this party: ")(GetAuthorizingAgentName())(".")
            .Flush();
        return false;
    }

    if (!pMainAgent->ReserveOpeningTransNum(context)) {
        LogConsole()(OT_PRETTY_CLASS())("Failure: Authorizing "
                                        "agent (")(GetAuthorizingAgentName())(
            ") didn't have an opening transaction #, on party: ")(
            GetPartyName())(".")
            .Flush();
        return false;
    }
    // BELOW THIS POINT, the OPENING trans# has been RESERVED and
    // must be RETRIEVED in the event of failure, using this call:
    // HarvestAllTransactionNumbers(context);

    // RESERVE THE CLOSING TRANSACTION NUMBER for each asset account, LOCATED ON
    // ITS AUTHORIZED AGENT.
    // (Do this for each account on this party.)
    //
    for (auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pPartyAccount = it.second;
        OT_ASSERT(nullptr != pPartyAccount);

        if (!pPartyAccount->GetAgentName().Exists()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failure: Authorized "
                "agent name is blank for account: ")(pPartyAccount->GetName())(
                ".")
                .Flush();
            // We have to put them back before returning, since this function
            // has failed.
            HarvestAllTransactionNumbers(context);

            return false;
        }

        OTAgent* pAgent = GetAgent(pPartyAccount->GetAgentName().Get());

        if (nullptr == pAgent) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failure: Unable to "
                "locate Authorized agent for account: ")(
                pPartyAccount->GetName())(".")
                .Flush();
            // We have to put them back before returning, since this function
            // has failed.
            HarvestAllTransactionNumbers(context);

            return false;
        }
        // Below this point, pAgent is good.

        if (!pAgent->ReserveClosingTransNum(context, *pPartyAccount)) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failure: "
                "Authorizing agent (")(GetAuthorizingAgentName())(
                ") didn't have a closing transaction #, on party: ")(
                GetPartyName())(".")
                .Flush();
            // We have to put them back before returning, since this function
            // has failed.
            HarvestAllTransactionNumbers(context);

            return false;
        }
        // BELOW THIS POINT, the CLOSING TRANSACTION # has been reserved for
        // this account, and MUST BE RETRIEVED in the event of failure.
    }

    // BY THIS POINT, we have successfully reserved the Opening Transaction #
    // for the party (from its
    // authorizing agent) and we have also successfully reserved Closing
    // Transaction #s for EACH ASSET
    // ACCOUNT, from the authorized agent for each asset account.
    // Therefore we have reserved ALL the needed transaction #s, so let's return
    // true.

    return true;
}

void OTParty::Serialize(
    Tag& parent,
    bool bCalculatingID,
    bool bSpecifyInstrumentDefinitionID,
    bool bSpecifyParties) const
{
    TagPtr pTag(new Tag("party"));

    const auto numAgents = m_mapAgents.size();
    const auto numAccounts = m_mapPartyAccounts.size();

    pTag->add_attribute("name", GetPartyName());
    pTag->add_attribute(
        "ownerType", bCalculatingID ? "" : (m_bPartyIsNym ? "nym" : "entity"));
    pTag->add_attribute(
        "ownerID", (bCalculatingID && !bSpecifyParties) ? "" : m_str_owner_id);
    pTag->add_attribute(
        "openingTransNo",
        std::to_string(bCalculatingID ? 0 : m_lOpeningTransNo));
    pTag->add_attribute(
        "signedCopyProvided",
        formatBool((!bCalculatingID && m_strMySignedCopy->Exists())));
    // When an agent activates this contract, it's HIS opening trans#.
    pTag->add_attribute(
        "authorizingAgent", bCalculatingID ? "" : m_str_authorizing_agent);
    pTag->add_attribute(
        "numAgents", std::to_string(bCalculatingID ? 0 : numAgents));
    pTag->add_attribute("numAccounts", std::to_string(numAccounts));

    if (!bCalculatingID) {
        for (auto& it : m_mapAgents) {
            OTAgent* pAgent = it.second;
            OT_ASSERT(nullptr != pAgent);
            pAgent->Serialize(*pTag);
        }
    }

    for (auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT(nullptr != pAcct);
        pAcct->Serialize(*pTag, bCalculatingID, bSpecifyInstrumentDefinitionID);
    }

    if (!bCalculatingID && m_strMySignedCopy->Exists()) {
        auto ascTemp = Armored::Factory(m_strMySignedCopy);
        pTag->add_tag("mySignedCopy", ascTemp->Get());
    }

    parent.add_tag(pTag);
}

// Register the variables of a specific Bylaw into the Script interpreter,
// so we can execute a script.
//
void OTParty::RegisterAccountsForExecution(OTScript& theScript)
{
    for (auto& it : m_mapPartyAccounts) {
        const UnallocatedCString str_acct_name = it.first;
        OTPartyAccount* pAccount = it.second;
        OT_ASSERT((nullptr != pAccount) && (str_acct_name.size() > 0));

        pAccount->RegisterForExecution(theScript);
    }
}

// Done.
auto OTParty::Compare(const OTParty& rhs) const -> bool
{
    const UnallocatedCString str_party_name(rhs.GetPartyName());

    if (!(str_party_name.compare(GetPartyName()) == 0)) {
        LogConsole()(OT_PRETTY_CLASS())("Names don't match. ")(GetPartyName())(
            " / ")(str_party_name)(".")
            .Flush();
        return false;
    }

    // The party might first be added WITHOUT filling out the Nym/Agent info.
    // As long as the party's name is right, and the accounts are all there with
    // the
    // correct instrument definition IDs, then it should matter if LATER, when
    // the party
    // CONFIRMS
    // the agreement, he supplies himself as an entity or a Nym, or whether he
    // supplies this
    // agent or that agent.  That information is important and is stored, but is
    // not relevant
    // for a Compare().

    if ((GetOpeningTransNo() > 0) && (rhs.GetOpeningTransNo() > 0) &&
        (GetOpeningTransNo() != rhs.GetOpeningTransNo())) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Opening transaction numbers don't match "
            "for party ")(GetPartyName())(". (")(GetOpeningTransNo())(" / ")(
            rhs.GetOpeningTransNo())(").")
            .Flush();
        return false;
    }

    if ((GetPartyID().size() > 0) && (rhs.GetPartyID().size() > 0) &&
        !(GetPartyID().compare(rhs.GetPartyID()) == 0)) {
        LogConsole()(OT_PRETTY_CLASS())("Party IDs don't match for party ")(
            GetPartyName())(". (")(GetPartyID())(" / ")(rhs.GetPartyID())(").")
            .Flush();
        return false;
    }

    if ((GetAuthorizingAgentName().size() > 0) &&
        (rhs.GetAuthorizingAgentName().size() > 0) &&
        !(GetAuthorizingAgentName().compare(rhs.GetAuthorizingAgentName()) ==
          0)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Authorizing agent names don't match for "
            "party ")(GetPartyName())(". (")(GetAuthorizingAgentName())(" / ")(
            rhs.GetAuthorizingAgentName())(").")
            .Flush();
        return false;
    }

    // No need to compare agents... right?
    //
    //    mapOfAgents            m_mapAgents; // These are owned.

    if (GetAccountCount() != rhs.GetAccountCount()) {
        LogConsole()(OT_PRETTY_CLASS())("Mismatched number of accounts when "
                                        "comparing party ")(GetPartyName())(".")
            .Flush();
        return false;
    }

    for (const auto& it : m_mapPartyAccounts) {
        const UnallocatedCString str_acct_name = it.first;
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT(nullptr != pAcct);

        OTPartyAccount* p2 = rhs.GetAccount(str_acct_name);

        if (nullptr == p2) {
            LogConsole()(OT_PRETTY_CLASS())("Unable to find Account ")(
                str_acct_name)(" on rhs, when comparing party ")(
                GetPartyName())(".")
                .Flush();
            return false;
        }
        if (!pAcct->Compare(*p2)) {
            LogConsole()(OT_PRETTY_CLASS())("Accounts (")(
                str_acct_name)(") don't match when comparing party ")(
                GetPartyName())(".")
                .Flush();
            return false;
        }
    }

    return true;
}

// When confirming a party, a new version replaces the original. This is part of
// that process.
// *this is the old one, and theParty is the new one.
//
auto OTParty::CopyAcctsToConfirmingParty(OTParty& theParty) const -> bool
{
    theParty.CleanupAccounts();  // (We're going to copy our own accounts into
                                 // theParty.)

    for (const auto& it : m_mapPartyAccounts) {
        const UnallocatedCString str_acct_name = it.first;
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT(nullptr != pAcct);

        if (false == theParty.AddAccount(
                         pAcct->GetAgentName(),
                         pAcct->GetName(),
                         pAcct->GetAcctID(),
                         pAcct->GetInstrumentDefinitionID(),
                         pAcct->GetClosingTransNo())) {
            LogConsole()(OT_PRETTY_CLASS())("Unable to add Account ")(
                str_acct_name)(", when copying from *this party ")(
                GetPartyName())(".")
                .Flush();
            return false;
        }
    }

    return true;
}

OTParty::~OTParty()
{
    CleanupAgents();
    CleanupAccounts();

    if (nullptr != m_pstr_party_name) delete m_pstr_party_name;
    m_pstr_party_name = nullptr;

    m_pOwnerAgreement = nullptr;
}
}  // namespace opentxs
