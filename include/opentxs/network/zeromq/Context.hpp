// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Context;
}  // namespace internal
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class OPENTXS_EXPORT Context
{
public:
    virtual operator void*() const noexcept = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Context& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Context& = 0;

    Context(const Context&) = delete;
    Context(Context&&) = delete;
    auto operator=(const Context&) -> Context& = delete;
    auto operator=(Context&&) -> Context& = delete;

    virtual ~Context() = default;

protected:
    Context() noexcept = default;
};
}  // namespace opentxs::network::zeromq
