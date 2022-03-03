// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <future>
#include <iosfwd>
#include <memory>

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/WorkType.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace asio
{
class Endpoint;
}  // namespace asio
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::asio
{
class OPENTXS_EXPORT Socket
{
public:
    struct Imp;

    using SendStatus = std::promise<bool>;
    using Notification = std::unique_ptr<SendStatus>;

    auto Close() noexcept -> void;
    /**  Open an connection to a remote peer asynchronously
     *
     *   The result of this function will be delivered via the router socket
     *   bound to api::network::Asio::NotificationEndpoint() as a AsioConnect
     *   message in the case of success or an AsioDisconnect in the case of an
     *   error message as described in util/WorkType.hpp.
     *
     *   The caller must have completed the handshake with the asio notification
     *   endpoint prior to calling this function in order to receive subsequent
     *   status messages.
     *
     *   @param notify the connection id which will be notified of the
     *                 resolution of the connection attempt
     *
     *   \returns false if the asio context is shutting down or if the notify
     *            parameter is empty
     */
    auto Connect(const ReadView notify) noexcept -> bool;
    /**  Receive a specified number of bytes from a remote peer asynchronously
     *
     *   The result of this function will be delivered via the router socket
     *   bound to api::network::Asio::NotificationEndpoint() as a AsioDisconnect
     *   message as described in util/WorkType.hpp in the case of an error or
     *   the caller-specified type in the case of success.
     *
     *   The caller must have completed the handshake with the asio notification
     *   endpoint prior to calling this function in order to receive subsequent
     *   status messages.
     *
     *   @param notify the connection id which will be notified of the
     *                 resolution of the receive attempt
     *   @param type   the message type to be used for returning the received
     *                 data
     *   @param bytes  the number of bytes to wait for
     *
     *   \returns false if the asio context is shutting down or if the notify
     *            parameter is empty
     */
    auto Receive(
        const ReadView notify,
        const OTZMQWorkType type,
        const std::size_t bytes) noexcept -> bool;
    /**  Asynchronously deliver bytes to a remote peer
     *
     *   @param data      the bytes to be sent
     *   @param notifier  the callback to be executed with the result of the
     *                    transmission attempt
     *
     *   \returns false if the asio context is shutting down
     */
    auto Transmit(const ReadView data, Notification notifier) noexcept -> bool;

    OPENTXS_NO_EXPORT Socket(Imp* imp) noexcept;
    Socket(Socket&&) noexcept;

    ~Socket();

private:
    Imp* imp_;

    Socket() noexcept = delete;
    Socket(const Socket&) = delete;
    auto operator=(const Socket&) -> Socket& = delete;
    auto operator=(Socket&&) -> Socket& = delete;
};
}  // namespace opentxs::network::asio
