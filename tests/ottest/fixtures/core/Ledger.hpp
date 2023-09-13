// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

namespace ot = opentxs;

namespace ottest
{
struct OPENTXS_EXPORT Ledger : public ::testing::Test {
    const ot::api::session::Client& client_;
    const ot::api::session::Notary& server_;
    ot::PasswordPrompt reason_c_;
    ot::PasswordPrompt reason_s_;

    Ledger();
};
}  // namespace ottest
