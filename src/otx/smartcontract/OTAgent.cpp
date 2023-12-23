// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/smartcontract/OTAgent.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "internal/core/String.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/recurring/OTAgreement.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/otx/consensus/Base.hpp"
#include "internal/otx/consensus/Client.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/otx/consensus/ManagedNumber.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/otx/smartcontract/OTParty.hpp"
#include "internal/otx/smartcontract/OTPartyAccount.hpp"
#include "internal/otx/smartcontract/OTSmartContract.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// Have the agent try to verify his own signature against any contract.
//
// NOTE: This function assumes that you have already taken actions that would
// have loaded the Nym's pointer
// and placed it within this Agent. This is a low-level call and it expects that
// you have already been using
// calls such as HasAgent(), HasAuthorizingAgent(), LoadAuthorizingAgent(), etc.
// This function also assumes that once you are done, you will call
// ClearTemporaryPointers().
//
namespace opentxs
{
OTAgent::OTAgent(const api::Session& api)
    : api_(api)
    , nym_represents_self_(false)
    , is_an_individual_(false)
    , nym_(nullptr)
    , for_party_(nullptr)
    , name_(String::Factory())
    , nym_id_(String::Factory())
    , role_id_(String::Factory())
    , group_name_(String::Factory())
{
}

OTAgent::OTAgent(
    const api::Session& api,
    bool bNymRepresentsSelf,
    bool bIsAnIndividual,
    const String& strName,
    const String& strNymID,
    const String& strRoleID,
    const String& strGroupName)
    : api_(api)
    , nym_represents_self_(bNymRepresentsSelf)
    , is_an_individual_(bIsAnIndividual)
    , nym_(nullptr)
    , for_party_(nullptr)
    , name_(strName)
    , nym_id_(strNymID)
    , role_id_(strRoleID)
    , group_name_(strGroupName)
{
}

OTAgent::OTAgent(
    const api::Session& api,
    const UnallocatedCString& str_agent_name,
    const identity::Nym& theNym,
    const bool bNymRepresentsSelf)
    /*IF false, then: ROLE parameter goes here.*/
    : api_(api)
    , nym_represents_self_(bNymRepresentsSelf)
    , is_an_individual_(true)
    , nym_(&theNym)
    , for_party_(nullptr)
    , name_(String::Factory(str_agent_name.c_str()))
    , nym_id_(String::Factory())
    , role_id_(String::Factory())
    , group_name_(String::Factory())
{
    // Grab nym_id_
    auto theNymID = identifier::Nym{};
    theNym.GetIdentifier(theNymID);
    theNymID.GetString(api_.Crypto(), nym_id_);

    //

    if (!bNymRepresentsSelf) {
        // Todo: if the Nym represents an Entity, then RoleID should
        // be passed in, and set here.  I WILL PROBABLY make that part into a
        // SEPARATE CONSTRUCTOR.
        // (Once I get around to adding Entities.)
        //
        LogError()()("THIS HASN'T BEEN WRITTEN YET!!").Flush();
    }
}

auto OTAgent::VerifySignature(const Contract& theContract) const -> bool
{
    // Only individual agents can sign for things, not groups (groups vote, they
    // don't sign.)
    // Thus, an individual can verify a signature, whereas a voting group would
    // verify an election result (or whatever.)
    //
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        LogError()()("Entities and roles are not yet supported. Agent: ")(
            name_.get())(".")
            .Flush();
        return false;
    }  // todo: when adding entities, this will change.

