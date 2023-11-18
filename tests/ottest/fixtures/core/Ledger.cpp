// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/core/Ledger.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "internal/otx/Types.hpp"
#include "internal/otx/common/Ledger.hpp"  // IWYU pragma: keep
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
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

auto Ledger::get_nymbox(
    const ot::identifier::Nym& nym,
    const ot::identifier::Notary& server,
    bool create) const noexcept -> std::unique_ptr<opentxs::Ledger>
{
    return client_.Factory().Internal().Session().Ledger(
        nym, nym, server, ot::ledgerType::nymbox, create);
}
}  // namespace ottest
