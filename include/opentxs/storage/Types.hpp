// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>
#include <string_view>
#include <tuple>  // IWYU pragma: keep
#include <utility>
#include <variant>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::storage
{
enum class Bucket : bool { left, right };  // IWYU pragma: export
enum class Search : bool { ltr, rtl };     // IWYU pragma: export

using NullHash = std::monostate;
using UnencodedHash = FixedByteArray<32>;
// NOTE versions of opentxs prior to 1.172.0 stored hashes as base58 encoded
// strings. This type is retained for compatibility with existing data
// directories.
// Legacy databases will be upgraded to the new hash format after the first
// storage garbage collection run.
using Base58Hash = UnallocatedCString;
using Hash = std::variant<NullHash, UnencodedHash, Base58Hash>;
using Transaction = std::pair<std::span<const Hash>, std::span<const ReadView>>;

OPENTXS_EXPORT auto print(Bucket) noexcept -> std::string_view;
}  // namespace opentxs::storage
