// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class Mempool
{
public:
    virtual auto Dump() const noexcept
        -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto Query(ReadView txid) const noexcept
        -> std::shared_ptr<const bitcoin::block::Transaction> = 0;
    virtual auto Submit(ReadView txid) const noexcept -> bool = 0;
    virtual auto Submit(const UnallocatedVector<ReadView>& txids) const noexcept
        -> UnallocatedVector<bool> = 0;
    virtual auto Submit(std::unique_ptr<const bitcoin::block::Transaction> tx)
        const noexcept -> void = 0;

    virtual auto Heartbeat() noexcept -> void = 0;

    virtual ~Mempool() = default;
};
}  // namespace opentxs::blockchain::node::internal
