// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/api/Session.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Session;
}  // namespace internal

namespace network
{
class ZMQ;
}  // namespace network

namespace session
{
class Activity;
class Client;  // IWYU pragma: keep
class Contacts;
class OTX;
class UI;
class Workflow;
}  // namespace session
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/**
 Returns various API handles related to a client session.
 */
class OPENTXS_EXPORT opentxs::api::session::Client final : public api::Session
{
public:
    /// Returns the session's Activities.
    auto Activity() const -> const session::Activity&;
    /// Returns the session's Contacts.
    auto Contacts() const -> const api::session::Contacts&;
    /// Returns the OTX API for this session.
    auto OTX() const -> const session::OTX&;
    /// Returns the UI API for this session.
    auto UI() const -> const session::UI&;
    /// Returns the Workflow API for this session.
    auto Workflow() const -> const session::Workflow&;
    /// Returns the ZMQ API for this session. For message passing.
    auto ZMQ() const -> const network::ZMQ&;

    OPENTXS_NO_EXPORT Client(api::internal::Session* imp) noexcept;
    Client() = delete;
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    auto operator=(const Client&) -> Client& = delete;
    auto operator=(Client&&) -> Client& = delete;

    OPENTXS_NO_EXPORT ~Client() final;
};
