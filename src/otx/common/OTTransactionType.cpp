// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/OTTransactionType.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/transaction/Helpers.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
// keeping constructor private in order to force people to use the other
// constructors and therefore provide the requisite IDs.
OTTransactionType::OTTransactionType(const api::Session& api)
    : Contract(api)
    , account_id_()
    , notary_id_()
    , account_notary_id_()
    , account_nym_id_()
    , transaction_num_(0)
    , in_reference_to_transaction_(0)
    , number_of_origin_(0)
    , origin_type_(originType::not_applicable)
    , in_reference_to_(Armored::Factory(api.Crypto()))
    , load_securely_(true)
    , numlist_()
{
    // this function is private to prevent people from using it.
    // Should never actually get called.

    //  InitTransactionType(); // Just in case.
}

OTTransactionType::OTTransactionType(
    const api::Session& api,
    const identifier::Nym& theNymID,
    const identifier::Account& theAccountID,
    const identifier::Notary& theNotaryID,
    originType theOriginType)
    : Contract(api, theAccountID)
    , account_id_()
    , notary_id_(theNotaryID)
    , account_notary_id_()
    , account_nym_id_(theNymID)
    , transaction_num_(0)
    , in_reference_to_transaction_(0)
    , number_of_origin_(0)
    , origin_type_(theOriginType)
    , in_reference_to_(Armored::Factory(api.Crypto()))
    , load_securely_(true)
    , numlist_()
{
    // do NOT set account_id_ and account_notary_id_ here.  Let the child
    // classes LOAD them or GENERATE them.
}

OTTransactionType::OTTransactionType(
    const api::Session& api,
    const identifier::Nym& theNymID,
    const identifier::Account& theAccountID,
    const identifier::Notary& theNotaryID,
    std::int64_t lTransactionNum,
    originType theOriginType)
    : Contract(api, theAccountID)
    , account_id_()
    , notary_id_(theNotaryID)
    , account_notary_id_()
    , account_nym_id_(theNymID)
    , transaction_num_(lTransactionNum)
    , in_reference_to_transaction_(0)
    , number_of_origin_(0)
    , origin_type_(theOriginType)
    , in_reference_to_(Armored::Factory(api.Crypto()))
    , load_securely_(true)
    , numlist_()
{
    // do NOT set account_id_ and account_notary_id_ here.  Let the child
    // classes LOAD them or GENERATE them.
}

auto OTTransactionType::GetOriginTypeFromString(const String& strType)
    -> originType
{
    originType theType = originType::origin_error_state;

    if (strType.Compare("not_applicable")) {
        theType = originType::not_applicable;
    } else if (strType.Compare("origin_market_offer")) {
        theType = originType::origin_market_offer;
    } else if (strType.Compare("origin_payment_plan")) {
        theType = originType::origin_payment_plan;
    } else if (strType.Compare("origin_smart_contract")) {
        theType = originType::origin_smart_contract;
    } else if (strType.Compare("origin_pay_dividend")) {
        theType = originType::origin_pay_dividend;
    } else {
        theType = originType::origin_error_state;
    }

    return theType;
}

// -----------------------------------

// Used in finalReceipt and paymentReceipt
auto OTTransactionType::GetOriginType() const -> originType
{
    return origin_type_;
}

// Used in finalReceipt and paymentReceipt
void OTTransactionType::SetOriginType(originType theOriginType)
{
    origin_type_ = theOriginType;
}

// -----------------------------------

auto OTTransactionType::GetOriginTypeString() const -> const char*
{
    return GetOriginTypeToString(static_cast<int>(origin_type_));
}

// -----------------------------------

void OTTransactionType::GetNumList(NumList& theOutput)
{
    theOutput.Release();
    theOutput.Add(numlist_);
}

// Allows you to string-search the raw contract.
auto OTTransactionType::Contains(const String& strContains) -> bool
{
    return raw_file_->Contains(strContains);
}

// Allows you to string-search the raw contract.
auto OTTransactionType::Contains(const char* szContains) -> bool
{
    return raw_file_->Contains(szContains);
}

// We'll see if any new bugs pop up after adding this...
//
void OTTransactionType::Release_TransactionType()
{
    // If there were any dynamically allocated objects, clean them up here.

    //  id_.Release();
    account_id_.clear();  // Compare account_id_ to id_ after loading it from
                          // string
                          // or file. They should match, and signature should
                          // verify.

    //  notary_id_->Release(); // Notary ID as used to instantiate the
    //  transaction, based on expected NotaryID.
    account_notary_id_.clear();  // Actual NotaryID within the signed portion.
                                 // (Compare to notary_id_ upon loading.)

    //  account_nym_id_->Release();

    transaction_num_ = 0;
    in_reference_to_transaction_ = 0;
    number_of_origin_ = 0;

    in_reference_to_->Release();  // This item may be in reference to a
                                  // different item

    // This was causing OTLedger to fail loading. Can't set this to true until
    // the END
    // of loading. Todo: Starting reading the END TAGS during load. For example,
    // the OTLedger
    // END TAG could set this back to true...
    //
    //  load_securely_ = true; // defaults to true.

    numlist_.Release();
}

void OTTransactionType::Release()
{
    Release_TransactionType();

    Contract::Release();  // since I've overridden the base class, I call it
                          // now...
}

