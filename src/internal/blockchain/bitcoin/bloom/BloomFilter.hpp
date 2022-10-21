// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/util/Pimpl.hpp"
#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
class BloomFilter;
}  // namespace blockchain

class Data;
class Writer;

using OTBloomFilter = Pimpl<blockchain::BloomFilter>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain
{
class OPENTXS_EXPORT BloomFilter
{
public:
    virtual auto Serialize(Writer&&) const noexcept -> bool = 0;
    virtual auto Test(const Data& element) const noexcept -> bool = 0;

    virtual auto AddElement(const Data& element) noexcept -> void = 0;

    BloomFilter(const BloomFilter& rhs) = delete;
    BloomFilter(BloomFilter&& rhs) = delete;
    auto operator=(const BloomFilter& rhs) -> BloomFilter& = delete;
    auto operator=(BloomFilter&& rhs) -> BloomFilter& = delete;

    virtual ~BloomFilter() = default;

protected:
    BloomFilter() noexcept = default;

private:
    friend OTBloomFilter;

    virtual auto clone() const noexcept -> BloomFilter* = 0;
};
}  // namespace opentxs::blockchain
