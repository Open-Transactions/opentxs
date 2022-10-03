// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/block/header/Header.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/ByteArray.hpp"

namespace opentxs::blockchain::bitcoin::block
{
auto Header::Imp::MerkleRoot() const noexcept -> const blockchain::block::Hash&
{
    static const auto blank = blockchain::block::Hash{};

    return blank;
}

auto Header::Imp::Encode() const noexcept -> ByteArray
{
    static const auto blank = ByteArray{};

    return blank;
}
}  // namespace opentxs::blockchain::bitcoin::block

namespace opentxs::blockchain::bitcoin::block
{
Header::Header(Imp* imp) noexcept
    : blockchain::block::Header(imp)
    , imp_bitcoin_(imp)
{
    OT_ASSERT(nullptr != imp_bitcoin_);
}

Header::Header() noexcept
    : Header(std::make_unique<Header::Imp>().release())
{
}

Header::Header(const Header& rhs) noexcept
    : Header([&] {
        auto copy = rhs.imp_bitcoin_->clone_bitcoin();
        auto* out = copy->imp_bitcoin_;
        copy->imp_bitcoin_ = nullptr;
        copy->imp_ = nullptr;

        return out;
    }())
{
}

Header::Header(Header&& rhs) noexcept
    : Header()
{
    swap(rhs);
}

auto Header::operator=(const Header& rhs) noexcept -> Header&
{
    auto old = std::unique_ptr<Imp>(imp_bitcoin_);
    auto copy = rhs.imp_bitcoin_->clone_bitcoin();
    imp_bitcoin_ = copy->imp_bitcoin_;
    imp_ = imp_bitcoin_;
    copy->imp_bitcoin_ = nullptr;
    copy->imp_ = nullptr;

    return *this;
}

auto Header::operator=(Header&& rhs) noexcept -> Header&
{
    swap(rhs);

    return *this;
}

auto Header::as_Bitcoin() const noexcept -> const Header& { return *this; }

auto Header::MerkleRoot() const noexcept -> const blockchain::block::Hash&
{
    return imp_bitcoin_->MerkleRoot();
}

auto Header::Encode() const noexcept -> ByteArray
{
    return imp_bitcoin_->Encode();
}

auto Header::Nonce() const noexcept -> std::uint32_t
{
    return imp_bitcoin_->Nonce();
}

auto Header::nBits() const noexcept -> std::uint32_t
{
    return imp_bitcoin_->nBits();
}

auto Header::swap(Header& rhs) noexcept -> void
{
    swap_header(rhs);
    std::swap(imp_bitcoin_, rhs.imp_bitcoin_);
}

auto Header::Timestamp() const noexcept -> Time
{
    return imp_bitcoin_->Timestamp();
}

auto Header::Version() const noexcept -> std::uint32_t
{
    return imp_bitcoin_->Version();
}

Header::~Header()
{
    if (nullptr != imp_bitcoin_) {
        delete imp_bitcoin_;
        imp_bitcoin_ = nullptr;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::bitcoin::block
