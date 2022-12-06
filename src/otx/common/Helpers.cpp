// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/Helpers.hpp"  // IWYU pragma: associated

#include "internal/core/String.hpp"
#include "internal/otx/common/Account.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto TranslateAccountTypeStringToEnum(const String& acctTypeString) noexcept
    -> Account::AccountType
{
    Account::AccountType acctType = Account::err_acct;

    if (acctTypeString.Compare("user")) {
        acctType = Account::user;
    } else if (acctTypeString.Compare("issuer")) {
        acctType = Account::issuer;
    } else if (acctTypeString.Compare("basket")) {
        acctType = Account::basket;
    } else if (acctTypeString.Compare("basketsub")) {
        acctType = Account::basketsub;
    } else if (acctTypeString.Compare("mint")) {
        acctType = Account::mint;
    } else if (acctTypeString.Compare("voucher")) {
        acctType = Account::voucher;
    } else if (acctTypeString.Compare("stash")) {
        acctType = Account::stash;
    } else {
        LogError()(": Error: Unknown account type: ")(acctTypeString)(".")
            .Flush();
    }

    return acctType;
}

void TranslateAccountTypeToString(
    Account::AccountType type,
    String& acctType) noexcept
{
    switch (type) {
        case Account::user: {
            acctType.Set("user");
        } break;
        case Account::issuer: {
            acctType.Set("issuer");
        } break;
        case Account::basket: {
            acctType.Set("basket");
        } break;
        case Account::basketsub: {
            acctType.Set("basketsub");
        } break;
        case Account::mint: {
            acctType.Set("mint");
        } break;
        case Account::voucher: {
            acctType.Set("voucher");
        } break;
        case Account::stash: {
            acctType.Set("stash");
        } break;
        case Account::err_acct:
        default: {
            acctType.Set("err_acct");
        }
    }
}
}  // namespace opentxs
