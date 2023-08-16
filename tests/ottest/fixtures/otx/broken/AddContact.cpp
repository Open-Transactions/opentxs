// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/otx/broken/AddContact.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"

namespace ottest
{
namespace ot = opentxs;

const bool AddContact::have_hd_{
    ot::api::crypto::HaveHDKeys() &&
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Secp256k1)};

AddContact::AddContact()
    : api_alex_(OTTestEnvironment::GetOT().StartClientSession(0))
    , api_bob_(OTTestEnvironment::GetOT().StartClientSession(1))
    , api_chris_(OTTestEnvironment::GetOT().StartClientSession(2))
    , api_server_1_(OTTestEnvironment::GetOT().StartNotarySession(0))
{
    const_cast<Server&>(server_1_).init(api_server_1_);
    const_cast<User&>(alex_).init(api_alex_, server_1_);
    const_cast<User&>(bob_).init(api_bob_, server_1_);
    const_cast<User&>(chris_).init(api_chris_, server_1_);
}

}  // namespace ottest
