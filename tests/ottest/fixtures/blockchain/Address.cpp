// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/Address.hpp"  // IWYU pragma: associated

#include "opentxs/opentxs.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;

Address::Address()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
{
}
}  // namespace ottest
