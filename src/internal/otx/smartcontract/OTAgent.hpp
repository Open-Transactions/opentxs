// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

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
class Base;
class Server;
}  // namespace context
}  // namespace otx

class Contract;
class OTParty;
class OTPartyAccount;
class OTSmartContract;
class PasswordPrompt;
class Tag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
// Agent is always either the Owner Nym acting in his own interests,
// or is an employee Nym acting actively in a role on behalf of an Entity formed
// by contract
// or is a voting group acting passively in a role on behalf of an Entity formed
// by contract
//
// QUESTION: What about having an agent being one Nym representing another?
// NO: because then he needs a ROLE in order to act as agent. In which case, the
// other
// nym should just create an entity he controls, and make the first nym an agent
// for that entity.
//
class OTAgent
{
private:
    const api::Session& api_;
    bool nym_represents_self_;  // Whether this agent represents himself (a nym)
                                // or whether he represents an entity of some
                                // sort.
    bool is_an_individual_;     // Whether this agent is a voting group or Nym
                                // (whether Nym acting for himself or for some
                                // entity.)

    // If agent is active (has a nym), here is the sometimes-available pointer
    // to said Agent Nym. Someday may add a "role" pointer here.
    Nym_p nym_;

    OTParty* for_party_;  // The agent probably has a pointer to the party it
                          // acts on behalf of.

    /*
     <Agent type=“group”// could be “nym”, or “role”, or “group”.
            Nym_id=“” // In case of “nym”, this is the Nym’s ID. If “role”, this
     is NymID of employee in role.
            Role_id=“” // In case of “role”, this is the Role’s ID.
            Entity_id=“this” // same as OwnerID if ever used. Should remove.
            Group_Name=“class_A” // “class A shareholders” are the voting group
     that controls this agent.
     */

    OTString name_;  // agent name (accessible within script language.)

    // info about agent.
    //
    OTString nym_id_;  // If agent is a Nym, then this is the NymID of that
                       // Nym (whether that Nym is owner or not.)
    // If agent is a group (IsAGroup()) then this will be blank. This is
    // different than the
    // Nym stored in OTParty, which if present ALWAYS refers to the OWNER Nym
    // (Though this Nym
    // MAY ALSO be the owner, that fact is purely incidental here AND this NymID
    // could be blank.)
    OTString role_id_;     // If agent is Nym working in a role on behalf of an
                           // entity, then this is its RoleID in Entity.
    OTString group_name_;  // If agent is a voting group in an Entity, this
                           // is group's Name (inside Entity.)

public:
    OTAgent(const api::Session& api);
    OTAgent(
        const api::Session& api,
        const UnallocatedCString& str_agent_name,
        const identity::Nym& theNym,
        const bool bNymRepresentsSelf = true);
    /*IF false, then: ENTITY and ROLE parameters go here.*/
    //
    // Someday another constructor here like the above, for
    // instantiating with an Entity/Group instead of with a Nym.

    OTAgent(
        const api::Session& api,
        bool bNymRepresentsSelf,
        bool bIsAnIndividual,
        const String& strName,
        const String& strNymID,
        const String& strRoleID,
        const String& strGroupName);
    OTAgent() = delete;
    OTAgent(const OTAgent&) = delete;
    OTAgent(OTAgent&&) = delete;
    auto operator=(const OTAgent&) -> OTAgent& = delete;
    auto operator=(OTAgent&&) -> OTAgent& = delete;

    virtual ~OTAgent();

    void Serialize(Tag& parent) const;

