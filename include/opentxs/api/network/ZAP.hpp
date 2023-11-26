// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/api/network/Types.hpp"

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

class ZAP;  // IWYU pragma: keep
}  // namespace network
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/**
 api::network::ZAP used for accessing ZAP specific functionality.
 */
class OPENTXS_EXPORT opentxs::api::network::ZAP
{
public:
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::ZAP&;
    /** Register a 27/ZAP handler for a domain
     *
     *  The endpoint specified by handler must be bound by a ROUTER socket with
     *  a ZMQ_IDENTITY property equal to the domain which it handles.
     *
     *  All routing envelope frames must be preserved. See 27/ZAP for details.
     */
    auto RegisterDomain(std::string_view domain, std::string_view handler)
        const noexcept -> bool;
    /** Set default behavior for unregistered domains
     *
     *  Unless this setting is changed then all ZAP requests will be
     *  automatically accepted for any domain which does not have a specific
     *  handler registered.
     *
     *  Changing this value to Reject will block all incoming CURVE connections
     *  unless a custom domain handler is registered to provide a more
     *  permissive policy.
     */
    auto SetDefaultPolicy(ZAPPolicy policy) const noexcept -> bool;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::ZAP&;

    OPENTXS_NO_EXPORT ZAP(internal::ZAP* imp) noexcept;
    ZAP() = delete;
    ZAP(const ZAP&) = delete;
    ZAP(ZAP&&) = delete;
    auto operator=(const ZAP&) -> ZAP& = delete;
    auto operator=(ZAP&&) -> ZAP& = delete;

    OPENTXS_NO_EXPORT virtual ~ZAP();

protected:
    friend internal::ZAP;

    internal::ZAP* imp_;
};
