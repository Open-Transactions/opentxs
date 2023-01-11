// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <string_view>

#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace asio
{
namespace ip
{
class address;
class address_v4;
class address_v6;
}  // namespace ip
}  // namespace asio
}  // namespace boost

namespace opentxs
{
class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::asio
{
auto address_from_binary(ReadView in) noexcept
    -> std::optional<boost::asio::ip::address>;
auto address_from_string(std::string_view in) noexcept
    -> std::optional<boost::asio::ip::address>;
auto localhost4() noexcept -> boost::asio::ip::address_v4;
auto localhost4to6() noexcept -> boost::asio::ip::address;
auto localhost6() noexcept -> boost::asio::ip::address_v6;
[[nodiscard]] auto map_4_to_6(const boost::asio::ip::address& in) noexcept
    -> boost::asio::ip::address_v6;
auto map_4_to_6_inplace(boost::asio::ip::address& in) noexcept -> void;
auto serialize(const boost::asio::ip::address& in) noexcept -> ByteArray;
auto serialize(const boost::asio::ip::address_v4& in) noexcept -> ByteArray;
auto serialize(const boost::asio::ip::address_v6& in) noexcept -> ByteArray;
auto type(const boost::asio::ip::address& in) noexcept -> blockchain::Transport;
auto type(const boost::asio::ip::address_v4& in) noexcept
    -> blockchain::Transport;
auto type(const boost::asio::ip::address_v6& in) noexcept
    -> blockchain::Transport;
}  // namespace opentxs::network::asio
