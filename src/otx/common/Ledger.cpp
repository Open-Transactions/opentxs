// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/Ledger.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <utility>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/transaction/Helpers.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/otx/consensus/TransactionStatement.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "otx/common/OTStorage.hpp"

namespace opentxs
{
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
char const* const TypeStringsLedger[] = {
    "nymbox",  // the nymbox is per user account (versus per asset account) and
               // is used to receive new transaction numbers (and messages.)
    "inbox",  // each asset account has an inbox, with pending transfers as well
              // as receipts inside.
    "outbox",   // if you SEND a pending transfer, it sits in your outbox until
                // it's accepted, rejected, or canceled.
    "message",  // used in OTMessages, to send various lists of transactions
                // back
                // and forth.
    "paymentInbox",  // Used for client-side-only storage of incoming cheques,
    // invoices, payment plan requests, etc. (Coming in from the
    // Nymbox.)
    "recordBox",   // Used for client-side-only storage of completed items from
                   // the inbox, and the paymentInbox.
    "expiredBox",  // Used for client-side-only storage of expired items from
                   // the
                   // paymentInbox.
    "error_state"};

// ID refers to account ID.
// Since a ledger is normally used as an inbox for a specific account, in a
// specific file, then I've decided to restrict ledgers to a single account.
Ledger::Ledger(
    const api::Session& api,
    const identifier::Nym& theNymID,
    const identifier::Account& theAccountID,
    const identifier::Notary& theNotaryID)
    : OTTransactionType(api, theNymID, theAccountID, theNotaryID)
    , type_(ledgerType::message)
    , loaded_legacy_data_(false)
    , transactions_()
{
    InitLedger();
}

// ONLY call this if you need to load a ledger where you don't already know the
// person's NymID For example, if you need to load someone ELSE's inbox in
// order to send them a transfer, then you only know their account number, not
// their user ID. So you call this function to get it loaded up, and the NymID
// will hopefully be loaded up with the rest of it.
Ledger::Ledger(
    const api::Session& api,
    const identifier::Account& theAccountID,
    const identifier::Notary& theNotaryID)
    : OTTransactionType(api)
    , type_(ledgerType::message)
    , loaded_legacy_data_(false)
    , transactions_()
{
    InitLedger();
    SetRealAccountID(theAccountID);
    SetRealNotaryID(theNotaryID);
}

// This is private now and hopefully will stay that way.
Ledger::Ledger(const api::Session& api)
    : OTTransactionType(api)
    , type_(ledgerType::message)
    , loaded_legacy_data_(false)
    , transactions_()
{
    InitLedger();
}

auto Ledger::GetTypeString(ledgerType theType) -> char const*
{
    auto nType = static_cast<std::int32_t>(theType);
    return TypeStringsLedger[nType];
}

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
auto Ledger::VerifyAccount(const identity::Nym& theNym) -> bool
{
    switch (GetType()) {
        case ledgerType::message: {  // message ledgers do not load Box
                                     // Receipts. (They store full version
                                     // internally already.)
        } break;
        case ledgerType::nymbox:
        case ledgerType::inbox:
        case ledgerType::outbox:
        case ledgerType::paymentInbox:
        case ledgerType::recordBox:
        case ledgerType::expiredBox: {
            UnallocatedSet<std::int64_t> setUnloaded;
            LoadBoxReceipts(&setUnloaded);  // Note: Also useful for
                                            // suppressing errors here.
        } break;
        case ledgerType::error_state:
        default: {
            const auto nLedgerType = static_cast<std::int32_t>(GetType());
            const auto& theNymID = theNym.ID();
            const auto strNymID = String::Factory(theNymID, api_.Crypto());
            auto strAccountID = String::Factory();
            GetIdentifier(strAccountID);
            LogError()(OT_PRETTY_CLASS())("Failure: Bad ledger type: ")(
                nLedgerType)(", NymID: ")(strNymID.get())(", AcctID: ")(
                strAccountID.get())(".")
                .Flush();

            return false;
        }
    }

    return OTTransactionType::VerifyAccount(theNym);
}

// This makes sure that ALL transactions inside the ledger are saved as box
// receipts
// in their full (not abbreviated) form (as separate files.)
//
auto Ledger::SaveBoxReceipts()
    -> bool  // For ALL full transactions, save the actual
             // box receipt for each to its own place.
{
    bool bRetVal = true;
    for (auto& [number, pTransaction] : transactions_) {
        OT_ASSERT(pTransaction);

        // We only save full versions of transactions as box receipts, not
        // abbreviated versions.
        // (If it's not abbreviated, therefore it's the full version.)
        //
        if (!pTransaction->IsAbbreviated()) {  // This way we won't see an
                                               // error if it's not
                                               // abbreviated.
            bRetVal = pTransaction->SaveBoxReceipt(*this);
        }

        if (!bRetVal) {
            LogError()(OT_PRETTY_CLASS())("Failed calling SaveBoxReceipt "
                                          "on transaction: ")(number)(".")
                .Flush();
            break;
        }
    }
    return bRetVal;
}

auto Ledger::SaveBoxReceipt(const std::int64_t& lTransactionNum) -> bool
{

    // First, see if the transaction itself exists on this ledger.
    // Get a pointer to it.
    auto pTransaction = GetTransaction(lTransactionNum);

    if (false == bool(pTransaction)) {
        LogConsole()(OT_PRETTY_CLASS())("Unable to save box receipt ")(
            lTransactionNum)(": couldn't find the transaction on this ledger.")
            .Flush();
        return false;
    }

    return pTransaction->SaveBoxReceipt(*this);
}

auto Ledger::DeleteBoxReceipt(const std::int64_t& lTransactionNum) -> bool
{

    // First, see if the transaction itself exists on this ledger.
    // Get a pointer to it.
    auto pTransaction = GetTransaction(lTransactionNum);

    if (false == bool(pTransaction)) {
        LogConsole()(OT_PRETTY_CLASS())("Unable to delete (overwrite) box "
                                        "receipt ")(
            lTransactionNum)(": couldn't find the transaction on this ledger.")
            .Flush();
        return false;
    }

    return pTransaction->DeleteBoxReceipt(*this);
}

// This makes sure that ALL transactions inside the ledger are loaded in their
// full (not abbreviated) form.
//
// For ALL abbreviated transactions, load the actual box receipt for each.
//
// For all failures to load the box receipt, if a set pointer was passed in,
// then add that transaction# to the set. (psetUnloaded)

// if psetUnloaded passed in, then use it to return the #s that weren't there.
auto Ledger::LoadBoxReceipts(UnallocatedSet<std::int64_t>* psetUnloaded) -> bool
{
    // Grab a copy of all the transaction #s stored inside this ledger.
    //
    UnallocatedSet<std::int64_t> the_set;

    for (auto& [number, pTransaction] : transactions_) {
        OT_ASSERT(pTransaction);
        the_set.insert(number);
    }

    // Now iterate through those numbers and for each, load the box receipt.
    //
    bool bRetVal = true;

    for (const auto& it : the_set) {
        std::int64_t lSetNum = it;

        const auto pTransaction = GetTransaction(lSetNum);
        OT_ASSERT(pTransaction);

        // Failed loading the boxReceipt
        //
        if ((true == pTransaction->IsAbbreviated()) &&
            (false == LoadBoxReceipt(lSetNum))) {
            // WARNING: pTransaction must be re-Get'd below this point if
            // needed, since pointer
            // is bad if success on LoadBoxReceipt() call.
            //
            bRetVal = false;
            auto& log = (nullptr != psetUnloaded) ? LogDebug() : LogConsole();

            if (nullptr != psetUnloaded) { psetUnloaded->insert(lSetNum); }

            log(OT_PRETTY_CLASS())("Failed calling LoadBoxReceipt on "
                                   "abbreviated transaction number: ")(lSetNum)
                .Flush();
            // If psetUnloaded is passed in, then we don't want to break,
            // because we want to
            // populate it with the conmplete list of IDs that wouldn't load as
            // a Box Receipt.
            // Thus, we only break if psetUnloaded is nullptr, which is better
            // optimization in that case.
            // (If not building a list of all failures, then we can return at
            // first sign of failure.)
            //
            if (nullptr == psetUnloaded) { break; }
        }
        // else (success), no need for a block in that case.
    }

    // You might ask, why didn't I just iterate through the transactions
    // directly and just call
    // LoadBoxReceipt on each one? Answer: Because that function actually
    // deletes the transaction
    // and replaces it with a different object, if successful.

    return bRetVal;
}

/*
 While the box itself is stored at (for example) "nymbox/NOTARY_ID/NYM_ID"
 the box receipts for that box may be stored at: "nymbox/NOTARY_ID/NYM_ID.r"
 With a specific receipt denoted by transaction:
 "nymbox/NOTARY_ID/NYM_ID.r/TRANSACTION_ID.rct"
 */

auto Ledger::LoadBoxReceipt(const std::int64_t& lTransactionNum) -> bool
{
    // First, see if the transaction itself exists on this ledger.
    // Get a pointer to it.
    // Next, see if the appropriate file exists, and load it up from
    // local storage, into a string.
    // Finally, try to load the transaction from that string and see if
    // successful.
    // If it verifies, then replace the abbreviated receipt with the actual one.

    // First, see if the transaction itself exists on this ledger.
    // Get a pointer to it.
    //
    auto pTransaction = GetTransaction(lTransactionNum);

    if (false == bool(pTransaction)) {
        LogConsole()(OT_PRETTY_CLASS())("Unable to load box receipt ")(
            lTransactionNum)(": couldn't find abbreviated version already on "
                             "this ledger.")
            .Flush();
        return false;
    }
    // Todo: security analysis. By this point we've verified the hash of the
    // transaction against the stored
    // hash inside the abbreviated version. (VerifyBoxReceipt) We've also
    // verified a few other values like transaction
    // number, and the "in ref to" display number. We're then assuming based on
    // those, that the adjustment and display
    // amount are correct. (The hash is actually a zero knowledge proof of this
    // already.) This is good for speedier
    // optimization but may be worth revisiting in case any security holes.
    // UPDATE: We'll save this for optimization needs in the future.
    //  pBoxReceipt->SetAbbrevAdjustment( pTransaction->GetAbbrevAdjustment() );
    //  pBoxReceipt->SetAbbrevDisplayAmount(
    // pTransaction->GetAbbrevDisplayAmount() );

    //  otOut << "DEBUGGING:  OTLedger::LoadBoxReceipt: ledger type: %s \n",
    // GetTypeString());

    // LoadBoxReceipt already checks pTransaction to see if it's
    // abbreviated
    // (which it must be.) So I don't bother checking twice.
    //
    auto pBoxReceipt = ::opentxs::LoadBoxReceipt(api_, *pTransaction, *this);

    // success
    if (false != bool(pBoxReceipt)) {
        // Remove the existing, abbreviated receipt, and replace it with
        // the actual receipt.
        // (If this inbox/outbox/whatever is saved, it will later save in
        // abbreviated form again.)
        //
        RemoveTransaction(lTransactionNum);  // this deletes pTransaction
        std::shared_ptr<OTTransaction> receipt{pBoxReceipt.release()};
        AddTransaction(receipt);

        return true;
    }

    return false;
}

auto Ledger::GetTransactionNums(
    const UnallocatedSet<std::int32_t>* pOnlyForIndices /*=nullptr*/) const
    -> UnallocatedSet<std::int64_t>
{
    UnallocatedSet<std::int64_t> the_set{};

    std::int32_t current_index{-1};

    for (const auto& [number, pTransaction] : transactions_) {
        ++current_index;  // 0 on first iteration.

        OT_ASSERT(pTransaction);

        if (nullptr == pOnlyForIndices) {
            the_set.insert(number);
            continue;
        }

        auto it_indices = pOnlyForIndices->find(current_index);

        if (pOnlyForIndices->end() != it_indices) { the_set.insert(number); }
    }

    return the_set;
}

// the below four functions (load/save in/outbox) assume that the ID
// is already set properly.
// Then it uses the ID to form the path for the file that is opened.
// Easy, right?

auto Ledger::LoadInbox() -> bool
{
    bool bRetVal = LoadGeneric(ledgerType::inbox);

    return bRetVal;
}

// TODO really should verify the NotaryID after loading the ledger.
// Perhaps just call "VerifyContract" and we'll make sure, for ledgers
// VerifyContract is overriden and explicitly checks the notaryID.
// Should also check the Type at the same time.

auto Ledger::LoadOutbox() -> bool { return LoadGeneric(ledgerType::outbox); }

auto Ledger::LoadNymbox() -> bool { return LoadGeneric(ledgerType::nymbox); }

auto Ledger::LoadInboxFromString(const String& strBox) -> bool
{
    return LoadGeneric(ledgerType::inbox, strBox);
}

auto Ledger::LoadOutboxFromString(const String& strBox) -> bool
{
    return LoadGeneric(ledgerType::outbox, strBox);
}

auto Ledger::LoadNymboxFromString(const String& strBox) -> bool
{
    return LoadGeneric(ledgerType::nymbox, strBox);
}

auto Ledger::LoadPaymentInbox() -> bool
{
    return LoadGeneric(ledgerType::paymentInbox);
}

auto Ledger::LoadRecordBox() -> bool
{
    return LoadGeneric(ledgerType::recordBox);
}

auto Ledger::LoadExpiredBox() -> bool
{
    return LoadGeneric(ledgerType::expiredBox);
}

auto Ledger::LoadPaymentInboxFromString(const String& strBox) -> bool
{
    return LoadGeneric(ledgerType::paymentInbox, strBox);
}

auto Ledger::LoadRecordBoxFromString(const String& strBox) -> bool
{
    return LoadGeneric(ledgerType::recordBox, strBox);
}

auto Ledger::LoadExpiredBoxFromString(const String& strBox) -> bool
{
    return LoadGeneric(ledgerType::expiredBox, strBox);
}

/**
  OTLedger::LoadGeneric is called by LoadInbox, LoadOutbox, and LoadNymbox.
  Does NOT VerifyAccount after loading -- caller is responsible to do that.

  pString -- optional argument, for when  you prefer to load from a string
  instead of from a file.
 */
auto Ledger::LoadGeneric(ledgerType theType, const String& pString) -> bool
{
    const auto* const pszType = GetTypeString();
    const auto [valid, path1, path2, path3] = make_filename(theType);

    if (false == valid) {
        LogError()(OT_PRETTY_CLASS())("Failed to set filename").Flush();
        LogError()(OT_PRETTY_CLASS())("Path1: ")(path1).Flush();
        LogError()(OT_PRETTY_CLASS())("Path2: ")(path2).Flush();
        LogError()(OT_PRETTY_CLASS())("Path3: ")(path2).Flush();

        return false;
    }

    auto strRawFile = String::Factory();

    if (pString.Exists()) {  // Loading FROM A STRING.
        strRawFile->Set(pString.Get());
    } else {  // Loading FROM A FILE.
        if (!OTDB::Exists(
                api_, api_.DataFolder().string(), path1, path2, path3, "")) {
            LogDebug()(OT_PRETTY_CLASS())("does not exist in OTLedger::Load")(
                pszType)(": ")(path1)('/')(filename_.get())
                .Flush();
            return false;
        }

        // Try to load the ledger from local storage.
        UnallocatedCString strFileContents(OTDB::QueryPlainString(
            api_,
            api_.DataFolder().string(),
            path1,
            path2,
            path3,
            ""));  // <=== LOADING FROM DATA STORE.

        if (strFileContents.length() < 2) {
            LogError()(OT_PRETTY_CLASS())("Error reading file: ")(path1)('/')(
                filename_.get())
                .Flush();
            return false;
        }

        strRawFile->Set(strFileContents.c_str());
    }

    // NOTE: No need to deal with OT ARMORED INBOX file format here, since
    //       LoadContractFromString already handles that automatically.
    if (!strRawFile->Exists()) {
        LogError()(OT_PRETTY_CLASS())("Unable to load box (")(path1)('/')(
            filename_.get())(") from empty string.")
            .Flush();
        return false;
    }

    bool bSuccess = LoadContractFromString(strRawFile);

    if (!bSuccess) {
        LogError()(OT_PRETTY_CLASS())("Failed loading ")(pszType)(" ")(
            (pString.Exists()) ? "from string" : "from file")(
            " in OTLedger::Load")(pszType)(": ")(path1)('/')(filename_.get())
            .Flush();
        return false;
    } else {
        LogVerbose()(OT_PRETTY_CLASS())("Successfully loaded ")(pszType)(" ")(
            (pString.Exists()) ? "from string" : "from file")(
            " in OTLedger::Load")(pszType)(": ")(path1)('/')(filename_.get())
            .Flush();
    }

    return bSuccess;
}

auto Ledger::SaveGeneric(ledgerType theType) -> bool
{
    const auto* const pszType = GetTypeString();
    const auto [valid, path1, path2, path3] = make_filename(theType);

    if (false == valid) {
        LogError()(OT_PRETTY_CLASS())("Failed to set filename").Flush();
        LogError()(OT_PRETTY_CLASS())("Path1: ")(path1).Flush();
        LogError()(OT_PRETTY_CLASS())("Path2: ")(path2).Flush();
        LogError()(OT_PRETTY_CLASS())("Path3: ")(path2).Flush();

        return false;
    }

    auto strRawFile = String::Factory();

    if (!SaveContractRaw(strRawFile)) {
        LogError()(OT_PRETTY_CLASS())("Error saving ")(pszType)(filename_.get())
            .Flush();
        return false;
    }

    auto strFinal = String::Factory();
    auto ascTemp = Armored::Factory(api_.Crypto(), strRawFile);

    if (false == ascTemp->WriteArmoredString(strFinal, contract_type_->Get())) {
        LogError()(OT_PRETTY_CLASS())("Error saving ")(
            pszType)(" (failed writing armored string): ")(path1)('/')(
            filename_.get())
            .Flush();
        return false;
    }

    bool bSaved = OTDB::StorePlainString(
        api_,
        strFinal->Get(),
        api_.DataFolder().string(),
        path1,
        path2,
        path3,
        "");  // <=== SAVING TO DATA STORE.
    if (!bSaved) {
        LogError()(OT_PRETTY_CLASS())("Error writing ")(pszType)(" to file: ")(
            path1)('/')(filename_.get())
            .Flush();
        return false;
    } else {
        LogVerbose()(OT_PRETTY_CLASS())("Successfully saved ")(pszType)(": ")(
            path1)('/')(filename_.get())
            .Flush();
    }

    return bSaved;
}

// If you know you have an inbox, outbox, or nymbox, then call
// CalculateInboxHash,
// CalculateOutboxHash, or CalculateNymboxHash. Otherwise, if in doubt, call
// this.
// It's more generic but warning: performs less verification.
//
auto Ledger::CalculateHash(identifier::Generic& theOutput) const -> bool
{
    theOutput = api_.Factory().IdentifierFromPreimage(xml_unsigned_->Bytes());

    if (theOutput.empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed trying to calculate hash (for a ")(GetTypeString())(").")
            .Flush();

        return false;
    } else {

        return true;
    }
}

auto Ledger::CalculateInboxHash(identifier::Generic& theOutput) const -> bool
{
    if (type_ != ledgerType::inbox) {
        LogError()(OT_PRETTY_CLASS())("Wrong type.").Flush();

        return false;
    }

    return CalculateHash(theOutput);
}

auto Ledger::CalculateOutboxHash(identifier::Generic& theOutput) const -> bool
{
    if (type_ != ledgerType::outbox) {
        LogError()(OT_PRETTY_CLASS())("Wrong type.").Flush();

        return false;
    }

    return CalculateHash(theOutput);
}

auto Ledger::CalculateNymboxHash(identifier::Generic& theOutput) const -> bool
{
    if (type_ != ledgerType::nymbox) {
        LogError()(OT_PRETTY_CLASS())("Wrong type.").Flush();

        return false;
    }

    return CalculateHash(theOutput);
}

auto Ledger::make_filename(const ledgerType theType) -> std::
    tuple<bool, UnallocatedCString, UnallocatedCString, UnallocatedCString>
{
    std::tuple<bool, UnallocatedCString, UnallocatedCString, UnallocatedCString>
        output{false, "", "", ""};
    auto& [valid, one, two, three] = output;
    type_ = theType;
    const char* pszFolder = nullptr;

    switch (theType) {
        case ledgerType::nymbox: {
            pszFolder = api_.Internal().Legacy().Nymbox();
        } break;
        case ledgerType::inbox: {
            pszFolder = api_.Internal().Legacy().Inbox();
        } break;
        case ledgerType::outbox: {
            pszFolder = api_.Internal().Legacy().Outbox();
        } break;
        case ledgerType::paymentInbox: {
            pszFolder = api_.Internal().Legacy().PaymentInbox();
        } break;
        case ledgerType::recordBox: {
            pszFolder = api_.Internal().Legacy().RecordBox();
        } break;
        case ledgerType::expiredBox: {
            pszFolder = api_.Internal().Legacy().ExpiredBox();
        } break;
        case ledgerType::message:
        case ledgerType::error_state:
        default: {
            LogError()(OT_PRETTY_CLASS())(
                "Error: unknown box type. (This should never happen).")
                .Flush();

            return output;
        }
    }

    foldername_ = String::Factory(pszFolder);
    one = foldername_->Get();

    if (GetRealNotaryID().empty()) {
        LogError()(OT_PRETTY_CLASS())("Notary ID not set").Flush();

        return output;
    }

    two = GetRealNotaryID().asBase58(api_.Crypto());
    auto ledgerID = String::Factory();
    GetIdentifier(ledgerID);

    if (ledgerID->empty()) {
        LogAbort()(OT_PRETTY_CLASS())("ID not set").Abort();
    }

    three = ledgerID->Get();

    if (false == filename_->Exists()) {
        filename_->Set((fs::path{two} / fs::path{three}).string().c_str());
    }

    if (2 > one.size()) { return output; }

    if (2 > two.size()) { return output; }

    if (2 > three.size()) { return output; }

    valid = true;

    return output;
}

auto Ledger::save_box(
    const ledgerType type,
    identifier::Generic& hash,
    bool (Ledger::*calc)(identifier::Generic&) const) -> bool
{
    OT_ASSERT(nullptr != calc);

    if (type_ != type) {
        LogError()(OT_PRETTY_CLASS())("Wrong type.").Flush();

        return false;
    }

    const bool saved = SaveGeneric(type_);

    if (saved) {
        hash.clear();

        if (false == (this->*calc)(hash)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed trying to calculate box hash.")
                .Flush();
        }
    }

    return saved;
}

// If you're going to save this, make sure you sign it first.
auto Ledger::SaveNymbox() -> bool
{
    auto hash = identifier::Generic{};

    return SaveNymbox(hash);
}

// If you're going to save this, make sure you sign it first.
auto Ledger::SaveNymbox(identifier::Generic& hash) -> bool
{
    return save_box(ledgerType::nymbox, hash, &Ledger::CalculateNymboxHash);
}

// If you're going to save this, make sure you sign it first.
auto Ledger::SaveInbox() -> bool
{
    auto hash = identifier::Generic{};

    return SaveInbox(hash);
}

// If you're going to save this, make sure you sign it first.
auto Ledger::SaveInbox(identifier::Generic& hash) -> bool
{
    return save_box(ledgerType::inbox, hash, &Ledger::CalculateInboxHash);
}

// If you're going to save this, make sure you sign it first.
auto Ledger::SaveOutbox() -> bool
{
    auto hash = identifier::Generic{};

    return SaveOutbox(hash);
}

// If you're going to save this, make sure you sign it first.
auto Ledger::SaveOutbox(identifier::Generic& hash) -> bool
{
    return save_box(ledgerType::outbox, hash, &Ledger::CalculateOutboxHash);
}

// If you're going to save this, make sure you sign it first.
auto Ledger::SavePaymentInbox() -> bool
{
    if (type_ != ledgerType::paymentInbox) {
        LogError()(OT_PRETTY_CLASS())("Wrong ledger type passed.").Flush();
        return false;
    }

    return SaveGeneric(type_);
}

// If you're going to save this, make sure you sign it first.
auto Ledger::SaveRecordBox() -> bool
{
    if (type_ != ledgerType::recordBox) {
        LogError()(OT_PRETTY_CLASS())("Wrong ledger type passed.").Flush();
        return false;
    }

    return SaveGeneric(type_);
}

// If you're going to save this, make sure you sign it first.
auto Ledger::SaveExpiredBox() -> bool
{
    if (type_ != ledgerType::expiredBox) {
        LogError()(OT_PRETTY_CLASS())("Wrong ledger type passed.").Flush();
        return false;
    }

    return SaveGeneric(type_);
}

auto Ledger::generate_ledger(
    const identifier::Nym& theNymID,
    const identifier::Account& theAcctID,
    const identifier::Notary& theNotaryID,
    ledgerType theType,
    bool bCreateFile) -> bool
{
    switch (theType) {
        case ledgerType::nymbox: {
            foldername_ = String::Factory(api_.Internal().Legacy().Nymbox());
            filename_->Set(api_.Internal()
                               .Legacy()
                               .LedgerFileName(theNotaryID, theAcctID)
                               .string()
                               .c_str());
        } break;
        case ledgerType::inbox: {
            foldername_ = String::Factory(api_.Internal().Legacy().Inbox());
            filename_->Set(api_.Internal()
                               .Legacy()
                               .LedgerFileName(theNotaryID, theAcctID)
                               .string()
                               .c_str());
        } break;
        case ledgerType::outbox: {
            foldername_ = String::Factory(api_.Internal().Legacy().Outbox());
            filename_->Set(api_.Internal()
                               .Legacy()
                               .LedgerFileName(theNotaryID, theAcctID)
                               .string()
                               .c_str());
        } break;
        case ledgerType::paymentInbox: {
            foldername_ =
                String::Factory(api_.Internal().Legacy().PaymentInbox());
            filename_->Set(api_.Internal()
                               .Legacy()
                               .LedgerFileName(theNotaryID, theAcctID)
                               .string()
                               .c_str());
        } break;
        case ledgerType::recordBox: {
            foldername_ = String::Factory(api_.Internal().Legacy().RecordBox());
            filename_->Set(api_.Internal()
                               .Legacy()
                               .LedgerFileName(theNotaryID, theAcctID)
                               .string()
                               .c_str());
        } break;
        case ledgerType::expiredBox: {
            foldername_ =
                String::Factory(api_.Internal().Legacy().ExpiredBox());
            filename_->Set(api_.Internal()
                               .Legacy()
                               .LedgerFileName(theNotaryID, theAcctID)
                               .string()
                               .c_str());
        } break;
        case ledgerType::message: {
            LogTrace()(OT_PRETTY_CLASS())("Generating message ledger...")
                .Flush();
            SetRealAccountID(theAcctID);
            SetPurportedAccountID(theAcctID);  // It's safe to set these the
                                               // same here, since we're
                                               // creating the ledger now.
            SetRealNotaryID(theNotaryID);
            SetPurportedNotaryID(theNotaryID);  // Always want the server ID on
            // anything that the server signs.
            type_ = theType;
            return true;
        }
        case ledgerType::error_state:
        default: {
            OT_FAIL_MSG("OTLedger::GenerateLedger: GenerateLedger is only for "
                        "message, nymbox, inbox, outbox, and paymentInbox "
                        "ledgers.\n");
        }
    }

    type_ = theType;  // Todo make this Get/Set methods

    SetRealAccountID(theAcctID);  // set this before calling LoadContract... (In
                                  // this case, will just be the Nym ID as
                                  // well...)
    SetRealNotaryID(theNotaryID);  // (Ledgers/transactions/items were
                                   // originally meant just for account-related
                                   // functions.)

    if (bCreateFile) {
        const auto strNotaryID = String::Factory(theNotaryID, api_.Crypto());
        const auto strFilename = String::Factory(theAcctID, api_.Crypto());
        const char* szFolder1name =
            foldername_->Get();  // "nymbox" (or "inbox" or "outbox")
        const char* szFolder2name = strNotaryID->Get();  // "nymbox/NOTARY_ID"
        const char* szFilename =
            strFilename->Get();  // "nymbox/NOTARY_ID/NYM_ID"  (or
                                 // "inbox/NOTARY_ID/ACCT_ID" or
                                 // "outbox/NOTARY_ID/ACCT_ID")

        if (OTDB::Exists(
                api_,
                api_.DataFolder().string(),
                szFolder1name,
                szFolder2name,
                szFilename,
                "")) {
            LogConsole()(OT_PRETTY_CLASS())(
                "ERROR: trying to generate ledger that already exists: ")(
                szFolder1name)('/')(szFolder2name)('/')(szFilename)(".")
                .Flush();
            return false;
        }

        // Okay, it doesn't already exist. Let's generate it.
        LogDetail()(OT_PRETTY_CLASS())("Generating ")(szFolder1name)('/')(
            szFolder2name)('/')(szFilename)(".")
            .Flush();
    }

    SetNymID(theNymID);
    SetPurportedAccountID(theAcctID);
    SetPurportedNotaryID(theNotaryID);

    // Notice I still don't actually create the file here.  The programmer still
    // has to call "SaveNymbox", "SaveInbox" or "SaveOutbox" or "SaveRecordBox"
    // or "SavePaymentInbox" to actually save the file. But he cannot do that
    // unless he generates it first here, and the "bCreateFile" parameter
    // insures that he isn't overwriting one that is already there (even if we
    // don't actually save the file in this function.)

    return true;
}

auto Ledger::GenerateLedger(
    const identifier::Account& theAcctID,
    const identifier::Notary& theNotaryID,
    ledgerType theType,
    bool bCreateFile) -> bool
{
    auto nymID = identifier::Nym{};

    if ((ledgerType::inbox == theType) || (ledgerType::outbox == theType)) {
        // Have to look up the NymID here. No way around it. We need that ID.
        // Plus it helps verify things.
        auto account = api_.Wallet().Internal().Account(theAcctID);

        if (account) {
            nymID = account.get().GetNymID();
        } else {
            LogError()(OT_PRETTY_CLASS())(
                "Failed in OTAccount::LoadExistingAccount().")
                .Flush();
            return false;
        }
    } else if (ledgerType::recordBox == theType) {
        // RecordBox COULD be by NymID OR AcctID. So we TRY to lookup the acct.
        auto account = api_.Wallet().Internal().Account(theAcctID);

        if (account) {
            nymID = account.get().GetNymID();
        } else {
            // Must be based on NymID, not AcctID (like Nymbox. But RecordBox
            // can go either way.)
            nymID = api_.Factory().Internal().NymIDConvertSafe(theAcctID);
            // In the case of nymbox, and sometimes with recordBox, the acct ID
            // IS the user ID.
        }
    } else {
        // In the case of paymentInbox, expired box, and nymbox, the acct ID IS
        // the user ID. (Should change it to "owner ID" to make it sound right
        // either way.)
        nymID = api_.Factory().Internal().NymIDConvertSafe(theAcctID);
    }

    return generate_ledger(nymID, theAcctID, theNotaryID, theType, bCreateFile);
}

auto Ledger::GenerateLedger(
    const identifier::Nym& nymAsAccount,
    const identifier::Notary& theNotaryID,
    ledgerType theType,
    bool bCreateFile) -> bool
{
    using enum identifier::AccountSubtype;

    return generate_ledger(
        nymAsAccount,
        api_.Factory().AccountIDFromHash(
            nymAsAccount.Bytes(), custodial_account),
        theNotaryID,
        theType,
        bCreateFile);
}

auto Ledger::CreateLedger(
    const identifier::Nym& theNymID,
    const identifier::Account& theAcctID,
    const identifier::Notary& theNotaryID,
    ledgerType theType,
    bool bCreateFile) -> bool
{
    return generate_ledger(
        theNymID, theAcctID, theNotaryID, theType, bCreateFile);
}

void Ledger::InitLedger()
{
    contract_type_ = String::Factory(
        "LEDGER");  // CONTRACT, MESSAGE, TRANSACTION, LEDGER, TRANSACTION ITEM

    // This is the default type for a ledger.
    // Inboxes and Outboxes are generated with the right type, with files.
    // Until the GenerateLedger function is called, message is the default type.
    type_ = ledgerType::message;

    loaded_legacy_data_ = false;
}

auto Ledger::GetTransactionMap() const -> const mapOfTransactions&
{
    return transactions_;
}

/// If transaction #87, in reference to #74, is in the inbox, you can remove it
/// by calling this function and passing in 87. Deletes.
///
auto Ledger::RemoveTransaction(const TransactionNumber number) -> bool
{
    if (0 == transactions_.erase(number)) {
        LogError()(OT_PRETTY_CLASS())(
            "Attempt to remove Transaction from ledger, when "
            "not already there: ")(number)(".")
            .Flush();

        return false;
    }

    return true;
}

auto Ledger::AddTransaction(std::shared_ptr<OTTransaction> theTransaction)
    -> bool
{
    const auto number = theTransaction->GetTransactionNum();
    const auto [it, added] = transactions_.emplace(number, theTransaction);

    if (false == added) {
        LogError()(OT_PRETTY_CLASS())(
            "Attempt to add Transaction to ledger when already there for "
            "that number: ")(number)
            .Flush();

        return false;
    }

    return true;
}

// Do NOT delete the return value, it's owned by the ledger.
auto Ledger::GetTransaction(transactionType theType)
    -> std::shared_ptr<OTTransaction>
{
    // loop through the items that make up this transaction

    for (auto& it : transactions_) {
        auto pTransaction = it.second;
        OT_ASSERT(pTransaction);

        if (theType == pTransaction->GetType()) { return pTransaction; }
    }

    return nullptr;
}

// if not found, returns -1
auto Ledger::GetTransactionIndex(const TransactionNumber target) -> std::int32_t
{
    // loop through the transactions inside this ledger
    // If a specific transaction is found, returns its index inside the ledger
    //
    std::int32_t output{0};

    for (const auto& [number, pTransaction] : transactions_) {
        if (target == number) {

            return output;
        } else {
            ++output;
        }
    }

    return -1;
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
//
// Do NOT delete the return value, it's owned by the ledger.
//
auto Ledger::GetTransaction(const TransactionNumber number) const
    -> std::shared_ptr<OTTransaction>
{
    try {

        return transactions_.at(number);
    } catch (...) {

        return {};
    }
}

// Return a count of all the transactions in this ledger that are IN REFERENCE
// TO a specific trans#.
//
// Might want to change this so that it only counts ACCEPTED receipts.
//
auto Ledger::GetTransactionCountInRefTo(std::int64_t lReferenceNum) const
    -> std::int32_t
{
    std::int32_t nCount{0};

    for (const auto& it : transactions_) {
        const auto pTransaction = it.second;
        OT_ASSERT(pTransaction);

        if (pTransaction->GetReferenceToNum() == lReferenceNum) { nCount++; }
    }

    return nCount;
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
//
auto Ledger::GetTransactionByIndex(std::int32_t nIndex) const
    -> std::shared_ptr<OTTransaction>
{
    // Out of bounds.
    if ((nIndex < 0) || (nIndex >= GetTransactionCount())) { return nullptr; }

    std::int32_t nIndexCount = -1;

    for (const auto& it : transactions_) {
        nIndexCount++;  // On first iteration, this is now 0, same as nIndex.
        auto pTransaction = it.second;
        OT_ASSERT(pTransaction);  // Should always be good.

        // If this transaction is the one at the requested index
        if (nIndexCount == nIndex) { return pTransaction; }
    }

    return nullptr;  // Should never reach this point, since bounds are checked
                     // at the top.
}

// Nymbox-only.
// Looks up replyNotice by REQUEST NUMBER.
//
auto Ledger::GetReplyNotice(const std::int64_t& lRequestNum)
    -> std::shared_ptr<OTTransaction>
{
    // loop through the transactions that make up this ledger.
    for (auto& it : transactions_) {
        auto pTransaction = it.second;
        OT_ASSERT(pTransaction);

        if (transactionType::replyNotice !=
            pTransaction->GetType()) {  // <=======
            continue;
        }

        if (pTransaction->GetRequestNum() == lRequestNum) {
            return pTransaction;
        }
    }

    return nullptr;
}

auto Ledger::GetTransferReceipt(std::int64_t lNumberOfOrigin)
    -> std::shared_ptr<OTTransaction>
{
    // loop through the transactions that make up this ledger.
    for (auto& it : transactions_) {
        auto pTransaction = it.second;
        OT_ASSERT(pTransaction);

        if (transactionType::transferReceipt == pTransaction->GetType()) {
            auto strReference = String::Factory();
            pTransaction->GetReferenceString(strReference);

            auto pOriginalItem{api_.Factory().InternalSession().Item(
                strReference,
                pTransaction->GetPurportedNotaryID(),
                pTransaction->GetReferenceToNum())};
            OT_ASSERT(pOriginalItem);

            if (pOriginalItem->GetType() != itemType::acceptPending) {
                LogError()(OT_PRETTY_CLASS())(
                    "Wrong item type attached to transferReceipt!")
                    .Flush();
                return nullptr;
            } else {
                // Note: the acceptPending USED to be "in reference to" whatever
                // the pending
                // was in reference to. (i.e. the original transfer.) But since
                // the KacTech
                // bug fix (for accepting multiple transfer receipts) the
                // acceptPending is now
                // "in reference to" the pending itself, instead of the original
                // transfer.
                //
                // It used to be that a caller of GetTransferReceipt would pass
                // in the InRefTo
                // expected from the pending in the outbox, and match it to the
                // InRefTo found
                // on the acceptPending (inside the transferReceipt) in the
                // inbox.
                // But this is no longer possible, since the acceptPending is no
                // longer InRefTo
                // whatever the pending is InRefTo.
                //
                // Therefore, in this place, it is now necessary to pass in the
                // NumberOfOrigin,
                // and compare it to the NumberOfOrigin, to find the match.
                //
                if (pOriginalItem->GetNumberOfOrigin() == lNumberOfOrigin) {
                    //              if (pOriginalItem->GetReferenceToNum() ==
                    // lTransactionNum)
                    return pTransaction;  // FOUND IT!
                }
            }
        }
    }

    return nullptr;
}

// This method loops through all the receipts in the ledger (inbox usually),
// to see if there's a chequeReceipt for a given cheque. For each cheque
// receipt,
// it will load up the original depositCheque item it references, and then load
// up
// the actual cheque which is attached to that item. At this point it can verify
// whether lChequeNum matches the transaction number on the cheque itself, and
// if
// so, return a pointer to the relevant chequeReceipt.
//
// The caller has the option of passing ppChequeOut if he wants the cheque
// returned
// (if he's going to load it anyway, no sense in loading it twice.) If the
// caller
// elects this option, he needs to delete the cheque when he's done with it.
// (But of course do NOT delete the OTTransaction that's returned, since that is
// owned by the ledger.)
//
auto Ledger::GetChequeReceipt(std::int64_t lChequeNum)
    -> std::shared_ptr<OTTransaction>
{
    for (auto& it : transactions_) {
        auto pCurrentReceipt = it.second;
        OT_ASSERT(nullptr != pCurrentReceipt);

        if ((pCurrentReceipt->GetType() != transactionType::chequeReceipt) &&
            (pCurrentReceipt->GetType() != transactionType::voucherReceipt)) {
            continue;
        }

        auto strDepositChequeMsg = String::Factory();
        pCurrentReceipt->GetReferenceString(strDepositChequeMsg);

        auto pOriginalItem{api_.Factory().InternalSession().Item(
            strDepositChequeMsg,
            GetPurportedNotaryID(),
            pCurrentReceipt->GetReferenceToNum())};

        if (false == bool(pOriginalItem)) {
            LogError()(OT_PRETTY_CLASS())(
                "Expected original depositCheque request item to be "
                "inside the chequeReceipt "
                "(but failed to load it...).")
                .Flush();
        } else if (itemType::depositCheque != pOriginalItem->GetType()) {
            auto strItemType = String::Factory();
            pOriginalItem->GetTypeString(strItemType);
            LogError()(OT_PRETTY_CLASS())(
                "Expected original depositCheque request item to be "
                "inside the chequeReceipt, "
                "but somehow what we found instead was a ")(strItemType.get())(
                "...")
                .Flush();
        } else {
            // Get the cheque from the Item and load it up into a Cheque object.
            //
            auto strCheque = String::Factory();
            pOriginalItem->GetAttachment(strCheque);

            auto pCheque{api_.Factory().InternalSession().Cheque()};
            OT_ASSERT(pCheque);

            if (!((strCheque->GetLength() > 2) &&
                  pCheque->LoadContractFromString(strCheque))) {
                LogError()(OT_PRETTY_CLASS())(
                    "Error loading cheque from string: ")(strCheque.get())(".")
                    .Flush();
            }
            // NOTE: Technically we don'T NEED to load up the cheque anymore,
            // since
            // we could just check the NumberOfOrigin, which should already
            // match the
            // transaction number on the cheque.
            // However, even that would have to load up the cheque once, if it
            // wasn't
            // already set, and this function already must RETURN a copy of the
            // cheque
            // (at least optionally), so we might as well just load it up,
            // verify it,
            // and return it. (That's why we are still loading the cheque here
            // instead
            // of checking the number of origin.)
            else {
                // Success loading the cheque.
                // Let's see if it's the right cheque...
                if (pCheque->GetTransactionNum() == lChequeNum) {

                    return pCurrentReceipt;
                }
            }
        }
    }

    return nullptr;
}

// Find the finalReceipt in this Inbox, that has lTransactionNum as its "in
// reference to".
// This is useful for cases where a marketReceipt or paymentReceipt has been
// found,
// yet the transaction # for that receipt isn't on my issued list... it's been
// closed.
// Normally this would be a problem: why is it in my inbox then? Because those
// receipts
// are still valid as long as there is a "FINAL RECEIPT" in the same inbox, that
// references
// the same original transaction that they do.  The below function makes it easy
// to find that
// final receipt, if it exists.
//
auto Ledger::GetFinalReceipt(std::int64_t lReferenceNum)
    -> std::shared_ptr<OTTransaction>
{
    // loop through the transactions that make up this ledger.
    for (auto& it : transactions_) {
        auto pTransaction = it.second;
        OT_ASSERT(pTransaction);

        if (transactionType::finalReceipt !=
            pTransaction->GetType()) {  // <=======
            continue;
        }

        if (pTransaction->GetReferenceToNum() == lReferenceNum) {
            return pTransaction;
        }
    }

    return nullptr;
}

/// Only if it is an inbox, a ledger will loop through the transactions
/// and produce the XML output for the report that's necessary during
/// a balance agreement. (Any balance agreement for an account must
/// include the list of transactions the nym has issued for use, as
/// well as a listing of the transactions in the inbox for that account.
/// This function does that last part :)
///
/// returns a new balance statement item containing the inbox report
/// CALLER IS RESPONSIBLE TO DELETE.
auto Ledger::GenerateBalanceStatement(
    const Amount& lAdjustment,
    const OTTransaction& theOwner,
    const otx::context::Server& context,
    const Account& theAccount,
    Ledger& theOutbox,
    const PasswordPrompt& reason) const -> std::unique_ptr<Item>
{
    return GenerateBalanceStatement(
        lAdjustment,
        theOwner,
        context,
        theAccount,
        theOutbox,
        UnallocatedSet<TransactionNumber>(),
        reason);
}

auto Ledger::GenerateBalanceStatement(
    const Amount& lAdjustment,
    const OTTransaction& theOwner,
    const otx::context::Server& context,
    const Account& theAccount,
    Ledger& theOutbox,
    const UnallocatedSet<TransactionNumber>& without,
    const PasswordPrompt& reason) const -> std::unique_ptr<Item>
{
    UnallocatedSet<TransactionNumber> removing = without;

    if (ledgerType::inbox != GetType()) {
        LogError()(OT_PRETTY_CLASS())("Wrong ledger type.").Flush();

        return nullptr;
    }

    if ((theAccount.GetPurportedAccountID() != GetPurportedAccountID()) ||
        (theAccount.GetPurportedNotaryID() != GetPurportedNotaryID()) ||
        (theAccount.GetNymID() != GetNymID())) {
        LogError()(OT_PRETTY_CLASS())("Wrong Account passed in.").Flush();

        return nullptr;
    }

    if ((theOutbox.GetPurportedAccountID() != GetPurportedAccountID()) ||
        (theOutbox.GetPurportedNotaryID() != GetPurportedNotaryID()) ||
        (theOutbox.GetNymID() != GetNymID())) {
        LogError()(OT_PRETTY_CLASS())("Wrong Outbox passed in.").Flush();

        return nullptr;
    }

    if ((context.Nym()->ID() != GetNymID())) {
        LogError()(OT_PRETTY_CLASS())("Wrong Nym passed in.").Flush();

        return nullptr;
    }

    // theOwner is the withdrawal, or deposit, or whatever, that wants to change
    // the account balance, and thus that needs a new balance agreement signed.
    //
    auto pBalanceItem{api_.Factory().InternalSession().Item(
        theOwner, itemType::balanceStatement, {})};  // <=== balanceStatement
                                                     // type, with user ID,
                                                     // server ID, account ID,
                                                     // transaction ID.

    // The above has an ASSERT, so this this will never actually happen.
    if (false == bool(pBalanceItem)) { return nullptr; }

    UnallocatedCString itemType;
    const auto number = theOwner.GetTransactionNum();

    switch (theOwner.GetType()) {
        // These six options will remove the transaction number from the issued
        // list, SUCCESS OR FAIL. Server will expect the number to be missing
        // from the list, in the case of these. Therefore I remove it here in
        // order to generate a proper balance agreement, acceptable to the
        // server.
        case transactionType::processInbox: {
            itemType = "processInbox";
            LogDetail()(OT_PRETTY_CLASS())("Removing number ")(number)(" for ")(
                itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::withdrawal: {
            itemType = "withdrawal";
            LogDetail()(OT_PRETTY_CLASS())("Removing number ")(number)(" for ")(
                itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::deposit: {
            itemType = "deposit";
            LogDetail()(OT_PRETTY_CLASS())("Removing number ")(number)(" for ")(
                itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::cancelCronItem: {
            itemType = "cancelCronItem";
            LogDetail()(OT_PRETTY_CLASS())("Removing number ")(number)(" for ")(
                itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::exchangeBasket: {
            itemType = "exchangeBasket";
            LogDetail()(OT_PRETTY_CLASS())("Removing number ")(number)(" for ")(
                itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::payDividend: {
            itemType = "payDividend";
            LogDetail()(OT_PRETTY_CLASS())("Removing number ")(number)(" for ")(
                itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::transfer:
        case transactionType::marketOffer:
        case transactionType::paymentPlan:
        case transactionType::smartContract: {
            // Nothing removed here since the transaction is still in play.
            // (Assuming success.) If the server replies with rejection for any
            // of these three, then I can remove the transaction number from my
            // list of issued/signed for. But if success, then I am responsible
            // for the transaction number until I sign off on closing it. Since
            // the Balance Statement ANTICIPATES SUCCESS, NOT FAILURE, it
            // assumes the number to be "in play" here, and thus DOES NOT remove
            // it (vs the cases above, which do.)
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Wrong owner transaction type: ")(
                theOwner.GetTypeString())(".")
                .Flush();
        } break;
    }

    UnallocatedSet<TransactionNumber> adding;
    auto statement = context.Statement(adding, removing, reason);

    if (!statement) { return nullptr; }

    pBalanceItem->SetAttachment(OTString(*statement));
    const auto lCurrentBalance{theAccount.GetBalance()};
    // The new (predicted) balance for after the transaction is complete.
    // (item.GetAmount)
    pBalanceItem->SetAmount(lCurrentBalance + lAdjustment);

    // loop through the INBOX transactions, and produce a sub-item onto
    // pBalanceItem for each, which will be a report on each transaction in this
    // inbox, therefore added to the balance item. (So the balance item contains
    // a complete report on the receipts in this inbox.)

    LogVerbose()(OT_PRETTY_CLASS())(
        "About to loop through the inbox items and produce a report for ")(
        "each one... ")
        .Flush();

    for (const auto& it : transactions_) {
        auto pTransaction = it.second;

        OT_ASSERT(pTransaction);

        LogVerbose()(OT_PRETTY_CLASS())("Producing a report... ").Flush();
        // This function adds a receipt sub-item to pBalanceItem, where
        // appropriate for INBOX items.
        pTransaction->ProduceInboxReportItem(*pBalanceItem, reason);
    }

    theOutbox.ProduceOutboxReport(*pBalanceItem, reason);
    pBalanceItem->SignContract(*context.Nym(), reason);
    pBalanceItem->SaveContract();

    return pBalanceItem;
}

// for inbox only, allows you to lookup the total value of pending transfers
// within the inbox.
// (And it really loads the items to check the amount, but does all this ONLY
// for pending transfers.)
//
auto Ledger::GetTotalPendingValue(const PasswordPrompt& reason) -> Amount
{
    Amount lTotalPendingValue = 0;

    if (ledgerType::inbox != GetType()) {
        LogError()(OT_PRETTY_CLASS())("Wrong ledger type (expected "
                                      "inbox).")
            .Flush();
        return 0;
    }

    for (auto& it : transactions_) {
        const auto pTransaction = it.second;
        OT_ASSERT(pTransaction);

        if (pTransaction->GetType() == transactionType::pending) {
            lTotalPendingValue += pTransaction->GetReceiptAmount(
                reason);  // this actually loads up the
        }
        // original item and reads the
        // amount.
    }

    return lTotalPendingValue;
}

// Called by the above function.
// This ledger is an outbox, and it is creating a report of itself,
// adding each report item to this balance item.
// DO NOT call this, it's meant to be used only by above function.
void Ledger::ProduceOutboxReport(
    Item& theBalanceItem,
    const PasswordPrompt& reason)
{
    if (ledgerType::outbox != GetType()) {
        LogError()(OT_PRETTY_CLASS())("Wrong ledger type.").Flush();
        return;
    }

    // loop through the OUTBOX transactions, and produce a sub-item onto
    // theBalanceItem for each, which will
    // be a report on each pending transfer in this outbox, therefore added to
    // the balance item.
    // (So the balance item contains a complete report on the outoing transfers
    // in this outbox.)
    for (auto& it : transactions_) {
        auto pTransaction = it.second;
        OT_ASSERT(pTransaction);

        // it only reports receipts where we don't yet have balance agreement.
        pTransaction->ProduceOutboxReportItem(
            theBalanceItem,
            reason);  // <======= This function adds a pending transfer
                      // sub-item to theBalanceItem, where appropriate.
    }
}

// Auto-detects ledger type. (message/nymbox/inbox/outbox)
// Use this instead of LoadContractFromString (for ledgers,
// for when you don't know their type already.)
// Otherwise if you know the type, then use LoadNymboxFromString() etc.
//
auto Ledger::LoadLedgerFromString(const String& theStr) -> bool
{
    bool bLoaded = false;

    // Todo security: Look how this is done...
    // Any vulnerabilities?
    //
    if (theStr.Contains("type=\"nymbox\"")) {
        bLoaded = LoadNymboxFromString(theStr);
    } else if (theStr.Contains("type=\"inbox\"")) {
        bLoaded = LoadInboxFromString(theStr);
    } else if (theStr.Contains("type=\"outbox\"")) {
        bLoaded = LoadOutboxFromString(theStr);
    } else if (theStr.Contains("type=\"paymentInbox\"")) {
        bLoaded = LoadPaymentInboxFromString(theStr);
    } else if (theStr.Contains("type=\"recordBox\"")) {
        bLoaded = LoadRecordBoxFromString(theStr);
    } else if (theStr.Contains("type=\"expiredBox\"")) {
        bLoaded = LoadExpiredBoxFromString(theStr);
    } else if (theStr.Contains("type=\"message\"")) {
        type_ = ledgerType::message;
        bLoaded = LoadContractFromString(theStr);
    }
    return bLoaded;
}

// SignContract will call this function at the right time.
void Ledger::UpdateContents(const PasswordPrompt& reason)  // Before
                                                           // transmission or
                                                           // serialization,
                                                           // this is where the
                                                           // ledger saves its
                                                           // contents
{
    switch (GetType()) {
        case ledgerType::message:
        case ledgerType::nymbox:
        case ledgerType::inbox:
        case ledgerType::outbox:
        case ledgerType::paymentInbox:
        case ledgerType::recordBox:
        case ledgerType::expiredBox: {
        } break;
        case ledgerType::error_state:
        default: {
            LogError()(OT_PRETTY_CLASS())("Error: unexpected box type (1st "
                                          "block). (This should never happen).")
                .Flush();
            return;
        }
    }

    // Abbreviated for all types but OTLedger::message.
    // A message ledger stores the full receipts directly inside itself. (No
    // separate files.)
    // For other types: These store abbreviated versions of themselves, with the
    // actual receipts
    // in separate files. Those separate files are created on server side when
    // first added to the
    // box, and on client side when downloaded from the server. They must match
    // the hash that
    // appears in the box.
    bool bSavingAbbreviated = GetType() != ledgerType::message;

    // We store this, so we know how many abbreviated records to read back
    // later.
    std::int32_t nPartialRecordCount = 0;
    if (bSavingAbbreviated) {
        nPartialRecordCount = static_cast<std::int32_t>(transactions_.size());
    }

    // Notice I use the PURPORTED Account ID and Notary ID to create the output.
    // That's because I don't want to inadvertantly substitute the real ID
    // for a bad one and then sign it.
    // So if there's a bad one in there when I read it, THAT's the one that I
    // write as well!
    auto strType = String::Factory(GetTypeString()),
         strLedgerAcctID =
             String::Factory(GetPurportedAccountID(), api_.Crypto()),
         strLedgerAcctNotaryID =
             String::Factory(GetPurportedNotaryID(), api_.Crypto()),
         strNymID = String::Factory(GetNymID(), api_.Crypto());

    OT_ASSERT(strType->Exists());
    OT_ASSERT(strLedgerAcctID->Exists());
    OT_ASSERT(strLedgerAcctNotaryID->Exists());
    OT_ASSERT(strNymID->Exists());

    // I release this because I'm about to repopulate it.
    xml_unsigned_->Release();

    Tag tag("accountLedger");

    tag.add_attribute("version", version_->Get());
    tag.add_attribute("type", strType->Get());
    tag.add_attribute("numPartialRecords", std::to_string(nPartialRecordCount));
    tag.add_attribute("accountID", strLedgerAcctID->Get());
    tag.add_attribute("nymID", strNymID->Get());
    tag.add_attribute("notaryID", strLedgerAcctNotaryID->Get());

    // loop through the transactions and print them out here.
    for (auto& it : transactions_) {
        auto pTransaction = it.second;
        OT_ASSERT(pTransaction);

        if (false == bSavingAbbreviated)  // only OTLedger::message uses this
                                          // block.
        {
            // Save the FULL version of the receipt inside the box, so
            // no separate files are necessary.
            auto strTransaction = String::Factory();

            pTransaction->SaveContractRaw(strTransaction);
            auto ascTransaction = Armored::Factory(api_.Crypto());
            ascTransaction->SetString(strTransaction, true);  // linebreaks =
                                                              // true

            tag.add_tag("transaction", ascTransaction->Get());
        } else  // true == bSavingAbbreviated
        {
            // ALL OTHER ledger types are saved here in abbreviated form.
            switch (GetType()) {
                case ledgerType::nymbox: {
                    pTransaction->SaveAbbreviatedNymboxRecord(tag, reason);
                } break;
                case ledgerType::inbox: {
                    pTransaction->SaveAbbreviatedInboxRecord(tag, reason);
                } break;
                case ledgerType::outbox: {
                    pTransaction->SaveAbbreviatedOutboxRecord(tag, reason);
                } break;
                case ledgerType::paymentInbox: {
                    pTransaction->SaveAbbrevPaymentInboxRecord(tag, reason);
                } break;
                case ledgerType::recordBox: {
                    pTransaction->SaveAbbrevRecordBoxRecord(tag, reason);
                } break;
                case ledgerType::expiredBox: {
                    pTransaction->SaveAbbrevExpiredBoxRecord(tag, reason);
                } break;
                case ledgerType::message:
                case ledgerType::error_state:
                default: {
                    LogAbort()(OT_PRETTY_CLASS())(
                        "Error: unexpected box type (2nd block). (This should "
                        "never happen)")
                        .Abort();
                }
            }
        }
    }

    UnallocatedCString str_result;
    tag.output(str_result);

    xml_unsigned_->Concatenate(String::Factory(str_result));
}

// LoadContract will call this function at the right time.
// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto Ledger::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{

    const auto strNodeName = String::Factory(xml->getNodeName());

    if (strNodeName->Compare("accountLedger")) {
        auto strType = String::Factory(),               // ledger type
            strLedgerAcctID = String::Factory(),        // purported
            strLedgerAcctNotaryID = String::Factory(),  // purported
            strNymID = String::Factory(),
             strNumPartialRecords =
                 String::Factory();  // Ledger contains either full
                                     // receipts, or abbreviated
                                     // receipts with hashes and partial
                                     // data.

        strType = String::Factory(xml->getAttributeValue("type"));
        version_ = String::Factory(xml->getAttributeValue("version"));

        if (strType->Compare("message")) {  // These are used for sending
            // transactions in messages. (Withdrawal
            // request, etc.)
            type_ = ledgerType::message;
        } else if (strType->Compare("nymbox")) {  // Used for receiving new
            // transaction numbers, and for
            // receiving notices.
            type_ = ledgerType::nymbox;
        } else if (strType->Compare("inbox")) {  // These are used for storing
                                                 // the
            // receipts in your inbox. (That
            // server must store until
            // signed-off.)
            type_ = ledgerType::inbox;
        } else if (strType->Compare("outbox")) {  // Outgoing, pending
                                                  // transfers.
            type_ = ledgerType::outbox;
        } else if (strType->Compare("paymentInbox")) {  // Receiving invoices,
                                                        // etc.
            type_ = ledgerType::paymentInbox;
        } else if (strType->Compare("recordBox")) {  // Where receipts go to die
                                                     // (awaiting user deletion,
            // completed from other boxes
            // already.)
            type_ = ledgerType::recordBox;
        } else if (strType->Compare("expiredBox")) {  // Where expired payments
                                                      // go
                                                      // to die (awaiting user
            // deletion, completed from
            // other boxes already.)
            type_ = ledgerType::expiredBox;
        } else {
            type_ = ledgerType::error_state;  // Danger, Will Robinson.
        }

        strLedgerAcctID = String::Factory(xml->getAttributeValue("accountID"));
        strLedgerAcctNotaryID =
            String::Factory(xml->getAttributeValue("notaryID"));
        strNymID = String::Factory(xml->getAttributeValue("nymID"));

        if (!strLedgerAcctID->Exists() || !strLedgerAcctNotaryID->Exists() ||
            !strNymID->Exists()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failure: missing strLedgerAcctID (")(strLedgerAcctID.get())(
                ") or strLedgerAcctNotaryID (")(strLedgerAcctNotaryID.get())(
                ") or strNymID (")(strNymID.get())(
                ") while loading transaction from ")(strType.get())(" ledger.")
                .Flush();
            return (-1);
        }

        const auto ACCOUNT_ID =
            api_.Factory().AccountIDFromBase58(strLedgerAcctID->Bytes());
        const auto NOTARY_ID =
            api_.Factory().NotaryIDFromBase58(strLedgerAcctNotaryID->Bytes());
        const auto NYM_ID = api_.Factory().NymIDFromBase58(strNymID->Bytes());

        SetPurportedAccountID(ACCOUNT_ID);
        SetPurportedNotaryID(NOTARY_ID);
        SetNymID(NYM_ID);

        if (!load_securely_) {
            SetRealAccountID(ACCOUNT_ID);
            SetRealNotaryID(NOTARY_ID);
        }

        // Load up the partial records, based on the expected count...
        //
        strNumPartialRecords =
            String::Factory(xml->getAttributeValue("numPartialRecords"));
        std::int32_t nPartialRecordCount =
            (strNumPartialRecords->Exists() ? atoi(strNumPartialRecords->Get())
                                            : 0);

        auto strExpected = String::Factory();  // The record type has a
                                               // different name for each box.
        NumList theNumList;
        NumList* pNumList = nullptr;

        switch (type_) {
            case ledgerType::nymbox: {
                strExpected->Set("nymboxRecord");
                pNumList = &theNumList;
            } break;
            case ledgerType::inbox: {
                strExpected->Set("inboxRecord");
            } break;
            case ledgerType::outbox: {
                strExpected->Set("outboxRecord");
            } break;
            case ledgerType::paymentInbox: {
                strExpected->Set("paymentInboxRecord");
            } break;
            case ledgerType::recordBox: {
                strExpected->Set("recordBoxRecord");
            } break;
            case ledgerType::expiredBox: {
                strExpected->Set("expiredBoxRecord");
            } break;
            case ledgerType::message: {
                if (nPartialRecordCount > 0) {
                    LogError()(OT_PRETTY_CLASS())("Error: There are ")(
                        nPartialRecordCount)(" unexpected abbreviated records "
                                             "in an OTLedger::message type "
                                             "ledger. (Failed loading ledger "
                                             "with accountID: ")(
                        strLedgerAcctID.get())(").")
                        .Flush();
                    return (-1);
                }
            } break;
            case ledgerType::error_state:
            default: {
                LogError()(OT_PRETTY_CLASS())("Unexpected ledger type (")(
                    strType.get())("). (Failed loading ledger for account: ")(
                    strLedgerAcctID.get())(").")
                    .Flush();
                return (-1);
            }
        }  // switch (to set strExpected to the abbreviated record type.)

        if (nPartialRecordCount > 0)  // message ledger will never enter this
                                      // block due to switch block (above.)
        {

            // We iterate to read the expected number of partial records from
            // the xml.
            // (They had better be there...)
            //
            while (nPartialRecordCount-- > 0) {
                //                xml->read(); // <==================
                if (!SkipToElement(xml)) {
                    LogConsole()(OT_PRETTY_CLASS())(
                        "Failure: Unable to find element when one was expected "
                        "(")(strExpected.get())(
                        ") for abbreviated record of receipt in ")(
                        GetTypeString())(" box: ")(raw_file_.get())(".")
                        .Flush();
                    return (-1);
                }

                // strExpected can be one of:
                //
                //                strExpected.Set("nymboxRecord");
                //                strExpected.Set("inboxRecord");
                //                strExpected.Set("outboxRecord");
                //
                // We're loading here either a nymboxRecord, inboxRecord, or
                // outboxRecord...
                //
                const auto strLoopNodeName =
                    String::Factory(xml->getNodeName());

                if (strLoopNodeName->Exists() &&
                    (xml->getNodeType() == irr::io::EXN_ELEMENT) &&
                    (strExpected->Compare(strLoopNodeName))) {
                    std::int64_t lNumberOfOrigin = 0;
                    originType theOriginType =
                        originType::not_applicable;  // default
                    TransactionNumber number{0};
                    std::int64_t lInRefTo = 0;
                    std::int64_t lInRefDisplay = 0;

                    auto the_DATE_SIGNED = Time{};
                    transactionType theType =
                        transactionType::error_state;  // default
                    auto strHash = String::Factory();

                    Amount lAdjustment = 0;
                    Amount lDisplayValue = 0;
                    std::int64_t lClosingNum = 0;
                    std::int64_t lRequestNum = 0;
                    bool bReplyTransSuccess = false;

                    std::int32_t nAbbrevRetVal = LoadAbbreviatedRecord(
                        xml,
                        lNumberOfOrigin,
                        theOriginType,
                        number,
                        lInRefTo,
                        lInRefDisplay,
                        the_DATE_SIGNED,
                        theType,
                        strHash,
                        lAdjustment,
                        lDisplayValue,
                        lClosingNum,
                        lRequestNum,
                        bReplyTransSuccess,
                        pNumList);  // This is for "transactionType::blank" and
                                    // "transactionType::successNotice",
                                    // otherwise nullptr.
                    if ((-1) == nAbbrevRetVal) {
                        return (-1);  // The function already logs
                    }
                    // appropriately.

                    //
                    // See if the same-ID transaction already exists in the
                    // ledger.
                    // (There can only be one.)
                    //
                    auto pExistingTrans = GetTransaction(number);
                    if (false != bool(pExistingTrans))  // Uh-oh, it's already
                                                        // there!
                    {
                        LogConsole()(OT_PRETTY_CLASS())(
                            "Error loading transaction ")(number)(" (")(
                            strExpected.get())("), since one was already "
                                               "there, in box for account: ")(
                            strLedgerAcctID.get())(".")
                            .Flush();
                        return (-1);
                    }

                    // CONSTRUCT THE ABBREVIATED RECEIPT HERE...

                    // Set all the values we just loaded here during actual
                    // construction of transaction
                    // (as abbreviated transaction) i.e. make a special
                    // constructor for abbreviated transactions
                    // which is ONLY used here.
                    //
                    auto pTransaction{
                        api_.Factory().InternalSession().Transaction(
                            NYM_ID,
                            ACCOUNT_ID,
                            NOTARY_ID,
                            lNumberOfOrigin,
                            static_cast<originType>(theOriginType),
                            number,
                            lInRefTo,  // lInRefTo
                            lInRefDisplay,
                            the_DATE_SIGNED,
                            static_cast<transactionType>(theType),
                            strHash,
                            lAdjustment,
                            lDisplayValue,
                            lClosingNum,
                            lRequestNum,
                            bReplyTransSuccess,
                            pNumList)};  // This is for "transactionType::blank"
                                         // and
                                         // "transactionType::successNotice",
                                         // otherwise nullptr.
                    OT_ASSERT(pTransaction);
                    //
                    // NOTE: For THIS CONSTRUCTOR ONLY, we DO set the purported
                    // AcctID and purported NotaryID.
                    // WHY? Normally you set the "real" IDs at construction, and
                    // then set the "purported" IDs
                    // when loading from string. But this constructor (only this
                    // one) is actually used when
                    // loading abbreviated receipts as you load their
                    // inbox/outbox/nymbox.
                    // Abbreviated receipts are not like real transactions,
                    // which have notaryID, AcctID, nymID,
                    // and signature attached, and the whole thing is
                    // base64-encoded and then added to the ledger
                    // as part of a list of contained objects. Rather, with
                    // abbreviated receipts, there are a series
                    // of XML records loaded up as PART OF the ledger itself.
                    // None of these individual XML records
                    // has its own signature, or its own record of the main IDs
                    // -- those are assumed to be on the parent
                    // ledger.
                    // That's the whole point: abbreviated records don't store
                    // redundant info, and don't each have their
                    // own signature, because we want them to be as small as
                    // possible inside their parent ledger.
                    // Therefore I will pass in the parent ledger's "real" IDs
                    // at construction, and immediately thereafter
                    // set the parent ledger's "purported" IDs onto the
                    // abbreviated transaction. That way, VerifyContractID()
                    // will still work and do its job properly with these
                    // abbreviated records.
                    //
                    // NOTE: Moved to OTTransaction constructor (for
                    // abbreviateds) for now.
                    //
                    //                    pTransaction->SetPurportedAccountID(
                    // GetPurportedAccountID());
                    //                    pTransaction->SetPurportedNotaryID(
                    // GetPurportedNotaryID());

                    // Add it to the ledger's list of transactions...
                    //

                    if (pTransaction->VerifyContractID()) {
                        // Add it to the ledger...
                        //
                        std::shared_ptr<OTTransaction> transaction{
                            pTransaction.release()};
                        transactions_[transaction->GetTransactionNum()] =
                            transaction;
                        transaction->SetParent(*this);
                    } else {
                        LogError()(OT_PRETTY_CLASS())(
                            "ERROR: verifying contract ID on "
                            "abbreviated transaction ")(
                            pTransaction->GetTransactionNum())(".")
                            .Flush();
                        return (-1);
                    }
                    //                    xml->read(); // <==================
                    // MIGHT need to add "skip after element" here.
                    //
                    // Update: Nope.
                } else {
                    LogError()(OT_PRETTY_CLASS())(
                        "Expected abbreviated record element.")
                        .Flush();
                    return (-1);  // error condition
                }
            }  // while
        }      // if (number of partial records > 0)

        LogTrace()(OT_PRETTY_CLASS())("Loading account ledger of type \"")(
            strType.get())("\", version: ")(version_.get())
            .Flush();

        // Since we just loaded this stuff, let's verify it. We may have to
        // remove this verification here and do it outside this call. But for
        // now...

        if (VerifyContractID()) {
            return 1;
        } else {
            return (-1);
        }
    }

    // Todo: When loading abbreviated list of records, set the abbreviated_ to
    // true.
    // Then in THIS block below, if that is set to true, then seek an existing
    // transaction instead of
    // instantiating a new one. Then repopulate the new one and verify the new
    // values against the ones
    // that were already there before overwriting anything.

    // Hmm -- technically this code should only execute for OTLedger::message,
    // and thus only if
    // is_abbreviated_ is FALSE. When the complete receipt is loaded,
    // "LoadBoxReceipt()" will be
    // called, and it will directly load the transaction starting in
    // OTTransaction::ProcessXMLNode().
    // THAT is where we must check for abbreviated mode and expect it already
    // loaded etc etc. Whereas
    // here in this spot, we basically want to error out if it's not a message
    // ledger.
    // UPDATE: However, I must consider legacy data. For now, I'll allow this to
    // load in any type of box.
    // I also need to check and see if the box receipt already exists (since its
    // normal creation point
    // may not have happened, when taking legacy data into account.) If it
    // doesn't already exist, then I
    // should save it again at this point.
    //
    else if (strNodeName->Compare("transaction")) {
        auto strTransaction = String::Factory();
        auto ascTransaction = Armored::Factory(api_.Crypto());

        // go to the next node and read the text.
        //        xml->read(); // <==================
        if (!SkipToTextField(xml)) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failure: Unable to find expected text field "
                "containing receipt transaction in box.")
                .Flush();
            return (-1);
        }

        if (irr::io::EXN_TEXT == xml->getNodeType()) {
            // the ledger contains a series of transactions.
            // Each transaction is initially stored as an Armored string.
            const auto strLoopNodeData = String::Factory(xml->getNodeData());

            if (strLoopNodeData->Exists()) {
                ascTransaction->Set(strLoopNodeData);  // Put the ascii-armored
            }
            // node data into the
            // ascii-armor object

            // Decode that into strTransaction, so we can load the transaction
            // object from that string.
            if (!ascTransaction->Exists() ||
                !ascTransaction->GetString(strTransaction)) {
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR: Missing expected transaction contents. Ledger "
                    "contents: ")(raw_file_.get())(".")
                    .Flush();
                return (-1);
            }

            // I belive we're only supposed to use purported numbers when
            // loading/saving, and to compare them (as distrusted)
            // against a more-trusted source, in order to verify them. Whereas
            // when actually USING the numbers (such as here,
            // when "GetRealAccountID()" is being used to instantiate the
            // transaction, then you ONLY use numbers that you KNOW
            // are good (the number you were expecting) versus whatever number
            // was actually in the file.
            // But wait, you ask, how do I know they are the same number then?
            // Because you verified that when you first loaded
            // everything into memory. Right after "load" was a "verify" that
            // makes sure the "real" account ID and the "purported"
            // account ID are actually the same.
            //
            // UPDATE: If this ledger is loaded from string, there's no
            // guarantee that the real IDs have even been set.
            // In some cases (Factory...) they definitely have not been. It
            // makes sense here when loading, to set the member
            // transactions to the same account/server IDs that were actually
            // loaded for their parent ledger. Therefore, changing
            // back here to Purported values.
            //
            //            OTTransaction * pTransaction = new
            // OTTransaction(GetNymID(), GetRealAccountID(),
            // GetRealNotaryID());
            auto pTransaction{api_.Factory().InternalSession().Transaction(
                GetNymID(), GetPurportedAccountID(), GetPurportedNotaryID())};
            OT_ASSERT(pTransaction);

            // Need this set before the LoadContractFromString().
            //
            if (!load_securely_) { pTransaction->SetLoadInsecure(); }

            // If we're able to successfully base64-decode the string and load
            // it up as
            // a transaction, then let's add it to the ledger's list of
            // transactions
            if (strTransaction->Exists() &&
                pTransaction->LoadContractFromString(strTransaction) &&
                pTransaction->VerifyContractID())
            // I responsible here to call pTransaction->VerifyContractID()
            // since
            // I am loading it here and adding it to the ledger. (So I do.)
            {

                auto pExistingTrans =
                    GetTransaction(pTransaction->GetTransactionNum());
                if (false != bool(pExistingTrans))  // Uh-oh, it's already
                                                    // there!
                {
                    const auto strPurportedAcctID =
                        String::Factory(GetPurportedAccountID(), api_.Crypto());
                    LogConsole()(OT_PRETTY_CLASS())(
                        "Error loading full transaction ")(
                        pTransaction->GetTransactionNum())(
                        ", since one was already there, in box for account: ")(
                        strPurportedAcctID.get())(".")
                        .Flush();
                    return (-1);
                }

                // It's not already there on this ledger -- so add it!
                std::shared_ptr<OTTransaction> transaction{
                    pTransaction.release()};
                transactions_[transaction->GetTransactionNum()] = transaction;
                transaction->SetParent(*this);

                switch (GetType()) {
                    case ledgerType::message: {
                    } break;
                    case ledgerType::nymbox:
                    case ledgerType::inbox:
                    case ledgerType::outbox:
                    case ledgerType::paymentInbox:
                    case ledgerType::recordBox:
                    case ledgerType::expiredBox: {
                        // For the sake of legacy data, check for existence of
                        // box
                        // receipt here,
                        // and re-save that box receipt if it doesn't exist.
                        //
                        LogConsole()(OT_PRETTY_CLASS())(
                            "--- Apparently this is old data (the "
                            "transaction "
                            "is still stored inside the ledger itself)... ")
                            .Flush();
                        loaded_legacy_data_ =
                            true;  // Only place this is set true.

                        const auto nBoxType =
                            static_cast<std::int32_t>(GetType());

                        const bool bBoxReceiptAlreadyExists =
                            VerifyBoxReceiptExists(
                                api_,
                                api_.DataFolder().string(),
                                transaction->GetRealNotaryID(),
                                transaction->GetNymID(),
                                transaction->GetRealAccountID(),  // If Nymbox
                                                                  // (vs
                                // inbox/outbox)
                                // the NYM_ID
                                // will be in this
                                // field also.
                                nBoxType,  // 0/nymbox, 1/inbox, 2/outbox
                                transaction->GetTransactionNum());
                        if (!bBoxReceiptAlreadyExists)  // Doesn't already
                                                        // exist separately.
                        {
                            // Okay then, let's create it...
                            //
                            LogConsole()(OT_PRETTY_CLASS())(
                                "--- The BoxReceipt doesn't exist "
                                "separately "
                                "(yet). Creating it in local storage...")
                                .Flush();

                            const auto lBoxType =
                                static_cast<std::int64_t>(nBoxType);

                            if (false == transaction->SaveBoxReceipt(
                                             lBoxType)) {  //  <======== SAVE
                                                           //  BOX RECEIPT
                                LogError()(OT_PRETTY_CLASS())(
                                    "--- FAILED trying to save BoxReceipt "
                                    "from legacy data to local storage!")
                                    .Flush();
                            }
                        }
                    } break;
                    case ledgerType::error_state:
                    default:
                        LogError()(OT_PRETTY_CLASS())(
                            "Unknown ledger type while loading "
                            "transaction!"
                            " (Should never happen).")
                            .Flush();  // todo: assert
                                       // here?
                                       // "should never
                                       // happen" ...
                        return (-1);
                }  // switch (GetType())

            }  // if transaction loads and verifies.
            else {
                LogError()(OT_PRETTY_CLASS())(
                    "Error loading or verifying transaction.")
                    .Flush();
                return (-1);
            }
        } else {
            LogError()(OT_PRETTY_CLASS())("Error: Transaction without value.")
                .Flush();
            return (-1);  // error condition
        }
        return 1;
    }

    return 0;
}

void Ledger::ReleaseTransactions()
{
    // If there were any dynamically allocated objects, clean them up here.

    transactions_.clear();
}

void Ledger::Release_Ledger() { ReleaseTransactions(); }

void Ledger::Release()
{
    Release_Ledger();

    ot_super::Release();  // since I've overridden the base class, I call it
                          // now...
}

Ledger::~Ledger() { Release_Ledger(); }
}  // namespace opentxs
