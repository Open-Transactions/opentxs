// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "internal/core/Factory.hpp"  // IWYU pragma: associated
#include "opentxs/core/Amount.hpp"    // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <boost/exception/exception.hpp>
#include <boost/lexical_cast/bad_lexical_cast.hpp>
#include <memory>

#include "core/Amount.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Writer.hpp"

namespace be = boost::endian;

namespace opentxs::factory
{
auto Amount(std::string_view str, bool normalize) noexcept(false)
    -> opentxs::Amount
{
    return std::make_unique<opentxs::Amount::Imp>(str, normalize).release();
}

auto Amount(const network::zeromq::Frame& in) noexcept(false) -> opentxs::Amount
{
    return std::make_unique<opentxs::Amount::Imp>(in.Bytes()).release();
}
}  // namespace opentxs::factory

namespace opentxs
{
auto signed_amount(long long ip, unsigned long long fp, unsigned long long div)
    -> Amount
{
    using Imp = Amount::Imp;

    if (ip < 0) {
        return -Amount(Imp::shift_left(-ip) + (Imp::shift_left(fp) / div));
    } else {
        return {Imp::shift_left(ip) + (Imp::shift_left(fp) / div)};
    }
}

auto unsigned_amount(
    unsigned long long ip,
    unsigned long long fp,
    unsigned long long div) -> Amount
{
    using Imp = Amount::Imp;

    return {Imp::shift_left(ip) + (Imp::shift_left(fp) / div)};
}

auto swap(Amount& lhs, Amount& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs

namespace opentxs
{
template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto operator==(T lhs, const Amount& rhs) noexcept -> bool
{
    return Amount{lhs}.operator==(rhs);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto operator!=(T lhs, const Amount& rhs) noexcept -> bool
{
    return Amount{lhs}.operator!=(rhs);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto operator<(T lhs, const Amount& rhs) noexcept -> bool
{
    return Amount{lhs}.operator<(rhs);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto operator<=(T lhs, const Amount& rhs) noexcept -> bool
{
    return Amount{lhs}.operator<=(rhs);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto operator>(T lhs, const Amount& rhs) noexcept -> bool
{
    return Amount{lhs}.operator>(rhs);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto operator>=(T lhs, const Amount& rhs) noexcept -> bool
{
    return Amount{lhs}.operator>=(rhs);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto operator*(T lhs, const Amount& rhs) noexcept(false) -> Amount
{
    return rhs.operator*(lhs);
}
}  // namespace opentxs

namespace opentxs::amount
{
auto FloatToInteger(const Float& rhs) noexcept(false) -> Integer
{
    return (rhs * GetScale()).convert_to<Integer>();
}

auto GetScale() noexcept -> Float
{
    static const auto output = Float{Integer{1u} << fractional_bits_};

    return output;
}

auto IntegerToFloat(const Integer& rhs) noexcept -> Float
{
    return rhs.convert_to<amount::Float>() / amount::GetScale();
}
}  // namespace opentxs::amount

namespace opentxs::internal
{
// NOLINTBEGIN(clang-analyzer-core.StackAddressEscape)
auto FloatToAmount(const amount::Float& rhs) noexcept(false)
    -> opentxs::Amount::Imp*
{
    return std::make_unique<opentxs::Amount::Imp>(amount::FloatToInteger(rhs))
        .release();
}
// NOLINTEND(clang-analyzer-core.StackAddressEscape)

auto Amount::SerializeBitcoinSize() noexcept -> std::size_t
{
    return sizeof(be::little_int64_buf_t);
}
}  // namespace opentxs::internal

namespace opentxs
{
Amount::Amount(Imp* rhs) noexcept
    : imp_(rhs)
{
    OT_ASSERT(nullptr != imp_);
}

Amount::Amount(const Imp& rhs) noexcept
    : Amount(std::make_unique<Imp>(rhs).release())
{
}

Amount::Amount(int rhs)
    : Amount(Imp::factory_signed(rhs))
{
}

Amount::Amount(long rhs)
    : Amount(Imp::factory_signed(rhs))
{
}

Amount::Amount(long long rhs)
    : Amount(Imp::factory_signed(rhs))
{
}

Amount::Amount(unsigned rhs)
    : Amount(Imp::factory_unsigned(rhs))
{
}

Amount::Amount(unsigned long rhs)
    : Amount(Imp::factory_unsigned(rhs))
{
}

Amount::Amount(unsigned long long rhs)
    : Amount(Imp::factory_unsigned(rhs))
{
}

Amount::Amount() noexcept
    : Amount(std::make_unique<Imp>().release())
{
}

Amount::Amount(const Amount& rhs) noexcept
    : Amount(std::make_unique<Imp>(*rhs.imp_).release())
{
}

Amount::Amount(Amount&& rhs) noexcept
    : Amount()
{
    operator=(std::move(rhs));
}

auto Amount::operator=(const Amount& rhs) noexcept -> Amount&
{
    auto old = std::unique_ptr<Imp>{imp_};
    imp_ = std::make_unique<Imp>(*rhs.imp_).release();

    return *this;
}

auto Amount::operator=(Amount&& rhs) noexcept -> Amount&
{
    swap(rhs);

    return *this;
}

auto Amount::operator<(const Amount& rhs) const noexcept -> bool
{
    return *imp_ < *rhs.imp_;
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto Amount::operator<(const T rhs) const noexcept -> bool
{
    return *imp_ < rhs;
}

auto Amount::operator>(const Amount& rhs) const noexcept -> bool
{
    return *imp_ > *rhs.imp_;
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto Amount::operator>(const T rhs) const noexcept -> bool
{
    return *imp_ > rhs;
}

auto Amount::operator==(const Amount& rhs) const noexcept -> bool
{
    return *imp_ == *rhs.imp_;
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto Amount::operator==(const T rhs) const noexcept -> bool
{
    return *imp_ == rhs;
}

auto Amount::operator!=(const Amount& rhs) const noexcept -> bool
{
    return *imp_ != *rhs.imp_;
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto Amount::operator!=(const T rhs) const noexcept -> bool
{
    return *imp_ != rhs;
}

auto Amount::operator<=(const Amount& rhs) const noexcept -> bool
{
    return *imp_ <= *rhs.imp_;
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto Amount::operator<=(const T rhs) const noexcept -> bool
{
    return *imp_ <= rhs;
}

auto Amount::operator>=(const Amount& rhs) const noexcept -> bool
{
    return *imp_ >= *rhs.imp_;
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto Amount::operator>=(const T rhs) const noexcept -> bool
{
    return *imp_ >= rhs;
}

auto Amount::operator+(const Amount& rhs) const noexcept(false) -> Amount
{
    return *imp_ + *rhs.imp_;
}

auto Amount::operator-(const Amount& rhs) const noexcept(false) -> Amount
{
    return *imp_ - *rhs.imp_;
}

auto Amount::operator*(const Amount& rhs) const noexcept(false) -> Amount
{
    return *imp_ * *rhs.imp_;
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto Amount::operator*(const T rhs) const noexcept(false) -> Amount
{
    return *imp_ * rhs;
}

auto Amount::operator/(const Amount& rhs) const noexcept(false) -> Amount
{
    return *imp_ / *rhs.imp_;
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto Amount::operator/(const T rhs) const noexcept(false) -> Amount
{
    return *imp_ / rhs;
}

auto Amount::operator%(const Amount& rhs) const noexcept(false) -> Amount
{
    return *imp_ % *rhs.imp_;
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int>>
auto Amount::operator%(const T rhs) const noexcept(false) -> Amount
{
    return *imp_ % rhs;
}

auto Amount::operator*=(const Amount& rhs) noexcept(false) -> Amount&
{
    *imp_ *= *rhs.imp_;

    return *this;
}

auto Amount::operator+=(const Amount& rhs) noexcept(false) -> Amount&
{
    *imp_ += *rhs.imp_;

    return *this;
}

auto Amount::operator-=(const Amount& rhs) noexcept(false) -> Amount&
{
    *imp_ -= *rhs.imp_;

    return *this;
}

auto Amount::operator-() -> Amount { return -*imp_; }

auto Amount::Internal() const noexcept -> const internal::Amount&
{
    return *imp_;
}

auto Amount::Serialize(Writer&& dest) const noexcept -> bool
{
    return imp_->Serialize(std::move(dest));
}

auto Amount::swap(Amount& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

Amount::~Amount()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs

namespace opentxs
{
// clang-format off
template auto operator==
    <int>(int, const Amount&) -> bool;
template auto operator==
    <long>(long, const Amount&) -> bool;
template auto operator==
    <long long>(long long, const Amount&) -> bool;
template auto operator==
    <unsigned>(unsigned, const Amount&) -> bool;
template auto operator==
    <unsigned long>(unsigned long, const Amount&) -> bool;
template auto operator==
    <unsigned long long>(unsigned long long, const Amount&) -> bool;

template auto operator!=
    <int>(int, const Amount&) -> bool;
template auto operator!=
    <long>(long, const Amount&) -> bool;
template auto operator!=
    <long long>(long long, const Amount&) -> bool;
template auto operator!=
    <unsigned>(unsigned, const Amount&) -> bool;
template auto operator!=
    <unsigned long>(unsigned long, const Amount&) -> bool;
template auto operator!=
    <unsigned long long>(unsigned long long, const Amount&) -> bool;

template auto operator<
    <int>(int, const Amount&) -> bool;
template auto operator<
    <long>(long, const Amount&) -> bool;
template auto operator<
    <long long>(long long, const Amount&) -> bool;
template auto operator<
    <unsigned>(unsigned, const Amount&) -> bool;
template auto operator<
    <unsigned long>(unsigned long, const Amount&) -> bool;
template auto operator<
    <unsigned long long>(unsigned long long, const Amount&) -> bool;

template auto operator<=
    <int>(int, const Amount&) -> bool;
template auto operator<=
    <long>(long, const Amount&) -> bool;
template auto operator<=
    <long long>(long long, const Amount&) -> bool;
template auto operator<=
    <unsigned>(unsigned, const Amount&) -> bool;
template auto operator<=
    <unsigned long>(unsigned long, const Amount&) -> bool;
template auto operator<=
    <unsigned long long>(unsigned long long, const Amount&) -> bool;

template auto operator>
    <int>(int, const Amount&) -> bool;
template auto operator>
    <long>(long, const Amount&) -> bool;
template auto operator>
    <long long>(long long, const Amount&) -> bool;
template auto operator>
    <unsigned>(unsigned, const Amount&) -> bool;
template auto operator>
    <unsigned long>(unsigned long, const Amount&) -> bool;
template auto operator>
    <unsigned long long>(unsigned long long, const Amount&) -> bool;

template auto operator>=
    <int>(int, const Amount&) -> bool;
template auto operator>=
    <long>(long, const Amount&) -> bool;
template auto operator>=
    <long long>(long long, const Amount&) -> bool;
template auto operator>=
    <unsigned>(unsigned, const Amount&) -> bool;
template auto operator>=
    <unsigned long>(unsigned long, const Amount&) -> bool;
template auto operator>=
    <unsigned long long>(unsigned long long, const Amount&) -> bool;

template auto operator*
    <int>(int, const Amount&) -> Amount;
template auto operator*
    <long>(long, const Amount&) -> Amount;
template auto operator*
    <long long>(long long, const Amount&) -> Amount;
template auto operator*
    <unsigned>(unsigned, const Amount&) -> Amount;
template auto operator*
    <unsigned long>(unsigned long, const Amount&) -> Amount;
template auto operator*
    <unsigned long long>(unsigned long long, const Amount&) -> Amount;

template auto opentxs::Amount::operator<
    <int>(int) const noexcept -> bool;
template auto opentxs::Amount::operator<
    <long>(long) const noexcept -> bool;
template auto opentxs::Amount::operator<
    <long long>(long long) const noexcept -> bool;
template auto opentxs::Amount::operator<
    <unsigned>( unsigned) const noexcept -> bool;
template auto opentxs::Amount::operator<
    <unsigned long>( unsigned long) const noexcept -> bool;
template auto opentxs::Amount::operator<
    <unsigned long long>( unsigned long long) const noexcept -> bool;

template auto opentxs::Amount::operator>
    <int>(int) const noexcept -> bool;
template auto opentxs::Amount::operator>
    <long>(long) const noexcept -> bool;
template auto opentxs::Amount::operator>
    <long long>(long long) const noexcept -> bool;
template auto opentxs::Amount::operator>
    <unsigned>(unsigned) const noexcept -> bool;
template auto opentxs::Amount::operator>
    <unsigned long>(unsigned long) const noexcept -> bool;
template auto opentxs::Amount::operator>
    <unsigned long long>(unsigned long long) const noexcept -> bool;

template auto opentxs::Amount::operator==
    <int>(int) const noexcept -> bool;
template auto opentxs::Amount::operator==
    <long>(long) const noexcept -> bool;
template auto opentxs::Amount::operator==
    <long long>(long long) const noexcept -> bool;
template auto opentxs::Amount::operator==
    <unsigned>(unsigned) const noexcept -> bool;
template auto opentxs::Amount::operator==
    <unsigned long>(unsigned long) const noexcept -> bool;
template auto opentxs::Amount::operator==
    <unsigned long long>(unsigned long long) const noexcept -> bool;

template auto opentxs::Amount::operator!=
    <int>(int) const noexcept -> bool;
template auto opentxs::Amount::operator!=
    <long>(long) const noexcept -> bool;
template auto opentxs::Amount::operator!=
    <long long>(long long) const noexcept -> bool;
template auto opentxs::Amount::operator!=
    <unsigned>(unsigned) const noexcept -> bool;
template auto opentxs::Amount::operator!=
    <unsigned long>(unsigned long) const noexcept -> bool;
template auto opentxs::Amount::operator!=
    <unsigned long long>(unsigned long long) const noexcept -> bool;

template auto opentxs::Amount::operator<=
    <int>(int) const noexcept -> bool;
template auto opentxs::Amount::operator<=
    <long> (long) const noexcept -> bool;
template auto opentxs::Amount::operator<=
    <long long>(long long) const noexcept -> bool;
template auto opentxs::Amount::operator<=
    <unsigned>(unsigned) const noexcept -> bool;
template auto opentxs::Amount::operator<=
    <unsigned long>(unsigned long) const noexcept -> bool;
template auto opentxs::Amount::operator<=
    <unsigned long long>(unsigned long long) const noexcept -> bool;

template auto opentxs::Amount::operator>=
    <int>(int) const noexcept -> bool;
template auto opentxs::Amount::operator>=
    <long>(long) const noexcept -> bool;
template auto opentxs::Amount::operator>=
    <long long>(long long) const noexcept -> bool;
template auto opentxs::Amount::operator>=
    <unsigned>(unsigned) const noexcept -> bool;
template auto opentxs::Amount::operator>=
    <unsigned long>(unsigned long) const noexcept -> bool;
template auto opentxs::Amount::operator>=
    <unsigned long long>(unsigned long long) const noexcept -> bool;

template auto Amount::operator*
    <int>(int) const -> Amount;
template auto Amount::operator*
    <long>(long) const -> Amount;
template auto Amount::operator*
    <long long>(long long) const -> Amount;
template auto Amount::operator*
    <unsigned>(unsigned) const -> Amount;
template auto Amount::operator*
    <unsigned long>(unsigned long) const -> Amount;
template auto Amount::operator*
    <unsigned long long>(unsigned long long) const -> Amount;

template auto Amount::operator/
    <int>(int) const -> Amount;
template auto Amount::operator/
    <long>(long) const -> Amount;
template auto Amount::operator/
    <long long>(long long) const -> Amount;
template auto Amount::operator/
    <unsigned>(unsigned) const -> Amount;
template auto Amount::operator/
    <unsigned long>(unsigned long) const -> Amount;
template auto Amount::operator/
    <unsigned long long>(unsigned long long) const -> Amount;

template auto Amount::operator%
    <int>(int) const -> Amount;
template auto Amount::operator%
    <long>(long) const -> Amount;
template auto Amount::operator%
    <long long>(long long) const -> Amount;
template auto Amount::operator%
    <unsigned>(unsigned) const -> Amount;
template auto Amount::operator%
    <unsigned long>(unsigned long) const -> Amount;
template auto Amount::operator%
    <unsigned long long>(unsigned long long) const -> Amount;
// clang-format on
}  // namespace opentxs
