// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace cfilter
{
class Hash;
class Header;
}  // namespace cfilter

namespace internal
{
class GCS;
}  // namespace internal

class GCSPrivate;
}  // namespace blockchain

class ByteArray;
class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain
{
class OPENTXS_EXPORT GCS : virtual public Allocated
{
public:
    using Targets = Vector<ReadView>;
    using Matches = Vector<Targets::const_iterator>;

    /// Serialized filter only, no element count
    auto Compressed(AllocateOutput out) const noexcept -> bool;
    auto ElementCount() const noexcept -> std::uint32_t;
    /// Element count as CompactSize followed by serialized filter
    auto Encode(AllocateOutput out) const noexcept -> bool;
    auto get_allocator() const noexcept -> allocator_type override;
    auto Hash() const noexcept -> cfilter::Hash;
    auto Header(const cfilter::Header& previous) const noexcept
        -> cfilter::Header;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::GCS&;
    auto IsValid() const noexcept -> bool;
    auto Match(const Targets& targets, allocator_type alloc = {}) const noexcept
        -> Matches;
    auto Serialize(AllocateOutput out) const noexcept -> bool;
    auto Test(const Data& target) const noexcept -> bool;
    auto Test(const ReadView target) const noexcept -> bool;
    auto Test(const Vector<ByteArray>& targets) const noexcept -> bool;
    auto Test(const Vector<Space>& targets) const noexcept -> bool;

    auto swap(GCS& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT GCS(GCSPrivate* imp) noexcept;
    GCS(allocator_type alloc = {}) noexcept;
    GCS(const GCS& rhs, allocator_type alloc = {}) noexcept;
    GCS(GCS&& rhs) noexcept;
    GCS(GCS&& rhs, allocator_type alloc) noexcept;
    auto operator=(const GCS& rhs) noexcept -> GCS&;
    auto operator=(GCS&& rhs) noexcept -> GCS&;

    ~GCS() override;

private:
    GCSPrivate* imp_;
};
}  // namespace opentxs::blockchain
