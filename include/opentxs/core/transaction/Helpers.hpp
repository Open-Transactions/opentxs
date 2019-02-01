// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_TRANSACTION_HELPERS_HPP
#define OPENTXS_CORE_TRANSACTION_HELPERS_HPP

#include "opentxs/Forward.hpp"
#include "opentxs/Types.hpp"

#include "opentxs/core/OTTransaction.hpp"

#include <cstdint>

namespace opentxs
{

class String;
class OTTransaction;
class Ledger;
class Identifier;
class NumList;

EXPORT const char* GetTransactionTypeString(
    int transactionTypeIndex);  // enum transactionType
EXPORT const char* GetOriginTypeToString(int originTypeIndex);  // enum
                                                                // originType

std::int32_t LoadAbbreviatedRecord(
    irr::io::IrrXMLReader*& xml,
    std::int64_t& lNumberOfOrigin,
    originType& theOriginType,
    std::int64_t& lTransactionNum,
    std::int64_t& lInRefTo,
    std::int64_t& lInRefDisplay,
    time64_t& the_DATE_SIGNED,
    transactionType& theType,
    String& strHash,
    std::int64_t& lAdjustment,
    std::int64_t& lDisplayValue,
    std::int64_t& lClosingNum,
    std::int64_t& lRequestNum,
    bool& bReplyTransSuccess,
    NumList* pNumList = nullptr);

EXPORT bool VerifyBoxReceiptExists(
    const std::string& dataFolder,
    const Identifier& NOTARY_ID,
    const identifier::Nym& NYM_ID,
    const Identifier& ACCOUNT_ID,  // If for Nymbox (vs inbox/outbox) then
    // pass NYM_ID in this field also.
    std::int32_t nBoxType,  // 0/nymbox, 1/inbox, 2/outbox
    const std::int64_t& lTransactionNum);

std::unique_ptr<OTTransaction> LoadBoxReceipt(
    OTTransaction& theAbbrev,
    Ledger& theLedger);

EXPORT std::unique_ptr<OTTransaction> LoadBoxReceipt(
    OTTransaction& theAbbrev,
    std::int64_t lLedgerType);

bool SetupBoxReceiptFilename(
    std::int64_t lLedgerType,
    OTTransaction& theTransaction,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename);

bool SetupBoxReceiptFilename(
    Ledger& theLedger,
    OTTransaction& theTransaction,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename);

bool SetupBoxReceiptFilename(
    std::int64_t lLedgerType,
    const String& strUserOrAcctID,
    const String& strNotaryID,
    const std::int64_t& lTransactionNum,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename);

}  // namespace opentxs

#endif
