// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "internal/network/zeromq/Handle.hpp"  // IWYU pragma: associated

#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"

namespace opentxs::network::zeromq::internal
{
Handle::Handle(
    const internal::Context& context,
    internal::Batch& batch) noexcept
    : batch_(batch)
    , context_(&context)
{
}

Handle::Handle(Handle&& rhs) noexcept
    : batch_(rhs.batch_)
    , context_(rhs.context_)
{
    rhs.context_ = nullptr;
}

auto Handle::Release() noexcept -> void
{
    if (nullptr != context_) {
        batch_.ClearCallbacks();
        context_->Stop(batch_.id_);
        context_ = nullptr;
    }
}

Handle::~Handle() { Release(); }
}  // namespace opentxs::network::zeromq::internal