// OTAccount, OTTransaction, Item, and OTLedger are all derived from
// this class (OTTransactionType). Therefore they can all quickly identify
// whether one of the other components belongs to the same account, using
// this method.
//
auto OTTransactionType::IsSameAccount(const OTTransactionType& rhs) const
    -> bool
{
    if ((GetNymID() != rhs.GetNymID()) ||
        (GetRealAccountID() != rhs.GetRealAccountID()) ||
        (GetRealNotaryID() != rhs.GetRealNotaryID())) {
        return false;
    }

    return true;
}

void OTTransactionType::GetReferenceString(String& theStr) const
{
    in_reference_to_->GetString(theStr);
}

void OTTransactionType::SetReferenceString(const String& theStr)
{
    in_reference_to_->SetString(theStr);
}

// Make sure this contract checks out. Very high level.
// Verifies ID and signature.
// I do NOT call VerifyOwner() here, because the server may
// wish to verify its signature on this account, even though
// the server may not be the actual owner.
// So if you wish to VerifyOwner(), then call it.
auto OTTransactionType::VerifyAccount(const identity::Nym& theNym) -> bool
{
    // Make sure that the supposed AcctID matches the one read from the file.
    //
    if (!VerifyContractID()) {
        LogError()(OT_PRETTY_CLASS())("Error verifying account ID.").Flush();

        return false;
    } else if (!VerifySignature(theNym)) {
        LogError()(OT_PRETTY_CLASS())("Error verifying signature.").Flush();

        return false;
    }

    LogTrace()(OT_PRETTY_CLASS())(
        "We now know that...1) The expected Account ID matches the ID that "
        "was found on the object. 2) The SIGNATURE VERIFIED on the object.")
        .Flush();

    return true;
}

auto OTTransactionType::VerifyContractID() const -> bool
{
    // account_id_ contains the number we read from the xml file
    // we can compare it to the existing and actual identifier.
    // account_id_  contains the "IDENTIFIER" of the object, according to the
    // xml file.
    //
    // Meanwhile id_ contains the same identifier, except it was generated.
    //
    // Now let's compare the two and make sure they match...
    // Also, for this class, we compare NotaryID as well.  They go hand in hand.

    if ((id_ != account_id_) || (notary_id_ != account_notary_id_)) {
        auto str1 = String::Factory(id_, api_.Crypto()),
             str2 = String::Factory(account_id_, api_.Crypto()),
             str3 = String::Factory(notary_id_, api_.Crypto()),
             str4 = String::Factory(account_notary_id_, api_.Crypto());
        LogError()(OT_PRETTY_CLASS())("Identifiers mismatch").Flush();
        LogError()("account_id_ actual: ")(account_id_, api_.Crypto())(
            " expected: ")(id_, api_.Crypto())
            .Flush();
        LogError()("notary_id_ actual: ")(account_notary_id_, api_.Crypto())(
            " expected: ")(notary_id_, api_.Crypto())
            .Flush();

        return false;
    } else {

        return true;
    }
}

// Need to know the transaction number of this transaction? Call this.
auto OTTransactionType::GetTransactionNum() const -> std::int64_t
{
    return transaction_num_;
}

void OTTransactionType::SetTransactionNum(std::int64_t lTransactionNum)
{
    transaction_num_ = lTransactionNum;
}

// virtual
void OTTransactionType::CalculateNumberOfOrigin()
{
    number_of_origin_ = transaction_num_;
}

// Need to know the transaction number of the ORIGINAL transaction? Call this.
// virtual
auto OTTransactionType::GetNumberOfOrigin() -> std::int64_t
{
    if (0 == number_of_origin_) { CalculateNumberOfOrigin(); }

    return number_of_origin_;
}

// Gets WITHOUT calculating.
auto OTTransactionType::GetRawNumberOfOrigin() const -> std::int64_t
{
    return number_of_origin_;
}

void OTTransactionType::SetNumberOfOrigin(std::int64_t lTransactionNum)
{
    number_of_origin_ = lTransactionNum;
}

void OTTransactionType::SetNumberOfOrigin(OTTransactionType& setFrom)
{
    number_of_origin_ = setFrom.GetNumberOfOrigin();
}

// Allows you to compare any OTTransaction or Item to any other OTTransaction
// or Item,
// and see if they share the same origin number.
//
// Let's say Alice sends a transfer #100 to Bob.
// Then Bob receives a pending in his inbox, #800, which is in reference to
// #100.
// Then Bob accepts the pending with processInbox #45, which is in reference to
// #800.
// Then Alice receives a transferReceipt #64, which is in reference to #45.
// Then Alice accepts the transferReceipt with processInbox #91, in reference to
// #64.
//
// ALL OF THOSE transactions and receipts will have origin #100 attached to
// them.
//
auto OTTransactionType::VerifyNumberOfOrigin(OTTransactionType& compareTo)
    -> bool
{
    // Have to use the function here, NOT the internal variable.
    // (Because subclasses may override the function.)
    //
    return (GetNumberOfOrigin() == compareTo.GetNumberOfOrigin());
}

// Need to know the transaction number that this is in reference to? Call this.
auto OTTransactionType::GetReferenceToNum() const -> std::int64_t
{
    return in_reference_to_transaction_;
}

void OTTransactionType::SetReferenceToNum(std::int64_t lTransactionNum)
{
    in_reference_to_transaction_ = lTransactionNum;
}

auto OTTransactionType::GetRealAccountID() const -> identifier::Account
{
    using enum identifier::AccountSubtype;

    return api_.Factory().AccountIDFromHash(id_.Bytes(), custodial_account);
}

OTTransactionType::~OTTransactionType() { Release_TransactionType(); }
}  // namespace opentxs
