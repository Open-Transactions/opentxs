// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <memory>

#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "internal/otx/common/cron/OTCronItem.hpp"
#include "internal/otx/common/script/OTScriptable.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace imp
{
class Factory;
}  // namespace imp
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Generic;
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
class Client;
class Server;
}  // namespace context

namespace internal
{
class AccountList;
}  // namespace internal
}  // namespace otx

class NumList;
class OTParty;
class OTScript;
class OTStash;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OTSmartContract : public OTCronItem
{
private:  // Private prevents erroneous use by other classes.
    using ot_super = OTCronItem;

public:
    using mapOfAccounts = UnallocatedMap<UnallocatedCString, SharedAccount>;
    using mapOfStashes = UnallocatedMap<UnallocatedCString, OTStash*>;

    auto GetOriginType() const -> originType override
    {
        return originType::origin_smart_contract;
    }

    void SetDisplayLabel(
        const UnallocatedCString* pstrLabel = nullptr) override;
    // FOR RECEIPTS
    // These IDs are stored for cases where this Cron Item is sitting in a
    // receipt
    // in an inbox somewhere, so that, whether the payment was a success or
    // failure,
    // we can see the intended sender/recipient user/acct IDs. They are cleared
    // and
    // then set right when a MoveAcctFunds() or StashAcctFunds() is being
    // performed.
    //
    auto GetLastSenderNymID() const -> const String&
    {
        return last_sender_user_;
    }
    auto GetLastSenderAcctID() const -> const String&
    {
        return last_sender_acct_;
    }
    auto GetLastRecipientNymID() const -> const String&
    {
        return last_recipient_user_;
    }
    auto GetLastRecipientAcctID() const -> const String&
    {
        return last_recipient_acct_;
    }
    auto GetCountStashes() const -> std::int32_t;
    auto GetCountStashAccts() const -> std::int32_t;
    // Merchant Nym is passed here so we can verify the signature before
    // confirming.
    // These notes are from OTAgreement/OTPaymentPlan but they are still
    // relevant:
    //
    // This function verifies both Nyms and both signatures.
    // Due to the peculiar nature of how OTAgreement/OTPaymentPlan works, there
    // are two signed
    // copies stored. The merchant signs first, adding his transaction numbers
    // (2), and then he
    // sends it to the customer, who also adds two numbers and signs. (Also
    // resetting the creation date.)
    // The problem is, adding the additional transaction numbers invalidates the
    // first (merchant's)
    // signature.
    // The solution is, when the customer confirms the agreement, he stores an
    // internal copy of the
    // merchant's signed version.  This way later, in VERIFY AGREEMENT, the
    // internal copy can be loaded,
    // and BOTH Nyms can be checked to verify that BOTH transaction numbers are
    // valid for each.
    // The two versions of the contract can also be compared to each other, to
    // make sure that none of
    // the vital terms, values, clauses, etc are different between the two.
    //
    auto Compare(OTScriptable& rhs) const -> bool override;
    // From OTCronItem (parent class of this)
    auto CanRemoveItemFromCron(const otx::context::Client& context)
        -> bool override;

    void HarvestOpeningNumber(otx::context::Server& context) override;
    void HarvestClosingNumbers(otx::context::Server& context) override;

    // Server-side. Similar to below:
    void CloseoutOpeningNumbers(const PasswordPrompt& reason);
    using ot_super::HarvestClosingNumbers;
    void HarvestClosingNumbers(
        const identity::Nym& pSignerNym,
        const PasswordPrompt& reason,
        UnallocatedSet<OTParty*>* pFailedParties = nullptr);  // Used on
                                                              // server-side.
                                                              // Assumes the
    // related Nyms are already loaded and
    // known to *this. Purpose of
    // pSignerNymm is to pass in the
    // server Nym, since internally a nullptr
    // is automatically interpeted as
    // "each nym signs for himself" (which
    // you don't want, on the server
    // side.)

    // Return True if should stay on OTCron's list for more processing.
    // Return False if expired or otherwise should be removed.
    auto ProcessCron(const PasswordPrompt& reason)
        -> bool override;  // OTCron calls
                           // this regularly,
                           // which is my
                           // chance to
                           // expire, etc.

    auto HasTransactionNum(const std::int64_t& lInput) const -> bool override;
    void GetAllTransactionNumbers(NumList& numlistOutput) const override;

    auto AddParty(OTParty& theParty) -> bool override;  // Takes ownership.
    auto ConfirmParty(
        OTParty& theParty,
        otx::context::Server& context,
        const PasswordPrompt& reason) -> bool override;  // Takes ownership.
    // Returns true if it was empty (and thus successfully set).
    auto SetNotaryIDIfEmpty(const identifier::Notary& theID) -> bool;

    auto VerifySmartContract(
        const identity::Nym& theNym,
        const Account& theAcct,
        const identity::Nym& theServerNym,
        const PasswordPrompt& reason,
        bool bBurnTransNo = false) -> bool;