    //    if (DoesRepresentAnEntity)
    //    {
    //        // The original version of a smartcontract might show that Frank,
    // the Sales Director, signed it.
    //        // Years later, Frank is fired, and Jim is appointed to his former
    // Role of sales director, in the same entity.
    //        // The original copy of the smart contract still contains Frank's
    // signature, and thus we still need to load Frank
    //        // in order to verify that original signature.  That's why we load
    // Frank by the NymID stored there. He was the Nym
    //        // at the time, so that's the key we load.
    //        //
    //        // NEXT: What if JIM tries to verify the signature on the
    // contract, even though FRANK was the original signer?
    //        // Should OTAgent be smart enough here to substitute Frank
    // whenever Jim tries to verify? I argue no: this function is
    //        // too low-level. Plus it's backwards. If Jim tries to DO an
    // action, THEN OT should be smart enough to verify that Jim
    //        // is in the proper Role and that Jim's signature is good enough
    // to authorize actions. But if OT is verifying Frank's
    //        // signature on some old copy of something that Frank formerly
    // signed, then this function should clearly tell me if Frank's
    //        // sig verified... or not.
    //        //
    //        // Therefore the "DoesRepresentAnEntity()" option is useless here,
    // since we are verifying the same Nym's signature whether
    //        // he represents an entity or not.
    //        //
    //    }
    //    else
    if (nullptr == nym_) {
        auto strTemp = String::Factory(theContract);
        LogError()()("Attempted to verify signature on "
                     "contract, "
                     "but no Nym had ever been loaded for this agent: ")(
            strTemp.get())(".")
            .Flush();
        return false;
    }

    return theContract.VerifySignature(*nym_);
}

// Low-level.
// Don't call this unless you're sure the same Nym isn't already loaded, or
// unless
// you are prepared to compare the returned Nym with all the Nyms you already
// have loaded.
//
// This call may always fail for a specific agent, if the agent isn't a Nym
// (the agent could be a voting group.)
//
auto OTAgent::LoadNym() -> Nym_p
{
    auto theAgentNymID = identifier::Nym{};
    const bool bNymID = GetNymID(theAgentNymID);

    if (bNymID) {
        nym_ = api_.Wallet().Nym(theAgentNymID);
        assert_false(nullptr == nym_);

        return nym_;
    } else {
        LogError()()("Failure. Are you sure this agent IS a Nym at all?")
            .Flush();
    }

    return nullptr;
}

void OTAgent::SetParty(OTParty& theOwnerParty)  // This happens when the agent
                                                // is
                                                // added to the party.
{
    for_party_ = &theOwnerParty;

    // A Nym can only act as agent for himself or for an entity
    // (never for another Nym. Start an entity if you want that.)
    // If the owner party is a Nym, therefore this agent IS the Nym acting for
    // himself.
    // Whereas if the owner party were an entity, then this agent could be a Nym
    // or a voting group.
    // Since inside this block the owner party IS a Nym, not an entity, then the
    // agent can therefore
    // only be THAT Nym, acting as an agent for himself. Remember, a Nym cannot
    // act as agent for
    // another Nym. That is only possible by an agreement between them, and that
    // agreement becomes
    // the entity (therefore it's only possible using an entity.)
    //
    if (theOwnerParty.IsNym())  // Thus, this basically means the agent IS the
                                // party.
    {
        nym_represents_self_ = true;
        is_an_individual_ = true;

        bool bGetOwnerNymID = false;
        const UnallocatedCString str_owner_nym_id =
            theOwnerParty.GetNymID(&bGetOwnerNymID);
        nym_id_->Set(bGetOwnerNymID ? str_owner_nym_id.c_str() : "");

        // Todo here, instead of copying the Owner's Nym ID like above, just
        // make sure they match.
        // Similarly, make sure that the RoleID or GroupName, whichever is
        // relevant, is validated for the owner.
    }
}

// If the agent is a Nym acting for himself, this will be true. Otherwise, if
// agent is a Nym acting in a role for an entity, or if agent is a voting group
// acting for the entity to which it belongs, either way, this will be false.

auto OTAgent::DoesRepresentHimself() const -> bool
{
    return nym_represents_self_;
}

// Whether the agent is a voting group acting for an entity, or is a Nym acting
// in a Role for an entity, this will be true either way. (Otherwise, if agent
// is a Nym acting for himself, then this will be false.)

auto OTAgent::DoesRepresentAnEntity() const -> bool
{
    return !nym_represents_self_;
}

// Only one of these can be true:
//
// - Agent is either a Nym acting for himself or some entity,
// - or agent is a group acting for some entity.

