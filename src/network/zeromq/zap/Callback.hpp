// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <string>

#include "opentxs/network/zeromq/zap/Callback.hpp"
#include "opentxs/network/zeromq/zap/Reply.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Reply;
class Request;
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::zap::implementation
{
class Callback final : virtual zap::Callback
{
public:
    using Lambda = zap::Callback::ReceiveCallback;

    auto Process(const zap::Request& request) const -> Reply final;
    auto SetDomain(const std::string& domain, const ReceiveCallback& callback)
        const -> bool final;
    auto SetPolicy(const Policy policy) const -> bool final;

    ~Callback() final = default;

private:
    friend zap::Callback;

    const Lambda default_callback_;
    mutable std::pmr::map<std::string, Lambda> domains_;
    mutable std::mutex domain_lock_;
    mutable std::atomic<Policy> policy_;

    auto clone() const -> Callback* final { return new Callback(); }
    auto default_callback(const zap::Request& in) const -> Reply;
    auto get_domain(const ReadView domain) const -> const Lambda&;

    Callback();
    Callback(const Callback&) = delete;
    Callback(Callback&&) = delete;
    auto operator=(const Callback&) -> Callback& = delete;
    auto operator=(Callback&&) -> Callback& = delete;
};
}  // namespace opentxs::network::zeromq::zap::implementation
