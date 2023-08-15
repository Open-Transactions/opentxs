// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "opentxs/blockchain/protocol/bitcoin/base/block/Header.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/block/header/HeaderPrivate.hpp"
#include "blockchain/protocol/bitcoin/base/block/header/HeaderPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/ByteArray.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
Header::Header(blockchain::block::HeaderPrivate* imp) noexcept
    : blockchain::block::Header(std::move(imp))
{
}

Header::Header(allocator_type alloc) noexcept
    : Header(HeaderPrivate::Blank(alloc))
{
}

Header::Header(const Header& rhs, allocator_type alloc) noexcept
    : Header(rhs.imp_->clone(alloc))
{
}

Header::Header(Header&& rhs) noexcept
    : Header(std::exchange(rhs.imp_, nullptr))
{
}

Header::Header(Header&& rhs, allocator_type alloc) noexcept
    : blockchain::block::Header(std::move(rhs), alloc)
{
}

auto Header::Blank() noexcept -> Header&
{
    static auto blank = Header{};

    return blank;
}

auto Header::MerkleRoot() const noexcept -> const blockchain::block::Hash&
{
    return imp_->asBitcoinPrivate()->MerkleRoot();
}

auto Header::Encode() const noexcept -> ByteArray
{
    return imp_->asBitcoinPrivate()->Encode();
}

auto Header::Nonce() const noexcept -> std::uint32_t
{
    return imp_->asBitcoinPrivate()->Nonce();
}

auto Header::nBits() const noexcept -> std::uint32_t
{
    return imp_->asBitcoinPrivate()->nBits();
}

auto Header::operator=(const Header& rhs) noexcept -> Header&
{
    return pmr::copy_assign_child<blockchain::block::Header>(*this, rhs);
}

auto Header::operator=(Header&& rhs) noexcept -> Header&
{
    return pmr::move_assign_child<blockchain::block::Header>(
        *this, std::move(rhs));
}

auto Header::Timestamp() const noexcept -> Time
{
    return imp_->asBitcoinPrivate()->Timestamp();
}

auto Header::Version() const noexcept -> std::uint32_t
{
    return imp_->asBitcoinPrivate()->Version();
}

Header::~Header() = default;
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block
