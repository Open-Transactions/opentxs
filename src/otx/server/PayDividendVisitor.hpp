// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/AccountVisitor.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Notary;
}  // namespace identifier

namespace server
{
class Server;
}  // namespace server

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
// Note: from OTUnitDefinition.h and .cpp.
// This is a subclass of AccountVisitor, which is used whenever OTUnitDefinition
// needs to loop through all the accounts for a given instrument definition (its
// own.) This subclass needs to call Server method to do its job, so it can't be
// defined in otlib, but must be defined here in otserver (so it can see the
// methods that it needs...)
class PayDividendVisitor final : public AccountVisitor
{
    server::Server& server_;
    const identifier::Nym nym_id_;
    const identifier::UnitDefinition payout_unit_type_id_;
    const identifier::Generic voucher_acct_id_;
    OTString memo_;  // contains the original payDividend item from
                     // the payDividend transaction request.
                     // (Stored in the memo field for each
                     // voucher.)
    Amount payout_per_share_{0};
    Amount amount_paid_out_{0};  // as we pay each voucher out, we keep a
                                 // running count.
    Amount amount_returned_{0};  // as we pay each voucher out, we keep a
                                 // running count.

public:
    auto GetNymID() -> const identifier::Nym& { return nym_id_; }
    auto GetPayoutUnitTypeId() -> const identifier::UnitDefinition&
    {
        return payout_unit_type_id_;
    }
    auto GetVoucherAcctID() -> const identifier::Generic&
    {
        return voucher_acct_id_;
    }
    auto GetMemo() -> OTString { return memo_; }
    auto GetServer() -> server::Server& { return server_; }
    auto GetPayoutPerShare() -> const Amount& { return payout_per_share_; }
    auto GetAmountPaidOut() -> const Amount& { return amount_paid_out_; }
    auto GetAmountReturned() -> const Amount& { return amount_returned_; }

    auto Trigger(const Account& theAccount, const PasswordPrompt& reason)
        -> bool final;

    PayDividendVisitor(
        server::Server& theServer,
        const identifier::Notary& theNotaryID,
        const identifier::Nym& theNymID,
        const identifier::UnitDefinition& thePayoutUnitTypeId,
        const identifier::Generic& theVoucherAcctID,
        const String& strMemo,
        const Amount& lPayoutPerShare);
    PayDividendVisitor() = delete;

    ~PayDividendVisitor() final;
};
}  // namespace opentxs
