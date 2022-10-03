// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_shared_guarded.h>
#include <shared_mutex>

#include "internal/network/otdht/Node.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Position;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
class Node::Shared final : public opentxs::implementation::Allocated
{
public:
    class Data final : opentxs::Allocated
    {
    public:
        Map<opentxs::blockchain::Type, opentxs::blockchain::block::Position>
            state_;

        auto get_allocator() const noexcept -> allocator_type final;

        Data(allocator_type alloc) noexcept;
        Data() = delete;
        Data(const Data&) = delete;
        Data(Data&&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;

        ~Data() final;
    };

    using Guarded = libguarded::shared_guarded<Data, std::shared_mutex>;

    const zeromq::BatchID batch_id_;
    mutable Guarded data_;

    static auto Chains() noexcept -> const Set<opentxs::blockchain::Type>&;

    Shared(zeromq::BatchID batchID, allocator_type alloc) noexcept;
    Shared() = delete;
    Shared(const Shared&) = delete;
    Shared(Shared&&) = delete;
    auto operator=(const Shared&) -> Shared& = delete;
    auto operator=(Shared&&) -> Shared& = delete;

    ~Shared() final;
};
}  // namespace opentxs::network::otdht
