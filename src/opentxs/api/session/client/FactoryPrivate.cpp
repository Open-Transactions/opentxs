// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/client/FactoryPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/core/contract/peer/Factory.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Client.internal.hpp"
#include "opentxs/otx/blind/Purse.hpp"

namespace opentxs::api::session::client
{
FactoryPrivate::FactoryPrivate(
    const internal::Client& api,
    const api::Factory& parent)
    : session::FactoryPrivate(api, parent)
    , client_(api)
{
}

auto FactoryPrivate::PeerObject(
    const Nym_p& senderNym,
    const UnallocatedCString& message) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{opentxs::factory::PeerObject(
        client_.asClientPublic(), senderNym, message)};
}

auto FactoryPrivate::PeerObject(
    const Nym_p& senderNym,
    const UnallocatedCString& payment,
    const bool isPayment) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{opentxs::factory::PeerObject(
        client_.asClientPublic(), senderNym, payment, isPayment)};
}
auto FactoryPrivate::PeerObject(
    const Nym_p& senderNym,
    otx::blind::Purse&& purse) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{opentxs::factory::PeerObject(
        client_.asClientPublic(), senderNym, std::move(purse))};
}

auto FactoryPrivate::PeerObject(
    const contract::peer::Request& request,
    const contract::peer::Reply& reply,
    const VersionNumber version) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{opentxs::factory::PeerObject(
        client_.asClientPublic(), request, reply, version)};
}

auto FactoryPrivate::PeerObject(
    const contract::peer::Request& request,
    const VersionNumber version) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{opentxs::factory::PeerObject(
        client_.asClientPublic(), request, version)};
}

auto FactoryPrivate::PeerObject(
    const Nym_p& signerNym,
    const protobuf::PeerObject& serialized) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{opentxs::factory::PeerObject(
        client_.asClientPublic(), signerNym, serialized)};
}

auto FactoryPrivate::PeerObject(
    const Nym_p& recipientNym,
    const opentxs::Armored& encrypted,
    const opentxs::PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{opentxs::factory::PeerObject(
        client_.asClientPublic(), recipientNym, encrypted, reason)};
}
}  // namespace opentxs::api::session::client
