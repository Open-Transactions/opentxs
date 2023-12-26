// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <cstddef>
#include <cstdint>

#include "opentxs/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
OPENTXS_EXPORT auto copy(const ReadView in, Writer&& out) noexcept -> bool;
OPENTXS_EXPORT auto copy(
    const ReadView in,
    Writer&& out,
    const std::size_t limit) noexcept -> bool;
OPENTXS_EXPORT auto preallocated(const std::size_t size, void* out) noexcept
    -> Writer;
OPENTXS_EXPORT auto reader(const Space& in) noexcept -> ReadView;
OPENTXS_EXPORT auto reader(const Vector<std::byte>& in) noexcept -> ReadView;
OPENTXS_EXPORT auto reader(const UnallocatedVector<std::uint8_t>& in) noexcept
    -> ReadView;
OPENTXS_EXPORT auto space(const std::size_t size) noexcept -> Space;
OPENTXS_EXPORT auto space(const std::size_t size, alloc::Default alloc) noexcept
    -> Vector<std::byte>;
OPENTXS_EXPORT auto space(const ReadView bytes) noexcept -> Space;
OPENTXS_EXPORT auto space(const ReadView bytes, alloc::Default alloc) noexcept
    -> Vector<std::byte>;
OPENTXS_EXPORT auto valid(const ReadView view) noexcept -> bool;
OPENTXS_EXPORT auto writer(CString& in) noexcept -> Writer;
OPENTXS_EXPORT auto writer(UnallocatedCString& in) noexcept -> Writer;
OPENTXS_EXPORT auto writer(UnallocatedCString* protobuf) noexcept -> Writer;
OPENTXS_EXPORT auto writer(Space& in) noexcept -> Writer;
OPENTXS_EXPORT auto writer(Vector<std::byte>& in) noexcept -> Writer;
}  // namespace opentxs
