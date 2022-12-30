// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/network/zeromq/zap/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
namespace internal
{
class ZAP;
}  // namespace internal
}  // namespace network
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network
{
/**
 api::network::ZAP used for accessing ZAP specific functionality.
 */
class OPENTXS_EXPORT ZAP
{
public:
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::ZAP& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::ZAP& = 0;

    ZAP(const ZAP&) = delete;
    ZAP(ZAP&&) = delete;
    auto operator=(const ZAP&) -> ZAP& = delete;
    auto operator=(ZAP&&) -> ZAP& = delete;

    OPENTXS_NO_EXPORT virtual ~ZAP() = default;

protected:
    ZAP() = default;
};
}  // namespace opentxs::api::network
