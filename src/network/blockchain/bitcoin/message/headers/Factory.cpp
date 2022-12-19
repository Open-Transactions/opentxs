// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <optional>

#include "internal/network/blockchain/bitcoin/message/Headers.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/bitcoin/message/headers/Imp.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Container.hpp"

namespace opentxs::factory
{
auto BitcoinP2PHeaders(
    const api::Session& api,
    const blockchain::Type chain,
    std::span<blockchain::block::Header> headers,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Headers
{
    using ReturnType = network::blockchain::bitcoin::message::headers::Message;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);
        pmr.construct(
            out,
            api,
            chain,
            std::nullopt,
            move_construct<blockchain::block::Header>(headers, alloc));

        return out;
    } catch (const std::exception& e) {
        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {alloc};
    }
}
}  // namespace opentxs::factory
