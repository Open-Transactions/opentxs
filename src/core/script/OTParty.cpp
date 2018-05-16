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

#include "opentxs/stdafx.hpp"

#include "opentxs/core/script/OTParty.hpp"

#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/script/OTAgent.hpp"
#include "opentxs/core/script/OTPartyAccount.hpp"
#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>

namespace opentxs
{

// Checks opening number on party, and closing numbers on his accounts.
bool OTParty::HasTransactionNum(const std::int64_t& lInput) const
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
std::int32_t OTParty::GetAccountCount(std::string str_agent_name) const
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

// Party is always either an Owner Nym, or an Owner Entity formed by Contract.
//
// Either way, the agents are there to represent the interests of the parties.
//
// This is meant in the sense of "actually" since the agent is not just a
// trusted
// friend of the party, but is either the party himself (if party is a Nym), OR
// is
// a voting group or employee that belongs to the party. (If party is an
// entity.)
// Either way, the point is that in this context, the agent is ACTUALLY
// authorized
// by the party by virtue of its existence, versus being a "separate but
// authorized"
// party in the legal sense. No need exists to "grant" the authority since the
// authority is already INHERENT.
//
// A party may also have multiple agents.
//

OTParty::OTParty()
    : m_pstr_party_name(nullptr)
    , m_bPartyIsNym(false)
    , m_lOpeningTransNo(0)
    , m_pOwnerAgreement(nullptr)
{
}

OTParty::OTParty(
    const char* szName,
    bool bIsOwnerNym,
    const char* szOwnerID,
    const char* szAuthAgent,
    bool bCreateAgent)
    : m_pstr_party_name(nullptr)
    , m_bPartyIsNym(bIsOwnerNym)
    , m_str_owner_id(szOwnerID != nullptr ? szOwnerID : "")
    , m_str_authorizing_agent(szAuthAgent != nullptr ? szAuthAgent : "")
    , m_lOpeningTransNo(0)
    , m_pOwnerAgreement(nullptr)
{
    m_pstr_party_name = new std::string(szName != nullptr ? szName : "");

    if (bCreateAgent) {
        const String strName(m_str_authorizing_agent.c_str()), strNymID(""),
            strRoleID(""), strGroupName("");
        OTAgent* pAgent = new OTAgent(
            true /*bNymRepresentsSelf*/,
            true /*bIsAnIndividual*/,
            strName,
            strNymID,
            strRoleID,
            strGroupName);
        OT_ASSERT(nullptr != pAgent);

        if (!AddAgent(*pAgent)) {
            otErr << "OTParty::OTParty: *** Failed *** while adding default "
                     "agent in CONSTRUCTOR! 2\n";
            delete pAgent;
            pAgent = nullptr;
        }
    }
}

OTParty::OTParty(
    std::string str_PartyName,
    const Nym& theNym,  // Nym is BOTH owner AND agent, when using
                        // this constructor.
    const std::string str_agent_name,
    Account* pAccount,
    const std::string* pstr_account_name,
    std::int64_t lClosingTransNo)
    : m_pstr_party_name(new std::string(str_PartyName))
    , m_bPartyIsNym(true)
    , m_lOpeningTransNo(0)
    , m_pOwnerAgreement(nullptr)
{
    //  m_pstr_party_name = new std::string(str_PartyName);
    OT_ASSERT(nullptr != m_pstr_party_name);

    // theNym is owner, therefore save his ID information, and create the agent
    // for this Nym automatically (that's why it was passed in.)
    // This code won't compile until you do.  :-)

    String strNymID;
    theNym.GetIdentifier(strNymID);
    m_str_owner_id = strNymID.Get();

    OTAgent* pAgent = new OTAgent(
        str_agent_name, theNym);  // (The third arg, bRepresentsSelf,
                                  // defaults here to true.)
    OT_ASSERT(nullptr != pAgent);

    if (!AddAgent(*pAgent)) {
        otErr << "OTParty::OTParty: *** Failed *** while adding default agent "
                 "in CONSTRUCTOR!\n";
        delete pAgent;
        pAgent = nullptr;
    } else
        m_str_authorizing_agent = str_agent_name;

    // if pAccount is NOT nullptr, then an account was passed in, so
    // let's also create a default partyaccount for it.
    //
    if (nullptr != pAccount) {
        OT_ASSERT(
            nullptr != pstr_account_name);  // If passing an account, then you
                                            // MUST pass an account name also.

        bool bAdded = AddAccount(
            str_agent_name.c_str(),
            pstr_account_name->c_str(),
            *pAccount,
            lClosingTransNo);

        if (!bAdded)
            otErr << "OTParty::OTParty: *** Failed *** while adding default "
                     "account in CONSTRUCTOR!\n";
    }
}

bool OTParty::AddAgent(OTAgent& theAgent)
{
    const std::string str_agent_name = theAgent.GetName().Get();

    if (!OTScriptable::ValidateName(str_agent_name)) {
        otErr << "OTParty::AddAgent: Failed validating Agent name.";
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
            std::pair<std::string, OTAgent*>(str_agent_name, &theAgent));

        // Make sure it has a pointer back to me.
        theAgent.SetParty(*this);

        return true;
    } else
        otOut << "OTParty::AddAgent: Failed -- Agent was already there named "
              << str_agent_name << ".\n";

    return false;
}

