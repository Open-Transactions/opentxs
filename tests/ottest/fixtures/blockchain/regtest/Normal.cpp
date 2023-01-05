// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/regtest/Normal.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <utility>

namespace ottest
{
Regtest_fixture_normal::Regtest_fixture_normal(
    const ot::api::Context& ot,
    const int clientCount,
    ot::Options minerArgs,
    ot::Options clientArgs)
    : Regtest_fixture_base(
          ot,
          true,
          clientCount,
          std::move(minerArgs),
          std::move(clientArgs))
{
}

Regtest_fixture_normal::Regtest_fixture_normal(
    const ot::api::Context& ot,
    const int clientCount,
    ot::Options clientArgs)
    : Regtest_fixture_base(
          ot,
          true,
          clientCount,
          ot::Options{},
          std::move(clientArgs))
{
}

Regtest_fixture_normal::Regtest_fixture_normal(
    const ot::api::Context& ot,
    const int clientCount)
    : Regtest_fixture_normal(ot, clientCount, ot::Options{})
{
}
}  // namespace ottest
