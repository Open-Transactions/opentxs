// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::ledgerType

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <memory>

#include "internal/core/String.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace imp
{
class Wallet;
}  // namespace imp

namespace notary
{
class Wallet;
}  // namespace notary
}  // namespace session

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
class Base;
}  // namespace context
}  // namespace otx

class Account;
class Ledger;
class OTWallet;
class PasswordPrompt;
class Tag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
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

    static auto GetTypeString(AccountType accountType) -> char const*;

    auto Alias() const -> UnallocatedCString;
    auto ConsensusHash(
        const otx::context::Base& context,
        identifier::Generic& theOutput,
        const PasswordPrompt& reason) const -> bool;
    auto DisplayStatistics(String& contents) const -> bool override;
    auto GetBalance() const -> Amount;
    auto GetInstrumentDefinitionID() const -> const identifier::UnitDefinition&;
    auto GetStashTransNum() const -> TransactionNumber
    {
        return stash_trans_num_;
    }
    auto GetTypeString() const -> char const*
    {
        return GetTypeString(acct_type_);
    }
    auto IsAllowedToGoNegative() const -> bool;
    auto IsInternalServerAcct() const -> bool;
    auto IsOwnedByUser() const -> bool;
    auto IsOwnedByEntity() const -> bool;
    auto IsIssuer() const -> bool;
    // For accounts used by smart contracts, to stash funds while running.
    auto IsStashAcct() const -> bool { return (acct_type_ == stash); }
    auto LoadInbox(const identity::Nym& nym) const -> std::unique_ptr<Ledger>;
    auto LoadOutbox(const identity::Nym& nym) const -> std::unique_ptr<Ledger>;
    // Compares the NymID loaded from the account file with whatever Nym the
    // programmer wants to verify.
    auto VerifyOwner(const identity::Nym& candidate) const -> bool;
    auto VerifyOwnerByID(const identifier::Nym& nymId) const -> bool;

    // Debit a certain amount from the account (presumably the same amount is
    // being added somewhere)
    auto Debit(const Amount& amount) -> bool;
    // Credit a certain amount from the account (presumably the same amount is
    // being subtracted somewhere)
    auto Credit(const Amount& amount) -> bool;
    auto GetInboxHash(identifier::Generic& output) -> bool;
    auto GetOutboxHash(identifier::Generic& output) -> bool;
    auto InitBoxes(const identity::Nym& signer, const PasswordPrompt& reason)
        -> bool;
    // If you pass the identifier in, the inbox hash is recorded there
    auto SaveInbox(Ledger& box) -> bool;
    auto SaveInbox(Ledger& box, identifier::Generic& hash) -> bool;
    // If you pass the identifier in, the outbox hash is recorded there
    auto SaveOutbox(Ledger& box) -> bool;
    auto SaveOutbox(Ledger& box, identifier::Generic& hash) -> bool;
    void SetAlias(const UnallocatedCString& alias);
    void SetInboxHash(const identifier::Generic& input);
    void SetOutboxHash(const identifier::Generic& input);
    void SetStashTransNum(const TransactionNumber transNum)
    {
        stash_trans_num_ = transNum;
    }

    Account() = delete;

    ~Account() override;

private:
    friend OTWallet;
    friend opentxs::api::session::imp::Wallet;
    friend opentxs::api::session::notary::Wallet;

    AccountType acct_type_{err_acct};
    // These are all the variables from the account file itself.
    identifier::UnitDefinition acct_instrument_definition_id_;
    OTString balance_date_;
    OTString balance_amount_;
    // the Transaction Number of a smart contract running on cron, if this is a
    // stash account.
    TransactionNumber stash_trans_num_{0};
    // Default FALSE. When set to true, saves a "DELETED" flag with this Account
    bool mark_for_deletion_{false};
    // for easy cleanup later when the server is doing some maintenance.
    // Hash of this account's Inbox, so we don't download it more often than
    // necessary.
    identifier::Generic inbox_hash_;
    // Hash of this account's Outbox, so we don't download it more often than
    // necessary.
    identifier::Generic outbox_hash_;
    UnallocatedCString alias_;

    static auto GenerateNewAccount(
        const api::Session& api,
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID,
        const identity::Nym& serverNym,
        const identifier::Nym& userNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const PasswordPrompt& reason,
        AccountType acctType = user,
        TransactionNumber stashTransNum = 0) -> Account*;
    // Let's say you don't have or know the NymID, and you just want to load
    // the damn thing up. Then call this function. It will set nymID for you.
    static auto LoadExistingAccount(
        const api::Session& api,
        const identifier::Generic& accountId,
        const identifier::Notary& notaryID) -> Account*;

    auto SaveContractWallet(Tag& parent) const -> bool override;

    auto create_box(
        std::unique_ptr<Ledger>& box,
        const identity::Nym& signer,
        const ledgerType type,
        const PasswordPrompt& reason) -> bool;
    auto GenerateNewAccount(
        const identity::Nym& server,
        const identifier::Nym& userNymID,
        const identifier::Notary& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const PasswordPrompt& reason,
        AccountType acctType = user,
        std::int64_t stashTransNum = 0) -> bool;
    void InitAccount();
    // overriding this so I can set filename automatically inside based on ID.
    auto LoadContract() -> bool override;
    auto LoadContractFromString(const String& theStr) -> bool override;
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;
    void Release() override;
    void Release_Account();
    // generates filename based on accounts path and account ID. Saves to the
    // standard location for an acct.
    auto SaveAccount() -> bool;

    auto save_box(
        Ledger& box,
        identifier::Generic& hash,
        bool (Ledger::*save)(identifier::Generic&),
        void (Account::*set)(const identifier::Generic&)) -> bool;

    void UpdateContents(const PasswordPrompt& reason) override;

    Account(
        const api::Session& api,
        const identifier::Nym& nymID,
        const identifier::Generic& accountId,
        const identifier::Notary& notaryID,
        const String& name);
    Account(
        const api::Session& api,
        const identifier::Nym& nymID,
        const identifier::Generic& accountId,
        const identifier::Notary& notaryID);
    Account(
        const api::Session& api,
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID);
    Account(const api::Session& api);
};
}  // namespace opentxs
