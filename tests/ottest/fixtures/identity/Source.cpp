// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/identity/Source.hpp"  // IWYU pragma: associated

#include <NymIDSource.pb.h>  // IWYU pragma: keep
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <memory>

#include "2_Factory.hpp"
#include "internal/api/crypto/Seed.hpp"
#include "internal/identity/Authority.hpp"
#include "internal/identity/Nym.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;

namespace ottest
{
Source::Source()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , reason_(api_.Factory().PasswordPrompt(__func__))
    , words_(api_.Factory().SecretFromText(
          ottest::GetPaymentCodeVector3().alice_.words_))
    , phrase_(api_.Factory().SecretFromText(
          ottest::GetPaymentCodeVector3().alice_.words_))
    , parameters_(api_.Factory())
    , source_{nullptr}
    , internal_nym_{nullptr}
    , authority_{nullptr}
{
}

void Source::Authority()
{
    const auto& seeds = api_.Crypto().Seed().Internal();
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

    source_.reset(ot::Factory::NymIDSource(api_, parameters_, reason_));

    internal_nym_.reset(ot::Factory::Nym(
        api_, parameters_, ot::identity::Type::individual, alias_, reason_));

    const auto& nn =
        dynamic_cast<const opentxs::identity::Nym&>(*internal_nym_);

    authority_.reset(
        ot::Factory().Authority(api_, nn, *source_, parameters_, 6, reason_));
}

void Source::setupSourceForBip47(ot::crypto::SeedStyle seedStyle)
{
    auto seed = api_.Crypto().Seed().ImportSeed(
        words_, phrase_, seedStyle, ot::crypto::Language::en, reason_);

    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::Parameters::DefaultType(),
        ot::crypto::Parameters::DefaultCredential(),
        ot::identity::SourceType::Bip47,
        version_};
    parameters.SetSeed(seed);
    parameters.SetNym(nym_);

    source_.reset(ot::Factory::NymIDSource(api_, parameters, reason_));
}

void Source::setupSourceForPubKey(ot::crypto::SeedStyle seedStyle)
{
    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::asymmetric::Algorithm::Secp256k1,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        version_};

    auto seed = api_.Crypto().Seed().ImportSeed(
        words_, phrase_, seedStyle, ot::crypto::Language::en, reason_);

    parameters.SetSeed(seed);
    parameters.SetNym(nym_);
    source_.reset(ot::Factory::NymIDSource(api_, parameters, reason_));
}
}  // namespace ottest
