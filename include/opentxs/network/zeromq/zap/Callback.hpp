// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "opentxs/Export.hpp"
#include "opentxs/network/zeromq/zap/Reply.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Callback;
class Reply;
class Request;
}  // namespace zap
}  // namespace zeromq
}  // namespace network

using OTZMQZAPCallback = Pimpl<network::zeromq::zap::Callback>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::zap
{
class OPENTXS_EXPORT Callback
{
public:
    using ReceiveCallback = std::function<Reply(const Request&)>;

    enum class Policy : bool { Accept = true, Reject = false };

    static auto Factory(
        const UnallocatedCString& domain,
        const ReceiveCallback& callback) -> OTZMQZAPCallback;
    static auto Factory() -> OTZMQZAPCallback;

    virtual auto Process(const Request& request) const -> Reply = 0;
    virtual auto SetDomain(
        const UnallocatedCString& domain,
        const ReceiveCallback& callback) const -> bool = 0;
    virtual auto SetPolicy(const Policy policy) const -> bool = 0;

    Callback(const Callback&) = delete;
    Callback(Callback&&) = default;
    auto operator=(const Callback&) -> Callback& = delete;
    auto operator=(Callback&&) -> Callback& = default;

    virtual ~Callback() = default;

protected:
    Callback() = default;

private:
    friend OTZMQZAPCallback;

    virtual auto clone() const -> Callback* = 0;
};
}  // namespace opentxs::network::zeromq::zap
