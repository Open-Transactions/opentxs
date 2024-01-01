// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace contract
{
namespace peer
{
class Reply;
class Request;
}  // namespace peer
}  // namespace contract

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind
}  // namespace otx

namespace protobuf
{
class PeerObject;
}  // namespace protobuf

class Armored;
class PasswordPrompt;
class PeerObject;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto PeerObject(
    const api::Session& api,
    const Nym_p& senderNym,
    const UnallocatedCString& message) noexcept
    -> std::unique_ptr<opentxs::PeerObject>;
auto PeerObject(
    const api::Session& api,
    const Nym_p& senderNym,
    const UnallocatedCString& payment,
    const bool isPayment) noexcept -> std::unique_ptr<opentxs::PeerObject>;
auto PeerObject(
    const api::Session& api,
    const Nym_p& senderNym,
    otx::blind::Purse&& purse) noexcept -> std::unique_ptr<opentxs::PeerObject>;
auto PeerObject(
    const api::Session& api,
    const contract::peer::Request& request,
    const contract::peer::Reply& reply,
    const VersionNumber version) noexcept
    -> std::unique_ptr<opentxs::PeerObject>;
auto PeerObject(
    const api::Session& api,
    const contract::peer::Request& request,
    const VersionNumber version) noexcept
    -> std::unique_ptr<opentxs::PeerObject>;
auto PeerObject(
    const api::session::Client& api,
    const Nym_p& signerNym,
    const protobuf::PeerObject& serialized) noexcept
    -> std::unique_ptr<opentxs::PeerObject>;
auto PeerObject(
    const api::session::Client& api,
    const Nym_p& recipientNym,
    const opentxs::Armored& encrypted,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<opentxs::PeerObject>;
}  // namespace opentxs::factory
