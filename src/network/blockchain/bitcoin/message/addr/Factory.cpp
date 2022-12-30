// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <exception>
#include <iterator>
#include <optional>
#include <vector>

#include "internal/network/blockchain/bitcoin/message/Addr.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/bitcoin/message/addr/Imp.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PAddr(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    std::span<network::blockchain::Address> addresses,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Addr
{
    using ReturnType = network::blockchain::bitcoin::message::addr::Message;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);
        pmr.construct(out, api, chain, std::nullopt, version, [&] {
            auto vec = ReturnType::AddressVector{pmr};
            vec.reserve(addresses.size());
            vec.clear();
            std::move(
                addresses.begin(), addresses.end(), std::back_inserter(vec));

            return vec;
        }());

        return out;
    } catch (const std::exception& e) {
        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {alloc};
    }
}
}  // namespace opentxs::factory
