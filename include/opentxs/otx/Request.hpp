// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace otx
{
class Request;
}  // namespace otx

namespace proto
{
class ServerRequest;
}  // namespace proto

using OTXRequest = Pimpl<otx::Request>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx
{
class OPENTXS_EXPORT Request : virtual public opentxs::contract::Signable
{
public:
    static const VersionNumber DefaultVersion;
    static const VersionNumber MaxVersion;

    static auto Factory(
        const api::Session& api,
        const Nym_p signer,
        const identifier::Notary& server,
        const otx::ServerRequestType type,
        const RequestNumber number,
        const PasswordPrompt& reason) -> Pimpl<opentxs::otx::Request>;
    OPENTXS_NO_EXPORT static auto Factory(
        const api::Session& api,
        const proto::ServerRequest serialized) -> Pimpl<opentxs::otx::Request>;
    static auto Factory(const api::Session& api, const ReadView& view)
        -> Pimpl<opentxs::otx::Request>;

    virtual auto Initiator() const -> const identifier::Nym& = 0;
    virtual auto Number() const -> RequestNumber = 0;
    using Signable::Serialize;
    virtual auto Serialize(AllocateOutput destination) const -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(
        proto::ServerRequest& serialized) const -> bool = 0;
    virtual auto Server() const -> const identifier::Notary& = 0;
    virtual auto Type() const -> otx::ServerRequestType = 0;

    virtual auto SetIncludeNym(const bool include, const PasswordPrompt& reason)
        -> bool = 0;

    ~Request() override = default;

protected:
    Request() = default;

private:
    friend OTXRequest;

#ifndef _WIN32
    auto clone() const noexcept -> Request* override = 0;
#endif

    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request&) -> Request& = delete;
    auto operator=(Request&&) -> Request& = delete;
};
}  // namespace opentxs::otx
