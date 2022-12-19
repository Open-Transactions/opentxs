// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/blockchain/block/Header.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{

namespace block
{
class Hash;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::internal
{
class Header : virtual public blockchain::block::internal::Header
{
public:
    static auto Blank() noexcept -> Header&;

    auto asBitcoin() const noexcept
        -> const blockchain::bitcoin::block::internal::Header& final
    {
        return *this;
    }
    virtual auto MerkleRoot() const noexcept -> const blockchain::block::Hash&;
    virtual auto Encode() const noexcept -> ByteArray;
    virtual auto Nonce() const noexcept -> std::uint32_t;
    virtual auto nBits() const noexcept -> std::uint32_t;
    virtual auto Timestamp() const noexcept -> Time;
    virtual auto Version() const noexcept -> std::uint32_t;

    auto asBitcoin() noexcept
        -> blockchain::bitcoin::block::internal::Header& final
    {
        return *this;
    }

    ~Header() override = default;
};
}  // namespace opentxs::blockchain::bitcoin::block::internal
