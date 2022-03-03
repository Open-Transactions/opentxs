// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <tuple>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace node
{
namespace wallet
{
class Work;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
using Cookie = unsigned long long int;
using BlockMap = UnallocatedMap<Cookie, Work*>;
using Indices = UnallocatedVector<Bip32Index>;
using Result = std::pair<ReadView, Indices>;
using Results = UnallocatedVector<Result>;
using ProgressBatch = UnallocatedVector<
    std::pair<std::reference_wrapper<const block::Position>, std::size_t>>;
}  // namespace opentxs::blockchain::node::wallet
