// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <memory>
#include <tuple>

#include "internal/otx/Types.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
class Server;
}  // namespace context
}  // namespace otx

class Account;
class Item;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
// transaction ID is a std::int64_t, assigned by the server. Each transaction
// has one. FIRST the server issues the ID. THEN we create the blank transaction
// object with the ID in it and store it in our inbox. THEN if we want to send a
// transaction, we use the blank to do so. If there is no blank available, we
// message the server and request one.
using mapOfTransactions =
    UnallocatedMap<TransactionNumber, std::shared_ptr<OTTransaction>>;

// the "inbox" and "outbox" functionality is implemented in this class
class Ledger : public OTTransactionType
{
public:
    ledgerType type_;
    // So the server can tell if it just loaded a legacy box or a hashed box.
    // (Legacy boxes stored ALL of the receipts IN the box. No more.)
    bool loaded_legacy_data_;

    inline auto GetType() const -> ledgerType { return type_; }

    auto LoadedLegacyData() const -> bool { return loaded_legacy_data_; }

    // This function assumes that this is an INBOX.
    // If you don't use an INBOX to call this method, then it will return
    // nullptr immediately. If you DO use an inbox, then it will create a
    // balanceStatement item to go onto your transaction.  (Transactions require
    // balance statements. And when you get the atBalanceStatement reply from
    // the server, KEEP THAT RECEIPT. Well, OT will do that for you.) You only
    // have to keep the latest receipt, unlike systems that don't store balance
    // agreement.  We also store a list of issued transactions, the new balance,
    // and the outbox hash.
    auto GenerateBalanceStatement(
        const Amount& lAdjustment,
        const OTTransaction& theOwner,
        const otx::context::Server& context,
        const Account& theAccount,
        Ledger& theOutbox,
        const PasswordPrompt& reason) const -> std::unique_ptr<Item>;
    auto GenerateBalanceStatement(
        const Amount& lAdjustment,
        const OTTransaction& theOwner,
        const otx::context::Server& context,
        const Account& theAccount,
        Ledger& theOutbox,
        const UnallocatedSet<TransactionNumber>& without,
        const PasswordPrompt& reason) const -> std::unique_ptr<Item>;

    void ProduceOutboxReport(
        Item& theBalanceItem,
        const PasswordPrompt& reason);

    auto AddTransaction(std::shared_ptr<OTTransaction> theTransaction) -> bool;
    auto RemoveTransaction(const TransactionNumber number)
        -> bool;  // if false,
                  // transaction
                  // wasn't
                  // found.

    auto GetTransactionNums(
        const UnallocatedSet<std::int32_t>* pOnlyForIndices = nullptr) const
        -> UnallocatedSet<std::int64_t>;

    auto GetTransaction(transactionType theType)
        -> std::shared_ptr<OTTransaction>;
    auto GetTransaction(const TransactionNumber number) const
        -> std::shared_ptr<OTTransaction>;
    auto GetTransactionByIndex(std::int32_t nIndex) const
        -> std::shared_ptr<OTTransaction>;
    auto GetFinalReceipt(std::int64_t lReferenceNum)
        -> std::shared_ptr<OTTransaction>;
    auto GetTransferReceipt(std::int64_t lNumberOfOrigin)
        -> std::shared_ptr<OTTransaction>;
    auto GetChequeReceipt(std::int64_t lChequeNum)
        -> std::shared_ptr<OTTransaction>;
    auto GetTransactionIndex(const TransactionNumber number)
        -> std::int32_t;  // if not
                          // found,
                          // returns
                          // -1
    auto GetReplyNotice(const std::int64_t& lRequestNum)
        -> std::shared_ptr<OTTransaction>;

    // This calls OTTransactionType::VerifyAccount(), which calls
    // VerifyContractID() as well as VerifySignature().
    //
    // But first, this OTLedger version also loads the box receipts,
    // if doing so is appropriate. (message ledger == not appropriate.)
    //
    // Use this method instead of Contract::VerifyContract, which
    // expects/uses a pubkey from inside the contract in order to verify
    // it.
    //
    auto VerifyAccount(const identity::Nym& theNym) -> bool override;
    // For ALL abbreviated transactions, load the actual box receipt for each.
    auto LoadBoxReceipts(UnallocatedSet<std::int64_t>* psetUnloaded = nullptr)
        -> bool;                     // if psetUnloaded
                                     // passed
                                     // in, then use it to
                                     // return the #s that
                                     // weren't there.
    auto SaveBoxReceipts() -> bool;  // For all "full version"
                                     // transactions, save the actual box
                                     // receipt for each.
    // Verifies the abbreviated form exists first, and then loads the
    // full version and compares the two. Returns success / fail.
    //
    auto LoadBoxReceipt(const std::int64_t& lTransactionNum) -> bool;
    // Saves the Box Receipt separately.
    auto SaveBoxReceipt(const std::int64_t& lTransactionNum) -> bool;
    // "Deletes" it by adding MARKED_FOR_DELETION to the bottom of the file.
    auto DeleteBoxReceipt(const std::int64_t& lTransactionNum) -> bool;

