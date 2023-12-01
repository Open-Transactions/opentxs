// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/api/Network.internal.hpp"

#pragma once

#include <cs_plain_guarded.h>
#include <cstddef>
#include <memory>
#include <string_view>
#include <tuple>
#include <utility>

#include "BoostAsio.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

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

    struct Buffer {
        using Index = std::size_t;
        using Read = decltype(boost::asio::buffer(
            std::declval<const void*>(),
            std::declval<std::size_t>()));
        using Write = decltype(boost::asio::buffer(
            std::declval<void*>(),
            std::declval<std::size_t>()));
        using SendParams = std::tuple<Index, Read, zeromq::Envelope>;
        using ReceiveParams = std::tuple<
            Index,
            Write,
            UnallocatedCString,
            OTZMQWorkType,
            zeromq::Envelope>;

        auto Finish(Index index) noexcept -> void;
        auto Receive(
            const zeromq::Envelope& notify,
            const OTZMQWorkType type,
            const std::string_view endpoint,
            const std::size_t bytes) noexcept -> ReceiveParams*;
        auto Transmit(
            const zeromq::Envelope& notify,
            const ReadView data) noexcept -> SendParams*;

    private:
        std::size_t index_{};
        UnallocatedMap<std::size_t, ByteArray> buffer_{};
        UnallocatedMap<std::size_t, SendParams> transmit_{};
        UnallocatedMap<std::size_t, ReceiveParams> receive_{};
    };

    using GuardedBuffer = libguarded::plain_guarded<Buffer>;

    const Endpoint endpoint_;
    api::network::internal::Asio& asio_;
    GuardedBuffer buffer_;
    tcp::socket socket_;

    static auto Destroy(void* imp) noexcept -> void;
    static auto Get(void* imp) noexcept -> Imp&;

    auto Close() noexcept -> void;
    auto Connect(const zeromq::Envelope& id) noexcept -> bool;
    auto Receive(
        const zeromq::Envelope& notify,
        const OTZMQWorkType type,
        const std::size_t bytes) noexcept -> bool;
    auto Transmit(const zeromq::Envelope& notify, const ReadView data) noexcept
        -> bool;

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
