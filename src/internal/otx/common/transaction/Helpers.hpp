// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <memory>

#include "internal/otx/Types.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
}  // namespace identifier

class Amount;
class Identifier;
class Ledger;
class NumList;
class OTTransaction;
class String;

auto GetTransactionTypeString(int transactionTypeIndex) -> const
    char*;  // enum transactionType
auto GetOriginTypeToString(int originTypeIndex) -> const char*;  // enum
                                                                 // originType

auto LoadAbbreviatedRecord(
    irr::io::IrrXMLReader*& xml,
    std::int64_t& lNumberOfOrigin,
    originType& theOriginType,
    std::int64_t& lTransactionNum,
    std::int64_t& lInRefTo,
    std::int64_t& lInRefDisplay,
    Time& the_DATE_SIGNED,
    transactionType& theType,
    String& strHash,
    Amount& lAdjustment,
    Amount& lDisplayValue,
    std::int64_t& lClosingNum,
    std::int64_t& lRequestNum,
    bool& bReplyTransSuccess,
    NumList* pNumList = nullptr) -> std::int32_t;

auto VerifyBoxReceiptExists(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const identifier::Notary& NOTARY_ID,
    const identifier::Nym& NYM_ID,
    const Identifier& ACCOUNT_ID,  // If for Nymbox (vs inbox/outbox) then
    // pass NYM_ID in this field also.
    std::int32_t nBoxType,  // 0/nymbox, 1/inbox, 2/outbox
    const std::int64_t& lTransactionNum) -> bool;

auto LoadBoxReceipt(
    const api::Session& api,
    OTTransaction& theAbbrev,
    Ledger& theLedger) -> std::unique_ptr<OTTransaction>;

auto LoadBoxReceipt(
    const api::Session& api,
    OTTransaction& theAbbrev,
    std::int64_t lLedgerType) -> std::unique_ptr<OTTransaction>;

auto SetupBoxReceiptFilename(
    const api::Session& api,
    std::int64_t lLedgerType,
    OTTransaction& theTransaction,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename) -> bool;

auto SetupBoxReceiptFilename(
    const api::Session& api,
    Ledger& theLedger,
    OTTransaction& theTransaction,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename) -> bool;

auto SetupBoxReceiptFilename(
    const api::Session& api,
    std::int64_t lLedgerType,
    const String& strUserOrAcctID,
    const String& strNotaryID,
    const std::int64_t& lTransactionNum,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename) -> bool;
}  // namespace opentxs