bool OTParty::AddAccount(
    const String& strAgentName,
    const String& strName,
    const String& strAcctID,
    const String& strInstrumentDefinitionID,
    std::int64_t lClosingTransNo)
{
    OTPartyAccount* pPartyAccount = new OTPartyAccount(
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

bool OTParty::AddAccount(
    const String& strAgentName,
    const char* szAcctName,
    Account& theAccount,
    std::int64_t lClosingTransNo)
{
    OTPartyAccount* pPartyAccount = new OTPartyAccount(
        szAcctName, strAgentName, theAccount, lClosingTransNo);
    OT_ASSERT(nullptr != pPartyAccount);

    if (!AddAccount(*pPartyAccount)) {
        delete pPartyAccount;
        return false;
    }

    return true;
}

bool OTParty::RemoveAccount(const std::string str_Name)
{
    for (mapOfPartyAccounts::iterator it = m_mapPartyAccounts.begin();
         it != m_mapPartyAccounts.end();
         ++it) {
        OTPartyAccount* pAcct = it->second;
        OT_ASSERT(nullptr != pAcct);

        const std::string str_acct_name = pAcct->GetName().Get();

        if (0 == str_acct_name.compare(str_Name)) {
            m_mapPartyAccounts.erase(it);
            delete pAcct;
            pAcct = nullptr;
            return true;
        }
    }

    return false;
}

bool OTParty::AddAccount(OTPartyAccount& thePartyAcct)
{
    const std::string str_acct_name = thePartyAcct.GetName().Get();

    if (!OTScriptable::ValidateName(str_acct_name)) {
        otErr << "OTParty::AddAccount: Failed validating Account name.";
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
        m_mapPartyAccounts.insert(std::pair<std::string, OTPartyAccount*>(
            str_acct_name, &thePartyAcct));

        // Make sure it has a pointer back to me.
        thePartyAcct.SetParty(*this);

        return true;
    } else
        otOut << "OTParty::AddAccount: Failed -- Account was already on party "
                 "named "
              << str_acct_name << ".\n";

    return false;
}

std::int64_t OTParty::GetClosingTransNo(std::string str_for_acct_name) const
{
    auto it = m_mapPartyAccounts.find(str_for_acct_name);

    if (m_mapPartyAccounts.end() == it)  // If it wasn't already there...
    {
        otOut << "OTParty::GetClosingTransNo: Failed -- Account wasn't found: "
              << str_for_acct_name << ".\n";
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

OTParty::~OTParty()
{
    CleanupAgents();
    CleanupAccounts();

    if (nullptr != m_pstr_party_name) delete m_pstr_party_name;
    m_pstr_party_name = nullptr;

    m_pOwnerAgreement = nullptr;
}

void OTParty::ClearTemporaryPointers()
{
    for (auto& it : m_mapAgents) {
        OTAgent* pAgent = it.second;
        OT_ASSERT_MSG(
            nullptr != pAgent,
            "Unexpected nullptr agent pointer in party map.");

        pAgent->ClearTemporaryPointers();
    }

    for (auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT_MSG(
            nullptr != pAcct,
            "Unexpected nullptr partyaccount pointer in party map.");

        pAcct->ClearTemporaryPointers();
    }
}

// as used "IN THE SCRIPT."
//
std::string OTParty::GetPartyName(bool* pBoolSuccess) const
{
    std::string retVal("");

    // "sales_director", "marketer", etc
    if (nullptr == m_pstr_party_name) {
        if (nullptr != pBoolSuccess) *pBoolSuccess = false;

        return retVal;
    }

    if (nullptr != pBoolSuccess) *pBoolSuccess = true;

    retVal = *m_pstr_party_name;

    return retVal;
}

bool OTParty::SetPartyName(const std::string& str_party_name_input)
{
    if (!OTScriptable::ValidateName(str_party_name_input)) {
        otErr << "OTParty::SetPartyName: Failed: Invalid name was passed in.\n";
        return false;
    }

    if (nullptr == m_pstr_party_name)
        OT_ASSERT(nullptr != (m_pstr_party_name = new std::string));

    *m_pstr_party_name = str_party_name_input;

    return true;
}

// ACTUAL PARTY OWNER (Only ONE of these can be true...)
// Debating whether these two functions should be private. (Should it matter to
// outsider?)
//
bool OTParty::IsNym() const
{
    // If the party is a Nym. (The party is the actual owner/beneficiary.)
    return m_bPartyIsNym;
}

bool OTParty::IsEntity() const
{
    // If the party is an Entity. (Either way, the AGENT carries out all
    // wishes.)
    return !m_bPartyIsNym;
}

// ACTUAL PARTY OWNER
//
std::string OTParty::GetNymID(bool* pBoolSuccess) const
{
    if (IsNym() && (m_str_owner_id.size() > 0)) {
        if (nullptr != pBoolSuccess) *pBoolSuccess = true;

        return m_str_owner_id;
    }

    if (nullptr != pBoolSuccess) *pBoolSuccess = false;

    std::string retVal("");

    return retVal;  // empty ID on failure.
}

std::string OTParty::GetEntityID(bool* pBoolSuccess) const
{
    if (IsEntity() && (m_str_owner_id.size() > 0)) {
        if (nullptr != pBoolSuccess) *pBoolSuccess = true;

        return m_str_owner_id;
    }

    if (nullptr != pBoolSuccess) *pBoolSuccess = false;

    std::string retVal("");

    return retVal;
}

std::string OTParty::GetPartyID(bool* pBoolSuccess) const
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
bool OTParty::HasActiveAgent() const
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
OTAgent* OTParty::GetAgent(const std::string& str_agent_name) const
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
        otErr << __FUNCTION__ << ": Failed: str_agent_name is invalid...\n";

    return nullptr;
}

/// Get Agent pointer by Index. Returns nullptr on failure.
///
OTAgent* OTParty::GetAgentByIndex(std::int32_t nIndex) const
{
    if (!((nIndex >= 0) &&
          (nIndex < static_cast<std::int64_t>(m_mapAgents.size())))) {
        otErr << __FUNCTION__ << ": Index out of bounds: " << nIndex << "\n";
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
OTPartyAccount* OTParty::GetAccount(const std::string& str_acct_name) const
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
        otErr << "OTParty::GetAccount: Failed: str_acct_name is invalid.\n";

    return nullptr;
}

/// Get OTPartyAccount pointer by Index. Returns nullptr on failure.
///
OTPartyAccount* OTParty::GetAccountByIndex(std::int32_t nIndex)
{
    if (!((nIndex >= 0) &&
          (nIndex < static_cast<std::int64_t>(m_mapPartyAccounts.size())))) {
        otErr << __FUNCTION__ << ": Index out of bounds: " << nIndex << "\n";
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
OTPartyAccount* OTParty::GetAccountByAgent(const std::string& str_agent_name)
{
    if (OTScriptable::ValidateName(str_agent_name)) {
        for (auto& it : m_mapPartyAccounts) {
            OTPartyAccount* pAcct = it.second;
            OT_ASSERT(nullptr != pAcct);

            if (pAcct->GetAgentName().Compare(str_agent_name.c_str()))
                return pAcct;
        }
    } else
        otErr << __FUNCTION__ << ": Failed: str_agent_name is invalid.\n";

    return nullptr;
}

// Get PartyAccount pointer by Acct ID.
//
// Returns nullptr on failure.
OTPartyAccount* OTParty::GetAccountByID(const Identifier& theAcctID) const
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
bool OTParty::HasAccountByID(
    const Identifier& theAcctID,
    OTPartyAccount** ppPartyAccount) const
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
bool OTParty::HasAccount(Account& theAccount, OTPartyAccount** ppPartyAccount)
    const
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
bool OTParty::HasAgent(const Nym& theNym, OTAgent** ppAgent) const
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

bool OTParty::HasAgentByNymID(const Identifier& theNymID, OTAgent** ppAgent)
    const
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
bool OTParty::HasAuthorizingAgent(
    const Nym& theNym,
    OTAgent** ppAgent) const  // ppAgent lets you get the agent ptr if it was
                              // there.
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
            otErr << "OTParty::HasAuthorizingAgent: named agent wasn't found "
                     "on list.\n";
    }

    return false;
}

bool OTParty::HasAuthorizingAgentByNymID(
    const Identifier& theNymID,
    OTAgent** ppAgent) const  // ppAgent
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
            otErr << "OTParty::HasAuthorizingAgentByNymID: named agent wasn't "
                     "found on list.\n";
    }

    return false;
}

void OTParty::RetrieveNymPointers(mapOfConstNyms& map_Nyms_Already_Loaded)
{
    for (auto& it : m_mapAgents) {
        OTAgent* pAgent = it.second;
        OT_ASSERT(nullptr != pAgent);

        pAgent->RetrieveNymPointer(map_Nyms_Already_Loaded);
    }
}

// Load up the Nym that authorized the agreement for this party
// (the nym who supplied the opening trans# to sign it.)
//
// This function ASSUMES that you ALREADY called HasAuthorizingAgentNym(), for
// example
// to verify that the serverNym isn't the one you were looking for.
// This is a low-level function.
// CALLER IS RESPONSIBLE TO DELETE.

// ppAgent lets you get the agent ptr if it was there.
Nym* OTParty::LoadAuthorizingAgentNym(
    const Nym& theSignerNym,
    OTAgent** ppAgent)
{
    if (OTScriptable::ValidateName(m_str_authorizing_agent)) {
        auto it = m_mapAgents.find(m_str_authorizing_agent);

        if (m_mapAgents.end() != it)  // If we found something...
        {
            OTAgent* pAgent = it->second;
            OT_ASSERT(nullptr != pAgent);

            Nym* pNym = nullptr;

            if (!pAgent->IsAnIndividual())
                otErr << "OTParty::LoadAuthorizingAgentNym: This agent is not "
                         "an individual--there's no Nym to load.\n";
            else if (nullptr == (pNym = pAgent->LoadNym(theSignerNym)))
                otErr << "OTParty::LoadAuthorizingAgentNym: Failed loading "
                         "Nym.\n";
            else {
                if (nullptr !=
                    ppAgent)  // Pass the agent back, too, if it was requested.
                    *ppAgent = pAgent;

                return pNym;  // Success
            }
        } else  // found nothing.
            otErr << "OTParty::LoadAuthorizingAgentNym: named agent wasn't "
                     "found on list.\n";
    }

    return nullptr;
}

bool OTParty::VerifyOwnershipOfAccount(const Account& theAccount) const
{
    if (IsNym())  // For those cases where the party is actually just a
                  // solitary Nym (not an entity.)
    {
        bool bNymID = false;
        std::string str_nym_id =
            GetNymID(&bNymID);  // If the party is a Nym, this is the Nym's
                                // ID. Otherwise this is false.

        if (!bNymID || (str_nym_id.size() <= 0)) {
            otErr << " OTParty::VerifyOwnershipOfAccount: Although party is a "
                     "Nym, unable to retrieve NymID!\n";
            return false;
        }

        const auto thePartyNymID = Identifier::Factory(str_nym_id);

        return theAccount.VerifyOwnerByID(thePartyNymID);
    } else if (IsEntity())
        otErr << "OTParty::VerifyOwnershipOfAccount: Error: Entities have not "
                 "been implemented yet, "
                 "but somehow this party is an entity.\n";
    else
        otErr << "OTParty::VerifyOwnershipOfAccount: Error: Unknown party "
                 "type.\n";

    return false;
}

// This is only for SmartContracts, NOT all scriptables.
//
bool OTParty::DropFinalReceiptToInboxes(
    mapOfNyms* pNymMap,
    const String& strNotaryID,
    Nym& theServerNym,
    const std::int64_t& lNewTransactionNumber,
    const String& strOrigCronItem,
    String* pstrNote,
    String* pstrAttachment)
{
    bool bSuccess = true;  // Success is defined as "all inboxes were notified"
    const char* szFunc = "OTParty::DropFinalReceiptToInboxes";

    OTSmartContract* pSmartContract = nullptr;

    if (nullptr == m_pOwnerAgreement) {
        otErr << szFunc << ": Missing pointer to owner agreement.\n";
        return false;
    } else if (
        nullptr ==
        (pSmartContract = dynamic_cast<OTSmartContract*>(m_pOwnerAgreement))) {
        otErr << szFunc
              << ": Can only drop finalReceipts for smart contracts.\n";
        return false;
    }

    // By this point, we know pSmartContract is a good pointer.

    for (auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT_MSG(
            nullptr != pAcct,
            "Unexpected nullptr partyaccount pointer in party map.");

        if (false == pAcct->DropFinalReceiptToInbox(
                         pNymMap,  // contains any Nyms who might already be
                                   // loaded, mapped by ID.
                         strNotaryID,
                         theServerNym,
                         *pSmartContract,
                         lNewTransactionNumber,
                         strOrigCronItem,
                         pstrNote,
                         pstrAttachment)) {
            otErr << szFunc
                  << ": Failed dropping final Receipt to agent's Inbox.\n";
            bSuccess = false;  // Notice: no break. We still try to notify them
                               // all, even if one fails.
        }
    }

    return bSuccess;
}

// This is only for SmartContracts, NOT all scriptables.
//
bool OTParty::DropFinalReceiptToNymboxes(
    const std::int64_t& lNewTransactionNumber,
    const String& strOrigCronItem,
    String* pstrNote,
    String* pstrAttachment,
    Nym* pActualNym)
{
    bool bSuccess =
        false;  // Success is defined as "at least one agent was notified"

    OTSmartContract* pSmartContract = nullptr;

    if (nullptr == m_pOwnerAgreement) {
        otErr << "OTParty::DropFinalReceiptToNymboxes: Missing pointer to "
                 "owner agreement.\n";
        return false;
    } else if (
        nullptr ==
        (pSmartContract = dynamic_cast<OTSmartContract*>(m_pOwnerAgreement))) {
        otErr << "OTParty::DropFinalReceiptToNymboxes: Can only drop "
                 "finalReceipts for smart contracts.\n";
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
                         pstrNote,
                         pstrAttachment,
                         pActualNym))
            otErr << "OTParty::DropFinalReceiptToNymboxes: Failed dropping "
                     "final Receipt to agent's Nymbox.\n";
        else
            bSuccess = true;
    }

    return bSuccess;
}

