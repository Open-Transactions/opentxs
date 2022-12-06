// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/block/Outpoint.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "internal/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace be = boost::endian;

namespace opentxs::blockchain::block
{
Outpoint::Outpoint() noexcept
    : txid_()
    , index_()
{
    static_assert(sizeof(*this) == 36u);
}

Outpoint::Outpoint(const Outpoint& rhs) noexcept

    = default;

// NOLINTBEGIN(cert-oop11-cpp)
Outpoint::Outpoint(Outpoint&& rhs) noexcept
    : Outpoint(rhs)  // copy constructor, rhs is an lvalue
{
}
// NOLINTEND(cert-oop11-cpp)

Outpoint::Outpoint(const ReadView in) noexcept(false)
    : txid_()
    , index_()
{
    if (in.size() < sizeof(*this)) {
        throw std::runtime_error("Invalid bytes");
    }

    std::memcpy(static_cast<void*>(this), in.data(), sizeof(*this));
}

Outpoint::Outpoint(const ReadView txid, const std::uint32_t index) noexcept(
    false)
    : txid_()
    , index_()
{
    if (txid_.size() != txid.size()) {
        throw std::runtime_error("Invalid txid");
    }

    const auto buf = be::little_uint32_buf_t{index};

    static_assert(sizeof(index_) == sizeof(buf));

    std::memcpy(static_cast<void*>(txid_.data()), txid.data(), txid_.size());
    std::memcpy(static_cast<void*>(index_.data()), &buf, index_.size());
}

auto Outpoint::operator=(const Outpoint& rhs) noexcept -> Outpoint&
{
    if (&rhs != this) {
        txid_ = rhs.txid_;
        index_ = rhs.index_;
    }

    return *this;
}

auto Outpoint::operator=(Outpoint&& rhs) noexcept -> Outpoint&
{
    return operator=(rhs);  // copy assignment, rhs is an lvalue
}

auto Outpoint::Bytes() const noexcept -> ReadView
{
    return ReadView{reinterpret_cast<const char*>(this), sizeof(*this)};
}

auto spaceship(const Outpoint& lhs, const Outpoint& rhs) noexcept -> int;
auto spaceship(const Outpoint& lhs, const Outpoint& rhs) noexcept -> int
{
    const auto val = std::memcmp(lhs.Txid().data(), rhs.Txid().data(), 32u);

    if (0 != val) { return val; }

    const auto lIndex = lhs.Index();
    const auto rIndex = rhs.Index();

    if (lIndex < rIndex) {

        return -1;
    } else if (rIndex < lIndex) {

        return 1;
    } else {

        return 0;
    }
}

auto Outpoint::operator<(const Outpoint& rhs) const noexcept -> bool
{
    return 0 > spaceship(*this, rhs);
}

auto Outpoint::operator<=(const Outpoint& rhs) const noexcept -> bool
{
    return 0 >= spaceship(*this, rhs);
}

auto Outpoint::operator>(const Outpoint& rhs) const noexcept -> bool
{
    return 0 < spaceship(*this, rhs);
}

auto Outpoint::operator>=(const Outpoint& rhs) const noexcept -> bool
{
    return 0 <= spaceship(*this, rhs);
}

auto Outpoint::operator==(const Outpoint& rhs) const noexcept -> bool
{
    return (index_ == rhs.index_) && (txid_ == rhs.txid_);
}

auto Outpoint::operator!=(const Outpoint& rhs) const noexcept -> bool
{
    return (index_ != rhs.index_) || (txid_ != rhs.txid_);
}

auto Outpoint::Index() const noexcept -> std::uint32_t
{
    auto buf = be::little_uint32_buf_t{};

    static_assert(sizeof(index_) == sizeof(buf));

    std::memcpy(static_cast<void*>(&buf), index_.data(), index_.size());

    return buf.value();
}

auto Outpoint::str() const noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};

    for (const auto byte : txid_) {
        out << std::hex << std::setfill('0') << std::setw(2)
            << std::to_integer<int>(byte);
    }

    out << ':' << std::dec << Index();

    return out.str();
}

auto Outpoint::Txid() const noexcept -> ReadView { return reader(txid_); }
}  // namespace opentxs::blockchain::block
