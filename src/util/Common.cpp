// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "opentxs/util/Types.hpp"  // IWYU pragma: associated

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
using namespace std::literals;

auto print(BlockchainProfile in) noexcept -> std::string_view
{
    try {
        using namespace std::literals;
        static const auto map = Map<BlockchainProfile, std::string_view>{
            {BlockchainProfile::mobile, "mobile"sv},
            {BlockchainProfile::desktop, "desktop"sv},
            {BlockchainProfile::desktop_native, "desktop_native"sv},
            {BlockchainProfile::server, "server"sv},
        };

        return map.at(in);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid BlockchainProfile: ")(
            static_cast<std::uint8_t>(in))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs
