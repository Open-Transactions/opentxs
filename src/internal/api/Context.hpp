// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/api/Context.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Legacy;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

class Options;
class PasswordCaller;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::internal
{
class Context : virtual public api::Context
{
public:
    static auto MaxJobs() noexcept -> unsigned int;
    static auto SetMaxJobs(const opentxs::Options& args) noexcept -> void;

    virtual auto GetPasswordCaller() const noexcept -> PasswordCaller& = 0;
    auto Internal() const noexcept -> const Context& final { return *this; }
    virtual auto Legacy() const noexcept -> const api::Legacy& = 0;
    virtual auto ShuttingDown() const noexcept -> bool = 0;

    virtual auto Init() noexcept -> void = 0;
    auto Internal() noexcept -> Context& final { return *this; }
    virtual auto Shutdown() noexcept -> void = 0;

    ~Context() override = default;
};
}  // namespace opentxs::api::internal

namespace opentxs
{
auto get_zeromq() noexcept -> std::weak_ptr<const network::zeromq::Context>;
}  // namespace opentxs
