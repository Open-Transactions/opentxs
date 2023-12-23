// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/storage/Types.hpp"  // IWYU pragma: associated

#include <string_view>

namespace opentxs::storage
{
using namespace std::literals;

auto print(Bucket in) noexcept -> std::string_view
{
    using enum Bucket;

    if (left == in) {

        return "left"sv;
    } else {

        return "right"sv;
    }
}
}  // namespace opentxs::storage
