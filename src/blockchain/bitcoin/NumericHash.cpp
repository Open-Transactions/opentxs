// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/bitcoin/NumericHash.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/NumericHash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace be = boost::endian;

namespace opentxs::factory
{
auto NumericHashNBits(const std::uint32_t input) noexcept
    -> std::unique_ptr<blockchain::NumericHash>
{
    using ReturnType = blockchain::implementation::NumericHash;
    using ArgumentType = ReturnType::Type;
    using MantissaType = bmp::checked_cpp_int;

    const auto mantissa = std::uint32_t{input & 0x007fffff};
    const auto exponent = static_cast<std::uint8_t>((input & 0xff000000) >> 24);
    auto target = ArgumentType{};

    try {
        if (exponent > 3) {
            target = MantissaType{mantissa} << (8 * (exponent - 3));
        } else {
            target = MantissaType{mantissa} << (8 * (3 - exponent));
        }
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to calculate target")
            .Flush();

        return std::make_unique<ReturnType>();
    }

    return std::make_unique<ReturnType>(target);
}

auto NumericHash(const Data& hash) noexcept
    -> std::unique_ptr<blockchain::NumericHash>
{
    using ReturnType = blockchain::implementation::NumericHash;
    ReturnType::Type value{};

    if (hash.IsNull()) { return std::make_unique<ReturnType>(); }

    try {
        // Interpret hash as little endian
        bmp::import_bits(value, hash.begin(), hash.end(), 8, false);
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(": Failed to decode hash")
            .Flush();

        return std::make_unique<ReturnType>();
    }

    return std::make_unique<ReturnType>(value);
}
}  // namespace opentxs::factory

namespace opentxs
{
auto operator==(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept -> bool
{
    return lhs.get() == rhs;
}

auto operator!=(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept -> bool
{
    return lhs.get() != rhs;
}

auto operator<(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept -> bool
{
    return lhs.get() < rhs;
}

auto operator<=(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept -> bool
{
    return lhs.get() <= rhs;
}
}  // namespace opentxs

namespace opentxs::blockchain
{
auto HashToNumber(const api::Session& api, ReadView hex) noexcept
    -> UnallocatedCString
{
    return HashToNumber(api.Factory().DataFromHex(hex));
}

auto HashToNumber(const Hash& hash) noexcept -> UnallocatedCString
{
    const auto number = factory::NumericHash(hash);

    return number->asHex();
}

auto NumericHash::MaxTarget(const blockchain::Type chain) noexcept
    -> std::int32_t
{
    try {

        return params::Chains().at(chain).nBits_;
    } catch (...) {

        return {};
    }
}

auto NumberToHash(const api::Session& api, ReadView hex) noexcept -> pHash
{
    const auto hash = api.Factory().DataFromHex(hex);
    auto out = api.Factory().Data();

    for (auto i{hash->size()}; i > 0u; --i) {
        out += std::to_integer<std::uint8_t>(hash->at(i - 1u));
    }

    return out;
}
}  // namespace opentxs::blockchain

namespace opentxs::blockchain::implementation
{
NumericHash::NumericHash(const Type& data) noexcept
    : blockchain::NumericHash()
    , data_(data)
{
}

NumericHash::NumericHash() noexcept
    : NumericHash(Type{})
{
}

NumericHash::NumericHash(const NumericHash& rhs) noexcept
    : NumericHash(rhs.data_)
{
}

auto NumericHash::operator==(const blockchain::NumericHash& rhs) const noexcept
    -> bool
{
    const auto& input = dynamic_cast<const NumericHash&>(rhs);

    return data_ == input.data_;
}

auto NumericHash::operator!=(const blockchain::NumericHash& rhs) const noexcept
    -> bool
{
    const auto& input = dynamic_cast<const NumericHash&>(rhs);

    return data_ != input.data_;
}

auto NumericHash::operator<(const blockchain::NumericHash& rhs) const noexcept
    -> bool
{
    const auto& input = dynamic_cast<const NumericHash&>(rhs);

    return data_ < input.data_;
}

auto NumericHash::operator<=(const blockchain::NumericHash& rhs) const noexcept
    -> bool
{
    const auto& input = dynamic_cast<const NumericHash&>(rhs);

    return data_ <= input.data_;
}

auto NumericHash::asHex(const std::size_t minimumBytes) const noexcept
    -> UnallocatedCString
{
    UnallocatedVector<unsigned char> bytes;

    try {
        // Export as big endian
        bmp::export_bits(data_, std::back_inserter(bytes), 8, true);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to encode number").Flush();

        return {};
    }

    while (minimumBytes > bytes.size()) { bytes.insert(bytes.begin(), 0x0); }

    return opentxs::Data::Factory(bytes.data(), bytes.size())->asHex();
}
}  // namespace opentxs::blockchain::implementation
