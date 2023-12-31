// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/contract/peer/reply/Factory.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>

#include "core/contract/peer/reply/storesecret/Implementation.hpp"
#include "core/contract/peer/reply/storesecret/StoreSecretPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto StoreSecretReply(
    const api::Session& api,
    const Nym_p& signer,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    bool value,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*
{
    using ReturnType = contract::peer::reply::storesecret::Implementation;
    using BlankType = contract::peer::reply::StoreSecretPrivate;

    try {
        if (false == signer.operator bool()) {

            throw std::runtime_error{"invalid signer"};
        }

        auto* out = pmr::construct<ReturnType>(
            alloc.result_,
            api,
            signer,
            initiator,
            signer->ID(),
            request,
            value);

        if (false == out->Finish(reason)) {

            throw std::runtime_error{"failed to sign peer reply"};
        }

        return out;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto StoreSecretReply(
    const api::Session& api,
    const Nym_p& signer,
    const protobuf::PeerReply& proto,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*
{
    using ReturnType = contract::peer::reply::storesecret::Implementation;
    using BlankType = contract::peer::reply::StoreSecretPrivate;

    try {
        if (false == signer.operator bool()) {

            throw std::runtime_error{"invalid signer"};
        }

        auto* out =
            pmr::construct<ReturnType>(alloc.result_, api, signer, proto);

        if (false == out->Validate()) {

            throw std::runtime_error{"invalid peer reply"};
        }

        return out;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}
}  // namespace opentxs::factory
