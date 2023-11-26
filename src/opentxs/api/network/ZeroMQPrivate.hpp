// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/network/ZAP.hpp"
#include "opentxs/api/network/ZeroMQ.internal.hpp"
#include "opentxs/network/zeromq/Context.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
class ZeroMQPrivate;  // IWYU pragma: keep
}  // namespace network
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::network::ZeroMQPrivate final
    : virtual public internal::ZeroMQ
{
public:
    auto Context() const noexcept
        -> const opentxs::network::zeromq::Context& final
    {
        return context_;
    }
    auto ZAP() const noexcept -> const network::ZAP& final { return zap_; }

    ZeroMQPrivate(
        const opentxs::network::zeromq::Context& context,
        const network::ZAP& zap) noexcept;
    ZeroMQPrivate() = delete;
    ZeroMQPrivate(const ZeroMQPrivate&) = delete;
    ZeroMQPrivate(ZeroMQPrivate&&) = delete;
    auto operator=(const ZeroMQPrivate&) -> ZeroMQPrivate& = delete;
    auto operator=(const ZeroMQPrivate&&) -> ZeroMQPrivate& = delete;

    ~ZeroMQPrivate() final;

private:
    const opentxs::network::zeromq::Context& context_;
    const network::ZAP& zap_;
};
