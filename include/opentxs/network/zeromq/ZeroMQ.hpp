// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
OPENTXS_EXPORT auto MakeArbitraryInproc() noexcept -> UnallocatedCString;
OPENTXS_EXPORT auto MakeArbitraryInproc(alloc::Default alloc) noexcept
    -> CString;
auto MakeDeterministicInproc(
    const std::string_view path,
    const int instance,
    const int version) noexcept -> UnallocatedCString;
auto MakeDeterministicInproc(
    const std::string_view path,
    const int instance,
    const int version,
    const std::string_view suffix) noexcept -> UnallocatedCString;
auto RawToZ85(const ReadView input, Writer&& output) noexcept -> bool;
auto Z85ToRaw(const ReadView input, Writer&& output) noexcept -> bool;
}  // namespace opentxs::network::zeromq
