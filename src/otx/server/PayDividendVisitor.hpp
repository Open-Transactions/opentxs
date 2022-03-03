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
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace identifier
{
class Notary;
}  // namespace identifier

namespace server
{
class Server;
}  // namespace server

class PasswordPrompt;
// }  // namespace v1
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
    const OTNymID nymId_;
    const OTUnitID payoutUnitTypeId_;
    const OTIdentifier voucherAcctId_;
    OTString m_pstrMemo;  // contains the original payDividend item from
                          // the payDividend transaction request.
                          // (Stored in the memo field for each
                          // voucher.)
    Amount m_lPayoutPerShare{0};
    Amount m_lAmountPaidOut{0};   // as we pay each voucher out, we keep a
                                  // running count.
    Amount m_lAmountReturned{0};  // as we pay each voucher out, we keep a
                                  // running count.

    PayDividendVisitor() = delete;

public:
    PayDividendVisitor(
        server::Server& theServer,
        const identifier::Notary& theNotaryID,
        const identifier::Nym& theNymID,
        const identifier::UnitDefinition& thePayoutUnitTypeId,
        const Identifier& theVoucherAcctID,
        const String& strMemo,
        const Amount& lPayoutPerShare);

    auto GetNymID() -> const identifier::Nym& { return nymId_; }
    auto GetPayoutUnitTypeId() -> const identifier::UnitDefinition&
    {
        return payoutUnitTypeId_;
    }
    auto GetVoucherAcctID() -> const Identifier& { return voucherAcctId_; }
    auto GetMemo() -> OTString { return m_pstrMemo; }
    auto GetServer() -> server::Server& { return server_; }
    auto GetPayoutPerShare() -> const Amount& { return m_lPayoutPerShare; }
    auto GetAmountPaidOut() -> const Amount& { return m_lAmountPaidOut; }
    auto GetAmountReturned() -> const Amount& { return m_lAmountReturned; }

    auto Trigger(const Account& theAccount, const PasswordPrompt& reason)
        -> bool final;

    ~PayDividendVisitor() final;
};
}  // namespace opentxs
