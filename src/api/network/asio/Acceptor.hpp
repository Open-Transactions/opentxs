// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/network/Asio.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace asio
{
class io_context;
}  // namespace asio
}  // namespace boost

namespace opentxs
{
namespace network
{
namespace asio
{
class Endpoint;
}  // namespace asio
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::asio
{
class Acceptor
{
public:
    using Callback = internal::Asio::AcceptCallback;

    auto Start() noexcept -> void;
    auto Stop() noexcept -> void;

    Acceptor(
        const opentxs::network::asio::Endpoint& endpoint,
        internal::Asio& asio,
        boost::asio::io_context& ios,
        Callback&& cb) noexcept(false);
    Acceptor() = delete;
    Acceptor(const Acceptor&) = delete;
    Acceptor(Acceptor&&) = delete;
    auto operator=(const Acceptor&) -> Acceptor& = delete;
    auto operator=(Acceptor&&) -> Acceptor& = delete;

    ~Acceptor();

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::api::network::asio
