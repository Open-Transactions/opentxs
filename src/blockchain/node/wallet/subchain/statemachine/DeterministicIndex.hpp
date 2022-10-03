// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <optional>

#include "blockchain/node/wallet/subchain/statemachine/Index.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Index.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class Deterministic;
}  // namespace crypto

namespace node
{
namespace wallet
{
class DeterministicStateData;
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class DeterministicIndex final : public Index::Imp
{
public:
    DeterministicIndex(
        const boost::shared_ptr<const SubchainStateData>& parent,
        const DeterministicStateData& deterministic,
        const network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    DeterministicIndex() = delete;
    DeterministicIndex(const Imp&) = delete;
    DeterministicIndex(DeterministicIndex&&) = delete;
    auto operator=(const DeterministicIndex&) -> DeterministicIndex& = delete;
    auto operator=(DeterministicIndex&&) -> DeterministicIndex& = delete;

    ~DeterministicIndex() final = default;

private:
    const crypto::Deterministic& subaccount_;

    auto need_index(const std::optional<Bip32Index>& current) const noexcept
        -> std::optional<Bip32Index> final;

    auto process(
        const std::optional<Bip32Index>& current,
        Bip32Index target) noexcept -> void final;
};
}  // namespace opentxs::blockchain::node::wallet
