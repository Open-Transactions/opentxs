// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/util/P0330.hpp"

namespace opentxs::protobuf::inline syntax
{
static constexpr auto MAX_LABEL_SIZE = 512_uz;
static constexpr auto MAX_PLAUSIBLE_IDENTIFIER = 116_uz;
static constexpr auto MAX_PLAUSIBLE_KEYSIZE = 8192_uz;
static constexpr auto MAX_PLAUSIBLE_SCRIPT = 1048576_uz;
static constexpr auto MAX_PLAUSIBLE_WORK = 128_uz;
static constexpr auto MAX_TRANSACTION_MEMO_SIZE = 512_uz;
static constexpr auto MAX_VALID_CONTACT_VALUE = 512_uz;
static constexpr auto MAX_VALID_PORT = std::uint32_t{65535U};
static constexpr auto MIN_PLAUSIBLE_IDENTIFIER = 20_uz;
static constexpr auto MIN_PLAUSIBLE_KEYSIZE = 16_uz;
static constexpr auto MIN_PLAUSIBLE_SCRIPT = 2_uz;
static constexpr auto MIN_PLAUSIBLE_SIGNATURE = 32_uz;
}  // namespace opentxs::protobuf::inline syntax
