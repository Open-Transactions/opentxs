// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "internal/network/zeromq/Handle.hpp"  // IWYU pragma: associated

#include <cassert>
#include <utility>

#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"

namespace opentxs::network::zeromq::internal
{
Handle::Handle(
    std::shared_ptr<const zeromq::Context> context,
    std::shared_ptr<internal::Batch> batch) noexcept
    : batch_p_(std::move(batch))
    , batch_(*batch_p_)
    , context_(std::move(context))
{
    assert(batch_p_);
    assert(context_);
}

Handle::Handle(Handle&& rhs) noexcept
    : batch_p_(nullptr)
    , batch_(rhs.batch_)
    , context_(nullptr)
{
    using std::swap;
    swap(batch_p_, rhs.batch_p_);
    swap(context_, rhs.context_);
}

auto Handle::Release() noexcept -> void { batch_.ClearCallbacks(); }

Handle::~Handle()
{
    if (context_) {
        Release();
        context_->Internal().Stop(batch_.id_);
    }
}
}  // namespace opentxs::network::zeromq::internal
