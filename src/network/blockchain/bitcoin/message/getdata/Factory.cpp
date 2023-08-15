// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <optional>

#include "network/blockchain/bitcoin/Inventory.hpp"
#include "internal/network/blockchain/bitcoin/message/Getdata.hpp"
#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/getdata/Imp.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Container.hpp"

namespace opentxs::factory
{
auto BitcoinP2PGetdata(
    const api::Session& api,
    const blockchain::Type chain,
    std::span<network::blockchain::bitcoin::Inventory> payload,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Getdata
{
    using ReturnType = network::blockchain::bitcoin::message::getdata::Message;

    try {

        return pmr::construct<ReturnType>(
            alloc,
            api,
            chain,
            std::nullopt,
            move_construct<network::blockchain::bitcoin::Inventory>(
                payload, alloc));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {alloc};
    }
}
}  // namespace opentxs::factory
