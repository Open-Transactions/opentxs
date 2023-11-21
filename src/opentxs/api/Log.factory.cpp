// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/internal.factory.hpp"  // IWYU pragma: associated

#include "opentxs/api/Log.internal.hpp"
#include "opentxs/api/LogPrivate.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto Log(
    const opentxs::network::zeromq::Context& zmq,
    std::string_view endpoint) noexcept -> api::internal::Log
{
    using ReturnType = api::internal::LogPrivate;

    return std::make_unique<ReturnType>(zmq, UnallocatedCString{endpoint});
}
}  // namespace opentxs::factory
