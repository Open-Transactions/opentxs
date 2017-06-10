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

#ifndef OPENTXS_CORE_SCRIPT_OTSMARTCONTRACT_HPP
#define OPENTXS_CORE_SCRIPT_OTSMARTCONTRACT_HPP

#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/AccountList.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"

#include <stdint.h>
#include <map>
#include <set>
#include <string>

namespace opentxs
{

class Account;
class Nym;
class NumList;
class OTParty;
class OTScript;
class OTStash;

typedef std::map<std::string, Account*> mapOfAccounts;
typedef std::map<std::string, OTStash*> mapOfStashes;

class OTSmartContract : public OTCronItem
{
private: // Private prevents erroneous use by other classes.
    typedef OTCronItem ot_super;

private:
    // In OTSmartContract, none of this normal crap is used.
    // The Sender/Recipient are unused.
    // The Opening and Closing Trans#s are unused.
    //
    // Instead, all that stuff goes through OTParty list (each with agents
    // and accounts) and OTBylaw list (each with clauses and variables.)
    // Todo: convert existing payment plan and markets to use this system since
    // it is much cleaner.
    //
    //    OTIdentifier    m_RECIPIENT_ACCT_ID;
    //    OTIdentifier    m_RECIPIENT_NYM_ID;
    // This is where the scripts inside the smart contract can stash money,
    // after it starts operating.
    //
    mapOfStashes m_mapStashes; // The server will NOT allow any smart contract
                               // to be activated unless these lists are empty.
    // A smart contract may have any number of "stashes" which are stored by
    // name. Each stash
    // can be queried for balance for ANY ASSET TYPE. So stash "alice" might
    // have 5 instrument definitions
    // in it, AND stash "bob" might also have 5 instrument definitions stored in
    // it.
    AccountList m_StashAccts; // The actual accounts where stash funds are
                              // stored
                              // (so they will turn up properly on an audit.)
    // Assuming that Alice and Bob both use the same instrument definitions,
    // there will be
    // 5 stash accounts here,
    // not 10.  That's because, even if you create a thousand stashes, if they
    // use the same 2 instrument definitions
    // then OT is smart enough here to only create 2 stash accounts. The rest of
    // the information is
    // stored in m_mapStashes, not in the accounts themselves, which are only
    // reserves for those stashes.
    String m_strLastSenderUser;    // These four strings are here so that each
                                   // sender or recipient (of a transfer of
                                   // funds)
    String m_strLastSenderAcct;    // is clearly saved in each inbox receipt.
                                   // That way, if the receipt has a monetary
                                   // value, then
    String m_strLastRecipientUser; // we know who was sending and who was
                                   // receiving. Also, if a STASH was the last
                                   // action, then
    String m_strLastRecipientAcct; // the sender (or recipient) will be blank,
                                   // signifying that the source or
                                   // destination was a stash.

    // If onProcess() is on a timer (say, to wake up in a week) then this will
    // contain the
    time64_t m_tNextProcessDate{0}; // date that it WILL be, in a week. (Or zero.)

    // For moving money from one nym's account to another.
    // it is also nearly identically copied in OTPaymentPlan.
    bool MoveFunds(const mapOfConstNyms& map_NymsAlreadyLoaded,
                   const int64_t& lAmount, const Identifier& SOURCE_ACCT_ID,
                   const Identifier& SENDER_NYM_ID,
                   const Identifier& RECIPIENT_ACCT_ID,
                   const Identifier& RECIPIENT_NYM_ID);

protected:
    void onActivate() override; // called by OTCronItem::HookActivationOnCron().

    void onFinalReceipt(OTCronItem& theOrigCronItem,
                                const int64_t& lNewTransactionNumber,
                                Nym& theOriginator, Nym* pRemover) override;
    void onRemovalFromCron() override;
    // Above are stored the user and acct IDs of the last sender and recipient
    // of funds.
    // (It's stored there so that the info will be available on receipts.)
    // This function clears those values. Used internally to this class.
    //
    void ReleaseLastSenderRecipientIDs();
    // (These two are lower level, and used by SetNextProcessTime).
    void SetNextProcessDate(const time64_t& tNEXT_DATE)
    {
        m_tNextProcessDate = tNEXT_DATE;
    }
    const time64_t& GetNextProcessDate() const
    {
        return m_tNextProcessDate;
    }

public:
    originType GetOriginType() const override
    { return originType::origin_smart_contract; }

    void SetDisplayLabel(const std::string* pstrLabel = nullptr) override;
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
    const String& GetLastSenderNymID() const
    {
        return m_strLastSenderUser;
    }
    const String& GetLastSenderAcctID() const
    {
        return m_strLastSenderAcct;
    }
    const String& GetLastRecipientNymID() const
    {
        return m_strLastRecipientUser;
    }
    const String& GetLastRecipientAcctID() const
    {
        return m_strLastRecipientAcct;
    }
    int32_t GetCountStashes() const;
    int32_t GetCountStashAccts() const;
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
    bool Compare(OTScriptable& rhs) const override;
    // From OTCronItem (parent class of this)
    bool CanRemoveItemFromCron(const ClientContext& context) override;

    void HarvestOpeningNumber(ServerContext& context) override;
    void HarvestClosingNumbers(ServerContext& context) override;

