// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Contract.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
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

class Identifier;
class OTAgent;
class OTBylaw;
class OTClause;
class OTParty;
class OTPartyAccount;
class OTScript;
class OTVariable;
class PasswordPrompt;
class Tag;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
using mapOfBylaws = UnallocatedMap<UnallocatedCString, OTBylaw*>;
using mapOfClauses = UnallocatedMap<UnallocatedCString, OTClause*>;
using mapOfParties = UnallocatedMap<UnallocatedCString, OTParty*>;
using mapOfVariables = UnallocatedMap<UnallocatedCString, OTVariable*>;

auto vectorToString(const UnallocatedVector<std::int64_t>& v)
    -> UnallocatedCString;
auto stringToVector(const UnallocatedCString& s)
    -> UnallocatedVector<std::int64_t>;

class OTScriptable : public Contract
{
public:
    auto openingNumsInOrderOfSigning() const
        -> const UnallocatedVector<std::int64_t>&
    {
        return openingNumsInOrderOfSigning_;
    }

    void specifyParties(bool bNewState);
    void specifyAssetTypes(bool bNewState);
    auto arePartiesSpecified() const -> bool;
    auto areAssetTypesSpecified() const -> bool;

    virtual void SetDisplayLabel(const UnallocatedCString* pstrLabel = nullptr);
    auto GetPartyCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(m_mapParties.size());
    }
    auto GetBylawCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(m_mapBylaws.size());
    }
    virtual auto AddParty(OTParty& theParty) -> bool;  // Takes
                                                       // ownership.
    virtual auto AddBylaw(OTBylaw& theBylaw) -> bool;  // takes
                                                       // ownership.
    virtual auto ConfirmParty(
        OTParty& theParty,  // Takes ownership.
        otx::context::Server& context,
        const PasswordPrompt& reason) -> bool;
    auto RemoveParty(UnallocatedCString str_Name) -> bool;
    auto RemoveBylaw(UnallocatedCString str_Name) -> bool;
    auto GetParty(UnallocatedCString str_party_name) const -> OTParty*;
    auto GetBylaw(UnallocatedCString str_bylaw_name) const -> OTBylaw*;
    auto GetClause(UnallocatedCString str_clause_name) const -> OTClause*;
    auto GetPartyByIndex(std::int32_t nIndex) const -> OTParty*;
    auto GetBylawByIndex(std::int32_t nIndex) const -> OTBylaw*;
    auto FindPartyBasedOnNymAsAgent(
        const identity::Nym& theNym,
        OTAgent** ppAgent = nullptr) const -> OTParty*;
    auto FindPartyBasedOnNymAsAuthAgent(
        const identity::Nym& theNym,
        OTAgent** ppAgent = nullptr) const -> OTParty*;
    auto FindPartyBasedOnAccount(
        const Account& theAccount,
        OTPartyAccount** ppPartyAccount = nullptr) const -> OTParty*;
    auto FindPartyBasedOnNymIDAsAgent(
        const identifier::Nym& theNymID,
        OTAgent** ppAgent = nullptr) const -> OTParty*;
    auto FindPartyBasedOnNymIDAsAuthAgent(
        const identifier::Nym& theNymID,
        OTAgent** ppAgent = nullptr) const -> OTParty*;
    auto FindPartyBasedOnAccountID(
        const Identifier& theAcctID,
        OTPartyAccount** ppPartyAccount = nullptr) const -> OTParty*;
    auto GetAgent(UnallocatedCString str_agent_name) const -> OTAgent*;
    auto GetPartyAccount(UnallocatedCString str_acct_name) const
        -> OTPartyAccount*;
    auto GetPartyAccountByID(const Identifier& theAcctID) const
        -> OTPartyAccount*;
    // This function returns the count of how many trans#s a Nym needs in order
    // to confirm as
    // a specific agent for a contract. (An opening number is needed for every
    // party of which
    // agent is the authorizing agent, plus a closing number for every acct of
    // which agent is the
    // authorized agent.)
    //
    auto GetCountTransNumsNeededForAgent(
        UnallocatedCString str_agent_name) const -> std::int32_t;
    // Verifies that Nym is actually an agent for this agreement.
    // (Verifies that Nym has signed this agreement, if it's a trade or a
    // payment plan, OR
    // that the authorizing agent for Nym's party has done so,
    // and in that case, that theNym is listed as an agent for that party.)
    // Basically this means that the agreement's owner approves of theNym.
    //
    virtual auto VerifyNymAsAgent(
        const identity::Nym& theNym,
        const identity::Nym& theSignerNym) const -> bool;

    // NEED TO CALL BOTH METHODS. (above / below)

    // Verifies that theNym is actually an agent for theAccount, according to
    // the PARTY.
    // Also verifies that theNym is an agent for theAccount, according to the
    // ACCOUNT.
    //
    virtual auto VerifyNymAsAgentForAccount(
        const identity::Nym& theNym,
        const Account& theAccount) const -> bool;
    auto VerifyPartyAuthorization(
        OTParty& theParty,  // The party that supposedly is authorized for this
                            // supposedly executed agreement.
        const identity::Nym& theSignerNym,  // For verifying signature on the
                                            // authorizing Nym, when loading it
        const String& strNotaryID,  // For verifying issued num, need the
                                    // notaryID the # goes with.
        const PasswordPrompt& reason,
        bool bBurnTransNo = false)
        -> bool;  // In Server::VerifySmartContract(), it
                  // not only wants to
    // verify the # is properly issued, but it additionally
    // wants to see that it hasn't been USED yet -- AND it wants
    // to burn it, so it can't be used again!  This bool allows
    // you to tell the function whether or not to do that.

    auto VerifyPartyAcctAuthorization(
        const PasswordPrompt& reason,
        OTPartyAccount& thePartyAcct,  // The party is assumed to have been
                                       // verified already via
                                       // VerifyPartyAuthorization()
        const String& strNotaryID,     // For verifying issued num, need the
                                       // notaryID the # goes with.
        bool bBurnTransNo = false)
        -> bool;  // In Server::VerifySmartContract(), it
                  // not only wants to
    // verify the closing # is properly issued, but it
    // additionally wants to see that it hasn't been USED yet --
    // AND it wants to burn it, so it can't be used again!  This
    // bool allows you to tell the function whether or not to do
    // that.
    auto VerifyThisAgainstAllPartiesSignedCopies() -> bool;
    auto AllPartiesHaveSupposedlyConfirmed() -> bool;

    void ClearTemporaryPointers();
    // Look up all clauses matching a specific hook.
    // (Across all Bylaws) Automatically removes any duplicates.
    // That is, you can have multiple clauses registered to the same
    // hook name, but you can NOT have the same clause name repeated
    // multiple times in theResults. Each clause can only trigger once.
    //
    auto GetHooks(UnallocatedCString str_HookName, mapOfClauses& theResults)
        -> bool;
    auto GetCallback(UnallocatedCString str_CallbackName)
        -> OTClause*;  // See if a
                       // scripted
                       // clause was
                       // provided for
                       // any given
                       // callback name.
    auto GetVariable(UnallocatedCString str_VarName)
        -> OTVariable*;            // See if a variable
                                   // exists for a
                                   // given variable
                                   // name.
    auto IsDirty() const -> bool;  // So you can tell if any of the persistent
                                   // or important variables have CHANGED since
                                   // it was last set clean.
    auto IsDirtyImportant() const -> bool;  // So you can tell if ONLY the
                                            // IMPORTANT variables have CHANGED
                                            // since it was last set clean.
    void SetAsClean();  // Sets the variables as clean, so you can check later
                        // and see if any have been changed (if it's DIRTY
                        // again.)
    auto SendNoticeToAllParties(
        bool bSuccessMsg,
        const identity::Nym& theServerNym,
        const identifier::Notary& theNotaryID,
        const std::int64_t& lNewTransactionNumber,
        // const std::int64_t& lInReferenceTo, //
        // each party has its own opening trans #.
        const String& strReference,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory(),
        identity::Nym* pActualNym = nullptr) const -> bool;
    // This is an OT Native call party_may_execute_clause
    // It returns true/false whether party is allowed to execute clause.
    // The default return value, for a legitimate party, is true.
    // If you want to override that behavior, add a script callback named
    // callback_party_may_execute_clause to your OTScriptable.
    // CanExecuteClause will call ExecuteCallback() if that script exists, so
    // the script can reply true/false.
    //
    auto CanExecuteClause(
        UnallocatedCString str_party_name,
        UnallocatedCString str_clause_name) -> bool;  // This calls (if
                                                      // available) the
    // scripted clause:
    // bool party_may_execute_clause(party_name,
    // clause_name)

    //
    // Also: callback_party_may_execute_clause should expect two parameters:
    // param_party_name and param_clause_name, both strings.
    // Also: callback_party_may_execute_clause should return a bool.

    auto ExecuteCallback(
        OTClause& theCallbackClause,
        mapOfVariables& theParameters,
        OTVariable& varReturnVal) -> bool;

    virtual void RegisterOTNativeCallsWithScript(OTScript& theScript);
    virtual auto Compare(OTScriptable& rhs) const -> bool;

    // Make sure a string contains only alpha, numeric, or '_'
    // And make sure it's not blank. This is for script variable names, clause
    // names, party names, etc.
    //
    static auto ValidateName(const UnallocatedCString& str_name) -> bool;
    static auto ValidateBylawName(const UnallocatedCString& str_name) -> bool;
    static auto ValidatePartyName(const UnallocatedCString& str_name) -> bool;
    static auto ValidateAgentName(const UnallocatedCString& str_name) -> bool;
    static auto ValidateAccountName(const UnallocatedCString& str_name) -> bool;
    static auto ValidateVariableName(const UnallocatedCString& str_name)
        -> bool;
    static auto ValidateClauseName(const UnallocatedCString& str_name) -> bool;
    static auto ValidateHookName(const UnallocatedCString& str_name) -> bool;
    static auto ValidateCallbackName(const UnallocatedCString& str_name)
        -> bool;

    // For use from inside server-side scripts.
    //
    static auto GetTime()
        -> UnallocatedCString;  // Returns a string, containing seconds as
                                // std::int32_t. (Time in seconds.)

    void UpdateContentsToTag(Tag& parent, bool bCalculatingID) const;
    void CalculateContractID(Identifier& newID) const override;

    void Release() override;
    void Release_Scriptable();
    void UpdateContents(const PasswordPrompt& reason) override;

    ~OTScriptable() override;

