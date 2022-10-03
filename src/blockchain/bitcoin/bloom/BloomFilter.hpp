// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/dynamic_bitset.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>

#include "opentxs/blockchain/bitcoin/bloom/BloomFilter.hpp"
#include "opentxs/blockchain/bitcoin/bloom/Types.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace be = boost::endian;

namespace opentxs::blockchain::implementation
{
class BloomFilter final : virtual public blockchain::BloomFilter
{
public:
    using FalsePositiveRate = double;
    using Filter = boost::dynamic_bitset<>;
    using Tweak = std::uint32_t;

    auto Serialize(AllocateOutput) const noexcept -> bool final;
    auto Test(const Data& element) const noexcept -> bool final;

    auto AddElement(const Data& element) noexcept -> void final;

    BloomFilter(
        const api::Session& api,
        const Tweak tweak,
        const BloomUpdateFlag update,
        const std::size_t functionCount,
        const Filter& data) noexcept;
    BloomFilter(
        const api::Session& api,
        const Tweak tweak,
        const BloomUpdateFlag update,
        const std::size_t targets,
        const FalsePositiveRate rate) noexcept;
    BloomFilter(
        const api::Session& api,
        const Tweak tweak,
        const BloomUpdateFlag update,
        const std::size_t functionCount,
        const Data& data) noexcept;
    BloomFilter() = delete;
    BloomFilter(const BloomFilter& rhs) noexcept;
    BloomFilter(BloomFilter&&) = delete;
    auto operator=(const BloomFilter&) -> BloomFilter& = delete;
    auto operator=(BloomFilter&&) -> BloomFilter& = delete;

    ~BloomFilter() final = default;

private:
    static const std::size_t max_filter_bytes_;
    static const std::size_t max_hash_function_count_;
    static const std::uint32_t seed_;

    const api::Session& api_;
    Tweak tweak_{};
    BloomUpdateFlag flags_{};
    std::size_t function_count_{};
    Filter filter_;

    auto clone() const noexcept -> BloomFilter* final
    {
        return new BloomFilter(*this);
    }
    auto hash(const Data& input, std::size_t hash_index) const noexcept
        -> std::uint32_t;
};
}  // namespace opentxs::blockchain::implementation
