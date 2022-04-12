// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocated.hpp"
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

namespace opentxs::blockchain
{
class GCS::Imp : virtual public Allocated, virtual public internal::GCS
{
public:
    using Targets = blockchain::GCS::Targets;
    using Matches = blockchain::GCS::Matches;

    const allocator_type alloc_;

    virtual auto clone(allocator_type alloc) const noexcept
        -> std::unique_ptr<Imp>
    {
        return std::make_unique<Imp>(alloc);
    }
    virtual auto Compressed(AllocateOutput out) const noexcept -> bool
    {
        return {};
    }
    virtual auto ElementCount() const noexcept -> std::uint32_t { return {}; }
    virtual auto Encode(AllocateOutput out) const noexcept -> bool
    {
        return {};
    }
    auto get_allocator() const noexcept -> allocator_type final
    {
        return alloc_;
    }
    virtual auto Hash() const noexcept -> cfilter::Hash { return {}; }
    virtual auto Header(const cfilter::Header& previous) const noexcept
        -> cfilter::Header
    {
        return {};
    }
    virtual auto IsValid() const noexcept -> bool { return false; }
    virtual auto Match(const Targets&, allocator_type) const noexcept -> Matches
    {
        return {};
    }
    auto Match(const gcs::Hashes& prehashed) const noexcept
        -> PrehashedMatches override
    {
        return {};
    }
    auto Range() const noexcept -> gcs::Range override { return {}; }
    auto Serialize(proto::GCS& out) const noexcept -> bool override
    {
        return {};
    }
    virtual auto Serialize(AllocateOutput out) const noexcept -> bool
    {
        return {};
    }
    virtual auto Test(const Data& target) const noexcept -> bool { return {}; }
    virtual auto Test(const ReadView target) const noexcept -> bool
    {
        return {};
    }
    virtual auto Test(const Vector<OTData>& targets) const noexcept -> bool
    {
        return {};
    }
    virtual auto Test(const Vector<Space>& targets) const noexcept -> bool
    {
        return {};
    }
    auto Test(const gcs::Hashes& targets) const noexcept -> bool override
    {
        return {};
    }

    Imp(allocator_type alloc) noexcept
        : alloc_(alloc)
    {
    }

    ~Imp() override = default;
};
}  // namespace opentxs::blockchain

namespace opentxs::blockchain::implementation
{
class GCS final : public blockchain::GCS::Imp
{
public:
    auto clone(allocator_type alloc) const noexcept
        -> std::unique_ptr<Imp> final
    {
        return std::make_unique<GCS>(*this, alloc);
    }
    auto Compressed(AllocateOutput out) const noexcept -> bool final;
    auto ElementCount() const noexcept -> std::uint32_t final { return count_; }
    auto Encode(AllocateOutput out) const noexcept -> bool final;
    auto Hash() const noexcept -> cfilter::Hash final;
    auto Header(const cfilter::Header& previous) const noexcept
        -> cfilter::Header final;
    auto IsValid() const noexcept -> bool final { return true; }
    auto Match(const Targets&, allocator_type) const noexcept -> Matches final;
    auto Match(const gcs::Hashes& prehashed) const noexcept
        -> PrehashedMatches final;
    auto Range() const noexcept -> gcs::Range final;
    auto Serialize(proto::GCS& out) const noexcept -> bool final;
    auto Serialize(AllocateOutput out) const noexcept -> bool final;
    auto Test(const Data& target) const noexcept -> bool final;
    auto Test(const ReadView target) const noexcept -> bool final;
    auto Test(const Vector<OTData>& targets) const noexcept -> bool final;
    auto Test(const Vector<Space>& targets) const noexcept -> bool final;
    auto Test(const gcs::Hashes& targets) const noexcept -> bool final;

    GCS(const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        const ReadView key,
        const ReadView encoded,
        allocator_type alloc)
    noexcept(false);
    GCS(const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        const ReadView key,
        gcs::Elements&& hashed,
        Vector<std::byte>&& compressed,
        allocator_type alloc)
    noexcept(false);
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
        const Vector<OTData>& in,
        allocator_type alloc) noexcept -> Targets;
    static auto transform(
        const Vector<Space>& in,
        allocator_type alloc) noexcept -> Targets;

    auto decompress() const noexcept -> const gcs::Elements&;
    auto hashed_set_construct(
        const Vector<OTData>& elements,
        allocator_type alloc) const noexcept -> gcs::Elements;
    auto hashed_set_construct(
        const Vector<Space>& elements,
        allocator_type alloc) const noexcept -> gcs::Elements;
    auto hashed_set_construct(const gcs::Hashes& targets, allocator_type alloc)
        const noexcept -> gcs::Elements;
    auto hashed_set_construct(const Targets& elements, allocator_type alloc)
        const noexcept -> gcs::Elements;
    auto test(const gcs::Elements& targetHashes) const noexcept -> bool;
    auto hash_to_range(const ReadView in) const noexcept -> gcs::Range;

    GCS(const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        const ReadView key,
        Vector<std::byte>&& encoded,
        allocator_type alloc)
    noexcept(false);
    GCS(const VersionNumber version,
        const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        std::optional<gcs::Elements>&& elements,
        Vector<std::byte>&& compressed,
        ReadView key,
        allocator_type alloc)
    noexcept(false);
};
}  // namespace opentxs::blockchain::implementation
