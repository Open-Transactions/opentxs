// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/dynamic_bitset.hpp>
#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>

#include "internal/blockchain/bloom/Filter.hpp"
#include "internal/blockchain/bloom/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Data;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace be = boost::endian;

namespace opentxs::blockchain::bloom::implementation
{
class Filter final : virtual public blockchain::bloom::Filter
{
public:
    using FalsePositiveRate = double;
    using Raw = boost::dynamic_bitset<>;
    using Tweak = std::uint32_t;

    auto Serialize(Writer&&) const noexcept -> bool final;
    auto Test(const Data& element) const noexcept -> bool final;

    auto AddElement(const Data& element) noexcept -> void final;

    Filter(
        const api::Session& api,
        const Tweak tweak,
        const UpdateFlag update,
        const std::size_t functionCount,
        const Raw& data) noexcept;
    Filter(
        const api::Session& api,
        const Tweak tweak,
        const UpdateFlag update,
        const std::size_t targets,
        const FalsePositiveRate rate) noexcept;
    Filter(
        const api::Session& api,
        const Tweak tweak,
        const UpdateFlag update,
        const std::size_t functionCount,
        const Data& data) noexcept;
    Filter() = delete;
    Filter(const Filter& rhs) noexcept;
    Filter(Filter&&) = delete;
    auto operator=(const Filter&) -> Filter& = delete;
    auto operator=(Filter&&) -> Filter& = delete;

    ~Filter() final = default;

private:
    static const std::size_t max_filter_bytes_;
    static const std::size_t max_hash_function_count_;
    static const std::uint32_t seed_;

    const api::Session& api_;
    Tweak tweak_{};
    UpdateFlag flags_{};
    std::size_t function_count_{};
    Raw filter_;

    auto clone() const noexcept -> Filter* final { return new Filter(*this); }
    auto hash(const Data& input, std::size_t hash_index) const noexcept
        -> std::uint32_t;
};
}  // namespace opentxs::blockchain::bloom::implementation
