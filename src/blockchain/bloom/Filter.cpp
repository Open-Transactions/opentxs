// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bloom/Filter.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <stdexcept>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/bloom/Filter.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BloomFilter(
    const api::Session& api,
    const std::uint32_t tweak,
    const blockchain::bloom::UpdateFlag update,
    const std::size_t targets,
    const double fpRate) -> blockchain::bloom::Filter*
{
    using ReturnType = blockchain::bloom::implementation::Filter;

    return new ReturnType(api, tweak, update, targets, fpRate);
}

auto BloomFilter(const api::Session& api, const Data& serialized)
    -> blockchain::bloom::Filter*
{
    using ReturnType = blockchain::bloom::implementation::Filter;
    blockchain::internal::SerializedBloomFilter raw{};

    if (sizeof(raw) > serialized.size()) {
        LogError()()("Input too short").Flush();

        return nullptr;
    }

    const auto filterSize = serialized.size() - sizeof(raw);
    auto filter =
        (0 == filterSize)
            ? ByteArray{}
            : ByteArray{
                  static_cast<const std::byte*>(serialized.data()), filterSize};
    std::memcpy(
        reinterpret_cast<std::byte*>(&raw),
        static_cast<const std::byte*>(serialized.data()) + filterSize,
        sizeof(raw));

    return new ReturnType(
        api,
        raw.tweak_.value(),
        static_cast<blockchain::bloom::UpdateFlag>(raw.flags_.value()),
        raw.function_count_.value(),
        filter);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::bloom::implementation
{
const std::size_t Filter::max_filter_bytes_ = 36000;
const std::size_t Filter::max_hash_function_count_ = 50;
const std::uint32_t Filter::seed_{4221880213};  // 0xFBA4C795

Filter::Filter(
    const api::Session& api,
    const Tweak tweak,
    const UpdateFlag update,
    const std::size_t functionCount,
    const Raw& data) noexcept
    : bloom::Filter()
    , api_(api)
    , tweak_(tweak)
    , flags_(update)
    , function_count_(functionCount)
    , filter_(data)
{
}

Filter::Filter(
    const api::Session& api,
    const Tweak tweak,
    const UpdateFlag update,
    const std::size_t targets,
    const FalsePositiveRate rate) noexcept
    : Filter(api, tweak, update, 0, Raw(std::size_t(64)))
{
    const auto pre_calc_filter_size = static_cast<std::size_t>(
        static_cast<std::size_t>(
            (-1) / std::pow(std::log(2), 2) * std::log(rate)) *
        targets);

    const auto ideal_filter_bytesize = static_cast<std::size_t>(std::max(
        std::size_t(1),
        (std::min(pre_calc_filter_size, Filter::max_filter_bytes_ * 8) / 8)));

    filter_.resize(ideal_filter_bytesize * 8u);

    // Optimal number of hash functions for given filter size and element count
    const auto precalc_hash_function_count = static_cast<std::size_t>(
        (ideal_filter_bytesize * 8u) /
        static_cast<std::size_t>(static_cast<double>(targets) * std::log(2)));

    function_count_ = static_cast<std::size_t>(std::max(
        std::size_t(1),
        std::min(
            precalc_hash_function_count, Filter::max_hash_function_count_)));
}

Filter::Filter(
    const api::Session& api,
    const Tweak tweak,
    const UpdateFlag update,
    const std::size_t functionCount,
    const Data& data) noexcept
    : Filter(
          api,
          tweak,
          update,
          functionCount,
          Raw{static_cast<const std::uint8_t*>(data.data()),
              static_cast<const std::uint8_t*>(data.data()) + data.size()})
{
}

Filter::Filter(const Filter& rhs) noexcept
    : Filter(rhs.api_, rhs.tweak_, rhs.flags_, rhs.function_count_, rhs.filter_)
{
}

auto Filter::AddElement(const Data& in) noexcept -> void
{
    const auto bitsize = filter_.size();

    for (std::size_t i{0}; i < function_count_; ++i) {
        const auto bit_index = hash(in, i) % bitsize;
        filter_.set(bit_index);
    }
}

auto Filter::hash(const Data& input, std::size_t hash_index) const noexcept
    -> std::uint32_t
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
    assert_true(std::numeric_limits<std::uint32_t>::max() >= hash_index);
#pragma GCC diagnostic pop

    auto seed = seed_ * static_cast<std::uint32_t>(hash_index);
    seed += tweak_;
    auto hash = std::uint32_t{};
    api_.Crypto().Hash().MurmurHash3_32(seed, input, hash);

    return hash;
}

auto Filter::Serialize(Writer&& out) const noexcept -> bool
{
    try {
        static constexpr auto fixed =
            sizeof(blockchain::internal::SerializedBloomFilter);
        const auto filter = [&] {
            auto filters = UnallocatedVector<std::uint8_t>{};
            boost::to_block_range(filter_, std::back_inserter(filters));

            return filters;
        }();
        const auto bytes = fixed + filter.size();
        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        const auto data = blockchain::internal::SerializedBloomFilter{
            tweak_, flags_, function_count_};
        auto* i = output.as<std::byte>();
        std::memcpy(i, filter.data(), filter.size());
        std::advance(i, filter.size());
        std::memcpy(i, static_cast<const void*>(&data), fixed);
        std::advance(i, fixed);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto Filter::Test(const Data& in) const noexcept -> bool
{
    const auto bitsize = filter_.size();

    for (std::size_t i{0}; i < function_count_; ++i) {
        const auto bit_index = hash(in, i) % bitsize;

        if (!filter_.test(bit_index)) { return false; }
    }

    return true;
}
}  // namespace opentxs::blockchain::bloom::implementation
