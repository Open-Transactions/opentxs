// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/otdht/node/Shared.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <utility>

#include "opentxs/blockchain/Type.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::network::otdht
{
Node::Shared::Shared(
    zeromq::BatchID batchID,
    const ReadView publicKey,
    const Secret& secretKey,
    allocator_type alloc) noexcept
    : Allocated(alloc)
    , batch_id_(std::move(batchID))
    , private_key_(secretKey)
    , public_key_(publicKey)
    , data_(alloc)
{
}

auto Node::Shared::Chains() noexcept -> const Set<opentxs::blockchain::Type>&
{
    static const auto chains = [] {
        const auto& set = opentxs::blockchain::supported_chains();
        auto out = Set<opentxs::blockchain::Type>{};
        std::ranges::copy(set, std::inserter(out, out.end()));
        out.emplace(opentxs::blockchain::Type::UnitTest);

        return out;
    }();

    return chains;
}

Node::Shared::~Shared() = default;
}  // namespace opentxs::network::otdht
