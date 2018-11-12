// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/OTTransactionType.hpp"

#include "opentxs/core/transaction/Helpers.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <ostream>

#define OT_METHOD "opentxs::OTTransactionType::"

namespace opentxs
{
// keeping constructor private in order to force people to use the other
// constructors and therefore provide the requisite IDs.
OTTransactionType::OTTransactionType(const api::Core& core)
    : Contract(core)
    , m_AcctID(Identifier::Factory())
    , m_NotaryID(Identifier::Factory())
    , m_AcctNotaryID(Identifier::Factory())
    , m_AcctNymID(Identifier::Factory())
    , m_lTransactionNum(0)
    , m_lInReferenceToTransaction(0)
    , m_lNumberOfOrigin(0)
    , m_originType(originType::not_applicable)
    , m_ascInReferenceTo(Armored::Factory())
    , m_bLoadSecurely(true)
    , m_Numlist()
{
    // this function is private to prevent people from using it.
    // Should never actually get called.

    //  InitTransactionType(); // Just in case.
}

OTTransactionType::OTTransactionType(
    const api::Core& core,
    const Identifier& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    originType theOriginType)
    : Contract(core, theAccountID)
    , m_AcctID(Identifier::Factory())
    , m_NotaryID(Identifier::Factory(theNotaryID))
    , m_AcctNotaryID(Identifier::Factory())
    , m_AcctNymID(Identifier::Factory(theNymID))
    , m_lTransactionNum(0)
    , m_lInReferenceToTransaction(0)
    , m_lNumberOfOrigin(0)
    , m_originType(theOriginType)
    , m_ascInReferenceTo(Armored::Factory())
    , m_bLoadSecurely(true)
    , m_Numlist()
{
    // do NOT set m_AcctID and m_AcctNotaryID here.  Let the child classes
    // LOAD them or GENERATE them.
}

OTTransactionType::OTTransactionType(
    const api::Core& core,
    const Identifier& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    std::int64_t lTransactionNum,
    originType theOriginType)
    : Contract(core, theAccountID)
    , m_AcctID(Identifier::Factory())
    , m_NotaryID(Identifier::Factory(theNotaryID))
    , m_AcctNotaryID(Identifier::Factory())
    , m_AcctNymID(Identifier::Factory(theNymID))
    , m_lTransactionNum(lTransactionNum)
    , m_lInReferenceToTransaction(0)
    , m_lNumberOfOrigin(0)
    , m_originType(theOriginType)
    , m_ascInReferenceTo(Armored::Factory())
    , m_bLoadSecurely(true)
    , m_Numlist()
{
    // do NOT set m_AcctID and m_AcctNotaryID here.  Let the child classes
    // LOAD them or GENERATE them.
}

originType OTTransactionType::GetOriginTypeFromString(const String& strType)
{
    originType theType = originType::origin_error_state;

    if (strType.Compare("not_applicable"))
        theType = originType::not_applicable;
    else if (strType.Compare("origin_market_offer"))
        theType = originType::origin_market_offer;
    else if (strType.Compare("origin_payment_plan"))
        theType = originType::origin_payment_plan;
    else if (strType.Compare("origin_smart_contract"))
        theType = originType::origin_smart_contract;
    else if (strType.Compare("origin_pay_dividend"))
        theType = originType::origin_pay_dividend;
    else
        theType = originType::origin_error_state;

    return theType;
}

// -----------------------------------

// Used in finalReceipt and paymentReceipt
originType OTTransactionType::GetOriginType() const { return m_originType; }

// Used in finalReceipt and paymentReceipt
void OTTransactionType::SetOriginType(originType theOriginType)
{
    m_originType = theOriginType;
}

// -----------------------------------

const char* OTTransactionType::GetOriginTypeString() const
{
    return GetOriginTypeToString(static_cast<int>(m_originType));
}

// -----------------------------------

void OTTransactionType::GetNumList(NumList& theOutput)
{
    theOutput.Release();
    theOutput.Add(m_Numlist);
}

// Allows you to string-search the raw contract.
bool OTTransactionType::Contains(const String& strContains)
{
    return m_strRawFile->Contains(strContains);
}

// Allows you to string-search the raw contract.
bool OTTransactionType::Contains(const char* szContains)
{
    return m_strRawFile->Contains(szContains);
}

// We'll see if any new bugs pop up after adding this...
//
void OTTransactionType::Release_TransactionType()
{
    // If there were any dynamically allocated objects, clean them up here.

    //  m_ID.Release();
    m_AcctID->Release();  // Compare m_AcctID to m_ID after loading it from
                          // string
                          // or file. They should match, and signature should
                          // verify.

    //  m_NotaryID->Release(); // Notary ID as used to instantiate the
    //  transaction, based on expected NotaryID.
    m_AcctNotaryID->Release();  // Actual NotaryID within the signed portion.
                                // (Compare to m_NotaryID upon loading.)

    //  m_AcctNymID->Release();

    m_lTransactionNum = 0;
    m_lInReferenceToTransaction = 0;
    m_lNumberOfOrigin = 0;

    m_ascInReferenceTo->Release();  // This item may be in reference to a
                                    // different item

    // This was causing OTLedger to fail loading. Can't set this to true until
    // the END
    // of loading. Todo: Starting reading the END TAGS during load. For example,
    // the OTLedger
    // END TAG could set this back to true...
    //
    //  m_bLoadSecurely = true; // defaults to true.

    m_Numlist.Release();
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
bool OTTransactionType::IsSameAccount(const OTTransactionType& rhs) const
{
    if ((GetNymID() != rhs.GetNymID()) ||
        (GetRealAccountID() != rhs.GetRealAccountID()) ||
        (GetRealNotaryID() != rhs.GetRealNotaryID()))
        return false;
    return true;
}

void OTTransactionType::GetReferenceString(String& theStr) const
{
    m_ascInReferenceTo->GetString(theStr);
}

void OTTransactionType::SetReferenceString(const String& theStr)
{
    m_ascInReferenceTo->SetString(theStr);
}

// Make sure this contract checks out. Very high level.
// Verifies ID and signature.
// I do NOT call VerifyOwner() here, because the server may
// wish to verify its signature on this account, even though
// the server may not be the actual owner.
// So if you wish to VerifyOwner(), then call it.
bool OTTransactionType::VerifyAccount(const Nym& theNym)
{
    // Make sure that the supposed AcctID matches the one read from the file.
    //
    if (!VerifyContractID()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error verifying account ID.")
            .Flush();

        return false;
    } else if (!VerifySignature(theNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error verifying signature.")
            .Flush();

        return false;
    }

    LogTrace(OT_METHOD)(__FUNCTION__)(
        ": We now know that...1) The expected Account ID matches the ID that "
        "was found on the object. 2) The SIGNATURE VERIFIED on the object.")
        .Flush();

    return true;
}

bool OTTransactionType::VerifyContractID() const
{
    // m_AcctID contains the number we read from the xml file
    // we can compare it to the existing and actual identifier.
    // m_AcctID  contains the "IDENTIFIER" of the object, according to the xml
    // file.
    //
    // Meanwhile m_ID contains the same identifier, except it was generated.
    //
    // Now let's compare the two and make sure they match...
    // Also, for this class, we compare NotaryID as well.  They go hand in hand.

    if ((m_ID != m_AcctID) || (m_NotaryID != m_AcctNotaryID)) {
        auto str1 = String::Factory(m_ID), str2 = String::Factory(m_AcctID),
             str3 = String::Factory(m_NotaryID),
             str4 = String::Factory(m_AcctNotaryID);
        LogOutput(OT_METHOD)(__FUNCTION__)(": Identifiers mismatch").Flush();
        LogOutput("m_AcctID actual: ")(m_AcctID)(" expected: ")(m_ID).Flush();
        LogOutput("m_NotaryID actual: ")(m_AcctNotaryID)(" expected: ")(
            m_NotaryID)
            .Flush();

        return false;
    } else {

        return true;
    }
}

// Need to know the transaction number of this transaction? Call this.
std::int64_t OTTransactionType::GetTransactionNum() const
{
    return m_lTransactionNum;
}

void OTTransactionType::SetTransactionNum(std::int64_t lTransactionNum)
{
    m_lTransactionNum = lTransactionNum;
}

// virtual
void OTTransactionType::CalculateNumberOfOrigin()
{
    m_lNumberOfOrigin = m_lTransactionNum;
}

// Need to know the transaction number of the ORIGINAL transaction? Call this.
// virtual
std::int64_t OTTransactionType::GetNumberOfOrigin()
{
    if (0 == m_lNumberOfOrigin) CalculateNumberOfOrigin();

    return m_lNumberOfOrigin;
}

// Gets WITHOUT calculating.
std::int64_t OTTransactionType::GetRawNumberOfOrigin() const
{
    return m_lNumberOfOrigin;
}

void OTTransactionType::SetNumberOfOrigin(std::int64_t lTransactionNum)
{
    m_lNumberOfOrigin = lTransactionNum;
}

void OTTransactionType::SetNumberOfOrigin(OTTransactionType& setFrom)
{
    m_lNumberOfOrigin = setFrom.GetNumberOfOrigin();
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
bool OTTransactionType::VerifyNumberOfOrigin(OTTransactionType& compareTo)
{
    // Have to use the function here, NOT the internal variable.
    // (Because subclasses may override the function.)
    //
    return (GetNumberOfOrigin() == compareTo.GetNumberOfOrigin());
}

// Need to know the transaction number that this is in reference to? Call this.
std::int64_t OTTransactionType::GetReferenceToNum() const
{
    return m_lInReferenceToTransaction;
}

void OTTransactionType::SetReferenceToNum(std::int64_t lTransactionNum)
{
    m_lInReferenceToTransaction = lTransactionNum;
}

OTTransactionType::~OTTransactionType() { Release_TransactionType(); }
}  // namespace opentxs