    // NOTE: Current iteration, these functions ASSUME that nym_ is loaded.
    // They will definitely fail if you haven't already loaded the Nym.
    //
    auto VerifyIssuedNumber(
        const TransactionNumber& lNumber,
        const String& strNotaryID) -> bool;
    auto VerifyTransactionNumber(
        const TransactionNumber& lNumber,
        const String& strNotaryID) -> bool;
    auto RemoveIssuedNumber(
        const TransactionNumber& lNumber,
        const String& strNotaryID,
        const PasswordPrompt& reason) -> bool;
    auto RemoveTransactionNumber(
        const TransactionNumber& lNumber,
        const String& strNotaryID,
        const PasswordPrompt& reason) -> bool;
    auto RecoverTransactionNumber(
        const TransactionNumber& lNumber,
        otx::context::Base& context) -> bool;
    auto RecoverTransactionNumber(
        const TransactionNumber& lNumber,
        const String& strNotaryID,
        const PasswordPrompt& reason) -> bool;
    auto ReserveOpeningTransNum(otx::context::Server& context) -> bool;
    auto ReserveClosingTransNum(
        otx::context::Server& context,
        OTPartyAccount& thePartyAcct) -> bool;
    auto SignContract(Contract& theInput, const PasswordPrompt& reason) const
        -> bool;

    // Verify that this agent somehow has legitimate agency over this account.
    // (According to the account.)
    //
    auto VerifyAgencyOfAccount(const Account& theAccount) const -> bool;
    auto VerifySignature(const Contract& theContract) const
        -> bool;  // Have the agent try to verify his own signature against any
                  // contract.

    void SetParty(OTParty& theOwnerParty);  // This happens when the agent is
                                            // added to the party.

    auto IsValidSigner(const identity::Nym& theNym) -> bool;
    auto IsValidSignerID(const identifier::Generic& theNymID) -> bool;

    auto IsAuthorizingAgentForParty()
        -> bool;  // true/false whether THIS agent is the
                  // authorizing agent for his party.
    auto GetCountAuthorizedAccts() -> std::int32_t;  // The number of accounts,
                                                     // owned by this
    // agent's party, that this agent is the
    // authorized agent FOR.

    // Only one of these can be true:
    // (I wrestle with making these 2 calls private, since technically it should
    // be irrelevant to the external.)
    //
    auto DoesRepresentHimself() const
        -> bool;  // If the agent is a Nym acting for
    // himself, this will be true. Otherwise,
    // if agent is a Nym acting in a role for
    // an entity, or if agent is a voting
    // group acting for the entity to which
    // it belongs, either way, this will be
    // false.
    // ** OR **
    auto DoesRepresentAnEntity() const
        -> bool;  // Whether the agent is a voting group
                  // acting for an entity, or is a Nym
                  // acting in a Role for an entity, this
                  // will be true either way. (Otherwise,
    // if agent is a Nym acting for himself,
    // then this will be false.)

    // Only one of these can be true:
    // - Agent is either a Nym acting for himself or some entity,
    // - or agent is a group acting for some entity.

    auto IsAnIndividual() const -> bool;  // Agent is an individual Nym.
                                          // (Meaning either he IS ALSO
                                          // the party and thus
    // represents himself, OR he is an agent
    // for an entity who is the party, and
    // he's acting in a role for that
    // entity.) If agent were a group, this
    // would be false.
    // ** OR **
    auto IsAGroup() const
        -> bool;  // OR: Agent is a voting group, which cannot take
                  // proactive or instant action, but only passive and
                  // delayed. Entity-ONLY. (A voting group cannot
    // decide on behalf of individual, but only on behalf
    // of the entity it belongs to.)

    // FYI: A Nym cannot act as agent for another Nym.
    // Nor can a Group act as agent for a Nym.
    //
    // If you want those things, then the owner Nym should form an Entity, and
    // then groups and nyms can act as agents for that entity. You cannot have
    // an agent without an entity formed by contract, since you otherwise have
    // no agency agreement.

    // For when the agent is an individual:
    //
    auto GetNymID(identifier::Generic& theOutput) const
        -> bool;  // If IsIndividual(),
                  // then this is his
                  // own personal NymID,
    // (whether he DoesRepresentHimself() or DoesRepresentAnEntity()
    // -- either way). Otherwise if IsGroup(), this returns false.

    auto GetRoleID(identifier::Generic& theOutput) const
        -> bool;  // IF IsIndividual() AND
                  // DoesRepresentAnEntity(),
                  // then this is his RoleID
                  // within that Entity.
                  // Otherwise, if IsGroup()
                  // or
                  // DoesRepresentHimself(),
                  // then this returns false.

