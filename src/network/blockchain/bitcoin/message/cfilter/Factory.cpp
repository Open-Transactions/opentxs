// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <optional>

#include "internal/network/blockchain/bitcoin/message/Cfilter.hpp"
#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/cfilter/Imp.hpp"
#include "opentxs/blockchain/cfilter/GCS.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PCfilter(
    const api::Session& api,
    const blockchain::Type chain,
    const blockchain::cfilter::Type type,
    const blockchain::block::Hash& hash,
    const blockchain::cfilter::GCS& filter,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Cfilter
{
    using ReturnType = network::blockchain::bitcoin::message::cfilter::Message;

    try {

        return pmr::construct<ReturnType>(
            alloc,
            api,
            chain,
            std::nullopt,
            type,
            hash,
            filter.ElementCount(),
            [&] {
                auto gcs = ByteArray{};
                filter.Compressed(gcs.WriteInto());

                return gcs;
            }());
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {alloc};
    }
}
}  // namespace opentxs::factory
