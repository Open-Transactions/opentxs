// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/storage/Types.hpp"

namespace opentxs::storage
{
using namespace std::literals;

enum class ErrorReporting : bool { verbose, silent };  // IWYU pragma: export

constexpr auto blank_hash() noexcept -> std::string_view
{
    return "blankblankblankblankblank"sv;
}
auto get_search_order(Bucket) noexcept -> Search;
auto is_valid(const Hash& hash) noexcept -> bool;
auto next(Bucket) noexcept -> Bucket;
auto read(ReadView bytes) noexcept -> Hash;
auto to_string(const Hash&) noexcept -> UnallocatedCString;
auto unencoded_view(const Hash&) noexcept -> ReadView;
auto write(const Hash& hash, std::string& proto) noexcept -> void;

struct EncodedView {
    auto operator()(const NullHash&) noexcept -> ReadView
    {
        return blank_hash();
    }
    auto operator()(const UnencodedHash& in) noexcept -> ReadView
    {
        buf_ = in.asHex();

        return buf_;
    }
    auto operator()(const Base58Hash& in) noexcept -> ReadView { return in; }

private:
    UnallocatedCString buf_{};
};
}  // namespace opentxs::storage
