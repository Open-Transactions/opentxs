// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_SCRIPT_OTAGENT_HPP
#define OPENTXS_CORE_SCRIPT_OTAGENT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include <map>

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
    const api::Wallet& wallet_;
    bool m_bNymRepresentsSelf;  // Whether this agent represents himself (a nym)
                                // or whether he represents an entity of some
                                // sort.
    bool m_bIsAnIndividual;     // Whether this agent is a voting group or Nym
                                // (whether Nym acting for himself or for some
                                // entity.)

    // If agent is active (has a nym), here is the sometimes-available pointer
    // to said Agent Nym. Someday may add a "role" pointer here.
    ConstNym m_pNym;

    OTParty* m_pForParty;  // The agent probably has a pointer to the party it
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

    OTString m_strName;  // agent name (accessible within script language.)

    // info about agent.
    //
    OTString m_strNymID;  // If agent is a Nym, then this is the NymID of that
                          // Nym (whether that Nym is owner or not.)
    // If agent is a group (IsAGroup()) then this will be blank. This is
    // different than the
    // Nym stored in OTParty, which if present ALWAYS refers to the OWNER Nym
    // (Though this Nym
    // MAY ALSO be the owner, that fact is purely incidental here AND this NymID
    // could be blank.)
    OTString m_strRoleID;  // If agent is Nym working in a role on behalf of an
                           // entity, then this is its RoleID in Entity.
    OTString m_strGroupName;  // If agent is a voting group in an Entity, this
                              // is group's Name (inside Entity.)

    OTAgent() = delete;

public:
    OTAgent(const api::Wallet& wallet);
    OTAgent(
        const api::Wallet& wallet,
        const std::string& str_agent_name,
        const Nym& theNym,
        const bool bNymRepresentsSelf = true);
    /*IF false, then: ENTITY and ROLE parameters go here.*/
    //
    // Someday another constructor here like the above, for
    // instantiating with an Entity/Group instead of with a Nym.

    OTAgent(
        const api::Wallet& wallet,
        bool bNymRepresentsSelf,
        bool bIsAnIndividual,
        const String& strName,
        const String& strNymID,
        const String& strRoleID,
        const String& strGroupName);

    virtual ~OTAgent();

    void Serialize(Tag& parent) const;

    // NOTE: Current iteration, these functions ASSUME that m_pNym is loaded.
    // They will definitely fail if you haven't already loaded the Nym.
    //
    bool VerifyIssuedNumber(
        const TransactionNumber& lNumber,
        const String& strNotaryID);
    bool VerifyTransactionNumber(
        const TransactionNumber& lNumber,
        const String& strNotaryID);
    bool RemoveIssuedNumber(
        const TransactionNumber& lNumber,
        const String& strNotaryID);
    bool RemoveTransactionNumber(
        const TransactionNumber& lNumber,
        const String& strNotaryID);
    bool RecoverTransactionNumber(
        const TransactionNumber& lNumber,
        Context& context);
    bool RecoverTransactionNumber(
        const TransactionNumber& lNumber,
        const String& strNotaryID);
    bool ReserveOpeningTransNum(ServerContext& context);
    bool ReserveClosingTransNum(
        ServerContext& context,
        OTPartyAccount& thePartyAcct);
    EXPORT bool SignContract(Contract& theInput) const;

    // Verify that this agent somehow has legitimate agency over this account.
    // (According to the account.)
    //
    bool VerifyAgencyOfAccount(const Account& theAccount) const;
    bool VerifySignature(const Contract& theContract) const;  // Have the agent
                                                              // try
                                                              // to
    // verify his own signature
    // against any contract.

    void SetParty(OTParty& theOwnerParty);  // This happens when the agent is
                                            // added to the party.

    EXPORT bool IsValidSigner(const Nym& theNym);
    EXPORT bool IsValidSignerID(const identifier::Nym& theNymID);

    bool IsAuthorizingAgentForParty();  // true/false whether THIS agent is the
                                        // authorizing agent for his party.
    std::int32_t GetCountAuthorizedAccts();  // The number of accounts, owned by
                                             // this
    // agent's party, that this agent is the
    // authorized agent FOR.

    // Only one of these can be true:
    // (I wrestle with making these 2 calls private, since technically it should
    // be irrelevant to the external.)
    //
    bool DoesRepresentHimself() const;  // If the agent is a Nym acting for
    // himself, this will be true. Otherwise,
    // if agent is a Nym acting in a role for
    // an entity, or if agent is a voting
    // group acting for the entity to which
    // it belongs, either way, this will be
    // false.
    // ** OR **
    bool DoesRepresentAnEntity() const;  // Whether the agent is a voting group
                                         // acting for an entity, or is a Nym
                                         // acting in a Role for an entity, this
                                         // will be true either way. (Otherwise,
    // if agent is a Nym acting for himself,
    // then this will be false.)

    // Only one of these can be true:
    // - Agent is either a Nym acting for himself or some entity,
    // - or agent is a group acting for some entity.

    EXPORT bool IsAnIndividual() const;  // Agent is an individual Nym. (Meaning
                                         // either he IS ALSO the party and thus
    // represents himself, OR he is an agent
    // for an entity who is the party, and
    // he's acting in a role for that
    // entity.) If agent were a group, this
    // would be false.
    // ** OR **
    bool IsAGroup() const;  // OR: Agent is a voting group, which cannot take
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
    EXPORT bool GetNymID(Identifier& theOutput) const;  // If IsIndividual(),
                                                        // then this is his
                                                        // own personal NymID,
    // (whether he DoesRepresentHimself() or DoesRepresentAnEntity()
    // -- either way). Otherwise if IsGroup(), this returns false.

    bool GetRoleID(Identifier& theOutput) const;  // IF IsIndividual() AND
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
    bool GetSignerID(Identifier& theOutput) const;
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
    bool GetEntityID(Identifier& theOutput) const;  // IF represents an
                                                    // entity, this is its ID.
                                                    // Else fail.

    EXPORT const String& GetName()
    {
        return m_strName;
    }  // agent's name as used in a script.
    // For when the agent is a voting group:
    //
    bool GetGroupName(String& strGroupName);  // The GroupName group will be
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
    bool GetPartyID(Identifier& theOutput) const;

    OTParty* GetParty() const { return m_pForParty; }

    // IDEA: Put a Nym in the Nyms folder for each entity. While it may
    // not have a public key in the pubkey folder, or embedded within it,
    // it can still have information about the entity or role related to it,
    // which becomes accessible when that Nym is loaded based on the Entity ID.
    // This also makes sure that Nyms and Entities don't ever share IDs, so the
    // IDs become more and more interchangeable.

    ConstNym LoadNym();

    bool DropFinalReceiptToNymbox(
        OTSmartContract& theSmartContract,
        const std::int64_t& lNewTransactionNumber,
        const String& strOrigCronItem,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory());

    bool DropFinalReceiptToInbox(
        const String& strNotaryID,
        OTSmartContract& theSmartContract,
        const Identifier& theAccountID,
        const std::int64_t& lNewTransactionNumber,
        const std::int64_t& lClosingNumber,
        const String& strOrigCronItem,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory());

    bool DropServerNoticeToNymbox(
        const api::Core& api,
        bool bSuccessMsg,  // the notice can be "acknowledgment" or "rejection"
        const Nym& theServerNym,
        const Identifier& theNotaryID,
        const std::int64_t& lNewTransactionNumber,
        const std::int64_t& lInReferenceTo,
        const String& strReference,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory(),
        Nym* pActualNym = nullptr);
};
}  // namespace opentxs
#endif