    // theNym is trying to activate the smart contract, and has
    // supplied transaction numbers and a user/acct ID. theNym definitely IS the
    // owner of the account... that is
    // verified in Server::NotarizeTransaction(), before it even knows what
    // KIND of transaction it is processing!
    // (For all transactions.) So by the time Server::NotarizeSmartContract()
    // is called, we know that much.
    //
    // But for all other parties, we do not know this, so we still need to loop
    // them all, etc to verify this crap,
    // at least once. (And then maybe I can lessen some of the double-checking,
    // for optimization purposes, once
    // we've run this gamut.)
    //
    // One thing we still do not know, until VerifySmartContract is called, is
    // whether theNym really IS a valid
    // agent for this contract, and whether all the other agents are valid, and
    // whether the accounts are validly
    // owned by the agents they list, and whether the authorizing agent for each
    // party has signed their own copy,
    // and whether the authorizing agent for each party provided a valid opening
    // number--which must be recorded
    // as consumed--and whether the authorized agent for each account provided a
    // valid closing number, which likewise
    // must be recorded.
    //
    // IN THE FUTURE, it should be possible to place restrictions in the
    // contract, enforced by the server,
    // which allow parties to trust additional things such as, XYZ account will
    // only be used for this contract,
    // or ABC party cannot do DEF action without triggering a notice, etc.
    //
    // We call this just before activation (in OT_API::activateSmartContract) in
    // order
    // to make sure that certain IDs and transaction #s are set, so the smart
    // contract
    // will interoperate with the old Cron Item system of doing things.
    //
    void PrepareToActivate(
        const std::int64_t& lOpeningTransNo,
        const std::int64_t& lClosingTransNo,
        const identifier::Nym& theNymID,
        const identifier::Generic& theAcctID);

    //
    // HIGH LEVEL
    //

    // CALLBACKS that OT server uses occasionally. (Smart Contracts can
    // supply a special script that is activated for each callback.)

    //    bool OTScriptable::CanExecuteClause(UnallocatedCString str_party_name,
    // UnallocatedCString str_clause_name); // This calls (if available) the
    // scripted clause: bool party_may_execute_clause(party_name, clause_name)
    auto CanCancelContract(UnallocatedCString str_party_name)
        -> bool;  // This calls (if
                  // available) the
                  // scripted
                  // clause:
    // bool party_may_cancel_contract(party_name)
    // OT NATIVE FUNCTIONS -- Available for scripts to call:

    void SetRemainingTimer(
        UnallocatedCString str_seconds_from_now);  // onProcess
                                                   // will
                                                   // trigger X
                                                   // seconds
                                                   // from
                                                   // now...
                                                   // (And not
                                                   // until
                                                   // then,
                                                   // either.)
    auto GetRemainingTimer() const
        -> UnallocatedCString;  // returns seconds left on the
                                // timer,
                                // in string format, or "0".
    // class member, with string parameter
    auto MoveAcctFundsStr(
        UnallocatedCString from_acct_name,
        UnallocatedCString to_acct_name,
        UnallocatedCString str_Amount)
        -> bool;  // calls OTCronItem::MoveFunds()
    auto StashAcctFunds(
        UnallocatedCString from_acct_name,
        UnallocatedCString to_stash_name,
        UnallocatedCString str_Amount) -> bool;  // calls StashFunds()
    auto UnstashAcctFunds(
        UnallocatedCString to_acct_name,
        UnallocatedCString from_stash_name,
        UnallocatedCString str_Amount)
        -> bool;  // calls StashFunds(lAmount * (-1) )
    auto GetAcctBalance(UnallocatedCString from_acct_name)
        -> UnallocatedCString;
    auto GetStashBalance(
        UnallocatedCString stash_name,
        UnallocatedCString instrument_definition_id) -> UnallocatedCString;

    auto GetUnitTypeIDofAcct(UnallocatedCString from_acct_name)
        -> UnallocatedCString;

    // Todo: someday add "rejection notice" here too.
    // (Might be a demand for smart contracts to send failure notices.)
    // We already send a failure notice to all parties in the cash where
    // the smart contract fails to activate.
    auto SendNoticeToParty(
        UnallocatedCString party_name,
        const PasswordPrompt& reason) -> bool;
    auto SendANoticeToAllParties(const PasswordPrompt& reason) -> bool;

    void DeactivateSmartContract();

    // LOW LEVEL

    // from OTScriptable:
    // (Calls the parent FYI)
    //
    void RegisterOTNativeCallsWithScript(OTScript& theScript) override;

    // Low-level.

    // The STASH:
    // This is where the smart contract can store funds, internally.
    //
    // Done: Have a server backing account to double this record (like with cash
    // withdrawals) so it will turn up properly on an audit.
    //
    auto GetStash(UnallocatedCString str_stash_name) -> OTStash*;

    // Low-level.
    void ExecuteClauses(
        mapOfClauses& theClauses,
        const PasswordPrompt& reason,
        OTString pParam = String::Factory());

