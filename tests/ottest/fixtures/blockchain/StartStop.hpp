// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

#include "ottest/Basic.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace session
{
class Client;
}  // namespace session
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
namespace b = ot::blockchain;

class OPENTXS_EXPORT Test_StartStop : public ::testing::Test
{
public:
    const ot::api::session::Client& api_;

    Test_StartStop()
        : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    {
    }
};
}  // namespace ottest
