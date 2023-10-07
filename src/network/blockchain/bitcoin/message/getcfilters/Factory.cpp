// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <optional>

#include "internal/network/blockchain/bitcoin/message/Getcfilters.hpp"
#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/getcfilters/Imp.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PGetcfilters(
    const api::Session& api,
    const blockchain::Type chain,
    const blockchain::cfilter::Type type,
    const blockchain::block::Height start,
    const blockchain::block::Hash& stop,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Getcfilters
{
    using ReturnType =
        network::blockchain::bitcoin::message::getcfilters::Message;

    try {

        return pmr::construct<ReturnType>(
            alloc, api, chain, std::nullopt, type, start, stop);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {alloc};
    }
}
}  // namespace opentxs::factory