// Agent is an individual Nym. (Meaning either he IS ALSO the party and thus
// represents himself, OR he is an agent for an entity who is the party, and
// he's acting in a role for that entity.) If agent were a group, this would be
// false.

auto OTAgent::IsAnIndividual() const -> bool { return is_an_individual_; }

// OR: Agent is a voting group, which cannot take proactive or instant action,
// but only passive and delayed. Entity-ONLY. (A voting group cannot decide on
// behalf of individual, but only on behalf of the entity it belongs to.)

auto OTAgent::IsAGroup() const -> bool { return !is_an_individual_; }

// A Nym cannot act as "agent" for another Nym.
// Nor can a Group act as "agent" for a Nym. Why not? Because:
//
// An entity is COMPOSED of its voting groups and its employee Nyms.
// These don't merely act "on behalf" of the entity, but in fact they comprise
// the entity.
// Therefore the entity can use voting groups and employee Nyms to make
// decisions BECAUSE
// IT **HAS** voting groups and employee nyms that it can use.
//
// Whereas an individual Nym is NOT composed of voting groups or employee Nyms.
// So how could
// he designate to act "on his behalf" something that does not even exist?
// So we must ask, how can another Nym, then, be appointed to act as my agent
// without some
// agreement designating him as such? And that agreement is the entity itself,
// which is nothing
// more than an agreement between several owners to designate agents to operate
// according to their
// interests.

// To directly appoint one Nym to act on behalf of another, yet WITHOUT any
// agreement in place,
// is to behave as one is the same as the other. But still, who has the key? If
// both have absolutely
// access and rights to the same key, then why not just both keep a copy of it?
// In which case now
// it really IS only one Nym in reality, as well as in the software.
// But if one prefers to have his private key, and another his, then they will
// begin as separate and
// independent individuals. One Nym will not be found within the other as a
// std::int64_t-lost, under-developed
// twin!
// Just as reality enforces separate individuals, so does the software end up in
// the situation where
// two separate Nyms now wish to act with one as agent for the other. This wish
// is perfectly valid and
// can be accommodated, but logically the software cannot provide the same
// ONENESS of ACTUALLY SHARING
// THE PRIVATE KEY IN REAL LIFE (which allows the software to ACTUALLY only deal
// with a single key),
// as opposed to the oneness of "we have separate keys but we want to act in
// this way". There is oneness,
// and then there is oneness. OT is about contracts between Nyms, and therefore
// that is the mechanism
// for implementing ANY OTHER FORM OF AGENCY. Whether that agent is your "best
// man" or a board of voters,
// and whether the agent acts on behalf of you, or some corporation or
// democracy, either way, either his
// key is the same bits as your key, or they are separate keys in which case
// there is an agreement
// between them. Otherwise OT simply does not know which agents have
// authority--and which do not. This
// knowledge is necessary just for being able to function, and is imposed by the
// natural law.

// IDEA: Have a factory for smart contracts, such that not only are different
// subclasses instantiated,
// but more usefully, common configurations such as new democracy, new
// corporation, new board-with-veto, etc.
//

// For when the agent is an individual:
//

// If IsIndividual(), then this is his own personal NymID,
// (whether he DoesRepresentHimself() or DoesRepresentAnEntity() -- either way).
// Otherwise if IsGroup(), this returns false.
//
auto OTAgent::GetNymID(identifier::Generic& theOutput) const -> bool
{
    if (IsAnIndividual()) {
        theOutput = api_.Factory().IdentifierFromBase58(nym_id_->Bytes());

        return true;
    }

    return false;
}

// IF IsIndividual() AND DoesRepresentAnEntity(), then this is his RoleID within
// that Entity. Otherwise, if IsGroup() or DoesRepresentHimself(), then this
// returns false.

auto OTAgent::GetRoleID(identifier::Generic& theOutput) const -> bool
{
    if (IsAnIndividual() && DoesRepresentAnEntity()) {
        theOutput = api_.Factory().IdentifierFromBase58(role_id_->Bytes());

        return true;
    }

    return false;
}

