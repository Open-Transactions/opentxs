// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <optional>

#include "internal/network/blockchain/bitcoin/message/Getaddr.hpp"
#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/getaddr/Imp.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PGetaddr(
    const api::Session& api,
    const blockchain::Type chain,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Getaddr
{
    using ReturnType = network::blockchain::bitcoin::message::getaddr::Message;

    try {

        return pmr::construct<ReturnType>(alloc, api, chain, std::nullopt);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {alloc};
    }
}
}  // namespace opentxs::factory
