// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <shared_mutex>
#include <string_view>

#include "internal/network/otdht/Node.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "util/Allocated.hpp"

namespace opentxs::network::otdht
{
using namespace std::literals;

class Node::Shared final : public opentxs::implementation::Allocated
{
public:
    class Data final : opentxs::Allocated
    {
    public:
        Map<opentxs::blockchain::Type, opentxs::blockchain::block::Position>
            state_;

        auto get_allocator() const noexcept -> allocator_type final;

        auto get_deleter() noexcept -> delete_function final
        {
            return pmr::make_deleter(this);
        }

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
    const Secret private_key_;
    const FixedByteArray<32_uz> public_key_;
    mutable Guarded data_;

    static auto Chains() noexcept -> const Set<opentxs::blockchain::Type>&;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Shared(
        zeromq::BatchID batchID,
        const ReadView publicKey,
        const Secret& secretKey,
        allocator_type alloc) noexcept;
    Shared() = delete;
    Shared(const Shared&) = delete;
    Shared(Shared&&) = delete;
    auto operator=(const Shared&) -> Shared& = delete;
    auto operator=(Shared&&) -> Shared& = delete;

    ~Shared() final;
};
}  // namespace opentxs::network::otdht
