// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <future>
#include <memory>

#include "ottest/Basic.hpp"

namespace ottest
{
class OPENTXS_EXPORT MinedBlocks
{
public:
    using BlockHash = ot::blockchain::block::Hash;
    using Promise = std::promise<BlockHash>;
    using Future = std::shared_future<BlockHash>;

    auto get(const std::size_t index) const -> Future;

    auto allocate() noexcept -> Promise;

    MinedBlocks();

    ~MinedBlocks();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace ottest