    // Notice if the agent is a voting group, then it has no signer. (Instead it
    // will have an election.)
    // That is why certain agents are unacceptable in certain scripts.
    // There is an "active" agent who has a signerID, but there is also a
    // "passive" agent who only has
    // a group name, and acts based on notifications and replies in the
    // long-term, versus being immediately
    // able to act as part of the operation of a script.
    // Basically if !IsIndividual(), then GetSignerID() will fail and thus
    // anything needing it as part of the
    // script would also therefore be impossible.
    //
    auto GetSignerID(identifier::Generic& theOutput) const -> bool;
    // If IsIndividual() and DoesRepresentAnEntity() then this returns
    // GetRoleID().
    // else if Individual() and DoesRepresentHimself() then this returns
    // GetNymID().
    // else (if IsGroup()) then return false;

    // For when the agent DoesRepresentAnEntity():
    //
    // Whether this agent IsGroup() (meaning he is a voting group that
    // DoesRepresentAnEntity()),
    // OR whether this agent is an individual acting in a role for an entity
    // (IsIndividual() && DoesRepresentAnEntity())
    // ...EITHER WAY, the agent DoesRepresentAnEntity(), and this function
    // returns the ID of that Entity.
    //
    // Otherwise, if the agent DoesRepresentHimself(), then this returns false.
    // I'm debating making this function private along with DoesRepresentHimself
    // / DoesRepresentAnEntity().
    //
    auto GetEntityID(identifier::Generic& theOutput) const
        -> bool;  // IF represents an
                  // entity, this is its ID.
                  // Else fail.

    auto GetName() -> const String&
    {
        return name_;
    }  // agent's name as used in a script.
    // For when the agent is a voting group:
    //
    auto GetGroupName(String& strGroupName)
        -> bool;  // The GroupName group will be
                  // found in the EntityID entity.
    //
    // If !IsGroup() aka IsIndividual(), then this will return false.
    //

    //  bool DoesRepresentHimself();
    //  bool DoesRepresentAnEntity();
    //
    //  bool IsIndividual();
    //  bool IsGroup();

    // PARTY is either a NYM or an ENTITY. This returns ID for that Nym or
    // Entity.
    //
    // If DoesRepresentHimself() then return GetNymID()
    // else (thus DoesRepresentAnEntity()) so return GetEntityID()
    //
    auto GetPartyID(identifier::Generic& theOutput) const -> bool;

    auto GetParty() const -> OTParty* { return for_party_; }

    // IDEA: Put a Nym in the Nyms folder for each entity. While it may
    // not have a public key in the pubkey folder, or embedded within it,
    // it can still have information about the entity or role related to it,
    // which becomes accessible when that Nym is loaded based on the Entity ID.
    // This also makes sure that Nyms and Entities don't ever share IDs, so the
    // IDs become more and more interchangeable.

    auto LoadNym() -> Nym_p;

    auto DropFinalReceiptToNymbox(
        OTSmartContract& theSmartContract,
        const std::int64_t& lNewTransactionNumber,
        const String& strOrigCronItem,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory()) -> bool;

    auto DropFinalReceiptToInbox(
        const String& strNotaryID,
        OTSmartContract& theSmartContract,
        const identifier::Generic& theAccountID,
        const std::int64_t& lNewTransactionNumber,
        const std::int64_t& lClosingNumber,
        const String& strOrigCronItem,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory()) -> bool;

    auto DropServerNoticeToNymbox(
        const api::Session& api,
        bool bSuccessMsg,  // the notice can be "acknowledgment" or "rejection"
        const identity::Nym& theServerNym,
        const identifier::Notary& theNotaryID,
        const std::int64_t& lNewTransactionNumber,
        const std::int64_t& lInReferenceTo,
        const String& strReference,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory(),
        identity::Nym* pActualNym = nullptr) -> bool;
};
}  // namespace opentxs