// Notice if the agent is a voting group, then it has no signer. (Instead it
// will have an election.)
// That is why certain agents are unacceptable in certain scripts. They are
// PASSIVE.
//
// There is an "active" agent who has a signerID, but there is also a "passive"
// agent who only has
// a group name, and acts based on notifications and replies in the
// std::int64_t-term, versus being immediately
// able to act as part of the operation of a script.
//
// Basically if !IsIndividual(), then GetSignerID() will fail and thus anything
// needing that,
// as part of the script, would also therefore be impossible.
//
auto OTAgent::GetSignerID(identifier::Generic& theOutput) const -> bool
{
    // If IsIndividual() and DoesRepresentAnEntity() then this returns
    // GetRoleID().
    // else if Individual() and DoesRepresentHimself() then this returns
    // GetNymID().
    // else (if IsGroup()) then return false;

    if (IsAnIndividual()) {
        if (DoesRepresentAnEntity()) {
            return GetRoleID(theOutput);
        } else  // DoesRepresentHimself()
        {
            return GetNymID(theOutput);
        }
    }

    // else IsGroup()... unable to sign directly; must hold votes instead.
    //
    return false;
}

auto OTAgent::IsValidSignerID(const identifier::Generic& theNymID) -> bool
{
    auto theAgentNymID = identifier::Generic{};
    const bool bNymID = GetNymID(theAgentNymID);

    // If there's a NymID on this agent, and it matches theNymID...
    //
    if (bNymID && (theNymID == theAgentNymID)) { return true; }

    // TODO Entities...
    //
    return false;
}

// See if theNym is a valid signer for this agent.
//
auto OTAgent::IsValidSigner(const identity::Nym& theNym) -> bool
{
    auto theAgentNymID = identifier::Nym{};
    const bool bNymID = GetNymID(theAgentNymID);

    // If there's a NymID on this agent, and it matches theNym's ID...
    //
    if (bNymID && theNym.CompareID(theAgentNymID)) {
        // That means theNym *is* the Nym for this agent!
        // We'll save his pointer, for future reference...
        //
        nym_.reset(&theNym);

        return true;
    }

    // TODO Entity: Perhaps the original Nym was fired from his role... another
    // Nym has now taken his place. In which case, the original Nym should be
    // refused as a valid signer, and the new Nym should be allowed to sign in
    // his place!
    //
    // This means if DoesRepresentAnEntity(), then I have to load the Role, and
    // verify the Nym against that Role (which contains the updated status).
    // Since I haven't coded Entities/Roles yet, then I don't have to do this
    // just yet... Might even update the NymID on this agent, for updated copies
    // of the agreement. (Obviously the original can't be changed...)

    return false;
}

// For when the agent DoesRepresentAnEntity():
//
// Whether this agent IsGroup() (meaning he is a voting group that
// DoesRepresentAnEntity()),
// OR whether this agent is an individual acting in a role for an entity
// (IsIndividual() && DoesRepresentAnEntity())
// ...EITHER WAY, the agent DoesRepresentAnEntity(), and this function returns
// the ID of that Entity.
//
// Otherwise, if the agent DoesRepresentHimself(), then this returns false.
// I'm debating making this function private along with DoesRepresentHimself /
// DoesRepresentAnEntity().
//
auto OTAgent::GetEntityID(identifier::Generic& theOutput) const -> bool
{
    // IF represents an entity, then this is its ID. Else fail.
    //
    if (DoesRepresentAnEntity() && (nullptr != for_party_) &&
        for_party_->IsEntity()) {
        bool bSuccessEntityID = false;
        const UnallocatedCString str_entity_id =
            for_party_->GetEntityID(&bSuccessEntityID);

        if (bSuccessEntityID && (str_entity_id.size() > 0)) {
            auto strEntityID = String::Factory(str_entity_id.c_str());
            theOutput =
                api_.Factory().IdentifierFromBase58(strEntityID->Bytes());

            return true;
        }
    }

    return false;
}

// Returns true/false whether THIS agent is the authorizing agent for his party.
//
auto OTAgent::IsAuthorizingAgentForParty() -> bool
{
    if (nullptr == for_party_) { return false; }

    if (name_->Compare(for_party_->GetAuthorizingAgentName().c_str())) {
        return true;
    }

    return false;
}