    auto LoadInbox() -> bool;
    auto LoadNymbox() -> bool;
    auto LoadOutbox() -> bool;

    // If you pass the identifier in, the hash is recorded there
    auto SaveInbox() -> bool;
    auto SaveInbox(identifier::Generic& pInboxHash) -> bool;
    auto SaveNymbox() -> bool;
    auto SaveNymbox(identifier::Generic& pNymboxHash) -> bool;
    auto SaveOutbox() -> bool;
    auto SaveOutbox(identifier::Generic& pOutboxHash) -> bool;

    auto CalculateHash(identifier::Generic& theOutput) const -> bool;
    auto CalculateInboxHash(identifier::Generic& theOutput) const -> bool;
    auto CalculateOutboxHash(identifier::Generic& theOutput) const -> bool;
    auto CalculateNymboxHash(identifier::Generic& theOutput) const -> bool;
    auto SavePaymentInbox() -> bool;
    auto LoadPaymentInbox() -> bool;

    auto SaveRecordBox() -> bool;
    auto LoadRecordBox() -> bool;

    auto SaveExpiredBox() -> bool;
    auto LoadExpiredBox() -> bool;
    auto LoadLedgerFromString(const String& theStr) -> bool;  // Auto-detects
                                                              // ledger
                                                              // type.
    auto LoadInboxFromString(const String& strBox) -> bool;
    auto LoadOutboxFromString(const String& strBox) -> bool;
    auto LoadNymboxFromString(const String& strBox) -> bool;
    auto LoadPaymentInboxFromString(const String& strBox) -> bool;
    auto LoadRecordBoxFromString(const String& strBox) -> bool;
    auto LoadExpiredBoxFromString(const String& strBox) -> bool;
    // inline for the top one only.
    inline auto GetTransactionCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(transactions_.size());
    }
    auto GetTransactionCountInRefTo(std::int64_t lReferenceNum) const
        -> std::int32_t;
    auto GetTotalPendingValue(const PasswordPrompt& reason)
        -> Amount;  // for inbox only, allows you
                    // to
    // lookup the total value of pending
    // transfers within.
    auto GetTransactionMap() const -> const mapOfTransactions&;

    void Release() override;
    void Release_Ledger();

    void ReleaseTransactions();
    // ONLY call this if you need to load a ledger where you don't already know
    // the person's NymID
    // For example, if you need to load someone ELSE's inbox in order to send
    // them a transfer, then
    // you only know their account number, not their user ID. So you call this
    // function to get it
    // loaded up, and the NymID will hopefully be loaded up with the rest of
    // it.
    void InitLedger();

    [[deprecated]] auto GenerateLedger(
        const identifier::Generic& theAcctID,
        const identifier::Notary& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) -> bool;
    auto CreateLedger(
        const identifier::Nym& theNymID,
        const identifier::Generic& theAcctID,
        const identifier::Notary& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) -> bool;

    static auto GetTypeString(ledgerType theType) -> char const*;
    auto GetTypeString() const -> char const* { return GetTypeString(type_); }

    Ledger() = delete;

    ~Ledger() override;

protected:
    auto LoadGeneric(
        ledgerType theType,
        const String& pString = String::Factory()) -> bool;
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;
    auto SaveGeneric(ledgerType theType) -> bool;
    void UpdateContents(const PasswordPrompt& reason)
        override;  // Before transmission or
                   // serialization, this is where the
                   // ledger saves its contents

private:  // Private prevents erroneous use by other classes.
    friend api::session::imp::Factory;

    using ot_super = OTTransactionType;

    mapOfTransactions transactions_;  // a ledger contains a map of
                                      // transactions.

    auto make_filename(const ledgerType theType) -> std::
        tuple<bool, UnallocatedCString, UnallocatedCString, UnallocatedCString>;

    auto generate_ledger(
        const identifier::Nym& theNymID,
        const identifier::Generic& theAcctID,
        const identifier::Notary& theNotaryID,
        ledgerType theType,
        bool bCreateFile) -> bool;
    auto save_box(
        const ledgerType type,
        identifier::Generic& hash,
        bool (Ledger::*calc)(identifier::Generic&) const) -> bool;

    Ledger(const api::Session& api);
    Ledger(
        const api::Session& api,
        const identifier::Generic& theAccountID,
        const identifier::Notary& theNotaryID);
    Ledger(
        const api::Session& api,
        const identifier::Nym& theNymID,
        const identifier::Generic& theAccountID,
        const identifier::Notary& theNotaryID);
};
}  // namespace opentxs
