// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "ottest/fixtures/blockchain/SyncListener.hpp"
// IWYU pragma: no_include "ottest/fixtures/blockchain/TXOState.hpp"

#include "ottest/fixtures/blockchain/regtest/SyncClient.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

namespace ottest
{
Regtest_fixture_sync_client::Regtest_fixture_sync_client()
    : Regtest_fixture_normal(
          ot_,
          1,
          ot::Options{}.SetBlockchainWalletEnabled(false))
{
}
}  // namespace ottest