protected:
    // This is how we know the opening numbers for each signer, IN THE ORDER
    // that they signed.
    UnallocatedVector<std::int64_t> openingNumsInOrderOfSigning_;

    mapOfParties m_mapParties;  // The parties to the contract. Could be Nyms,
                                // or
                                // other entities. May be rep'd by an Agent.
    mapOfBylaws m_mapBylaws;    // The Bylaws for this contract.

    // While calculating the ID of smart contracts (and presumably other
    // scriptables)
    // we remove specifics such as instrument definitions, asset accounts, Nym
    // IDs,
    // stashes, etc.
    // We override Contract::CalculateContractID(), where we set
    // m_bCalculatingID to
    // true (it's normally false). Then we call UpdateContents(), which knows to
    // produce
    // an empty version of the contract if m_bCalculatingID is true. Then we
    // hash that
    // in order to get the contract ID, and then we set m_bCalculatingID back to
    // false
    // again.
    //
    // HOWEVER, there may be more options than with baskets (which also use the
    // above
    // trick. Should the smart contract specify a specific instrument
    // definition, or should
    // it leave
    // the instrument definition blank?  Should it specify certain parties, or
    // should it
    // leave the
    // parties blank? I think the accounts should always be blank for
    // calculating ID.
    // And the agents should. And stashes which should be blank in new contracts
    // (always.)
    // But for instrument definitions and parties, shouldn't people be able to
    // specify, for
    // a smart
    // contract template, whether the instrument definitions are part of the
    // contract or
    // whether they
    // are left blank?
    // Furthermore, doesn't this mean that variables need to ALWAYS store their
    // INITIAL
    // value, since they can change over time? We DO want to figure the
    // variable's initial
    // value into the contract ID, but we do NOT want to figure the variable's
    // CURRENT value
    // into that ID (because then comparing the IDs will fail once the variables
    // change.)
    //
    // Therefore, there needs to be a variable on the scriptable itself which
    // determines
    // the template type of the scriptable: m_bSpecifyInstrumentDefinitionID and
    // m_bSpecifyParties, which
    // must each be saved individually on OTScriptable.
    //
    // Agents should be entirely removed during contract ID calculating process,
    // since
    // the Parties can already be specified, and since they can choose their
    // agents at the
    // time of signing -- which are otherwise irrelevant since only the parties
    // are liable.
    //
    // Accounts, conversely, CAN exist on the contract while calculating its ID,
    // but the
    // actual account IDs will be left blank and the instrument definition IDs
    // will be left
    // blank
    // if m_bSpecifyInstrumentDefinitionID is false. (Just as Parties' Owner IDs
    // will be left
    // blank
    // if m_bSpecifyParties is false.)
    //
    // Transaction numbers on parties AND accounts should be set to 0 during
    // calculation
    // of contract ID. Agent name should be left blank on both of those as well.
    //
    // On OTParty, signed copy can be excluded. All agents can be excluded.
    // Authorizing agent
    // can be excluded and Owner ID is conditional on m_bSpecifyParties. (Party
    // name is kept.)
    // m_bPartyIsNym is conditional and so is m_lOpeningTransNo.
    //
    bool m_bCalculatingID{false};  // NOT serialized. Used during ID
                                   // calculation.

    bool m_bSpecifyInstrumentDefinitionID{false};  // Serialized. See above
                                                   // note.
    bool m_bSpecifyParties{false};  // Serialized. See above note.

    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

    OTString m_strLabel;  // OTSmartContract can put its trans# here. (Allowing
                          // us to use it in the OTScriptable methods where any
                          // smart contract would normally want to log its
                          // transaction #, not just the clause name.)

    OTScriptable(const api::Session& api);

private:
    using ot_super = Contract;

    static auto is_ot_namechar_invalid(char c) -> bool;

    OTScriptable() = delete;
};
}  // namespace opentxs