// Returns the number of accounts, owned by this agent's party, that this agent
// is the authorized agent FOR.
//
auto OTAgent::GetCountAuthorizedAccts() -> std::int32_t
{
    if (nullptr == for_party_) {
        LogError()()("Error: for_party_ was "
                     "nullptr.")
            .Flush();
        return 0;  // Maybe should log here...
    }

    return for_party_->GetAccountCount(name_->Get());
}

// For when the agent is a voting group:
// If !IsGroup() aka IsIndividual(), then this will return false.
//
auto OTAgent::GetGroupName(String& strGroupName) -> bool
{
    if (IsAGroup()) {
        strGroupName.Set(group_name_);

        return true;
    }

    return false;
}

// PARTY is either a NYM or an ENTITY. This returns ID for that Nym or Entity.
//
auto OTAgent::GetPartyID(identifier::Generic& theOutput) const -> bool
{
    if (DoesRepresentHimself()) { return GetNymID(theOutput); }

    return GetEntityID(theOutput);
}

auto OTAgent::VerifyAgencyOfAccount(const Account& theAccount) const -> bool
{
    auto theSignerID = identifier::Nym{};

    if (!GetSignerID(theSignerID)) {
        LogError()()("ERROR: Entities and roles "
                     "haven't been coded yet.")
            .Flush();
        return false;
    }

    return theAccount.VerifyOwnerByID(theSignerID);  // todo when entities and
                                                     // roles come, won't this
                                                     // "just work", or do I
                                                     // also have to warn the
                                                     // acct whether it's a Nym
                                                     // or a Role being passed?
}

auto OTAgent::DropFinalReceiptToInbox(
    const String& strNotaryID,
    OTSmartContract& theSmartContract,
    const identifier::Account& theAccountID,
    const std::int64_t& lNewTransactionNumber,
    const std::int64_t& lClosingNumber,
    const String& strOrigCronItem,
    const PasswordPrompt& reason,
    OTString pstrNote,
    OTString pstrAttachment) -> bool
{
    // TODO: When entites and ROLES are added, this function may change a bit to
    // accommodate them.

    auto theAgentNymID = identifier::Nym{};
    const bool bNymID = GetNymID(theAgentNymID);

    // Not all agents have Nyms. (Might be a voting group.) But in the case of
    // Inboxes for asset accounts, shouldn't the agent be a Nym?
    // Perhaps not... perhaps not... we shall see.

    if (bNymID) {
        // IsAnIndividual() is definitely true.

        auto context = api_.Wallet().Internal().ClientContext(theAgentNymID);

        assert_false(nullptr == context);

        if ((lClosingNumber > 0) &&
            context->VerifyIssuedNumber(lClosingNumber)) {
            return theSmartContract.DropFinalReceiptToInbox(
                theAgentNymID,
                theAccountID,
                lNewTransactionNumber,
                lClosingNumber,
                strOrigCronItem,
                theSmartContract.GetOriginType(),
                reason,
                pstrNote,
                pstrAttachment);  // pActualAcct=nullptr here. (This call will
                                  // load
                                  // the acct up and update its inbox hash.)
        } else {
            LogError()()("Error: lClosingNumber <=0, "
                         "or context->VerifyIssuedNum("
                         "lClosingNumber)) failed to verify.")
                .Flush();
        }
    } else {
        LogError()()("No NymID available for this agent...").Flush();
    }

    return false;
}

auto OTAgent::DropFinalReceiptToNymbox(
    OTSmartContract& theSmartContract,
    const std::int64_t& lNewTransactionNumber,
    const String& strOrigCronItem,
    const PasswordPrompt& reason,
    OTString pstrNote,
    OTString pstrAttachment) -> bool
{
    auto theAgentNymID = identifier::Nym{};
    const bool bNymID = GetNymID(theAgentNymID);

    // Not all agents have Nyms. (Might be a voting group.)

    if (true == bNymID) {
        return theSmartContract.DropFinalReceiptToNymbox(
            theAgentNymID,
            lNewTransactionNumber,
            strOrigCronItem,
            theSmartContract.GetOriginType(),
            reason,
            pstrNote,
            pstrAttachment);
    }

    // TODO: When entites and roles are added, this function may change a bit to
    // accommodate them.

    return false;
}

