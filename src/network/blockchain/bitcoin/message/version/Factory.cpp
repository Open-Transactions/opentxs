// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <cstring>
#include <optional>
#include <stdexcept>
#include <utility>

#include "BoostAsio.hpp"
#include "internal/network/blockchain/bitcoin/message/Version.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/bitcoin/message/version/Imp.hpp"
#include "opentxs/core/ByteArray.hpp"
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
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};
    const auto convert = [](const auto& address, const auto message) {
        const auto convert4 = [](const auto& in) {
            const auto bytes = in.Bytes();
            auto encoded = ip::address_v4::bytes_type{};

            if (encoded.size() != bytes.size()) {
                const auto error = UnallocatedCString{"expected "}
                                       .append(std::to_string(encoded.size()))
                                       .append(" bytes for ipv4 but received ")
                                       .append(std::to_string(bytes.size()))
                                       .append(" bytes");

                throw std::runtime_error(error);
            }

            std::memcpy(encoded.data(), bytes.data(), bytes.size());
            const auto v4 = ip::make_address_v4(encoded);

            return tcp::endpoint{ip::address_v6::v4_mapped(v4), in.Port()};
        };
        const auto convert6 = [](const auto& in) {
            const auto bytes = in.Bytes();
            auto encoded = ip::address_v6::bytes_type{};

            if (encoded.size() != bytes.size()) {
                const auto error = UnallocatedCString{"expected "}
                                       .append(std::to_string(encoded.size()))
                                       .append(" bytes for ipv6 but received ")
                                       .append(std::to_string(bytes.size()))
                                       .append(" bytes");

                throw std::runtime_error(error);
            }

            std::memcpy(encoded.data(), bytes.data(), bytes.size());

            return tcp::endpoint{ip::make_address_v6(encoded), in.Port()};
        };

        try {
            switch (address.Type()) {
                case ipv4: {

                    return std::make_pair(
                        convert4(address), address.Services());
                }
                case ipv6:
                case cjdns: {

                    return std::make_pair(
                        convert6(address), address.Services());
                }
                case zmq: {
                    switch (address.Subtype()) {
                        case ipv4: {

                            return std::make_pair(
                                convert4(address), address.Services());
                        }
                        case ipv6:
                        case cjdns: {

                            return std::make_pair(
                                convert6(address), address.Services());
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
                tcp::endpoint(
                    ip::make_address_v6("::ffff:127.0.0.1"sv), address.Port()),
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
