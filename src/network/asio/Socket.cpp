// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "opentxs/network/asio/Socket.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <memory>
#include <type_traits>
#include <utility>

#include "internal/api/network/Asio.hpp"
#include "network/asio/Socket.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::network::asio
{
Socket::Imp::Imp(const Endpoint& endpoint, Asio& asio) noexcept
    : endpoint_(endpoint)
    , asio_(asio)
    , socket_(asio_.IOContext())
{
}

Socket::Imp::Imp(Asio& asio, Endpoint&& endpoint, tcp::socket&& socket) noexcept
    : endpoint_(std::move(endpoint))
    , asio_(asio)
    , socket_(std::move(socket))
{
}

auto Socket::Imp::Close() noexcept -> void
{
    try {
        socket_.shutdown(tcp::socket::shutdown_both);
    } catch (...) {
    }

    try {
        socket_.close();
    } catch (...) {
    }
}

auto Socket::Imp::Connect(const ReadView id) noexcept -> bool
{
    return asio_.Connect(id, *this);
}

auto Socket::Imp::Receive(
    const ReadView id,
    const OTZMQWorkType type,
    const std::size_t bytes) noexcept -> bool
{
    return asio_.Receive(id, type, bytes, *this);
}

auto Socket::Imp::Transmit(const ReadView data, Notification notifier) noexcept
    -> bool
{
    using SharedStatus = std::shared_ptr<SendStatus>;
    auto* buf = [&] {
        auto out = std::make_unique<Space>();
        *out = space(data);

        return out.release();
    }();
    auto work =
        [this, buf, promise = SharedStatus{std::move(notifier)}]() -> void {
        auto cb = [buf, promise](auto& error, auto bytes) -> void {
            auto cleanup = std::unique_ptr<Space>{buf};

            try {
                if (promise) { promise->set_value(!error); }
            } catch (...) {
            }
        };
        boost::asio::async_write(
            socket_, boost::asio::buffer(buf->data(), buf->size()), cb);
    };

    return asio_.Post(ThreadPool::Network, std::move(work));
}

Socket::Imp::~Imp() { Close(); }

Socket::Socket(Imp* imp) noexcept
    : imp_(imp)
{
}

Socket::Socket(Socket&& rhs) noexcept
    : imp_()
{
    std::swap(imp_, rhs.imp_);
}

auto Socket::Close() noexcept -> void { imp_->Close(); }

auto Socket::Connect(const ReadView id) noexcept -> bool
{
    return imp_->Connect(id);
}

auto Socket::Receive(
    const ReadView id,
    const OTZMQWorkType type,
    const std::size_t bytes) noexcept -> bool
{
    return imp_->Receive(id, type, bytes);
}

auto Socket::Transmit(const ReadView data, Notification notifier) noexcept
    -> bool
{
    return imp_->Transmit(data, std::move(notifier));
}

Socket::~Socket() { std::unique_ptr<Imp>{imp_}.reset(); }
}  // namespace opentxs::network::asio