auto OTAgent::DropServerNoticeToNymbox(
    const api::Session& api,
    bool bSuccessMsg,  // Added this so we can notify smart contract parties
                       // when
                       // it FAILS to activate.
    const identity::Nym& theServerNym,
    const identifier::Notary& theNotaryID,
    const std::int64_t& lNewTransactionNumber,
    const std::int64_t& lInReferenceTo,
    const String& strReference,
    const PasswordPrompt& reason,
    OTString pstrNote,
    OTString pstrAttachment,
    identity::Nym* pActualNym) -> bool
{
    auto theAgentNymID = identifier::Nym{};
    const bool bNymID = GetNymID(theAgentNymID);

    // Not all agents have Nyms. (Might be a voting group.)

    if (true == bNymID) {

        return OTAgreement::DropServerNoticeToNymbox(
            api,
            bSuccessMsg,
            theServerNym,
            theNotaryID,
            theAgentNymID,
            lNewTransactionNumber,
            lInReferenceTo,
            strReference,
            otx::originType::origin_smart_contract,
            pstrNote,
            pstrAttachment,
            theAgentNymID,
            reason);
    }

    // TODO: When entites and roles are added, this function may change a bit to
    // accommodate them.

    return false;
}

auto OTAgent::SignContract(Contract& theInput, const PasswordPrompt& reason)
    const -> bool
{
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        LogError()()("Entities and roles are not yet supported. Agent: ")(
            name_.get())(".")
            .Flush();
        return false;
    }  // todo: when adding entities, this will change.

    if (nullptr == nym_) {
        LogError()()("Nym was nullptr while trying to sign contract. Agent: ")(
            name_.get())(".")
            .Flush();
        return false;
    }  // todo: when adding entities, this will change.

    return theInput.SignContract(*nym_, reason);
}

auto OTAgent::VerifyIssuedNumber(
    const TransactionNumber& lNumber,
    const String& strNotaryID) -> bool
{
    // Todo: this function may change when entities / roles are added.
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        LogError()()(
            "Error: Entities and Roles are not yet supported. Agent: ")(
            name_.get())(".")
            .Flush();
        return false;
    }

    if (nullptr != nym_) {
        auto context = api_.Wallet().Internal().Context(
            api_.Factory().NotaryIDFromBase58(strNotaryID.Bytes()), nym_->ID());

        assert_false(nullptr == context);

        return context->VerifyIssuedNumber(lNumber);
    } else {
        LogError()()("Error: nym_ was nullptr. For agent: ")(name_.get())(".")
            .Flush();
    }

    return false;
}

auto OTAgent::VerifyTransactionNumber(
    const TransactionNumber& lNumber,
    const String& strNotaryID) -> bool
{
    // Todo: this function may change when entities / roles are added.
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        LogError()()(
            "Error: Entities and Roles are not yet supported. Agent: ")(
            name_.get())(".")
            .Flush();
        return false;
    }

    if (nullptr != nym_) {
        auto context = api_.Wallet().Internal().Context(
            api_.Factory().NotaryIDFromBase58(strNotaryID.Bytes()), nym_->ID());

        assert_false(nullptr == context);

        return context->VerifyAvailableNumber(lNumber);
    } else {
        LogError()()("Error: nym_ was nullptr. For agent: ")(name_.get())(".")
            .Flush();
    }

    return false;
}

