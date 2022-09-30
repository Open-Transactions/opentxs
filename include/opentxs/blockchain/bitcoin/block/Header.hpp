// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class OPENTXS_EXPORT Header final : public blockchain::block::Header
{
public:
    class Imp;

    auto as_Bitcoin() const noexcept -> const Header& final;
    auto MerkleRoot() const noexcept -> const blockchain::block::Hash&;
    auto Encode() const noexcept -> ByteArray;
    auto Nonce() const noexcept -> std::uint32_t;
    auto nBits() const noexcept -> std::uint32_t;
    auto Timestamp() const noexcept -> Time;
    auto Version() const noexcept -> std::uint32_t;

    auto swap(Header& rhs) noexcept -> void;

    Header() noexcept;
    OPENTXS_NO_EXPORT Header(Imp*) noexcept;
    Header(const Header&) noexcept;
    Header(Header&&) noexcept;
    auto operator=(const Header&) noexcept -> Header&;
    auto operator=(Header&&) noexcept -> Header&;

    ~Header() final;

protected:
    Imp* imp_bitcoin_;
};
}  // namespace opentxs::blockchain::bitcoin::block