    // Low level.
    // This function (StashFunds) is called by StashAcctFunds() and
    // UnstashAcctFunds(),
    // In the same way that OTCronItem::MoveFunds() is called by
    // OTSmartContract::MoveAcctFunds().
    // Therefore this function is lower-level, and the proper way to use it,
    // especially from
    // a script, is to call StashAcctFunds() or UnstashAcctFunds() (BELOW)
    //
    auto StashFunds(
        const std::int64_t& lAmount,  // negative amount here means UNstash.
                                      // Positive
                                      // means STASH.
        const identifier::Generic& PARTY_ACCT_ID,
        const identifier::Nym& PARTY_NYM_ID,
        OTStash& theStash,
        const PasswordPrompt& reason) -> bool;

    void InitSmartContract();

    void Release() override;
    void Release_SmartContract();
    void ReleaseStashes();

    auto IsValidOpeningNumber(const std::int64_t& lOpeningNum) const
        -> bool override;

    auto GetOpeningNumber(const identifier::Nym& theNymID) const
        -> std::int64_t override;
    auto GetClosingNumber(const identifier::Generic& theAcctID) const
        -> std::int64_t override;
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

    void UpdateContents(const PasswordPrompt& reason)
        override;  // Before transmission or
                   // serialization, this is where the
                   // ledger saves its contents

    OTSmartContract() = delete;

    ~OTSmartContract() override;

protected:
    void onActivate(const PasswordPrompt& reason)
        override;  // called by
                   // OTCronItem::HookActivationOnCron().

    void onFinalReceipt(
        OTCronItem& theOrigCronItem,
        const std::int64_t& lNewTransactionNumber,
        Nym_p theOriginator,
        Nym_p pRemover,
        const PasswordPrompt& reason) override;
    void onRemovalFromCron(const PasswordPrompt& reason) override;
    // Above are stored the user and acct IDs of the last sender and recipient
    // of funds.
    // (It's stored there so that the info will be available on receipts.)
    // This function clears those values. Used internally to this class.
    //
    void ReleaseLastSenderRecipientIDs();
    // (These two are lower level, and used by SetNextProcessTime).
    void SetNextProcessDate(const Time tNEXT_DATE)
    {
        next_process_date_ = tNEXT_DATE;
    }
    auto GetNextProcessDate() const -> const Time { return next_process_date_; }

private:
    friend api::session::imp::Factory;

    // In OTSmartContract, none of this normal crap is used.
    // The Sender/Recipient are unused.
    // The Opening and Closing Trans#s are unused.
    //
    // Instead, all that stuff goes through OTParty list (each with agents
    // and accounts) and OTBylaw list (each with clauses and variables.)
    // Todo: convert existing payment plan and markets to use this system since
    // it is much cleaner.
    //
    //    identifier::Generic    recipient_account_id_;
    //    identifier::Generic    recipient_nym_id_;
    // This is where the scripts inside the smart contract can stash money,
    // after it starts operating.
    //
    mapOfStashes stashes_;  // The server will NOT allow any smart contract
                            // to be activated unless these lists are empty.
    // A smart contract may have any number of "stashes" which are stored by
    // name. Each stash can be queried for balance for ANY ASSET TYPE. So stash
    // "alice" might have 5 instrument definitions in it, AND stash "bob" might
    // also have 5 instrument definitions stored in it. The actual accounts
    // where stash funds are stored (so they will turn up properly on an audit.)
    // Assuming that Alice and Bob both use the same instrument definitions,
    // there will be 5 stash accounts here, not 10.  That's because, even if you
    // create a thousand stashes, if they use the same 2 instrument definitions
    // then OT is smart enough here to only create 2 stash accounts. The rest of
    // the information isstored in stashes_, not in the accounts themselves,
    // which are only reserves for those stashes.
    std::unique_ptr<otx::internal::AccountList> stash_accounts_;
    OTString last_sender_user_;     // These four strings are here so that each
                                    // sender or recipient (of a transfer of
                                    // funds)
    OTString last_sender_acct_;     // is clearly saved in each inbox receipt.
                                    // That way, if the receipt has a monetary
                                    // value, then
    OTString last_recipient_user_;  // we know who was sending and who was
                                    // receiving. Also, if a STASH was the
                                    // last action, then
    OTString last_recipient_acct_;  // the sender (or recipient) will be
                                    // blank, signifying that the source or
                                    // destination was a stash.

    // If onProcess() is on a timer (say, to wake up in a week) then this will
    // contain the
    Time next_process_date_;  // date that it WILL be, in a week. (Or
                              // zero.)

    // For moving money from one nym's account to another.
    // it is also nearly identically copied in OTPaymentPlan.
    auto MoveFunds(
        const std::int64_t& lAmount,
        const identifier::Generic& SOURCE_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const identifier::Generic& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID,
        const PasswordPrompt& reason) -> bool;

    OTSmartContract(const api::Session& api);
    OTSmartContract(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID);
};
}  // namespace opentxs