bool OTParty::SendNoticeToParty(
    bool bSuccessMsg,
    Nym& theServerNym,
    const Identifier& theNotaryID,
    const std::int64_t& lNewTransactionNumber,
    const String& strReference,
    String* pstrNote,
    String* pstrAttachment,
    Nym* pActualNym)
{
    bool bSuccess =
        false;  // Success is defined as "at least one agent was notified"

    if (nullptr == m_pOwnerAgreement) {
        otErr << __FUNCTION__ << ": Missing pointer to owner agreement.\n";
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
                             bSuccessMsg,
                             theServerNym,
                             theNotaryID,
                             lNewTransactionNumber,
                             lOpeningTransNo,  // lInReferenceTo
                             strReference,
                             pstrNote,
                             pstrAttachment,
                             pActualNym))
                otErr << __FUNCTION__
                      << ": Failed dropping server notice to agent's Nymbox.\n";
            else
                bSuccess = true;
        }
    }
    return bSuccess;
}

bool OTParty::LoadAndVerifyAssetAccounts(
    Nym& theServerNym,
    const String& strNotaryID,
    mapOfAccounts& map_Accts_Already_Loaded,
    mapOfAccounts& map_NewlyLoaded)
{
    std::set<std::string> theAcctIDSet;  // Make sure all the acct IDs are
                                         // unique.

    for (auto& it_acct : m_mapPartyAccounts) {
        const std::string str_acct_name = it_acct.first;
        OTPartyAccount* pPartyAcct = it_acct.second;
        OT_ASSERT(pPartyAcct != nullptr);

        bool bHadToLoadtheAcctMyself = true;
        Account* pAccount = nullptr;

        const String& strAcctID = pPartyAcct->GetAcctID();

        if (!strAcctID.Exists()) {
            otOut << "OTParty::LoadAndVerifyAssetAccounts: Bad: Acct ID is "
                     "blank for account: "
                  << str_acct_name << ", on party: " << GetPartyName() << ".\n";
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
            otOut << "OTParty::LoadAndVerifyAssetAccounts: Failure: Found a "
                     "duplicate Acct ID ("
                  << strAcctID << "), on acct: " << str_acct_name << ".\n";
            return false;
        }

        auto it = map_Accts_Already_Loaded.find(
            strAcctID.Get());  // If it's there, it's mapped by Acct ID, so we
                               // can look it up.

        if (map_Accts_Already_Loaded.end() != it)  // Found it.
        {
            pAccount = it->second;
            OT_ASSERT(nullptr != pAccount);

            // Now we KNOW the Account is "already loaded" and we KNOW the
            // partyaccount has a POINTER to that Acct:
            //
            const bool bIsPartyAcct = pPartyAcct->IsAccount(*pAccount);
            OT_ASSERT_MSG(
                bIsPartyAcct,
                "OTParty::LoadAndVerifyAssetAccounts: Failed call: "
                "pPartyAcct->IsAccount(*pAccount); \n");  // assert because the
                                                          // Acct was already
                                                          // mapped by ID, so it
                                                          // should already have
                                                          // been validated.
            if (!bIsPartyAcct)
                otErr << "OTParty::LoadAndVerifyAssetAccounts: Failed call: "
                         "pPartyAcct->IsAccount(*pAccount); \n";

            bHadToLoadtheAcctMyself = false;  // Whew. The Acct was already
                                              // loaded. Found it. (And the ptr
                                              // is now set.)
        }

        // Looks like the Acct wasn't already loaded....
        // Let's load it up...
        //
        if (bHadToLoadtheAcctMyself == true) {
            if (nullptr == (pAccount = pPartyAcct->LoadAccount(
                                theServerNym, strNotaryID)))  // This calls
            // VerifyAccount(),
            // AND it sets
            // pPartyAcct's
            // internal ptr.
            {
                otOut << "OTParty::LoadAndVerifyAssetAccounts: Failed loading "
                         "Account with name: "
                      << str_acct_name << " and ID: " << strAcctID << "\n";
                return false;
            }
            // Successfully loaded the Acct! We add to this map so it gets
            // cleaned-up properly later.
            map_NewlyLoaded.insert(
                std::pair<std::string, Account*>(strAcctID.Get(), pAccount));
        }
    }

    return true;
}

