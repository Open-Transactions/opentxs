// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/TXOState.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

namespace ottest
{
TXOState::TXOState() noexcept
    : wallet_()
    , nyms_()
{
}

TXOState::NymData::NymData() noexcept
    : nym_()
    , accounts_()
{
}

TXOState::Data::Data() noexcept
    : balance_()
    , data_()
{
}
}  // namespace ottest
