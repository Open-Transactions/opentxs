// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/network/Asio.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace asio
{
class io_context;
}  // namespace asio
}  // namespace boost

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

namespace opentxs::api::network::asio
{
class Acceptors
{
public:
    using Callback = Asio::AcceptCallback;

    auto Close(const opentxs::network::asio::Endpoint& endpoint) noexcept
        -> bool;
    auto Start(
        const opentxs::network::asio::Endpoint& endpoint,
        Callback cb) noexcept -> bool;
    auto Stop() noexcept -> void;

    Acceptors(
        internal::Asio& parent,
        boost::asio::io_context& context) noexcept;

    ~Acceptors();

private:
    struct Imp;

    Imp* imp_;

    Acceptors() = delete;
    Acceptors(const Acceptors&) = delete;
    Acceptors(Acceptors&&) = delete;
    Acceptors& operator=(const Acceptors&) = delete;
    Acceptors& operator=(Acceptors&&) = delete;
};
}  // namespace opentxs::api::network::asio