// After calling this, map_NewlyLoaded will contain pointers to Nyms that MUST
// BE CLEANED UP.
// This function will not bother loading any Nyms which appear on
// map_Nyms_Already_Loaded.
//
bool OTParty::LoadAndVerifyAgentNyms(
    Nym& theServerNym,
    mapOfConstNyms& map_Nyms_Already_Loaded,
    mapOfConstNyms& map_NewlyLoaded)
{
    const bool bIsNym = IsNym();

    if (!bIsNym)  // Owner MUST be a Nym (until I code Entities.)
    {
        otErr << "OTParty::LoadAndVerifyAgents: Entities and roles have not "
                 "been coded yet. Party owner MUST be a Nym. \n";
        return false;
    }
    if (GetOpeningTransNo() <= 0)  // Opening Trans Number MUST be set for the
                                   // party! VerifyPartyAuthorization() only
                                   // verifies it if it's set. Therefore
    {  // if we are verifying the agent Nyms based on the assumption that the
        // authorizing Nym is valid, then we want to make sure
        otErr << "OTParty::LoadAndVerifyAgents: This party doesn't have a "
                 "valid opening transaction number. Sorry. \n";  // the Opening
                                                                 // Num is being
                                                                 // checked for
                                                                 // that Nym.
                                                                 // (And if it's
                                                                 // above 0,
                                                                 // then it IS
                                                                 // being
                                                                 // checked.)
        return false;
    }

    bool bGotPartyNymID = false;
    const std::string str_owner_id = GetNymID(
        &bGotPartyNymID);  // str_owner_id  is the NymID of the party OWNER.
    OT_ASSERT(bGotPartyNymID);

    const String strServerNymID(theServerNym);

    for (auto& it_agent : m_mapAgents) {
        OTAgent* pAgent = it_agent.second;
        OT_ASSERT_MSG(
            pAgent != nullptr,
            "Unexpected nullptr agent pointer in party map.");

        if (!pAgent->IsAnIndividual() || !pAgent->DoesRepresentHimself()) {
            otErr << "OTParty::LoadAndVerifyAgents: Entities and roles have "
                     "not been coded yet. "
                     "Agent needs to be an individual who represents himself, "
                     "and Party owner needs to be the same Nym.\n";
            return false;
        }

        auto theNymID = Identifier::Factory();
        bool bGotAgentNymID = pAgent->GetNymID(theNymID);
        const String strNymID(theNymID);
        const std::string str_agent_id =
            bGotAgentNymID ? strNymID.Get()
                           : "";  // str_agent_id is the NymID of the AGENT.
        OT_ASSERT(bGotAgentNymID);

        // COMPARE THE IDS...... Since the Nym for this agent is representing
        // himself (he is also owner)
        // therefore they should have the same NymID.

        if (!(str_agent_id.compare(str_owner_id) ==
              0))  // If they don't match. (Until I code entities, a party can
                   // only be a Nym representing himself as an agent.)
        {
            otErr << "OTParty::LoadAndVerifyAgents: Nym supposedly represents "
                     "himself (owner AND agent) yet "
                     "they have different Nym IDs:  "
                  << str_owner_id << " / " << str_agent_id << ".\n";
            return false;
        }

        // Server Nym is not allowed as a party (at this time :-)
        if (str_agent_id.compare(strServerNymID.Get()) ==
            0)  // If they DO match.
        {
            otErr << "OTParty::LoadAndVerifyAgents: Server Nym is not allowed "
                     "to serve as an agent for smart contracts. Sorry.\n";
            return false;
        }

        // BY THIS POINT we know that the Party is a Nym, the Agent is an
        // individual representing himself, and
        // we know that they have the SAME NYM ID.
        //
        // Next step: See if the Nym is already loaded and if not, load him up.

        bool bHadToLoadtheNymMyself = true;
        const Nym* pNym = nullptr;

        auto it =
            map_Nyms_Already_Loaded.find(str_agent_id);  // If it's there, it's
                                                         // mapped by Nym ID, so
                                                         // we can look it up.

        if (map_Nyms_Already_Loaded.end() != it)  // Found it.
        {
            pNym = it->second;
            OT_ASSERT(nullptr != pNym);

            // Now we KNOW the Nym is "already loaded" and we KNOW the agent has
            // a POINTER to that Nym:
            //
            OT_ASSERT(pAgent->IsValidSigner(*pNym));  // assert because the Nym
                                                      // was already mapped by
            // ID, so it should already
            // have been validated.

            bHadToLoadtheNymMyself =
                false;  // Whew. He was already loaded. Found him.
        }

        // Looks like the Nym wasn't already loaded....
        // Let's load him up
        //
        if (bHadToLoadtheNymMyself) {
            if (nullptr == (pNym = pAgent->LoadNym(theServerNym))) {
                otErr << "OTParty::LoadAndVerifyAgents: Failed loading Nym "
                         "with ID: "
                      << str_agent_id << "\n";
                return false;
            }
            // Successfully loaded the Nym! We add to this map so it gets
            // cleaned-up properly later.
            map_NewlyLoaded.insert(std::pair<std::string, const Nym*>(
                str_agent_id, pNym));  // I use str_agent_id here because it
                                       // contains the right NymID.
        }

        // BY THIS POINT, we know the Nym is available for use, whether I had to
        // load it myself first or not.
        // We also know that if I DID have to load it myself, the pointer was
        // saved in map_NewlyLoaded for cleanup later.
        //
        // Until entities are coded, by this point we also KNOW that
        // the agent's NymID and the Party (owner)'s NymID are identical.
        //
        // Before this function was even called, we knew that
        // OTScriptable::VerifyPartyAuthorization() was already called
        // on all the parties, and we know that every party's signed copy was
        // verified against the signature of its authorizing
        // agent, and that the Opening trans# for that party is currently signed
        // out to THAT AGENT.
        //
        // If the NymIDs are identical between agent and owner, and the owner's
        // authorizing agent (that same nym) has SIGNED
        // its party's copy, and the Opening Trans# is signed out to that agent,
        // then we have basically verified that agent.
        // Right?
        //
        // WHAT if one of the Nyms loaded by this agent was NOT the same Nym as
        // the owner? In that case, it would have to be
        // a Nym working for an entity in a role, and I haven't coded entities
        // yet, so I just disallow that case entirely
        //
        // By this point, the call to LoadNym also did a LoadSignedNymFile() and
        // a call to VerifyPseudonym().
        //
        // FINALLY, the calls to pAgent->IsValidSigner( *pNym ) or
        // pAgent->LoadNym(theServerNym) (whichever occurred -- one or the
        // other)
        // have now insured by this point that pAgent continues to have an
        // INTERNAL POINTER to pNym...
    }

    return true;
}

