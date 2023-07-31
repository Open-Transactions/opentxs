// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/client/CreateNymHD.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <cassert>
#include <memory>
#include <string_view>

#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;
using namespace std::literals;

CreateNymHD::CreateNymHD()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , reason_(api_.Factory().PasswordPrompt(__func__))
    // these fingerprints are deterministic so we can share them among tests
    , seed_a_(api_.Crypto().Seed().ImportSeed(
          api_.Factory().SecretFromText(
              "spike nominee miss inquiry fee nothing belt list other daughter leave valley twelve gossip paper"sv),
          api_.Factory().SecretFromText(""sv),
          opentxs::crypto::SeedStyle::BIP39,
          opentxs::crypto::Language::en,
          api_.Factory().PasswordPrompt("Importing a BIP-39 seed")))
    , seed_b_(api_.Crypto().Seed().ImportSeed(
          api_.Factory().SecretFromText(
              "glimpse destroy nation advice seven useless candy move number toast insane anxiety proof enjoy lumber"sv),
          api_.Factory().SecretFromText(""sv),
          opentxs::crypto::SeedStyle::BIP39,
          opentxs::crypto::Language::en,
          api_.Factory().PasswordPrompt("Importing a BIP-39 seed")))
    , seed_c_(api_.Crypto().Seed().ImportSeed(
          api_.Factory().SecretFromText("park cabbage quit"sv),
          api_.Factory().SecretFromText(""sv),
          opentxs::crypto::SeedStyle::BIP39,
          opentxs::crypto::Language::en,
          api_.Factory().PasswordPrompt("Importing a BIP-39 seed")))
    , seed_d_(api_.Crypto().Seed().ImportSeed(
          api_.Factory().SecretFromText("federal dilemma rare"sv),
          api_.Factory().SecretFromText(""sv),
          opentxs::crypto::SeedStyle::BIP39,
          opentxs::crypto::Language::en,
          api_.Factory().PasswordPrompt("Importing a BIP-39 seed")))
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
