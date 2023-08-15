// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/block/header/HeaderPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/protocol/bitcoin/base/block/header/HeaderPrivate.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Header.hpp"

namespace opentxs::blockchain::block
{
HeaderPrivate::HeaderPrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

HeaderPrivate::HeaderPrivate(
    const HeaderPrivate&,
    allocator_type alloc) noexcept
    : HeaderPrivate(std::move(alloc))
{
}

auto HeaderPrivate::asBitcoinPrivate() const noexcept
    -> const protocol::bitcoin::base::block::HeaderPrivate*
{
    static const auto blank = protocol::bitcoin::base::block::HeaderPrivate{{}};

    return std::addressof(blank);
}

auto HeaderPrivate::asBitcoinPrivate() noexcept
    -> protocol::bitcoin::base::block::HeaderPrivate*
{
    static auto blank = protocol::bitcoin::base::block::HeaderPrivate{{}};

    return std::addressof(blank);
}

auto HeaderPrivate::asBitcoinPublic() const noexcept
    -> const protocol::bitcoin::base::block::Header&
{
    return protocol::bitcoin::base::block::Header::Blank();
}

auto HeaderPrivate::asBitcoinPublic() noexcept
    -> protocol::bitcoin::base::block::Header&
{
    return protocol::bitcoin::base::block::Header::Blank();
}

auto HeaderPrivate::Reset(block::Header& header) noexcept -> void
{
    header.imp_ = nullptr;
}

HeaderPrivate::~HeaderPrivate() = default;
}  // namespace opentxs::blockchain::block
