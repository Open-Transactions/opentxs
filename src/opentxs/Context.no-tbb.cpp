// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Context.hpp"  // IWYU pragma: associated

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"  // NOLINT
#include <boost/thread/thread.hpp>

#pragma GCC diagnostic pop

#include "internal/util/Thread.hpp"

namespace opentxs
{
auto RunJob(SimpleCallback cb) noexcept -> void
{
    static const auto attributes = [] {
        auto out = boost::thread::attributes{};
        out.set_stack_size(thread_pool_stack_size_);

        return out;
    }();
    boost::thread{attributes, cb}.detach();
}
}  // namespace opentxs