// This is only meant to be used in OTSmartContract::VerifySmartContract() RIGHT
// AFTER the call
// to VerifyPartyAuthorization(). It ASSUMES the nyms and asset accounts are all
// loaded up, with
// internal pointers to them available.
//
bool OTParty::VerifyAccountsWithTheirAgents(
    const String& strNotaryID,
    bool bBurnTransNo)
{
    OT_ASSERT(nullptr != m_pOwnerAgreement);

    bool bAllSuccessful = true;

    // By this time this function is called, ALL the Nyms and Asset Accounts
    // should ALREADY
    // be loaded up in memory!
    //
    for (auto& it : m_mapPartyAccounts) {
        const std::string str_acct_name = it.first;
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT_MSG(
            nullptr != pAcct,
            "Unexpected nullptr partyaccount pointer in party map.");

        const bool bVerified = m_pOwnerAgreement->VerifyPartyAcctAuthorization(
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
            otOut << "OTParty::" << __FUNCTION__
                  << ": Ownership, agency, or potentially "
                     "closing transaction # failed to verify on account: "
                  << str_acct_name << " \n";
        }
    }

    return bAllSuccessful;
}

// Done
// The party will use its authorizing agent.
//
bool OTParty::SignContract(Contract& theInput) const
{
    if (GetAuthorizingAgentName().size() <= 0) {
        otErr << "OTParty::" << __FUNCTION__
              << ": Error: Authorizing agent name is blank.\n";
        return false;
    }

    OTAgent* pAgent = GetAgent(GetAuthorizingAgentName());

    if (nullptr == pAgent) {
        otErr << "OTParty::" << __FUNCTION__
              << ": Error: Unable to find Authorizing agent ("
              << GetAuthorizingAgentName() << ") for party: " << GetPartyName()
              << ".\n";
        return false;
    }

    return pAgent->SignContract(theInput);
}

