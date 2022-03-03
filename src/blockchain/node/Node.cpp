// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "internal/blockchain/node/Node.hpp"  // IWYU pragma: associated
#include "opentxs/blockchain/node/Types.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <iosfwd>
#include <sstream>

#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/blockchain/node/TxoTag.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::node::internal
{
auto Config::print() const noexcept -> UnallocatedCString
{
    constexpr auto print_bool = [](const bool in) {
        if (in) {
            return "true";
        } else {
            return "false";
        }
    };

    auto output = std::stringstream{};
    output << "Blockchain client options\n";
    output << "  * download cfilters: " << print_bool(download_cfilters_)
           << '\n';
    output << "  * generate cfilters: " << print_bool(generate_cfilters_)
           << '\n';
    output << "  * provide sync server: " << print_bool(provide_sync_server_)
           << '\n';
    output << "  * use sync server: " << print_bool(use_sync_server_) << '\n';
    output << "  * disable wallet: " << print_bool(disable_wallet_) << '\n';

    return output.str();
}
}  // namespace opentxs::blockchain::node::internal

namespace opentxs
{
auto print(blockchain::node::TxoState in) noexcept -> UnallocatedCString
{
    using Type = blockchain::node::TxoState;
    static const auto map =
        robin_hood::unordered_flat_map<Type, UnallocatedCString>{
            {Type::Error, "error"},
            {Type::UnconfirmedNew, "unspent (unconfirmed)"},
            {Type::UnconfirmedSpend, "spent (unconfirmed)"},
            {Type::ConfirmedNew, "unspent"},
            {Type::ConfirmedSpend, "spent"},
            {Type::OrphanedNew, "orphaned"},
            {Type::OrphanedSpend, "orphaned"},
            {Type::Immature, "newly generated"},
        };

    try {

        return map.at(in);
    } catch (...) {

        return {};
    }
}

auto print(blockchain::node::TxoTag in) noexcept -> UnallocatedCString
{
    using Type = blockchain::node::TxoTag;
    static const auto map =
        robin_hood::unordered_flat_map<Type, UnallocatedCString>{
            {Type::Normal, "normal"},
            {Type::Generation, "generated"},
        };

    try {

        return map.at(in);
    } catch (...) {

        return {};
    }
}
}  // namespace opentxs
