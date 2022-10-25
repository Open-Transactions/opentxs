// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/api/network/Network.hpp"

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>

#include "BoostAsio.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace asio = boost::asio;
namespace ip = asio::ip;

namespace opentxs::network::asio
{
class Socket::Imp final : public std::enable_shared_from_this<Imp>
{
public:
    using Asio = api::network::internal::Asio;
    using tcp = ip::tcp;

    const Endpoint& endpoint_;
    api::network::internal::Asio& asio_;
    tcp::socket socket_;

    static auto Destroy(void* imp) noexcept -> void;
    static auto Get(void* imp) noexcept -> Imp&;

    auto Close() noexcept -> void;
    auto Connect(const ReadView id) noexcept -> bool;
    auto Receive(
        const ReadView notify,
        const OTZMQWorkType type,
        const std::size_t bytes) noexcept -> bool;
    auto Transmit(const ReadView notify, const ReadView data) noexcept -> bool;

    Imp(const Endpoint& endpoint, Asio& asio) noexcept;
    Imp(Asio& asio, Endpoint&& endpoint, tcp::socket&& socket) noexcept;
    Imp() noexcept = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp();

private:
    using Pointer = std::shared_ptr<Imp>;
};
}  // namespace opentxs::network::asio
