// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Outpoint;
class TransactionHash;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct hash<opentxs::blockchain::block::Outpoint> {
    auto operator()(const opentxs::blockchain::block::Outpoint& data)
        const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::blockchain::block
{
class OPENTXS_EXPORT Outpoint
{
public:
    std::array<std::byte, 32> txid_;
    std::array<std::byte, 4> index_;

    auto operator<(const Outpoint& rhs) const noexcept -> bool;
    auto operator<=(const Outpoint& rhs) const noexcept -> bool;
    auto operator>(const Outpoint& rhs) const noexcept -> bool;
    auto operator>=(const Outpoint& rhs) const noexcept -> bool;
    auto operator==(const Outpoint& rhs) const noexcept -> bool;
    auto operator!=(const Outpoint& rhs) const noexcept -> bool;

    auto Bytes() const noexcept -> ReadView;
    auto Index() const noexcept -> std::uint32_t;
    auto Txid() const noexcept -> ReadView;
    auto str() const noexcept -> UnallocatedCString;

    Outpoint(const ReadView serialized) noexcept(false);
    Outpoint(const TransactionHash& txid, const std::uint32_t index) noexcept(
        false);
    Outpoint() noexcept;
    Outpoint(const Outpoint&) noexcept;
    Outpoint(Outpoint&&) noexcept;
    auto operator=(const Outpoint&) noexcept -> Outpoint&;
    auto operator=(Outpoint&&) noexcept -> Outpoint&;
};
}  // namespace opentxs::blockchain::block
