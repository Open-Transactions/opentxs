// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/network/ZeroMQ.hpp"  // IWYU pragma: associated

#include "opentxs/api/network/ZeroMQ.internal.hpp"

namespace opentxs::api::network
{
ZeroMQ::ZeroMQ(internal::ZeroMQ* imp) noexcept
    : imp_(imp)
{
}

auto ZeroMQ::Context() const noexcept
    -> const opentxs::network::zeromq::Context&
{
    return imp_->Context();
}

auto ZeroMQ::Internal() const noexcept -> const internal::ZeroMQ&
{
    return *imp_;
}

auto ZeroMQ::Internal() noexcept -> internal::ZeroMQ& { return *imp_; }

ZeroMQ::~ZeroMQ()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api::network
