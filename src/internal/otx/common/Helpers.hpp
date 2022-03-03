// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/otx/common/Account.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
auto TranslateAccountTypeStringToEnum(const String& acctTypeString) noexcept
    -> Account::AccountType;
auto TranslateAccountTypeToString(
    Account::AccountType type,
    String& acctType) noexcept -> void;
}  // namespace opentxs
