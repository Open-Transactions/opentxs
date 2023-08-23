// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/Types.hpp"  // IWYU pragma: associated
#include "opentxs/util/storage/Types.hpp"   // IWYU pragma: associated

#include <string_view>

#include "internal/util/P0330.hpp"

namespace opentxs::storage
{
using namespace std::literals;

auto get_search_order(Bucket bucket) noexcept -> Search
{
    using enum Search;
    using enum Bucket;

    if (left == bucket) {

        return ltr;
    } else {

        return rtl;
    }
}

auto is_valid(const Hash& hash) noexcept -> bool
{
    struct Visitor {
        auto operator()(const NullHash&) const noexcept -> bool { return true; }
        auto operator()(const UnencodedHash& in) const noexcept -> bool
        {
            return in.IsNull() || (in.Bytes() == blank_hash());
        }
        auto operator()(const Base58Hash& in) const noexcept -> bool
        {
            return in.empty() || (in == blank_hash());
        }
    };

    return false == std::visit(Visitor{}, hash);
}

auto next(Bucket in) noexcept -> Bucket
{
    using enum Bucket;

    if (left == in) {

        return right;
    } else {

        return left;
    }
}

auto print(Bucket in) noexcept -> std::string_view
{
    using enum Bucket;

    if (left == in) {

        return "left"sv;
    } else {

        return "right"sv;
    }
}

auto read(const ReadView bytes) noexcept -> Hash
{
    static constexpr auto minimum = UnencodedHash::payload_size_;
    static constexpr auto blank = blank_hash();

    if (auto size = bytes.size(); (size < minimum) || (bytes == blank)) {

        return NullHash{};
    } else if (size == minimum) {

        return UnencodedHash{bytes};
    } else if (size == 2_uz * minimum) {
        if (auto out = UnencodedHash{}; out.DecodeHex(bytes)) {

            return out;
        } else {

            return Base58Hash{bytes};
        }
    } else {

        return Base58Hash{bytes};
    }
}

auto to_string(const Hash& hash) noexcept -> UnallocatedCString
{
    struct Visitor {
        auto operator()(const NullHash&) const noexcept
        {
            return UnallocatedCString{blank_hash()};
        }
        auto operator()(const UnencodedHash& in) const noexcept
        {
            return in.asHex();
        }
        auto operator()(const Base58Hash& in) const noexcept { return in; }
    };

    return std::visit(Visitor{}, hash);
}

auto unencoded_view(const Hash& hash) noexcept -> ReadView
{
    struct Visitor {
        auto operator()(const NullHash&) const noexcept -> ReadView
        {
            return blank_hash();
        }
        auto operator()(const UnencodedHash& in) const noexcept -> ReadView
        {
            return in.Bytes();
        }
        auto operator()(const Base58Hash& in) const noexcept -> ReadView
        {
            return in;
        }
    };

    return std::visit(Visitor{}, hash);
}

auto write(const Hash& hash, std::string& proto) noexcept -> void
{
    proto.assign(unencoded_view(hash));
}
}  // namespace opentxs::storage
