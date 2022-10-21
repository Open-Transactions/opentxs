// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
auto random_bytes_non_crypto(Writer&& dest, std::size_t bytes) noexcept -> bool;
}  // namespace opentxs
