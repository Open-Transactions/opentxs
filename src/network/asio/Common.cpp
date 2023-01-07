// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/asio/Types.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>
#include <cstring>

#include "BoostAsio.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep

namespace opentxs::network::asio
{
using namespace std::literals;

auto address_from_binary(ReadView in) noexcept
    -> std::optional<boost::asio::ip::address>
{
    using namespace boost::asio;

    auto buf4 = ip::address_v4::bytes_type{};
    auto buf6 = ip::address_v6::bytes_type{};
    constexpr auto ipv4 = buf4.size();
    constexpr auto ipv6 = buf6.size();

    switch (in.size()) {
        case ipv4: {
            std::memcpy(buf4.data(), in.data(), in.size());

            return ip::make_address_v4(buf4);
        }
        case ipv6: {
            std::memcpy(buf6.data(), in.data(), in.size());

            return ip::make_address_v6(buf6);
        }
        default: {
        }
    }

    return std::nullopt;
}

auto address_from_string(std::string_view in) noexcept
    -> std::optional<boost::asio::ip::address>
{
    using namespace boost::asio;

    auto ec = boost::system::error_code{};
    auto val = ip::make_address(in, ec);

    if (false == ec.operator bool()) { return val; }

    return std::nullopt;
}

auto localhost4() noexcept -> boost::asio::ip::address_v4
{
    return address_from_string("127.0.0.1"sv)->to_v4();
}

auto localhost4to6() noexcept -> boost::asio::ip::address
{
    static const auto addr = [] {
        using namespace boost::asio;
        auto localhost = ip::address{localhost4()};
        map_4_to_6_inplace(localhost);

        return localhost;
    }();

    return addr;
}

auto localhost6() noexcept -> boost::asio::ip::address_v6
{
    return address_from_string("::1"sv)->to_v6();
}

auto map_4_to_6(const boost::asio::ip::address& in) noexcept
    -> boost::asio::ip::address_v6
{
    using namespace boost::asio;

    if (in.is_v6()) {

        return in.to_v6();
    } else {

        return ip::make_address_v6(ip::v4_mapped, in.to_v4());
    }
}

auto map_4_to_6_inplace(boost::asio::ip::address& in) noexcept -> void
{
    using namespace boost::asio;

    if (in.is_v6()) {

        return;
    } else {

        in = ip::make_address_v6(ip::v4_mapped, in.to_v4());
    }
}

auto serialize(const boost::asio::ip::address& in) noexcept -> ByteArray
{
    if (in.is_v4()) {

        return serialize(in.to_v4());
    } else if (in.is_v6()) {

        return serialize(in.to_v6());
    } else {

        return {};
    }
}

auto serialize(const boost::asio::ip::address_v4& in) noexcept -> ByteArray
{
    const auto bytes = in.to_bytes();

    return ReadView{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}

auto serialize(const boost::asio::ip::address_v6& in) noexcept -> ByteArray
{
    const auto bytes = in.to_bytes();

    return ReadView{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}

auto type(const boost::asio::ip::address& in) noexcept -> blockchain::Transport
{
    using enum blockchain::Transport;

    if (in.is_v4()) {

        return ipv4;
    } else if (in.is_v6()) {

        return ipv6;
    } else {

        return invalid;
    }
}

auto type(const boost::asio::ip::address_v4& in) noexcept
    -> blockchain::Transport
{
    using enum blockchain::Transport;

    return ipv4;
}

auto type(const boost::asio::ip::address_v6& in) noexcept
    -> blockchain::Transport
{
    using enum blockchain::Transport;

    return ipv6;
}
}  // namespace opentxs::network::asio