auto OTAgent::RecoverTransactionNumber(
    const TransactionNumber& lNumber,
    otx::context::Base& context) -> bool
{
    // Todo: this function may change when entities / roles are added.
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        LogError()()(
            "Error: Entities and Roles are not yet supported. Agent: ")(
            name_.get())(".")
            .Flush();
        return false;
    }

    if (nullptr != nym_) {
        // This won't "add it back" unless we're SURE he had it in the first
        // place...
        const bool bSuccess = context.RecoverAvailableNumber(lNumber);

        if (bSuccess) {
            // The transaction is being removed from play, so we will remove it
            // from this list. That is, when we called RemoveTransactionNumber,
            // the number was being put into play until RemoveIssuedNumber is
            // called to close it out. But now RemoveIssuedNumber won't ever be
            // called, since we are harvesting it back for future use. Therefore
            // the number is currently no longer in play, therefore we remove it
            // from the list of open cron numbers.
            context.CloseCronItem(lNumber);

            return true;
        } else {
            LogError()()("Number (")(lNumber)(") failed to verify for agent: ")(
                name_.get())(" (Thus didn't bother 'adding it back').")
                .Flush();
        }
    } else {
        LogError()()("Error: nym_ was nullptr. For agent: ")(name_.get())(".")
            .Flush();
    }

    return false;
}

auto OTAgent::RecoverTransactionNumber(
    const TransactionNumber& lNumber,
    const String& strNotaryID,
    const PasswordPrompt& reason) -> bool
{
    if (nullptr != nym_) {
        auto context = api_.Wallet().Internal().mutable_Context(
            api_.Factory().NotaryIDFromBase58(strNotaryID.Bytes()),
            nym_->ID(),
            reason);

        return RecoverTransactionNumber(lNumber, context.get());
    } else {
        LogError()()("Error: nym_ was nullptr. For agent: ")(name_.get())(".")
            .Flush();
    }

    return false;
}

// This means the transaction number has just been USED (and it now must stay
// open/outstanding until CLOSED.) Therefore we also add it to the set of open
// cron items, which the server keeps track of (for opening AND closing numbers)
auto OTAgent::RemoveTransactionNumber(
    const TransactionNumber& lNumber,
    const String& strNotaryID,
    const PasswordPrompt& reason) -> bool
{
    // Todo: this function may change when entities / roles are added.
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        LogError()()(
            "Error: Entities and Roles are not yet supported. Agent: ")(
            name_.get())(".")
            .Flush();
        return false;
    }

    if (nullptr == nym_) {
        LogError()()("Error: nym_ was nullptr. For agent: ")(name_.get())(".")
            .Flush();

        return false;
    }

    auto context = api_.Wallet().Internal().mutable_Context(
        api_.Factory().NotaryIDFromBase58(strNotaryID.Bytes()),
        nym_->ID(),
        reason);

    if (context.get().ConsumeAvailable(lNumber)) {
        context.get().OpenCronItem(lNumber);
    } else {
        LogError()()(
            "Error, should never happen. (I'd assume you aren't removing "
            "numbers without verifying first if they're there).")
            .Flush();
    }

    return false;
}

// This means the transaction number has just been CLOSED.
// Therefore we remove it from the set of open cron items, which the server
// keeps track of (for opening AND closing numbers.)
//
auto OTAgent::RemoveIssuedNumber(
    const TransactionNumber& lNumber,
    const String& strNotaryID,
    const PasswordPrompt& reason) -> bool
{
    // Todo: this function may change when entities / roles are added.
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        LogError()()(
            "Error: Entities and Roles are not yet supported. Agent: ")(
            name_.get())(".")
            .Flush();

        return false;
    }

    if (nullptr == nym_) {
        LogError()()("Error: nym_ was nullptr. For agent: ")(name_.get())(".")
            .Flush();

        return false;
    }

    auto context = api_.Wallet().Internal().mutable_Context(
        api_.Factory().NotaryIDFromBase58(strNotaryID.Bytes()),
        nym_->ID(),
        reason);

    if (context.get().ConsumeIssued(lNumber)) {
        context.get().CloseCronItem(lNumber);
    } else {
        LogError()()(
            "Error, should never happen. (I'd assume you aren't removing "
            "issued numbers without verifying first if they're there).")
            .Flush();
    }

    return true;
}

