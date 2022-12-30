// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/network/ZAP.hpp"

#include "opentxs/network/zeromq/zap/Types.hpp"

namespace opentxs::api::network::internal
{
class ZAP : virtual public api::network::ZAP
{
public:
    auto Internal() const noexcept -> const internal::ZAP& final
    {
        return *this;
    }
    virtual auto RegisterDomain(
        const std::string_view domain,
        const opentxs::network::zeromq::zap::ReceiveCallback& callback) const
        -> bool = 0;
    virtual auto SetDefaultPolicy(
        const opentxs::network::zeromq::zap::Policy policy) const -> bool = 0;

    auto Internal() noexcept -> internal::ZAP& final { return *this; }

    ~ZAP() override = default;
};
}  // namespace opentxs::api::network::internal
