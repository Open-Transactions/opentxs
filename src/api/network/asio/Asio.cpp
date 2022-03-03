// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "opentxs/api/network/Asio.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "api/network/asio/Imp.hpp"
#include "network/asio/Socket.hpp"
#include "opentxs/network/asio/Socket.hpp"

namespace opentxs::api::network
{
Asio::Asio(const opentxs::network::zeromq::Context& zmq) noexcept
    : imp_(std::make_unique<Imp>(zmq).release())
{
}

auto Asio::Accept(const Asio::Endpoint& endpoint, AcceptCallback cb)
    const noexcept -> bool
{
    return imp_->Accept(endpoint, std::move(cb));
}

auto Asio::Close(const Endpoint& endpoint) const noexcept -> bool
{
    return imp_->Close(endpoint);
}

auto Asio::GetPublicAddress4() const noexcept -> std::shared_future<OTData>
{
    return imp_->GetPublicAddress4();
}

auto Asio::GetPublicAddress6() const noexcept -> std::shared_future<OTData>
{
    return imp_->GetPublicAddress6();
}

auto Asio::Init() noexcept -> void { imp_->Init(); }

auto Asio::Internal() const noexcept -> internal::Asio&
{
    return const_cast<Imp&>(*imp_);
}

auto Asio::MakeSocket(const opentxs::network::asio::Endpoint& endpoint)
    const noexcept -> opentxs::network::asio::Socket
{
    return {std::make_unique<opentxs::network::asio::Socket::Imp>(
                endpoint, *const_cast<Asio*>(this)->imp_)
                .release()};
}

auto Asio::NotificationEndpoint() const noexcept -> const char*
{
    return imp_->NotificationEndpoint();
}

auto Asio::Resolve(std::string_view server, std::uint16_t port) const noexcept
    -> Resolved
{
    return imp_->Resolve(server, port);
}

auto Asio::Shutdown() noexcept -> void { imp_->Shutdown(); }

Asio::~Asio() { std::unique_ptr<Imp>{imp_}.reset(); }
}  // namespace opentxs::api::network
