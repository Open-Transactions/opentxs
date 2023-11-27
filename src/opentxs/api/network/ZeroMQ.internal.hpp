// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
namespace internal
{
class ZeroMQ;  // IWYU pragma: keep
}  // namespace internal

class ZAP;
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

class opentxs::api::network::internal::ZeroMQ
{
public:
    virtual auto Context() const noexcept
        -> const opentxs::network::zeromq::Context& = 0;
    virtual auto ZAP() const noexcept -> const network::ZAP& = 0;

    ZeroMQ() = default;
    ZeroMQ(const ZeroMQ&) = delete;
    ZeroMQ(ZeroMQ&&) = delete;
    auto operator=(const ZeroMQ&) -> ZeroMQ& = delete;
    auto operator=(const ZeroMQ&&) -> ZeroMQ& = delete;

    virtual ~ZeroMQ() = default;
};
