// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/identity/wot/verification/Factory.hpp"  // IWYU pragma: associated

#include <Verification.pb.h>
#include <stdexcept>
#include <utility>

#include "identity/wot/verification/verification/Implementation.hpp"
#include "identity/wot/verification/verification/VerificationPrivate.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto Verification(
    const api::Session& api,
    const identifier::Nym& verifier,
    const opentxs::PasswordPrompt& reason,
    identity::wot::ClaimID claim,
    identity::wot::verification::Type value,
    Time start,
    Time stop,
    std::span<const identity::wot::VerificationID> superscedes,
    alloc::Strategy alloc) noexcept -> identity::wot::VerificationPrivate*
{
    using ReturnType =
        identity::wot::verification::implementation::Verification;
    using BlankType = identity::wot::VerificationPrivate;

    try {
        auto nym = api.Wallet().Nym(verifier);

        if (false == nym.operator bool()) {

            throw std::runtime_error{"failed to load verifier nym"};
        }

        auto* out = pmr::construct<ReturnType>(
            alloc.result_,
            api,
            std::move(nym),
            std::move(claim),
            value,
            start,
            stop,
            superscedes);

        if (false == out->Finish(reason)) {

            throw std::runtime_error{"failed to sign verification"};
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto Verification(
    const api::Session& api,
    const proto::Verification& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::VerificationPrivate*
{
    using ReturnType =
        identity::wot::verification::implementation::Verification;
    using BlankType = identity::wot::VerificationPrivate;

    try {
        const auto verifier = api.Factory().Internal().NymID(proto.verifier());
        auto nym = api.Wallet().Nym(verifier);

        if (false == nym.operator bool()) {

            throw std::runtime_error{"failed to load verifier nym"};
        }

        auto* out = pmr::construct<ReturnType>(
            alloc.result_, api, proto.item(), std::move(nym));

        if (false == out->Validate()) {

            throw std::runtime_error{"invalid verification"};
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto Verification(
    const api::Session& api,
    const identifier::Nym& verifier,
    const proto::VerificationItem& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::VerificationPrivate*
{
    using ReturnType =
        identity::wot::verification::implementation::Verification;
    using BlankType = identity::wot::VerificationPrivate;

    try {
        auto nym = api.Wallet().Nym(verifier);

        if (false == nym.operator bool()) {

            throw std::runtime_error{"failed to load verifier nym"};
        }

        auto* out = pmr::construct<ReturnType>(
            alloc.result_, api, proto, std::move(nym));

        if (false == out->Validate()) {

            throw std::runtime_error{"invalid verification"};
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}
}  // namespace opentxs::factory
