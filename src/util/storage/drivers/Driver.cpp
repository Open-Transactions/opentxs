// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/Driver.hpp"  // IWYU pragma: associated

#include <string_view>

#include "opentxs/util/Writer.hpp"  // IWYU pragma: keep

namespace opentxs::storage::implementation
{
using namespace std::literals;

Driver::Driver(const api::Crypto& crypto, const storage::Config& config)
    : crypto_(crypto)
    , config_(config)
{
}
}  // namespace opentxs::storage::implementation
