// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "internal/api/session/Client.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/util/LogMacros.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;

class Messages : public ::testing::Test
{
public:
    static const ot::UnallocatedCString SeedA_;
    static const ot::UnallocatedCString Alice_;
    static const ot::identifier::Nym alice_nym_id_;

    const ot::api::session::Client& client_;
    const ot::api::session::Notary& server_;
    ot::PasswordPrompt reason_c_;
    ot::PasswordPrompt reason_s_;
    const ot::identifier::Notary& server_id_;
    const ot::OTServerContract server_contract_;

    Messages();

    void import_server_contract(
        const ot::contract::Server& contract,
        const ot::api::session::Client& client);
    void init();
};

}  // namespace ottest
