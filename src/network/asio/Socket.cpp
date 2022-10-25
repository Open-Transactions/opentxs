// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "opentxs/network/asio/Socket.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "BoostAsio.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/asio/Socket.hpp"
#include "opentxs/network/asio/Endpoint.hpp"

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
    return asio_.Connect(id, shared_from_this());
}

auto Socket::Imp::Destroy(void* imp) noexcept -> void
{
    OT_ASSERT(nullptr != imp);

    auto p = std::unique_ptr<Pointer>{static_cast<Pointer*>(imp)};

    OT_ASSERT(p);

    p->reset();
    p.reset();
}

auto Socket::Imp::Get(void* imp) noexcept -> Imp&
{
    return **static_cast<Pointer*>(imp);
}

auto Socket::Imp::Receive(
    const ReadView id,
    const OTZMQWorkType type,
    const std::size_t bytes) noexcept -> bool
{
    return asio_.Receive(id, type, bytes, shared_from_this());
}

auto Socket::Imp::Transmit(const ReadView notify, const ReadView data) noexcept
    -> bool
{
    return asio_.Transmit(notify, data, shared_from_this());
}

Socket::Imp::~Imp() { Close(); }

Socket::Socket(std::function<void*()>&& builder) noexcept
    : imp_(std::invoke(builder))
{
    OT_ASSERT(nullptr != imp_);
}

Socket::Socket(Socket&& rhs) noexcept
    : imp_()
{
    std::swap(imp_, rhs.imp_);
}

auto Socket::Close() noexcept -> void { Imp::Get(imp_).Close(); }

auto Socket::Connect(const ReadView id) noexcept -> bool
{
    return Imp::Get(imp_).Connect(id);
}

auto Socket::Receive(
    const ReadView id,
    const OTZMQWorkType type,
    const std::size_t bytes) noexcept -> bool
{
    return Imp::Get(imp_).Receive(id, type, bytes);
}

auto Socket::Transmit(const ReadView notify, const ReadView data) noexcept
    -> bool
{
    return Imp::Get(imp_).Transmit(notify, data);
}

Socket::~Socket()
{
    if (nullptr != imp_) { Imp::Destroy(imp_); }
}
}  // namespace opentxs::network::asio
