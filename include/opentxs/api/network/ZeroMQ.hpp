// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
namespace internal
{
class ZeroMQ;
}  // namespace internal

class ZAP;
class ZeroMQ;  // IWYU pragma: keep
}  // namespace network
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/**
 api::network::ZMQ API used for accessing ZMQ-specific network functionality.
 */
class OPENTXS_EXPORT opentxs::api::network::ZeroMQ
{
public:
    auto Context() const noexcept -> const opentxs::network::zeromq::Context&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::ZeroMQ&;
    auto ZAP() const noexcept -> const network::ZAP&;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::ZeroMQ&;

    OPENTXS_NO_EXPORT ZeroMQ(internal::ZeroMQ* imp) noexcept;
    ZeroMQ() = delete;
    ZeroMQ(const ZeroMQ&) = delete;
    ZeroMQ(ZeroMQ&&) = delete;
    auto operator=(const ZeroMQ&) -> ZeroMQ& = delete;
    auto operator=(const ZeroMQ&&) -> ZeroMQ& = delete;

    OPENTXS_NO_EXPORT virtual ~ZeroMQ();

protected:
    friend internal::ZeroMQ;

    internal::ZeroMQ* imp_;
};