    // Server-side. Similar to below:
    void CloseoutOpeningNumbers();
    using ot_super::HarvestClosingNumbers;
    void HarvestClosingNumbers(Nym* pSignerNym = nullptr,
                               std::set<OTParty*>* pFailedParties =
                                   nullptr); // Used on server-side. Assumes the
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
    bool ProcessCron() override; // OTCron calls this regularly, which is my
                                // chance to expire, etc.

    bool HasTransactionNum(const int64_t& lInput) const override;
    void GetAllTransactionNumbers(NumList& numlistOutput) const override;

    bool AddParty(OTParty& theParty) override; // Takes ownership.
    bool ConfirmParty(
        OTParty& theParty,
        ServerContext& context) override; // Takes ownership.
    // Returns true if it was empty (and thus successfully set).
    EXPORT bool SetNotaryIDIfEmpty(const Identifier& theID);

    EXPORT bool VerifySmartContract(Nym& theNym, Account& theAcct,
                                    Nym& theServerNym,
                                    bool bBurnTransNo = false);

    // theNym is trying to activate the smart contract, and has
    // supplied transaction numbers and a user/acct ID. theNym definitely IS the
    // owner of the account... that is
    // verified in OTServer::NotarizeTransaction(), before it even knows what
    // KIND of transaction it is processing!
    // (For all transactions.) So by the time OTServer::NotarizeSmartContract()
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
    EXPORT void PrepareToActivate(const int64_t& lOpeningTransNo,
                                  const int64_t& lClosingTransNo,
                                  const Identifier& theNymID,
                                  const Identifier& theAcctID);

    //
    // HIGH LEVEL
    //

    // CALLBACKS that OT server uses occasionally. (Smart Contracts can
    // supply a special script that is activated for each callback.)

    //    bool OTScriptable::CanExecuteClause(std::string str_party_name,
    // std::string str_clause_name); // This calls (if available) the
    // scripted clause: bool party_may_execute_clause(party_name, clause_name)
    bool CanCancelContract(std::string str_party_name); // This calls (if
                                                        // available) the
                                                        // scripted
                                                        // clause:
    // bool party_may_cancel_contract(party_name)
    // OT NATIVE FUNCTIONS -- Available for scripts to call:

    void SetRemainingTimer(std::string str_seconds_from_now); // onProcess
                                                              // will
                                                              // trigger X
                                                              // seconds
                                                              // from
                                                              // now...
                                                              // (And not
                                                              // until
                                                              // then,
                                                              // either.)
    std::string GetRemainingTimer() const; // returns seconds left on the timer,
                                           // in string format, or "0".
    // class member, with string parameter
    bool MoveAcctFundsStr(std::string from_acct_name, std::string to_acct_name,
                          std::string str_Amount); // calls
                                                   // OTCronItem::MoveFunds()
    bool StashAcctFunds(std::string from_acct_name, std::string to_stash_name,
                        std::string str_Amount); // calls StashFunds()
    bool UnstashAcctFunds(std::string to_acct_name, std::string from_stash_name,
                          std::string str_Amount); // calls StashFunds(
                                                   // lAmount * (-1) )
    std::string GetAcctBalance(std::string from_acct_name);
    std::string GetStashBalance(std::string stash_name,
                                std::string instrument_definition_id);

    std::string GetInstrumentDefinitionIDofAcct(std::string from_acct_name);

    // Todo: someday add "rejection notice" here too.
    // (Might be a demand for smart contracts to send failure notices.)
    // We already send a failure notice to all parties in the cash where
    // the smart contract fails to activate.
    bool SendNoticeToParty(std::string party_name);
    bool SendANoticeToAllParties();

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
    OTStash* GetStash(std::string str_stash_name);

    // Low-level.
    EXPORT void ExecuteClauses(mapOfClauses& theClauses,
                               String* pParam = nullptr);

    // Low level.
    // This function (StashFunds) is called by StashAcctFunds() and
    // UnstashAcctFunds(),
    // In the same way that OTCronItem::MoveFunds() is called by
    // OTSmartContract::MoveAcctFunds().
    // Therefore this function is lower-level, and the proper way to use it,
    // especially from
    // a script, is to call StashAcctFunds() or UnstashAcctFunds() (BELOW)
    //
    EXPORT bool StashFunds(
        const mapOfConstNyms& map_NymsAlreadyLoaded,
        const int64_t& lAmount, // negative amount here means UNstash. Positive
                                // means STASH.
        const Identifier& PARTY_ACCT_ID,
        const Identifier& PARTY_NYM_ID, OTStash& theStash);
    EXPORT OTSmartContract();
    EXPORT OTSmartContract(const Identifier& NOTARY_ID);

    EXPORT virtual ~OTSmartContract();

    void InitSmartContract();

    void Release() override;
    void Release_SmartContract();
    void ReleaseStashes();

    static void CleanupNyms(mapOfConstNyms& theMap);
    static void CleanupAccts(mapOfAccounts& theMap);
    bool IsValidOpeningNumber(const int64_t& lOpeningNum) const override;

    int64_t GetOpeningNumber(const Identifier& theNymID) const override;
    int64_t GetClosingNumber(const Identifier& theAcctID) const override;
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

    void UpdateContents() override; // Before transmission or serialization, this
                                   // is where the ledger saves its contents
};

} // namespace opentxs

#endif // OPENTXS_CORE_SCRIPT_OTSMARTCONTRACT_HPP
