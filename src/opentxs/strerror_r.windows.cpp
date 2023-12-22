// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/strerror_r.hpp"  // IWYU pragma: associated

namespace opentxs
{
auto error_code_to_string(int) noexcept -> UnallocatedCString { return {}; }

auto error_code_to_string(int, alloc::Strategy alloc) noexcept -> CString
{
    return CString{alloc.result_};
}
}  // namespace opentxs
