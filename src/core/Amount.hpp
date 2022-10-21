// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/cstdint.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <exception>
#include <limits>
#include <memory>
#include <string_view>
#include <utility>

#include "internal/core/Amount.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace be = boost::endian;

namespace opentxs
{
class Amount::Imp final : virtual public internal::Amount
{
public:
    static auto factory_signed(long long rhs) noexcept -> Imp*
    {
        return std::make_unique<Imp>(rhs).release();
    }
    static auto factory_unsigned(unsigned long long rhs) noexcept -> Imp*
    {
        return std::make_unique<Imp>(rhs).release();
    }

    auto ExtractInt64() const noexcept(false) -> std::int64_t final
    {
        return extract_int<std::int64_t>();
    }
    auto ExtractUInt64() const noexcept(false) -> std::uint64_t final
    {
        return extract_int<std::uint64_t>();
    }
    auto operator<(const Imp& rhs) const { return amount_ < rhs.amount_; }

    template <typename T>
    auto operator<(const T rhs) const
    {
        return amount_ < shift_left(rhs);
    }
    auto operator>(const Imp& rhs) const { return amount_ > rhs.amount_; }

    template <typename T>
    auto operator>(const T rhs) const
    {
        return amount_ > shift_left(rhs);
    }

    auto operator==(const Imp& rhs) const { return amount_ == rhs.amount_; }

    template <typename T>
    auto operator==(const T rhs) const
    {
        return amount_ == shift_left(rhs);
    }

    auto operator!=(const Imp& rhs) const { return amount_ != rhs.amount_; }

    template <typename T>
    auto operator!=(const T rhs) const
    {
        return amount_ != shift_left(rhs);
    }

    auto operator<=(const Imp& rhs) const { return amount_ <= rhs.amount_; }

    template <typename T>
    auto operator<=(const T rhs) const
    {
        return amount_ <= shift_left(rhs);
    }

    auto operator>=(const Imp& rhs) const { return amount_ >= rhs.amount_; }

    template <typename T>
    auto operator>=(const T rhs) const
    {
        return amount_ >= shift_left(rhs);
    }

    auto operator+(const Imp& rhs) const noexcept(false) -> Imp
    {
        return amount_ + rhs.amount_;
    }

    auto operator-(const Imp& rhs) const noexcept(false) -> Imp
    {
        return amount_ - rhs.amount_;
    }

    auto operator*(const Imp& rhs) const noexcept(false) -> Imp
    {
        const auto total = amount_ * rhs.amount_;
        auto imp = Imp{total};
        return imp.shift_right();
    }

    template <typename T>
    auto operator*(const T rhs) const noexcept(false) -> Imp
    {
        return amount_ * rhs;
    }

    auto operator/(const Imp& rhs) const noexcept(false) -> Imp
    {
        const auto total = amount_ / rhs.amount_;
        return Imp::shift_left(total);
    }

    template <typename T>
    auto operator/(const T rhs) const noexcept(false) -> Imp
    {
        return amount_ / rhs;
    }

    auto operator%(const Imp& rhs) const noexcept(false) -> Imp
    {
        return amount_ % rhs.amount_;
    }

    template <typename T>
    auto operator%(const T rhs) const noexcept(false) -> Imp
    {
        return amount_ % shift_left(rhs);
    }

    auto operator*=(const Imp& amount) noexcept(false) -> Imp&
    {
        auto total = amount_ * amount.amount_;
        amount_ = Imp(total).shift_right();
        return *this;
    }

    auto operator+=(const Imp& amount) noexcept(false) -> Imp&
    {
        amount_ += amount.amount_;
        return *this;
    }

    auto operator-=(const Imp& amount) noexcept(false) -> Imp&
    {
        amount_ -= amount.amount_;
        return *this;
    }

    auto operator-() -> Imp { return -amount_; }

    auto Serialize(Writer&& dest) const noexcept -> bool
    {
        auto amount = UnallocatedCString{};

        try {
            amount = amount_.str();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())("Error serializing amount: ")(
                e.what())
                .Flush();
            return false;
        }

        return copy(amount, std::move(dest));
    }

    auto SerializeBitcoin(Writer&& dest) const noexcept -> bool final
    {
        const auto backend = shift_right();
        if (backend < 0 || backend > std::numeric_limits<std::int64_t>::max()) {
            return false;
        }

        auto amount = std::int64_t{};
        try {
            amount = backend.convert_to<std::int64_t>();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())("Error serializing bitcoin amount: ")(
                e.what())
                .Flush();
            return false;
        }
        const auto buffer = be::little_int64_buf_t(amount);

        const auto view =
            ReadView(reinterpret_cast<const char*>(&buffer), sizeof(buffer));

        copy(view, std::move(dest));

        return true;
    }

    auto ToFloat() const noexcept -> amount::Float final
    {
        return amount::IntegerToFloat(amount_);
    }

    template <typename T>
    auto extract_int() const noexcept -> T
    {
        try {
            return shift_right().convert_to<T>();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())("Error converting Amount to int: ")(
                e.what())
                .Flush();
            return {};
        }
    }

    template <typename T>
    auto extract_float() const noexcept -> T
    {
        try {
            return amount_.convert_to<T>() / shift_left(1).convert_to<T>();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())("Error converting Amount to float: ")(
                e.what())
                .Flush();
            return {};
        }
    }

    template <typename T>
    static auto shift_left(const T& amount) -> amount::Integer
    {
        if (amount < 0) {
            auto tmp = -amount::Integer{amount};
            tmp <<= amount::fractional_bits_;
            return -tmp;
        } else {
            return amount::Integer{amount} << amount::fractional_bits_;
        }
    }

    auto shift_right() const -> amount::Integer
    {
        if (amount_ < 0) {
            auto tmp = -amount_;
            tmp >>= amount::fractional_bits_;
            return -tmp;
        } else {
            return amount_ >> amount::fractional_bits_;
        }
    }

    Imp() noexcept
        : amount_{}
    {
    }
    Imp(const amount::Integer& rhs) noexcept
        : amount_{rhs}
    {
    }
    Imp(amount::Integer&& rhs) noexcept
        : amount_{std::move(rhs)}
    {
    }
    Imp(long long amount) noexcept
        : amount_{shift_left(amount)}
    {
    }

    Imp(unsigned long long amount) noexcept
        : amount_{shift_left(amount)}
    {
    }
    Imp(std::string_view str, bool normalize = false) noexcept(false)
        : amount_{amount::Integer{str}}
    {
        if (normalize) {
            if (amount_ < 0) {
                amount_ = -(-amount_ << amount::fractional_bits_);
            } else {
                amount_ <<= amount::fractional_bits_;
            }
        }
    }
    Imp(const Imp&) noexcept = default;
    Imp(Imp&& rhs) noexcept = delete;
    auto operator=(const Imp& imp) -> Imp&
    {
        amount_ = imp.amount_;
        return *this;
    }
    auto operator=(Imp&& rhs) -> Imp& = delete;

    ~Imp() final = default;

private:
    amount::Integer amount_;
};
}  // namespace opentxs
