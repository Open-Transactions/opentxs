// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/AccountVisitor.hpp"  // IWYU pragma: associated

namespace opentxs
{
AccountVisitor::AccountVisitor(
    const api::session::Wallet& wallet,
    const identifier::Notary& notaryID)
    : wallet_(wallet)
    , notary_id_(notaryID)
    , loaded_accounts_(nullptr)
{
}
}  // namespace opentxs
