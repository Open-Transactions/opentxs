// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/core/contract/peer/Object.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/blind/Purse.hpp"
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

namespace protobuf
{
class PeerObject;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::peer::implementation
{
class Object final : virtual public opentxs::PeerObject
{
public:
    auto Message() const noexcept
        -> const std::unique_ptr<UnallocatedCString>& final
    {
        return message_;
    }
    auto Nym() const noexcept -> const Nym_p& final { return nym_; }
    auto Payment() const noexcept
        -> const std::unique_ptr<UnallocatedCString>& final
    {
        return payment_;
    }
    auto Purse() const noexcept -> const otx::blind::Purse& final
    {
        return purse_;
    }
    auto Request() const noexcept -> const contract::peer::Request& final
    {
        return request_;
    }
    auto Reply() const noexcept -> const contract::peer::Reply& final
    {
        return reply_;
    }
    auto Serialize(protobuf::PeerObject& output) const noexcept -> bool final;
    auto Type() const noexcept -> contract::peer::ObjectType final
    {
        return type_;
    }
    auto Validate() const noexcept -> bool final;

    auto Message() noexcept -> std::unique_ptr<UnallocatedCString>& final
    {
        return message_;
    }
    auto Payment() noexcept -> std::unique_ptr<UnallocatedCString>& final
    {
        return payment_;
    }
    auto Purse() noexcept -> otx::blind::Purse& final { return purse_; }

    Object(
        const api::session::Client& api,
        const Nym_p& signerNym,
        const protobuf::PeerObject serialized) noexcept(false);
    Object(
        const api::Session& api,
        const Nym_p& senderNym,
        const UnallocatedCString& message) noexcept;
    Object(
        const api::Session& api,
        const Nym_p& senderNym,
        otx::blind::Purse&& purse) noexcept;
    Object(
        const api::Session& api,
        const UnallocatedCString& payment,
        const Nym_p& senderNym) noexcept;
    Object(
        const api::Session& api,
        contract::peer::Request request,
        contract::peer::Reply reply,
        const VersionNumber version) noexcept;
    Object(
        const api::Session& api,
        contract::peer::Request request,
        const VersionNumber version) noexcept;
    Object(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedCString& message,
        const UnallocatedCString& payment,
        contract::peer::Reply reply,
        contract::peer::Request request,
        otx::blind::Purse&& purse,
        const contract::peer::ObjectType type,
        const VersionNumber version) noexcept;
    Object() = delete;

    ~Object() final;

private:
    const api::Session& api_;
    Nym_p nym_;
    std::unique_ptr<UnallocatedCString> message_;
    std::unique_ptr<UnallocatedCString> payment_;
    contract::peer::Reply reply_;
    contract::peer::Request request_;
    otx::blind::Purse purse_;
    contract::peer::ObjectType type_;
    VersionNumber version_;
};
}  // namespace opentxs::peer::implementation
