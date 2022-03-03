// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/GCS.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class GCS;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::implementation
{
class GCS final : virtual public blockchain::GCS
{
public:
    auto Compressed() const noexcept -> Space final;
    auto ElementCount() const noexcept -> std::uint32_t final { return count_; }
    auto Encode() const noexcept -> OTData final;
    auto Hash() const noexcept -> OTData final;
    auto Header(const ReadView previous) const noexcept -> OTData final;
    auto Match(const Targets&) const noexcept -> Matches final;
    auto Serialize(proto::GCS& out) const noexcept -> bool final;
    auto Serialize(AllocateOutput out) const noexcept -> bool final;
    auto Test(const Data& target) const noexcept -> bool final;
    auto Test(const ReadView target) const noexcept -> bool final;
    auto Test(const UnallocatedVector<OTData>& targets) const noexcept
        -> bool final;
    auto Test(const UnallocatedVector<Space>& targets) const noexcept
        -> bool final;

    GCS(const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t filterElementCount,
        const ReadView key,
        const ReadView encoded)
    noexcept(false);
    GCS(const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const ReadView key,
        const UnallocatedVector<ReadView>& elements)
    noexcept(false);

    ~GCS() final = default;

private:
    using Elements = UnallocatedVector<std::uint64_t>;

    const VersionNumber version_;
    const api::Session& api_;
    const std::uint8_t bits_;
    const std::uint32_t false_positive_rate_;
    const std::uint32_t count_;
    const std::optional<Elements> elements_;
    const OTData compressed_;
    const OTData key_;

    static auto transform(const UnallocatedVector<OTData>& in) noexcept
        -> UnallocatedVector<ReadView>;
    static auto transform(const UnallocatedVector<Space>& in) noexcept
        -> UnallocatedVector<ReadView>;

    auto decompress() const noexcept -> const Elements&;
    auto hashed_set_construct(const UnallocatedVector<OTData>& elements)
        const noexcept -> UnallocatedVector<std::uint64_t>;
    auto hashed_set_construct(const UnallocatedVector<Space>& elements)
        const noexcept -> UnallocatedVector<std::uint64_t>;
    auto hashed_set_construct(const UnallocatedVector<ReadView>& elements)
        const noexcept -> UnallocatedVector<std::uint64_t>;
    auto test(const UnallocatedVector<std::uint64_t>& targetHashes)
        const noexcept -> bool;
    auto hash_to_range(const ReadView in) const noexcept -> std::uint64_t;

    GCS() = delete;
    GCS(const GCS&) = delete;
    GCS(GCS&&) = delete;
    auto operator=(const GCS&) -> GCS& = delete;
    auto operator=(GCS&&) -> GCS& = delete;
};
}  // namespace opentxs::blockchain::implementation
