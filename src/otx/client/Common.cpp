// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/client/Client.hpp"  // IWYU pragma: associated

namespace opentxs
{
auto operator<(
    const OT_DownloadNymboxType&,
    const OT_DownloadNymboxType&) noexcept -> bool
{
    return false;
}

auto operator<(
    const OT_GetTransactionNumbersType&,
    const OT_GetTransactionNumbersType&) noexcept -> bool
{
    return false;
}
}  // namespace opentxs
