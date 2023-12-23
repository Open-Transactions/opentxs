// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/ServerContract.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class ZeroMQ;
}  // namespace internal
}  // namespace session

class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq

class ServerConnection;
}  // namespace network

namespace otx
{
namespace context
{
class Server;
class ServerPrivate;
}  // namespace context
}  // namespace otx

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network
{
class ServerConnection
{
public:
    class Imp;

    enum class Push : bool {
        Enable = true,
        Disable = false,
    };

    static auto Factory(
        const api::Session& api,
        const api::session::internal::ZeroMQ& zmq,
        const zeromq::socket::Publish& updates,
        const OTServerContract& contract) -> ServerConnection;

    auto ChangeAddressType(const AddressType type) -> bool;
    auto ClearProxy() -> bool;
    auto EnableProxy() -> bool;
    auto Send(
        const otx::context::Server& context,
        const otx::context::ServerPrivate& data,
        const Message& message,
        const PasswordPrompt& reason,
        const Push push = Push::Enable) -> otx::client::NetworkReplyMessage;
    auto Status() const -> bool;

    auto swap(ServerConnection& rhs) noexcept -> void;

    ServerConnection(Imp* imp) noexcept;
    ServerConnection(const ServerConnection&) noexcept = delete;
    ServerConnection(ServerConnection&&) noexcept;
    auto operator=(const ServerConnection&) noexcept
        -> ServerConnection& = delete;
    auto operator=(ServerConnection&&) noexcept -> ServerConnection&;

    virtual ~ServerConnection();

private:
    Imp* imp_;
};
}  // namespace opentxs::network
