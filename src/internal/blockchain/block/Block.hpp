// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::internal
{
struct Block : virtual public block::Block {
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto ExtractElements(
        const cfilter::Type style,
        alloc::Default alloc) const noexcept -> Elements = 0;
    virtual auto FindMatches(
        const api::Session& api,
        const cfilter::Type type,
        const Patterns& txos,
        const Patterns& elements,
        const Log& log,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> Matches = 0;
    auto Internal() const noexcept -> const internal::Block& final
    {
        return *this;
    }

    auto Internal() noexcept -> internal::Block& final { return *this; }

    ~Block() override = default;
};
}  // namespace opentxs::blockchain::block::internal
