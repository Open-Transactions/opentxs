// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "blockchain/bitcoin/cfilter/GCSPrivate.hpp"
#include "internal/blockchain/bitcoin/cfilter/GCS.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class GCS;
}  // namespace proto

class ByteArray;
class Data;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::implementation
{
class GCS final : public blockchain::GCSPrivate
{
public:
    auto clone(allocator_type alloc) const noexcept -> GCS* final
    {
        return pmr::clone<GCS>(this, {alloc});
    }
    auto Compressed(Writer&& out) const noexcept -> bool final;
    auto ElementCount() const noexcept -> std::uint32_t final { return count_; }
    auto Encode(Writer&& out) const noexcept -> bool final;
    auto Hash() const noexcept -> cfilter::Hash final;
    auto Header(const cfilter::Header& previous) const noexcept
        -> cfilter::Header final;
    auto IsValid() const noexcept -> bool final { return true; }
    auto Match(
        const Targets& targets,
        allocator_type alloc,
        allocator_type monotonic) const noexcept -> Matches final;
    auto Match(const gcs::Hashes& prehashed, alloc::Default monotonic)
        const noexcept -> PrehashedMatches final;
    auto Range() const noexcept -> gcs::Range final;
    auto size() const noexcept -> std::size_t final
    {
        return compressed_.size();
    }
    auto Serialize(proto::GCS& out) const noexcept -> bool final;
    auto Serialize(Writer&& out) const noexcept -> bool final;
    auto Test(const Data& target, allocator_type monotonic) const noexcept
        -> bool final;
    auto Test(const ReadView target, allocator_type monotonic) const noexcept
        -> bool final;
    auto Test(const Vector<ByteArray>& targets, allocator_type monotonic)
        const noexcept -> bool final;
    auto Test(const Vector<Space>& targets, allocator_type monotonic)
        const noexcept -> bool final;
    auto Test(const gcs::Hashes& targets, alloc::Default monotonic)
        const noexcept -> bool final;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    GCS(const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        const ReadView key,
        const ReadView encoded,
        allocator_type alloc) noexcept(false);
    GCS(const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        const ReadView key,
        gcs::Elements&& hashed,
        Vector<std::byte>&& compressed,
        allocator_type alloc) noexcept(false);
    GCS(const GCS& rhs, allocator_type alloc = {}) noexcept;
    GCS() = delete;
    GCS(GCS&&) = delete;
    auto operator=(const GCS&) -> GCS& = delete;
    auto operator=(GCS&&) -> GCS& = delete;

    ~GCS() final = default;

private:
    using Key = std::array<std::byte, 16>;

    const VersionNumber version_;
    const api::Session& api_;
    const std::uint8_t bits_;
    const std::uint32_t false_positive_rate_;
    const std::uint32_t count_;
    const Key key_;
    const Vector<std::byte> compressed_;
    mutable std::optional<gcs::Elements> elements_;

    static auto transform(
        const Vector<ByteArray>& in,
        allocator_type alloc) noexcept -> Targets;
    static auto transform(
        const Vector<Space>& in,
        allocator_type alloc) noexcept -> Targets;

    auto decompress() const noexcept -> const gcs::Elements&;
    auto hashed_set_construct(
        const Vector<ByteArray>& elements,
        allocator_type alloc) const noexcept -> gcs::Elements;
    auto hashed_set_construct(
        const Vector<Space>& elements,
        allocator_type alloc) const noexcept -> gcs::Elements;
    auto hashed_set_construct(const gcs::Hashes& targets, allocator_type alloc)
        const noexcept -> gcs::Elements;
    auto hashed_set_construct(const Targets& elements, allocator_type alloc)
        const noexcept -> gcs::Elements;
    auto test(const gcs::Elements& targetHashes, allocator_type monotonic)
        const noexcept -> bool;
    auto hash_to_range(const ReadView in) const noexcept -> gcs::Range;

    GCS(const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        const ReadView key,
        Vector<std::byte>&& encoded,
        allocator_type alloc) noexcept(false);
    GCS(const VersionNumber version,
        const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        std::optional<gcs::Elements>&& elements,
        Vector<std::byte>&& compressed,
        ReadView key,
        allocator_type alloc) noexcept(false);
};
}  // namespace opentxs::blockchain::implementation
