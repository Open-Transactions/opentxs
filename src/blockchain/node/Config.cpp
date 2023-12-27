// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/Config.hpp"  // IWYU pragma: associated

#include <sstream>
#include <string_view>

#include "internal/util/P0330.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Type.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::internal
{
auto Config::PeerTarget(const blockchain::Type chain) const noexcept
    -> std::size_t
{
    if (Type::UnitTest == chain) { return 0_uz; }

    switch (profile_) {
        case BlockchainProfile::mobile: {

            return 2_uz;
        }
        case BlockchainProfile::desktop:
        case BlockchainProfile::desktop_native: {

            return 4_uz;
        }
        case BlockchainProfile::server: {

            return 8_uz;
        }
        default: {

            LogAbort()().Abort();
        }
    }
}

auto Config::Print(alloc::Default alloc) const noexcept -> CString
{
    constexpr auto print_bool = [](const bool in) {
        if (in) {
            return "true";
        } else {
            return "false";
        }
    };

    // TODO c++20 allocator
    auto output = std::stringstream{};
    output << "Blockchain client options\n";
    output << "  * profile: " << opentxs::print(profile_) << '\n';
    output << "  * provide sync server: " << print_bool(provide_sync_server_)
           << '\n';
    output << "  * disable wallet: " << print_bool(disable_wallet_) << '\n';

    return CString{alloc}.append(output.str());
}
}  // namespace opentxs::blockchain::node::internal
