// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/peer/request/Base.hpp"
#include "internal/core/contract/peer/request/Connection.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
}  // namespace identifier

namespace proto
{
class PeerRequest;
}  // namespace proto

class Factory;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::implementation
{
class Connection final : public internal::Connection,
                         public implementation::Request
{
public:
    auto asConnection() const noexcept -> const internal::Connection& final
    {
        return *this;
    }

    Connection(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const contract::peer::ConnectionInfoType type,
        const identifier::Notary& serverID);
    Connection(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);
    Connection() = delete;
    Connection(const Connection&);
    Connection(Connection&&) = delete;
    auto operator=(const Connection&) -> Connection& = delete;
    auto operator=(Connection&&) -> Connection& = delete;

    ~Connection() final = default;

private:
    friend opentxs::Factory;

    static constexpr auto current_version_ = VersionNumber{4};

    const contract::peer::ConnectionInfoType connection_type_;

    auto IDVersion() const -> SerializedType final;
};
}  // namespace opentxs::contract::peer::request::implementation
