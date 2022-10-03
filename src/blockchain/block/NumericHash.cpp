// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "opentxs/blockchain/block/NumericHash.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#include "blockchain/block/NumericHashPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain
{
auto HashToNumber(ReadView hex) noexcept -> UnallocatedCString
{
    return HashToNumber(ByteArray{IsHex, hex});
}

auto HashToNumber(const Hash& hash) noexcept -> UnallocatedCString
{
    return block::NumericHash{hash}.asHex();
}

auto NumberToHash(ReadView hex) noexcept -> pHash
{
    const auto hash = ByteArray{IsHex, hex};
    auto out = ByteArray{};

    for (auto i{hash.size()}; i > 0u; --i) {
        out += std::to_integer<std::uint8_t>(hash.at(i - 1u));
    }

    return out;
}
}  // namespace opentxs::blockchain

namespace opentxs::blockchain::block
{
NumericHash::NumericHash(NumericHashPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

NumericHash::NumericHash() noexcept
    : NumericHash(std::make_unique<NumericHashPrivate>().release())
{
}

NumericHash::NumericHash(std::uint32_t difficulty) noexcept
    : NumericHash(std::make_unique<NumericHashPrivate>(difficulty).release())
{
}

NumericHash::NumericHash(const Hash& block) noexcept
    : NumericHash(std::make_unique<NumericHashPrivate>(block).release())
{
}

NumericHash::NumericHash(const Data& data) noexcept
    : NumericHash(std::make_unique<NumericHashPrivate>(data).release())
{
}

NumericHash::NumericHash(const NumericHash& rhs) noexcept
    : NumericHash(std::make_unique<NumericHashPrivate>(*rhs.imp_).release())
{
}

NumericHash::NumericHash(NumericHash&& rhs) noexcept
    : NumericHash(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

auto NumericHash::asHex(const std::size_t minimumBytes) const noexcept
    -> UnallocatedCString
{
    return imp_->asHex(minimumBytes);
}

auto NumericHash::Decimal() const noexcept -> UnallocatedCString
{
    return imp_->Decimal();
}

auto NumericHash::operator<=>(const NumericHash& rhs) const noexcept
    -> std::strong_ordering
{
    return imp_->operator<=>(*rhs.imp_);
}

auto NumericHash::operator==(const NumericHash& rhs) const noexcept -> bool
{
    return imp_->operator==(*rhs.imp_);
}

auto NumericHash::operator=(const NumericHash& rhs) noexcept -> NumericHash&
{
    auto old = std::unique_ptr<NumericHashPrivate>{imp_};
    imp_ = std::make_unique<NumericHashPrivate>(*rhs.imp_).release();

    return *this;
}

auto NumericHash::operator=(NumericHash&& rhs) noexcept -> NumericHash&
{
    using std::swap;
    swap(imp_, rhs.imp_);

    return *this;
}

NumericHash::~NumericHash()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::block
