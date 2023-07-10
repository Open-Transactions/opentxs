// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/contract/peer/request/Factory.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>

#include "core/contract/peer/request/bailmentnotice/BailmentNoticePrivate.hpp"
#include "core/contract/peer/request/bailmentnotice/Implementation.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BailmentNoticeRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& responder,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const identifier::Generic& requestID,
    std::string_view txid,
    const opentxs::Amount& amount,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*
{
    using ReturnType = contract::peer::request::bailmentnotice::Implementation;
    using BlankType = contract::peer::request::BailmentNoticePrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        if (false == nym.operator bool()) {

            throw std::runtime_error{"invalid signer"};
        }

        out = pmr.allocate(1_uz);

        if (nullptr == out) {

            throw std::runtime_error{"failed to allocate peer request"};
        }

        pmr.construct(
            out,
            api,
            nym,
            nym->ID(),
            responder,
            serverID,
            unitID,
            requestID,
            txid,
            amount);

        if (false == out->Finish(reason)) {

            throw std::runtime_error{"failed to sign peer request"};
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

auto BailmentNoticeRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& proto,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*
{
    using ReturnType = contract::peer::request::bailmentnotice::Implementation;
    using BlankType = contract::peer::request::BailmentNoticePrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        if (false == nym.operator bool()) {

            throw std::runtime_error{"invalid signer"};
        }

        out = pmr.allocate(1_uz);

        if (nullptr == out) {

            throw std::runtime_error{"failed to allocate peer request"};
        }

        pmr.construct(out, api, nym, proto);

        if (false == out->Validate()) {

            throw std::runtime_error{"invalid peer request"};
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
