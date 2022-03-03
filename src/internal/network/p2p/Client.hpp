// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <string_view>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
class Blockchain;
}  // namespace network

class Session;
}  // namespace api

namespace network
{
namespace p2p
{
class Client;
}  // namespace p2p

namespace zeromq
{
namespace internal
{
class Handle;
class Thread;
}  // namespace internal
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::network::p2p::Client
{
public:
    auto Endpoint() const noexcept -> std::string_view;

    auto Init(const api::network::Blockchain& parent) noexcept -> void;

    Client(const api::Session& api) noexcept;

    ~Client();

private:
    class Imp;

    Imp* imp_;

    Client(
        const api::Session& api,
        opentxs::network::zeromq::internal::Handle&& batch) noexcept;
    Client() = delete;
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    auto operator=(const Client&) -> Client& = delete;
    auto operator=(Client&&) -> Client& = delete;
};
