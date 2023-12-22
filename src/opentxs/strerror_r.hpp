// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
auto error_code_to_string(int ec) noexcept -> UnallocatedCString;
auto error_code_to_string(int ec, alloc::Strategy alloc) noexcept -> CString;
}  // namespace opentxs
