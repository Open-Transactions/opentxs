// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/api/network/Network.hpp"

#pragma once

#include <boost/asio.hpp>
#include <cstddef>
#include <iosfwd>

#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/WorkType.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
namespace internal
{
class Asio;
}  // namespace internal
}  // namespace network
}  // namespace api

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

namespace asio = boost::asio;
namespace ip = asio::ip;

namespace opentxs::network::asio
{
struct Socket::Imp {
    using Asio = api::network::internal::Asio;
    using tcp = ip::tcp;

    const Endpoint& endpoint_;
    api::network::internal::Asio& asio_;
    tcp::socket socket_;

    auto Close() noexcept -> void;
    auto Connect(const ReadView id) noexcept -> bool;
    auto Receive(
        const ReadView notify,
        const OTZMQWorkType type,
        const std::size_t bytes) noexcept -> bool;
    auto Transmit(const ReadView data, Notification notifier) noexcept -> bool;

    Imp(const Endpoint& endpoint, Asio& asio) noexcept;
    Imp(Asio& asio, Endpoint&& endpoint, tcp::socket&& socket) noexcept;

    ~Imp();

private:
    Imp() noexcept = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    Imp& operator=(const Imp&) = delete;
    Imp& operator=(Imp&&) = delete;
};
}  // namespace opentxs::network::asio
