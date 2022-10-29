// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/blockchain/regtest/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Context;
}  // namespace api

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
class OPENTXS_EXPORT Regtest_fixture_normal : public Regtest_fixture_base
{
protected:
    Regtest_fixture_normal(const ot::api::Context& ot, const int clientCount);
    Regtest_fixture_normal(
        const ot::api::Context& ot,
        const int clientCount,
        ot::Options clientArgs);
};
}  // namespace ottest
