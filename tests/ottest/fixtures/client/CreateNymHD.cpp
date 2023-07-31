// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/client/CreateNymHD.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cassert>
#include <memory>

#include "internal/api/session/Client.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;

CreateNymHD::CreateNymHD()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , reason_(api_.Factory().PasswordPrompt(__func__))
    // these fingerprints are deterministic so we can share them among tests
    , seed_a_(api_.InternalClient().Exec().Wallet_ImportSeed(
          "spike nominee miss inquiry fee nothing belt list other daughter "
          "leave valley twelve gossip paper",
          ""))
    , seed_b_(api_.InternalClient().Exec().Wallet_ImportSeed(
          "glimpse destroy nation advice seven useless candy move number "
          "toast insane anxiety proof enjoy lumber",
          ""))
    , seed_c_(api_.InternalClient().Exec().Wallet_ImportSeed(
          "park cabbage quit",
          ""))
    , seed_d_(api_.InternalClient().Exec().Wallet_ImportSeed(
          "federal dilemma rare",
          ""))
    , alice_(api_.Wallet()
                 .Nym({api_.Factory(), seed_a_, 0, 1}, reason_, "Alice")
                 ->ID()
                 .asBase58(api_.Crypto()))
    , bob_(api_.Wallet()
               .Nym({api_.Factory(), seed_b_, 0, 1}, reason_, "Bob")
               ->ID()
               .asBase58(api_.Crypto()))
{
    assert(false == alice_.empty());
    assert(false == bob_.empty());
}

}  // namespace ottest
