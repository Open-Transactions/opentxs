// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/blockchain/block/Header.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Hash;
class HeaderPrivate;
}  // namespace block
}  // namespace blockchain

class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
class OPENTXS_EXPORT Header : public blockchain::block::Header
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Header&;

    auto MerkleRoot() const noexcept -> const blockchain::block::Hash&;
    auto Encode() const noexcept -> ByteArray;
    auto Nonce() const noexcept -> std::uint32_t;
    auto nBits() const noexcept -> std::uint32_t;
    auto Timestamp() const noexcept -> Time;
    auto Version() const noexcept -> std::uint32_t;

    OPENTXS_NO_EXPORT Header(blockchain::block::HeaderPrivate* imp) noexcept;
    Header(allocator_type alloc = {}) noexcept;
    Header(const Header& rhs, allocator_type alloc = {}) noexcept;
    Header(Header&& rhs) noexcept;
    Header(Header&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Header& rhs) noexcept -> Header&;
    auto operator=(Header&& rhs) noexcept -> Header&;

    ~Header() override;
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block
