// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/contract/peer/request/Factory.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>

#include "core/contract/peer/request/faucet/FaucetPrivate.hpp"
#include "core/contract/peer/request/faucet/Implementation.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto FaucetRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    opentxs::UnitType unit,
    std::string_view address,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*
{
    using ReturnType = contract::peer::request::faucet::Implementation;
    using BlankType = contract::peer::request::FaucetPrivate;

    try {
        if (false == nym.operator bool()) {

            throw std::runtime_error{"invalid signer"};
        }

        auto* out = pmr::construct<ReturnType>(
            alloc.result_, api, nym, nym->ID(), recipient, unit, address);

        if (false == out->Finish(reason)) {

            throw std::runtime_error{"failed to sign peer request"};
        }

        return out;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto FaucetRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& proto,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*
{
    using ReturnType = contract::peer::request::faucet::Implementation;
    using BlankType = contract::peer::request::FaucetPrivate;

    try {
        if (false == nym.operator bool()) {

            throw std::runtime_error{"invalid signer"};
        }

        auto* out = pmr::construct<ReturnType>(alloc.result_, api, nym, proto);

        if (false == out->Validate()) {

            throw std::runtime_error{"invalid peer request"};
        }

        return out;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}
}  // namespace opentxs::factory
