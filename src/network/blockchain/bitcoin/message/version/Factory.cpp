// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <optional>
#include <stdexcept>
#include <utility>

#include "BoostAsio.hpp"
#include "internal/network/asio/Types.hpp"
#include "internal/network/blockchain/bitcoin/message/Version.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/bitcoin/message/version/Imp.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::factory
{
using namespace std::literals;

auto BitcoinP2PVersion(
    const api::Session& api,
    const blockchain::Type chain,
    const std::int32_t version,
    const network::blockchain::Address& localAddress,
    const network::blockchain::Address& remoteAddress,
    const std::uint64_t nonce,
    const std::string_view userAgent,
    const blockchain::block::Height height,
    const bool bip37,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Version
{
    using ReturnType = network::blockchain::bitcoin::message::version::Message;
    using enum network::blockchain::Transport;
    using namespace network::asio;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};
    const auto convert = [](const auto& address, const auto message) {
        const auto encode = [](const auto& in, const auto m) {
            const auto serialized = in.Bytes();
            auto addr = address_from_binary(serialized.Bytes());

            if (false == addr.has_value()) {
                const auto error =
                    UnallocatedCString{"unable to encode "}.append(m).append(
                        " address");

                throw std::runtime_error{error};
            }

            map_4_to_6_inplace(*addr);

            return tcp::endpoint{std::move(*addr), in.Port()};
        };

        try {
            switch (address.Type()) {
                case ipv4:
                case ipv6:
                case cjdns: {

                    return std::make_pair(
                        encode(address, message), address.Services());
                }
                case zmq: {
                    switch (address.Subtype()) {
                        case ipv4:
                        case ipv6:
                        case cjdns: {

                            return std::make_pair(
                                encode(address, message), address.Services());
                        }
                        default: {
                            const auto error =
                                UnallocatedCString{"unable to encode "}
                                    .append(message)
                                    .append(" address as ipv6");

                            throw std::runtime_error{error};
                        }
                    }
                }
                default: {
                    const auto error = UnallocatedCString{"unable to encode "}
                                           .append(message)
                                           .append(" address as ipv6");

                    throw std::runtime_error{error};
                }
            }
        } catch (const std::exception& e) {
            LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

            return std::make_pair(
                tcp::endpoint(localhost4to6(), address.Port()),
                address.Services());
        }
    };

    try {
        auto [localEndpoint, localServices] = convert(localAddress, "local");
        auto [remoteEndpoint, remoteServices] =
            convert(remoteAddress, "remote");
        out = pmr.allocate(1_uz);
        pmr.construct(
            out,
            api,
            chain,
            std::nullopt,
            version,
            std::move(localEndpoint),
            std::move(remoteEndpoint),
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
