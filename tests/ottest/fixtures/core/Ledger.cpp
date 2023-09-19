// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/core/Ledger.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;

namespace ottest
{
Ledger::Ledger()
    : client_(OTTestEnvironment::GetOT().StartClientSession(0))
    , server_(OTTestEnvironment::GetOT().StartNotarySession(0))
    , reason_c_(client_.Factory().PasswordPrompt(__func__))
    , reason_s_(server_.Factory().PasswordPrompt(__func__))
{
}
}  // namespace ottest
