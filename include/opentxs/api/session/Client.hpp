// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
class ZMQ;
}  // namespace network

namespace session
{
namespace internal
{
class Client;
}  // namespace internal

class Activity;
class Contacts;
class OTX;
class UI;
class Workflow;
}  // namespace session
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
/**
 Returns various API handles related to a client session.
 */
class OPENTXS_EXPORT Client : virtual public api::Session
{
public:
    /// Returns the session's Activities.
    virtual auto Activity() const -> const session::Activity& = 0;
    /// Returns the session's Contacts.
    virtual auto Contacts() const -> const api::session::Contacts& = 0;
    OPENTXS_NO_EXPORT virtual auto InternalClient() const noexcept
        -> const internal::Client& = 0;
    /// Returns the OTX API for this session.
    virtual auto OTX() const -> const session::OTX& = 0;
    /// Returns the UI API for this session.
    virtual auto UI() const -> const session::UI& = 0;
    /// Returns the Workflow API for this session.
    virtual auto Workflow() const -> const session::Workflow& = 0;
    /// Returns the ZMQ API for this session. For message passing.
    virtual auto ZMQ() const -> const network::ZMQ& = 0;

    OPENTXS_NO_EXPORT virtual auto InternalClient() noexcept
        -> internal::Client& = 0;

    Client(const Client&) = delete;
    Client(Client&&) = delete;
    auto operator=(const Client&) -> Client& = delete;
    auto operator=(Client&&) -> Client& = delete;

    OPENTXS_NO_EXPORT ~Client() override = default;

protected:
    Client() = default;
};
}  // namespace opentxs::api::session
