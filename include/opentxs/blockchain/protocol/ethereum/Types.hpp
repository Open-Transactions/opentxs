// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace alloc
{
struct Strategy;
}  // namespace alloc

class Amount;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::ethereum
{
using AccountNonce = std::uint64_t;

OPENTXS_EXPORT auto amount_to_native(const Amount&) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto amount_to_native(const Amount&, alloc::Strategy) noexcept
    -> CString;
OPENTXS_EXPORT auto amount_to_native(const Amount&, Writer&&) noexcept -> bool;
OPENTXS_EXPORT auto native_to_amount(std::string_view hex) noexcept
    -> std::optional<Amount>;
}  // namespace opentxs::blockchain::protocol::ethereum
