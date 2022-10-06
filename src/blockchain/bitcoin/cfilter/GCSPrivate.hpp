// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "internal/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class GCS;
}  // namespace proto

class ByteArray;
class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain
{
class GCSPrivate : virtual public Allocated, virtual public internal::GCS
{
public:
    using Targets = blockchain::GCS::Targets;
    using Matches = blockchain::GCS::Matches;

    const allocator_type alloc_;

    virtual auto clone(allocator_type alloc) const noexcept
        -> std::unique_ptr<GCSPrivate>
    {
        return std::make_unique<GCSPrivate>(alloc);
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
    virtual auto Match(const Targets&, allocator_type, allocator_type)
        const noexcept -> Matches
    {
        return {};
    }
    auto Match(const gcs::Hashes& prehashed, alloc::Default) const noexcept
        -> PrehashedMatches override
    {
        return {};
    }
    auto Range() const noexcept -> gcs::Range override { return {}; }
    auto Serialize(proto::GCS&) const noexcept -> bool override { return {}; }
    virtual auto Serialize(AllocateOutput) const noexcept -> bool { return {}; }
    virtual auto Test(const Data&, allocator_type) const noexcept -> bool
    {
        return {};
    }
    virtual auto Test(const ReadView, allocator_type) const noexcept -> bool
    {
        return {};
    }
    virtual auto Test(const Vector<ByteArray>&, allocator_type) const noexcept
        -> bool
    {
        return {};
    }
    virtual auto Test(const Vector<Space>&, allocator_type) const noexcept
        -> bool
    {
        return {};
    }
    auto Test(const gcs::Hashes&, alloc::Default) const noexcept
        -> bool override
    {
        return {};
    }

    GCSPrivate(allocator_type alloc) noexcept
        : alloc_(alloc)
    {
    }

    ~GCSPrivate() override = default;
};
}  // namespace opentxs::blockchain
