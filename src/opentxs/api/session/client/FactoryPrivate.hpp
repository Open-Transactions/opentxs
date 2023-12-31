// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/api/session/FactoryPrivate.hpp"
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
namespace internal
{
class Client;
}  // namespace internal
}  // namespace session

class Factory;
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

namespace opentxs::api::session::client
{
class FactoryPrivate final : public session::FactoryPrivate
{
public:
    auto PeerObject(const Nym_p& senderNym, const UnallocatedCString& message)
        const -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const Nym_p& senderNym,
        const UnallocatedCString& payment,
        const bool isPayment) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(const Nym_p& senderNym, otx::blind::Purse&&) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const contract::peer::Request& request,
        const contract::peer::Reply& reply,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const contract::peer::Request& request,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const Nym_p& signerNym,
        const protobuf::PeerObject& serialized) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::PeerObject> final;

    FactoryPrivate(const internal::Client& api, const api::Factory& parent);
    FactoryPrivate() = delete;
    FactoryPrivate(const FactoryPrivate&) = delete;
    FactoryPrivate(FactoryPrivate&&) = delete;
    auto operator=(const FactoryPrivate&) -> FactoryPrivate& = delete;
    auto operator=(FactoryPrivate&&) -> FactoryPrivate& = delete;

    ~FactoryPrivate() final = default;

private:
    const internal::Client& client_;
};
}  // namespace opentxs::api::session::client
