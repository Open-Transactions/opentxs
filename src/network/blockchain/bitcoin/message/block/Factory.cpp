// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <optional>

#include "internal/network/blockchain/bitcoin/message/Block.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/bitcoin/message/block/Imp.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PBlock(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView block,
    alloc::Strategy alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Block
{
    using ReturnType = network::blockchain::bitcoin::message::block::Message;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);
        pmr.construct(out, api, chain, std::nullopt, block);

        return out;
    } catch (const std::exception& e) {
        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {alloc.result_};
    }
}
}  // namespace opentxs::factory
