// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <optional>

#include "internal/network/blockchain/bitcoin/message/Getheaders.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/bitcoin/message/getheaders/Imp.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Container.hpp"

namespace opentxs::factory
{
auto BitcoinP2PGetheaders(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::bitcoin::message::ProtocolVersionUnsigned
        version,
    std::span<blockchain::block::Hash> history,
    const blockchain::block::Hash& stop,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Getheaders
{
    using ReturnType =
        network::blockchain::bitcoin::message::getheaders::Message;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);
        pmr.construct(
            out,
            api,
            chain,
            std::nullopt,
            version,
            stop,
            move_construct<blockchain::block::Hash>(history, alloc));

        return out;
    } catch (const std::exception& e) {
        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {alloc};
    }
}
}  // namespace opentxs::factory
