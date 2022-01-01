// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <map>
#include <tuple>
#include <vector>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
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
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
using Cookie = unsigned long long int;
using BlockMap = std::pmr::map<Cookie, Work*>;
using Indices = std::pmr::vector<Bip32Index>;
using Result = std::pair<ReadView, Indices>;
using Results = std::pmr::vector<Result>;
using ProgressBatch = std::pmr::vector<
    std::pair<std::reference_wrapper<const block::Position>, std::size_t>>;
}  // namespace opentxs::blockchain::node::wallet
