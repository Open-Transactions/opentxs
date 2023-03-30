// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
auto Socket::Imp::Buffer::Finish(Index index) noexcept -> void
{
    buffer_.erase(index);
    transmit_.erase(index);
    receive_.erase(index);
}

auto Socket::Imp::Buffer::Receive(
    const zeromq::Envelope& notify,
    const OTZMQWorkType type,
    const std::string_view endpoint,
    const std::size_t bytes) noexcept -> ReceiveParams*
{
    const auto counter = index_++;
    auto& buf = [&]() -> auto&
    {
        auto& out = buffer_[counter];
        out.resize(bytes);

        return out;
    }
    ();
    auto& map = receive_;
    auto [it, added] = map.try_emplace(
        counter,
        counter,
        boost::asio::buffer(buf.data(), buf.size()),
        endpoint,
        type,
        notify);

    OT_ASSERT(added);

    auto& params = it->second;

    return std::addressof(params);
}

auto Socket::Imp::Buffer::Transmit(
    const zeromq::Envelope& notify,
    const ReadView data) noexcept -> SendParams*
{
    const auto counter = index_++;
    const auto& buf = [&]() -> const auto&
    {
        auto& out = buffer_[counter];
        out.Assign(data);

        return out;
    }
    ();
    auto& map = transmit_;
    auto [it, added] = map.try_emplace(
        counter, counter, boost::asio::buffer(buf.data(), buf.size()), notify);

    OT_ASSERT(added);

    auto& params = it->second;

    return std::addressof(params);
}
}  // namespace opentxs::network::asio

namespace opentxs::network::asio
{
Socket::Imp::Imp(const Endpoint& endpoint, Asio& asio) noexcept
    : endpoint_(endpoint)
    , asio_(asio)
    , buffer_()
    , socket_(asio_.IOContext())
{
}

Socket::Imp::Imp(Asio& asio, Endpoint&& endpoint, tcp::socket&& socket) noexcept
    : endpoint_(std::move(endpoint))
    , asio_(asio)
    , buffer_()
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

auto Socket::Imp::Connect(const zeromq::Envelope& id) noexcept -> bool
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
    const zeromq::Envelope& id,
    const OTZMQWorkType type,
    const std::size_t bytes) noexcept -> bool
{
    return asio_.Receive(id, type, bytes, shared_from_this());
}

auto Socket::Imp::Transmit(
    const zeromq::Envelope& notify,
    const ReadView data) noexcept -> bool
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

auto Socket::Close() noexcept -> void
{
    if (nullptr != imp_) {
        Imp::Destroy(imp_);
        imp_ = nullptr;
    }
}

auto Socket::Connect(const zeromq::Envelope& id) noexcept -> bool
{
    return Imp::Get(imp_).Connect(id);
}

auto Socket::Receive(
    const zeromq::Envelope& id,
    const OTZMQWorkType type,
    const std::size_t bytes) noexcept -> bool
{
    return Imp::Get(imp_).Receive(id, type, bytes);
}

auto Socket::Transmit(
    const zeromq::Envelope& notify,
    const ReadView data) noexcept -> bool
{
    return Imp::Get(imp_).Transmit(notify, data);
}

Socket::~Socket() { Close(); }
}  // namespace opentxs::network::asio
