// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_ACCOUNT_HPP
#define OPENTXS_CORE_ACCOUNT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"

#include <cstdint>

namespace opentxs
{
namespace api
{
namespace implementation
{
class Wallet;
}  // namespace implementation

namespace server
{
namespace implementation
{
class Wallet;
}  // namespace implementation
}  // namespace server
}  // namespace api

class Account : public OTTransactionType
{
public:
    // If you add any types to this list, update the list of strings at the
    // top of the .cpp file.
    enum AccountType {
        user,       // used by users
        issuer,     // used by issuers    (these can only go negative.)
        basket,     // issuer acct used by basket currencies (these can only go
                    // negative)
        basketsub,  // used by the server (to store backing reserves for basket
                    // sub-accounts)
        mint,       // used by mints (to store backing reserves for cash)
        voucher,    // used by the server (to store backing reserves for
                    // vouchers)
        stash,  // used by the server (to store backing reserves for stashes,
                // for smart contracts.)
        err_acct
    };

    EXPORT static char const* _GetTypeString(AccountType accountType);

    EXPORT std::string Alias() const;
    EXPORT bool ConsensusHash(const Context& context, Identifier& theOutput)
        const;
    EXPORT bool DisplayStatistics(String& contents) const override;
    EXPORT Amount GetBalance() const;
    EXPORT const Identifier& GetInstrumentDefinitionID() const;
    EXPORT TransactionNumber GetStashTransNum() const { return stashTransNum_; }
    EXPORT char const* GetTypeString() const
    {
        return _GetTypeString(acctType_);
    }
    EXPORT bool IsAllowedToGoNegative() const;
    EXPORT bool IsInternalServerAcct() const;
    EXPORT bool IsOwnedByUser() const;
    EXPORT bool IsOwnedByEntity() const;
    EXPORT bool IsIssuer() const;
    // For accounts used by smart contracts, to stash funds while running.
    EXPORT bool IsStashAcct() const { return (acctType_ == stash); }
    EXPORT std::unique_ptr<Ledger> LoadInbox(const Nym& nym) const;
    EXPORT std::unique_ptr<Ledger> LoadOutbox(const Nym& nym) const;
    // Compares the NymID loaded from the account file with whatever Nym the
    // programmer wants to verify.
    EXPORT bool VerifyOwner(const Nym& candidate) const;
    EXPORT bool VerifyOwnerByID(const identifier::Nym& nymId) const;

    // Debit a certain amount from the account (presumably the same amount is
    // being added somewhere)
    EXPORT bool Debit(const Amount amount);
    // Credit a certain amount from the account (presumably the same amount is
    // being subtracted somewhere)
    EXPORT bool Credit(const Amount amount);
    EXPORT bool GetInboxHash(Identifier& output);
    EXPORT bool GetOutboxHash(Identifier& output);
    EXPORT bool InitBoxes(const Nym& signer);
    // If you pass the identifier in, the inbox hash is recorded there
    EXPORT bool SaveInbox(Ledger& box);
    EXPORT bool SaveInbox(Ledger& box, Identifier& hash);
    // If you pass the identifier in, the outbox hash is recorded there
    EXPORT bool SaveOutbox(Ledger& box);
    EXPORT bool SaveOutbox(Ledger& box, Identifier& hash);
    EXPORT void SetAlias(const std::string& alias);
    EXPORT void SetInboxHash(const Identifier& input);
    EXPORT void SetOutboxHash(const Identifier& input);
    EXPORT void SetStashTransNum(const TransactionNumber transNum)
    {
        stashTransNum_ = transNum;
    }

    EXPORT virtual ~Account() override;

private:
    friend OTWallet;
    friend opentxs::api::implementation::Wallet;
    friend opentxs::api::server::implementation::Wallet;

    AccountType acctType_{err_acct};
    // These are all the variables from the account file itself.
    OTIdentifier acctInstrumentDefinitionID_;
    OTString balanceDate_;
    OTString balanceAmount_;
    // the Transaction Number of a smart contract running on cron, if this is a
    // stash account.
    TransactionNumber stashTransNum_{0};
    // Default FALSE. When set to true, saves a "DELETED" flag with this Account
    bool markForDeletion_{false};
    // for easy cleanup later when the server is doing some maintenance.
    // Hash of this account's Inbox, so we don't download it more often than
    // necessary.
    OTIdentifier inboxHash_;
    // Hash of this account's Outbox, so we don't download it more often than
    // necessary.
    OTIdentifier outboxHash_;
    std::string alias_;

    static Account* GenerateNewAccount(
        const api::Core& core,
        const identifier::Nym& nymID,
        const Identifier& notaryID,
        const Nym& serverNym,
        const identifier::Nym& userNymID,
        const Identifier& instrumentDefinitionID,
        AccountType acctType = user,
        TransactionNumber stashTransNum = 0);
    // Let's say you don't have or know the NymID, and you just want to load
    // the damn thing up. Then call this function. It will set nymID for you.
    static Account* LoadExistingAccount(
        const api::Core& core,
        const Identifier& accountId,
        const Identifier& notaryID);

    bool SaveContractWallet(Tag& parent) const override;

    bool create_box(
        std::unique_ptr<Ledger>& box,
        const Nym& signer,
        const ledgerType type);
    bool GenerateNewAccount(
        const Nym& server,
        const identifier::Nym& userNymID,
        const Identifier& notaryID,
        const Identifier& instrumentDefinitionID,
        AccountType acctType = user,
        std::int64_t stashTransNum = 0);
    void InitAccount();
    // overriding this so I can set filename automatically inside based on ID.
    bool LoadContract() override;
    bool LoadContractFromString(const String& theStr) override;
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;
    void Release() override;
    void Release_Account();
    // generates filename based on accounts path and account ID. Saves to the
    // standard location for an acct.
    bool SaveAccount();

    bool save_box(
        Ledger& box,
        Identifier& hash,
        bool (Ledger::*save)(Identifier&),
        void (Account::*set)(const Identifier&));

    void UpdateContents() override;

    Account(
        const api::Core& core,
        const identifier::Nym& nymID,
        const Identifier& accountId,
        const Identifier& notaryID,
        const String& name);
    Account(
        const api::Core& core,
        const identifier::Nym& nymID,
        const Identifier& accountId,
        const Identifier& notaryID);
    Account(
        const api::Core& core,
        const identifier::Nym& nymID,
        const Identifier& notaryID);
    Account(const api::Core& core);
    Account() = delete;
};
}  // namespace opentxs
#endif
