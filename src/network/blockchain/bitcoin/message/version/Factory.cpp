// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <optional>
#include <utility>

#include "BoostAsio.hpp"
#include "internal/network/blockchain/bitcoin/message/Version.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/bitcoin/message/version/Imp.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::factory
{
auto BitcoinP2PVersion(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::Transport style,
    const std::int32_t version,
    const Set<network::blockchain::bitcoin::Service>& localServices,
    const std::string_view localAddress,
    const std::uint16_t localPort,
    const Set<network::blockchain::bitcoin::Service>& remoteServices,
    const std::string_view remoteAddress,
    const std::uint16_t remotePort,
    const std::uint64_t nonce,
    const std::string_view userAgent,
    const blockchain::block::Height height,
    const bool bip37,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Version
{
    using ReturnType = network::blockchain::bitcoin::message::version::Message;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);
        auto [local, remote] = [&] {
            if (network::blockchain::Transport::zmq == style) {

                return std::make_pair(
                    tcp::endpoint(
                        ip::make_address_v6("::FFFF:7f0e:5801"), localPort),
                    tcp::endpoint(
                        ip::make_address_v6("::FFFF:7f0e:5802"), remotePort));
            } else {

                return std::make_pair(
                    tcp::endpoint(ip::make_address_v6(localAddress), localPort),
                    tcp::endpoint(
                        ip::make_address_v6(remoteAddress), remotePort));
            }
        }();
        pmr.construct(
            out,
            api,
            chain,
            std::nullopt,
            version,
            std::move(local),
            std::move(remote),
            localServices,
            localServices,
            remoteServices,
            nonce,
            CString{userAgent, alloc},
            height,
            bip37,
            Clock::now());

        return out;
    } catch (const std::exception& e) {
        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {alloc};
    }
}
}  // namespace opentxs::factory
