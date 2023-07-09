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
    alloc::Strategy alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Version
{
    using ReturnType = network::blockchain::bitcoin::message::version::Message;
    using enum network::blockchain::Transport;
    using namespace network::asio;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};
    const auto convert = [](const auto& address, const auto message) {
        const auto encode = [](const auto& in, const auto m) {
            const auto serialized = in.Bytes();
            auto addr = address_from_binary(serialized.Bytes());

            if (false == addr.has_value()) {

                throw std::runtime_error{"unable to encode "s + m + "address"s};
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

                            return std::make_pair(
                                tcp::endpoint(localhost4to6(), address.Port()),
                                address.Services());
                        }
                    }
                }
                default: {

                    throw std::runtime_error{
                        "unable to encode "s + message +
                        " address type "s.append(print(address.Type())) +
                        ", subtype "s.append(print(address.Subtype())) +
                        " as ipv6"s};
                }
            }
        } catch (const std::exception& e) {
            LogError()("opentxs::factory::BitcoinP2PVersion: ")(e.what())
                .Flush();

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
            CString{userAgent, alloc.result_},
            height,
            bip37,
            Clock::now(),
            std::nullopt,
            std::nullopt);

        return out;
    } catch (const std::exception& e) {
        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {alloc.result_};
    }
}
}  // namespace opentxs::factory
