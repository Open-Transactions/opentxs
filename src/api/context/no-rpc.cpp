// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/context/Context.hpp"  // IWYU pragma: associated

namespace opentxs::api::imp
{
auto Context::RPC(const ReadView command, Writer&& response) const noexcept
    -> bool
{
    return false;
}
}  // namespace opentxs::api::imp
