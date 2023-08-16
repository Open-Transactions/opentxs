// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/core/contract/ServerContract.hpp"

#define ALEX "Alice"
#define BOB "Bob"
#define ISSUER "Issuer"

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT DepositCheques : public ::testing::Test
{
public:
    static bool init_;
    static const bool have_hd_;
    static const ot::crypto::SeedID SeedA_;
    static const ot::crypto::SeedID SeedB_;
    static const ot::crypto::SeedID SeedC_;
    static const ot::identifier::Nym alice_nym_id_;
    static const ot::identifier::Nym bob_nym_id_;
    static const ot::identifier::Nym issuer_nym_id_;
    static ot::identifier::Generic contact_id_alice_bob_;
    static ot::identifier::Generic contact_id_alice_issuer_;
    static ot::identifier::Generic contact_id_bob_alice_;
    static ot::identifier::Generic contact_id_bob_issuer_;
    static ot::identifier::Generic contact_id_issuer_alice_;
    static ot::identifier::Generic contact_id_issuer_bob_;

    static const ot::api::session::Client* alice_;
    static const ot::api::session::Client* bob_;

    static ot::UnallocatedCString alice_payment_code_;
    static ot::UnallocatedCString bob_payment_code_;
    static ot::UnallocatedCString issuer_payment_code_;

    static ot::identifier::UnitDefinition unit_id_;
    static ot::identifier::Account alice_account_id_;
    static ot::identifier::Account issuer_account_id_;

    const ot::api::session::Client& alice_client_;
    const ot::api::session::Client& bob_client_;
    const ot::api::session::Notary& server_1_;
    const ot::api::session::Client& issuer_client_;
    const ot::OTServerContract server_contract_;

    DepositCheques();
    void import_server_contract(
        const ot::contract::Server& contract,
        const ot::api::session::Client& client);
    void init();
};

}  // namespace ottest
