// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/identity/Authority.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <memory>

#include "2_Factory.hpp"
#include "internal/api/crypto/Seed.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/identity/Authority.hpp"
#include "internal/identity/Nym.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"

namespace ottest
{
Authority::Authority()
    : client_(ot::Context().StartClientSession(0))
    , reason_(client_.Factory().PasswordPrompt(__func__))
    , non_const_reason_(client_.Factory().PasswordPrompt(__func__))
    , words_(client_.Factory().SecretFromText(
          ottest::GetPaymentCodeVector3().alice_.words_))
    , parameters_()
    , source_{nullptr}
    , internal_nym_{nullptr}
    , authority_{nullptr}
{
}

auto Authority::GetTag(
    const ot::crypto::asymmetric::Key& key,
    const ot::crypto::asymmetric::Key& dh,
    std::uint32_t& out) noexcept -> bool
{
    return key.Internal().CalculateTag(
        dh, authority_->GetMasterCredID(), reason_, out);
}

void Authority::SetUp()
{
    const auto& seeds = client_.Crypto().Seed().Internal();
    parameters_.SetCredset(0);
    auto nymIndex = ot::Bip32Index{0};
    auto fingerprint = parameters_.Seed();
    auto style = parameters_.SeedStyle();
    auto lang = parameters_.SeedLanguage();

    seeds.GetOrCreateDefaultSeed(
        fingerprint,
        style,
        lang,
        nymIndex,
        parameters_.SeedStrength(),
        reason_);

    auto seed = seeds.GetSeed(fingerprint, nymIndex, reason_);
    const auto defaultIndex = parameters_.UseAutoIndex();

    if (false == defaultIndex) { nymIndex = parameters_.Nym(); }

    const auto newIndex = static_cast<std::int32_t>(nymIndex) + 1;
    seeds.UpdateIndex(fingerprint, newIndex, reason_);
    parameters_.SetEntropy(seed);
    parameters_.SetSeed(fingerprint);
    parameters_.SetNym(nymIndex);

    source_.reset(ot::Factory::NymIDSource(client_, parameters_, reason_));

    internal_nym_.reset(ot::Factory::Nym(
        client_, parameters_, ot::identity::Type::individual, alias_, reason_));

    const auto& nn =
        dynamic_cast<const opentxs::identity::Nym&>(*internal_nym_);

    authority_.reset(ot::Factory().Authority(
        client_, nn, *source_, parameters_, version_, reason_));
}
}  // namespace ottest
