// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
class ZMQ;
}  // namespace network

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
}  // namespace context
}  // namespace otx

class PasswordPrompt;

using OTServerConnection = Pimpl<network::ServerConnection>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network
{
class OPENTXS_EXPORT ServerConnection
{
public:
    enum class Push : bool {
        Enable = true,
        Disable = false,
    };

    static auto Factory(
        const api::Session& api,
        const api::network::ZMQ& zmq,
        const zeromq::socket::Publish& updates,
        const OTServerContract& contract) -> OTServerConnection;

    virtual auto ChangeAddressType(const AddressType type) -> bool = 0;
    virtual auto ClearProxy() -> bool = 0;
    virtual auto EnableProxy() -> bool = 0;
    virtual auto Send(
        const otx::context::Server& context,
        const Message& message,
        const PasswordPrompt& reason,
        const Push push = Push::Enable) -> NetworkReplyMessage = 0;
    virtual auto Status() const -> bool = 0;

    virtual ~ServerConnection() = default;

protected:
    ServerConnection() = default;

private:
    friend OTServerConnection;

    /** WARNING: not implemented */
    virtual auto clone() const -> ServerConnection* = 0;

    ServerConnection(const ServerConnection&) = delete;
    ServerConnection(ServerConnection&&) = delete;
    auto operator=(const ServerConnection&) -> ServerConnection& = delete;
    auto operator=(ServerConnection&&) -> ServerConnection& = delete;
};
}  // namespace opentxs::network
