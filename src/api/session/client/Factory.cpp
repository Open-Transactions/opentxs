// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Factory
// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "api/session/client/Factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <utility>

#include "internal/api/session/Factory.hpp"
#include "internal/core/contract/peer/Factory.hpp"
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto SessionFactoryAPI(const api::session::Client& parent) noexcept
    -> std::unique_ptr<api::session::Factory>
{
    using ReturnType = api::session::client::Factory;

    try {

        return std::make_unique<ReturnType>(parent);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::api::session::client
{
Factory::Factory(const api::session::Client& parent)
    : session::imp::Factory(parent)
    , client_(parent)
{
}

auto Factory::PeerObject(
    const Nym_p& senderNym,
    const UnallocatedCString& message) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, senderNym, message)};
}

auto Factory::PeerObject(
    const Nym_p& senderNym,
    const UnallocatedCString& payment,
    const bool isPayment) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, senderNym, payment, isPayment)};
}
auto Factory::PeerObject(const Nym_p& senderNym, otx::blind::Purse&& purse)
    const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, senderNym, std::move(purse))};
}

auto Factory::PeerObject(
    const OTPeerRequest request,
    const OTPeerReply reply,
    const VersionNumber version) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, request, reply, version)};
}

auto Factory::PeerObject(
    const OTPeerRequest request,
    const VersionNumber version) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, request, version)};
}

auto Factory::PeerObject(
    const Nym_p& signerNym,
    const proto::PeerObject& serialized) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, signerNym, serialized)};
}

auto Factory::PeerObject(
    const Nym_p& recipientNym,
    const opentxs::Armored& encrypted,
    const opentxs::PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, recipientNym, encrypted, reason)};
}
}  // namespace opentxs::api::session::client
