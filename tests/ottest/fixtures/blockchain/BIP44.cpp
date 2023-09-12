// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/BIP44.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <memory>

#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/LogMacros.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;
using namespace opentxs::literals;

bool BIP44::init_{false};

ot::DeferredConstruction<ot::Nym_p> BIP44::nym_{};
ot::DeferredConstruction<ot::crypto::SeedID> BIP44::seed_id_{};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-reference"  // NOLINT
BIP44::BIP44()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , reason_(api_.Factory().PasswordPrompt(__func__))
    , nym_id_([&]() -> const ot::identifier::Nym& {
        if (false == init_) {
            const auto words = api_.Factory().SecretFromText(
                GetPaymentCodeVector3().alice_.words_);
            const auto phrase = api_.Factory().Secret(0);
            seed_id_.set_value(api_.Crypto().Seed().ImportSeed(
                words,
                phrase,
                ot::crypto::SeedStyle::BIP39,
                ot::crypto::Language::en,
                reason_));
            const auto& seed = seed_id_.get();

            OT_ASSERT(false == seed.empty());

            nym_.set_value(
                api_.Wallet().Nym({api_.Factory(), seed, 0}, reason_, "Alice"));
            const auto& nym = nym_.get();

            OT_ASSERT(nym);

            api_.Crypto().Blockchain().NewHDSubaccount(
                nym->ID(),
                ot::blockchain::crypto::HDProtocol::BIP_44,
                ot::blockchain::Type::UnitTest,
                reason_);
            init_ = true;
        }

        return nym_.get()->ID();
    }())
    , account_(api_.Crypto()
                   .Blockchain()
                   .Account(nym_id_, ot::blockchain::Type::UnitTest)
                   .GetHD()
                   .at(0))
    , account_id_(api_.Factory().IdentifierFromBase58(account_id_base58_))
{
}
#pragma GCC diagnostic pop
}  // namespace ottest
