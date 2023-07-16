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
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
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
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        auto nym = api.Wallet().Nym(verifier);

        if (false == nym.operator bool()) {

            throw std::runtime_error{"failed to load verifier nym"};
        }

        out = pmr.allocate(1_uz);

        if (nullptr == out) {

            throw std::runtime_error{"failed to allocate verification"};
        }

        pmr.construct(
            out,
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

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc.result_};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
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
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        const auto verifier = api.Factory().Internal().NymID(proto.verifier());
        auto nym = api.Wallet().Nym(verifier);

        if (false == nym.operator bool()) {

            throw std::runtime_error{"failed to load verifier nym"};
        }

        out = pmr.allocate(1_uz);

        if (nullptr == out) {

            throw std::runtime_error{"failed to allocate verification"};
        }

        pmr.construct(out, api, proto.item(), std::move(nym));

        if (false == out->Validate()) {

            throw std::runtime_error{"invalid verification"};
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc.result_};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
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
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        auto nym = api.Wallet().Nym(verifier);

        if (false == nym.operator bool()) {

            throw std::runtime_error{"failed to load verifier nym"};
        }

        out = pmr.allocate(1_uz);

        if (nullptr == out) {

            throw std::runtime_error{"failed to allocate verification"};
        }

        pmr.construct(out, api, proto, std::move(nym));

        if (false == out->Validate()) {

            throw std::runtime_error{"invalid verification"};
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc.result_};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}
}  // namespace opentxs::factory