// for whichever partyaccounts have agents that happen to be loaded, this will
// harvest the closing trans#s.
// Calls OTAgent::HarvestTransactionNumber
void OTParty::HarvestClosingNumbers(const String& strNotaryID)
{
    for (auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pAcct = it.second;

        OT_ASSERT_MSG(
            nullptr != pAcct,
            "OTParty::HarvestClosingNumbers: "
            "Unexpected nullptr partyaccount pointer in "
            "party map.");

        if (pAcct->GetClosingTransNo() <= 0) {
            continue;
        }

        const std::string str_agent_name(pAcct->GetAgentName().Get());

        if (str_agent_name.size() <= 0) {
            otErr << __FUNCTION__ << ": Missing agent name on party account: "
                  << pAcct->GetName() << " \n";

            continue;
        }

        OTAgent* pAgent = GetAgent(str_agent_name);

        if (nullptr == pAgent) {
            otErr << __FUNCTION__ << ": Couldn't find agent (" << str_agent_name
                  << ") for asset account: " << pAcct->GetName() << "\n";
        } else {
            pAgent->RecoverTransactionNumber(
                pAcct->GetClosingTransNo(), strNotaryID);
        }
    }
}

// Done
// Calls OTAgent::HarvestTransactionNumber
//
void OTParty::recover_closing_numbers(OTAgent& theAgent, ServerContext& context)
    const
{
    for (auto& it : m_mapPartyAccounts) {
        OTPartyAccount* pAcct = it.second;

        OT_ASSERT_MSG(
            nullptr != pAcct,
            "OTParty::HarvestClosingNumbers: "
            "Unexpected nullptr partyaccount pointer in "
            "partyaccount map.");

        if (pAcct->GetClosingTransNo() <= 0) {
            continue;
        }

        const std::string str_agent_name(pAcct->GetAgentName().Get());

        if (str_agent_name.size() <= 0) {
            continue;
        }

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
void OTParty::HarvestClosingNumbers(ServerContext& context)
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
void OTParty::HarvestOpeningNumber(ServerContext& context)
{
    OTAgent* pAgent = nullptr;

    if (HasAuthorizingAgent(*context.Nym(), &pAgent)) {
        OT_ASSERT(nullptr != pAgent);
        recover_opening_number(*pAgent, context);
    }
    // else no error, since many nyms could get passed in here (in a loop)
}  // The function above me, calls the one below.

void OTParty::recover_opening_number(OTAgent& theAgent, ServerContext& context)
    const
{
    if (!(GetAuthorizingAgentName().compare(theAgent.GetName().Get()) == 0)) {
        otErr << "OTParty::" << __FUNCTION__
              << ": Error: Agent name doesn't match:  "
              << GetAuthorizingAgentName() << " / " << theAgent.GetName()
              << "  \n";
    } else if (GetOpeningTransNo() > 0) {
        theAgent.RecoverTransactionNumber(GetOpeningTransNo(), context);
    } else {
        otOut << "OTParty::" << __FUNCTION__
              << ": Nothing to harvest, it was already 0 for party: "
              << GetPartyName() << "\n";
    }
}

void OTParty::HarvestAllTransactionNumbers(ServerContext& context)
{
    HarvestOpeningNumber(context);
    HarvestClosingNumbers(context);
}

// Calls OTAgent::RemoveIssuedNumber (above)
void OTParty::CloseoutOpeningNumber(const String& strNotaryID)
{
    if (GetAuthorizingAgentName().size() <= 0) {
        otErr << "OTParty::" << __FUNCTION__
              << ": Error: Authorizing agent name is blank.\n";
        return;
    }

    OTAgent* pAgent = GetAgent(GetAuthorizingAgentName());

    if (nullptr == pAgent) {
        otErr << "OTParty::" << __FUNCTION__
              << ": Error: Unable to find Authorizing agent ("
              << GetAuthorizingAgentName() << ") for party: " << GetPartyName()
              << ".\n";
    } else if (GetOpeningTransNo() > 0) {
        pAgent->RemoveIssuedNumber(GetOpeningTransNo(), strNotaryID);
    } else {
        otOut << "OTParty::" << __FUNCTION__
              << ": Nothing to closeout, it was already 0 for party: "
              << GetPartyName() << "\n";
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
bool OTParty::ReserveTransNumsForConfirm(ServerContext& context)
{
    // RESERVE THE OPENING TRANSACTION NUMBER, LOCATED ON THE AUTHORIZING AGENT
    // FOR THIS PARTY.

    if (GetAuthorizingAgentName().size() <= 0) {
        otOut << "OTParty::ReserveTransNumsForConfirm: Failure: Authorizing "
                 "agent's name is empty on this party: "
              << GetPartyName() << " \n";
        return false;
    }

    OTAgent* pMainAgent = GetAgent(GetAuthorizingAgentName());

    if (nullptr == pMainAgent) {
        otOut << "OTParty::ReserveTransNumsForConfirm: Failure: Authorizing "
                 "agent ("
              << GetPartyName()
              << ") not found on this party: " << GetAuthorizingAgentName()
              << " \n";
        return false;
    }

    if (!pMainAgent->ReserveOpeningTransNum(context)) {
        otOut << "OTParty::ReserveTransNumsForConfirm: Failure: Authorizing "
                 "agent ("
              << GetAuthorizingAgentName()
              << ") didn't have an opening transaction #, on party: "
              << GetPartyName() << " \n";
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
            otOut << "OTParty::ReserveTransNumsForConfirm: Failure: Authorized "
                     "agent name is blank for account: "
                  << pPartyAccount->GetName() << " \n";
            // We have to put them back before returning, since this function
            // has failed.
            HarvestAllTransactionNumbers(context);

            return false;
        }

        OTAgent* pAgent = GetAgent(pPartyAccount->GetAgentName().Get());

        if (nullptr == pAgent) {
            otOut << "OTParty::ReserveTransNumsForConfirm: Failure: Unable to "
                     "locate Authorized agent for account: "
                  << pPartyAccount->GetName() << " \n";
            // We have to put them back before returning, since this function
            // has failed.
            HarvestAllTransactionNumbers(context);

            return false;
        }
        // Below this point, pAgent is good.

        if (!pAgent->ReserveClosingTransNum(context, *pPartyAccount)) {
            otOut << "OTParty::ReserveTransNumsForConfirm: Failure: "
                     "Authorizing agent ("
                  << GetAuthorizingAgentName()
                  << ") didn't have a closing transaction #, on party: "
                  << GetPartyName() << " \n";
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

    std::uint32_t numAgents = m_mapAgents.size();
    std::uint32_t numAccounts = m_mapPartyAccounts.size();

    pTag->add_attribute("name", GetPartyName());
    pTag->add_attribute(
        "ownerType", bCalculatingID ? "" : (m_bPartyIsNym ? "nym" : "entity"));
    pTag->add_attribute(
        "ownerID", (bCalculatingID && !bSpecifyParties) ? "" : m_str_owner_id);
    pTag->add_attribute(
        "openingTransNo", formatLong(bCalculatingID ? 0 : m_lOpeningTransNo));
    pTag->add_attribute(
        "signedCopyProvided",
        formatBool((!bCalculatingID && m_strMySignedCopy.Exists())));
    // When an agent activates this contract, it's HIS opening trans#.
    pTag->add_attribute(
        "authorizingAgent", bCalculatingID ? "" : m_str_authorizing_agent);
    pTag->add_attribute(
        "numAgents", formatUint(bCalculatingID ? 0 : numAgents));
    pTag->add_attribute("numAccounts", formatUint(numAccounts));

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

    if (!bCalculatingID && m_strMySignedCopy.Exists()) {
        OTASCIIArmor ascTemp(m_strMySignedCopy);
        pTag->add_tag("mySignedCopy", ascTemp.Get());
    }

    parent.add_tag(pTag);
}

// Register the variables of a specific Bylaw into the Script interpreter,
// so we can execute a script.
//
void OTParty::RegisterAccountsForExecution(OTScript& theScript)
{
    for (auto& it : m_mapPartyAccounts) {
        const std::string str_acct_name = it.first;
        OTPartyAccount* pAccount = it.second;
        OT_ASSERT((nullptr != pAccount) && (str_acct_name.size() > 0));

        pAccount->RegisterForExecution(theScript);
    }
}

// Done.
bool OTParty::Compare(const OTParty& rhs) const
{
    const std::string str_party_name(rhs.GetPartyName());

    if (!(str_party_name.compare(GetPartyName()) == 0)) {
        otOut << "OTParty::Compare: Names don't match.  " << GetPartyName()
              << "  /  " << str_party_name << " \n";
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
        otOut << "OTParty::Compare: Opening transaction numbers don't match "
                 "for party "
              << GetPartyName() << ". ( " << GetOpeningTransNo() << "  /  "
              << rhs.GetOpeningTransNo() << " ) \n";
        return false;
    }

    if ((GetPartyID().size() > 0) && (rhs.GetPartyID().size() > 0) &&
        !(GetPartyID().compare(rhs.GetPartyID()) == 0)) {
        otOut << "OTParty::Compare: Party IDs don't match for party "
              << GetPartyName() << ". ( " << GetPartyID() << "  /  "
              << rhs.GetPartyID() << " ) \n";
        return false;
    }

    if ((GetAuthorizingAgentName().size() > 0) &&
        (rhs.GetAuthorizingAgentName().size() > 0) &&
        !(GetAuthorizingAgentName().compare(rhs.GetAuthorizingAgentName()) ==
          0)) {
        otOut << "OTParty::Compare: Authorizing agent names don't match for "
                 "party "
              << GetPartyName() << ". ( " << GetAuthorizingAgentName()
              << "  /  " << rhs.GetAuthorizingAgentName() << " ) \n";
        return false;
    }

    // No need to compare agents... right?
    //
    //    mapOfAgents            m_mapAgents; // These are owned.

    if (GetAccountCount() != rhs.GetAccountCount()) {
        otOut << "OTParty::Compare: Mismatched number of accounts when "
                 "comparing party "
              << GetPartyName() << ". \n";
        return false;
    }

    for (const auto& it : m_mapPartyAccounts) {
        const std::string str_acct_name = it.first;
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT(nullptr != pAcct);

        OTPartyAccount* p2 = rhs.GetAccount(str_acct_name);

        if (nullptr == p2) {
            otOut << "OTParty::Compare: Unable to find Account "
                  << str_acct_name << " on rhs, when comparing party "
                  << GetPartyName() << ". \n";
            return false;
        }
        if (!pAcct->Compare(*p2)) {
            otOut << "OTParty::Compare: Accounts (" << str_acct_name
                  << ") don't match when comparing party " << GetPartyName()
                  << ". \n";
            return false;
        }
    }

    return true;
}

// When confirming a party, a new version replaces the original. This is part of
// that process.
// *this is the old one, and theParty is the new one.
//
bool OTParty::CopyAcctsToConfirmingParty(OTParty& theParty) const
{
    theParty.CleanupAccounts();  // (We're going to copy our own accounts into
                                 // theParty.)

    for (const auto& it : m_mapPartyAccounts) {
        const std::string str_acct_name = it.first;
        OTPartyAccount* pAcct = it.second;
        OT_ASSERT(nullptr != pAcct);

        if (false == theParty.AddAccount(
                         pAcct->GetAgentName(),
                         pAcct->GetName(),
                         pAcct->GetAcctID(),
                         pAcct->GetInstrumentDefinitionID(),
                         pAcct->GetClosingTransNo())) {
            otOut
                << "OTParty::CopyAcctsToConfirmingParty: Unable to add Account "
                << str_acct_name << ", when copying from *this party "
                << GetPartyName() << ". \n";
            return false;
        }
    }

    return true;
}

}  // namespace opentxs
