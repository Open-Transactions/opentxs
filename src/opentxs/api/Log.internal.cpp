// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/Log.internal.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/api/LogPrivate.hpp"

namespace opentxs::api::internal
{
Log::Log(std::unique_ptr<LogPrivate> imp) noexcept
    : imp_(std::move(imp))
{
}

auto Log::Reset() noexcept -> void { imp_.reset(); }

Log::~Log() = default;
}  // namespace opentxs::api::internal