// Done
auto OTAgent::ReserveClosingTransNum(
    otx::context::Server& context,
    OTPartyAccount& thePartyAcct) -> bool
{
    if (IsAnIndividual() && DoesRepresentHimself() && (nullptr != nym_)) {
        if (thePartyAcct.GetClosingTransNo() > 0) {
            LogConsole()()(
                "Failure: The account ALREADY has a closing transaction number "
                "set on it. Don't you want to save that first, before "
                "overwriting it?")
                .Flush();

            return false;
        }

        // Need a closing number...
        const auto number = context.InternalServer().NextTransactionNumber(
            otx::MessageType::notarizeTransaction);

        if (0 == number.Value()) {
            LogError()()(
                "Error: Strangely, unable to get a transaction number.")
                .Flush();

            return false;
        }

        // Above this line, the transaction number will be recovered
        // automatically
        number.SetSuccess(true);
        LogError()()("Allocated closing transaction number ")(number.Value())(
            ".")
            .Flush();

        // BELOW THIS POINT, TRANSACTION # HAS BEEN RESERVED, AND MUST BE
        // SAVED...
        // Any errors below this point will require this call before returning:
        // HarvestAllTransactionNumbers(strNotaryID);
        //
        thePartyAcct.SetClosingTransNo(number.Value());
        thePartyAcct.SetAgentName(name_);

        return true;
    } else  // todo: when entities and roles are added... this function will
            // change.
    {
        LogError()()(
            "Either the Nym pointer isn't set properly, or you tried to use "
            "Entities when they haven't been coded yet. Agent: ")(name_.get())(
            ".")
            .Flush();
    }

    return false;
}

// Done
auto OTAgent::ReserveOpeningTransNum(otx::context::Server& context) -> bool
{
    if (IsAnIndividual() && DoesRepresentHimself() && (nullptr != nym_)) {
        if (nullptr == for_party_) {
            LogError()()(
                "Error: Party pointer was nullptr. SHOULD NEVER HAPPEN!!")
                .Flush();
            return false;
        }

        if (for_party_->GetOpeningTransNo() > 0) {
            LogConsole()()(
                "Failure: Party ALREADY had an opening transaction number set "
                "on it. Don't you want to save that first, before overwriting "
                "it?")
                .Flush();
            return false;
        }

        // Need opening number...
        const auto number = context.InternalServer().NextTransactionNumber(
            otx::MessageType::notarizeTransaction);

        if (0 == number.Value()) {
            LogError()()(
                "Error: Strangely, unable to get a transaction number.")
                .Flush();

            return false;
        }

        // Above this line, the transaction number will be recovered
        // automatically
        number.SetSuccess(true);
        LogError()()("Allocated opening transaction number ")(number.Value())(
            ".")
            .Flush();

        // BELOW THIS POINT, TRANSACTION # HAS BEEN RESERVED, AND MUST BE
        // SAVED...
        // Any errors below this point will require this call before returning:
        // HarvestAllTransactionNumbers(strNotaryID);
        //
        for_party_->SetOpeningTransNo(number.Value());
        for_party_->SetAuthorizingAgentName(name_->Get());

        return true;
    } else  // todo: when entities and roles are added... this function will
            // change.
    {
        LogError()()(
            "Either the Nym pointer isn't set properly, or you tried to use "
            "Entities when they haven't been coded yet. Agent: ")(name_.get())(
            ".")
            .Flush();
    }

    return false;
}

void OTAgent::Serialize(Tag& parent) const
{
    TagPtr pTag(new Tag("agent"));

    pTag->add_attribute("name", name_->Get());
    pTag->add_attribute(
        "doesAgentRepresentHimself", formatBool(nym_represents_self_));
    pTag->add_attribute("isAgentAnIndividual", formatBool(is_an_individual_));
    pTag->add_attribute("nymID", nym_id_->Get());
    pTag->add_attribute("roleID", role_id_->Get());
    pTag->add_attribute("groupName", group_name_->Get());

    parent.add_tag(pTag);
}

OTAgent::~OTAgent()
{
    for_party_ = nullptr;  // The agent probably has a pointer to the party it
                           // acts on behalf of.
}
}  // namespace opentxs
