// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/internal.factory.hpp"  // IWYU pragma: associated

#include "opentxs/rpc/Processor.internal.hpp"

namespace opentxs::factory
{
auto RPC(const api::Context&) noexcept -> std::unique_ptr<rpc::Processor>
{
    using ReturnType = rpc::Processor;

    return std::make_unique<ReturnType>();
}
}  // namespace opentxs::factory
