// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "network/zeromq/curve/Server.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <array>
#include <cstdint>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::network::zeromq::curve::implementation
{
Server::Server(zeromq::socket::implementation::Socket& socket) noexcept
    : parent_(socket)
{
}

auto Server::SetDomain(const UnallocatedCString& domain) const noexcept -> bool
{
    auto set =
        zmq_setsockopt(parent_, ZMQ_ZAP_DOMAIN, domain.data(), domain.size());

    if (0 != set) {
        LogError()(OT_PRETTY_CLASS())("Failed to set domain.").Flush();

        return false;
    }

    return true;
}

auto Server::SetPrivateKey(const Secret& key) const noexcept -> bool
{
    if (CURVE_KEY_BYTES != key.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid private key.").Flush();

        return false;
    }

    return set_private_key(key.data(), key.size());
}

auto Server::SetPrivateKey(const UnallocatedCString& z85) const noexcept -> bool
{
    if (CURVE_KEY_Z85_BYTES > z85.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid private key size (")(z85.size())(
            ").")
            .Flush();

        return false;
    }

    std::array<std::uint8_t, CURVE_KEY_BYTES> key;
    ::zmq_z85_decode(key.data(), z85.data());

    return set_private_key(key.data(), key.size());
}

auto Server::set_private_key(const void* key, const std::size_t keySize)
    const noexcept -> bool
{
    OT_ASSERT(nullptr != parent_);

    socket::implementation::Socket::SocketCallback cb{[&](const Lock&) -> bool {
        const int server{1};
        auto set =
            zmq_setsockopt(parent_, ZMQ_CURVE_SERVER, &server, sizeof(server));

        if (0 != set) {
            LogError()(OT_PRETTY_CLASS())("Failed to set ZMQ_CURVE_SERVER")
                .Flush();

            return false;
        }

        set = zmq_setsockopt(parent_, ZMQ_CURVE_SECRETKEY, key, keySize);

        if (0 != set) {
            LogError()(OT_PRETTY_CLASS())("Failed to set private key.").Flush();

            return false;
        }

        return true;
    }};

    return parent_.apply_socket(std::move(cb));
}
}  // namespace opentxs::network::zeromq::curve::implementation
