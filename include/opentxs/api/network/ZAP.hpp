// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/network/zeromq/zap/Types.hpp"

namespace opentxs::api::network
{
/**
 api::network::ZAP used for accessing ZAP specific functionality.
 */
class OPENTXS_EXPORT ZAP
{
public:
    /** Set a callback that will be triggered for any ZAP requests in a
     *  specified domain
     *
     *   \param[in] domain  The ZAP domain to be registered. Any non-empty
     *                      string is valid.
     *   \param[in] callback    The callback to be executed for the specified
     *                          domain.
     *
     *   \return True if the domain is valid and not already registered
     */
    virtual auto RegisterDomain(
        const std::string_view domain,
        const opentxs::network::zeromq::zap::ReceiveCallback& callback) const
        -> bool = 0;

    /** Configure ZAP policy for unhandled domains
     *
     *  Default behavior is Accept.
     *
     *  \param[in]  policy  Accept or reject ZAP requests for a domain which has
     *                      no registered callback
     */
    virtual auto SetDefaultPolicy(
        const opentxs::network::zeromq::zap::Policy policy) const -> bool = 0;

    ZAP(const ZAP&) = delete;
    ZAP(ZAP&&) = delete;
    auto operator=(const ZAP&) -> ZAP& = delete;
    auto operator=(ZAP&&) -> ZAP& = delete;

    OPENTXS_NO_EXPORT virtual ~ZAP() = default;

protected:
    ZAP() = default;
};
}  // namespace opentxs::api::network
