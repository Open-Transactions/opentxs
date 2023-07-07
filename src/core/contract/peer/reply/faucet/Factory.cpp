// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/contract/peer/reply/Factory.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>

#include "core/contract/peer/reply/faucet/FaucetPrivate.hpp"
#include "core/contract/peer/reply/faucet/Implementation.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto FaucetReply(
    const api::Session& api,
    const Nym_p& signer,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const blockchain::block::Transaction& transaction,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*
{
    using ReturnType = contract::peer::reply::faucet::Implementation;
    using BlankType = contract::peer::reply::FaucetPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        if (false == signer.operator bool()) {

            throw std::runtime_error{"invalid signer"};
        }

        out = pmr.allocate(1_uz);

        if (nullptr == out) {

            throw std::runtime_error{"failed to allocate peer reply"};
        }

        pmr.construct(
            out, api, signer, initiator, signer->ID(), request, transaction);

        if (false == out->Finish(reason)) {

            throw std::runtime_error{"failed to sign peer reply"};
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

auto FaucetReply(
    const api::Session& api,
    const Nym_p& signer,
    const proto::PeerReply& proto,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*
{
    using ReturnType = contract::peer::reply::faucet::Implementation;
    using BlankType = contract::peer::reply::FaucetPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        if (false == signer.operator bool()) {

            throw std::runtime_error{"invalid signer"};
        }

        out = pmr.allocate(1_uz);

        if (nullptr == out) {

            throw std::runtime_error{"failed to allocate peer reply"};
        }

        pmr.construct(out, api, signer, proto);

        if (false == out->Validate()) {

            throw std::runtime_error{"invalid peer reply"};
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
