// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace proto
{
class GCS;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
class OPENTXS_EXPORT GCS
{
public:
    using Targets = std::pmr::vector<ReadView>;
    using Matches = std::pmr::vector<Targets::const_iterator>;

    /// Serialized filter only, no element count
    virtual auto Compressed() const noexcept -> Space = 0;
    virtual auto ElementCount() const noexcept -> std::uint32_t = 0;
    /// Element count as CompactSize followed by serialized filter
    virtual auto Encode() const noexcept -> OTData = 0;
    virtual auto Hash() const noexcept -> filter::pHash = 0;
    virtual auto Header(const ReadView previous) const noexcept
        -> filter::pHeader = 0;
    virtual auto Match(const Targets&) const noexcept -> Matches = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(proto::GCS& out) const noexcept
        -> bool = 0;
    virtual auto Serialize(AllocateOutput out) const noexcept -> bool = 0;
    virtual auto Test(const Data& target) const noexcept -> bool = 0;
    virtual auto Test(const ReadView target) const noexcept -> bool = 0;
    virtual auto Test(const std::pmr::vector<OTData>& targets) const noexcept
        -> bool = 0;
    virtual auto Test(const std::pmr::vector<Space>& targets) const noexcept
        -> bool = 0;

    virtual ~GCS() = default;
};

}  // namespace blockchain
}  // namespace opentxs
