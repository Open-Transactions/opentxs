// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/DeterministicIndex.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <utility>

#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"
#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
auto Index::DeterministicFactory(
    const boost::shared_ptr<const SubchainStateData>& parent,
    const DeterministicStateData& deterministic) noexcept -> Index
{
    const auto& asio = parent->api_.Network().ZeroMQ().Internal();
    const auto batchID = asio.PreallocateBatch();
    // TODO the version of libc++ present in android ndk 23.0.7599858
    // has a broken std::allocate_shared function so we're using
    // boost::shared_ptr instead of std::shared_ptr

    return Index{boost::allocate_shared<DeterministicIndex>(
        alloc::PMR<DeterministicIndex>{asio.Alloc(batchID)},
        parent,
        deterministic,
        batchID)};
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
DeterministicIndex::DeterministicIndex(
    const boost::shared_ptr<const SubchainStateData>& parent,
    const DeterministicStateData& deterministic,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Imp(parent, batch, alloc)
    , subaccount_(deterministic.deterministic_)
{
}

auto DeterministicIndex::need_index(const std::optional<Bip32Index>& current)
    const noexcept -> std::optional<Bip32Index>
{
    const auto generated = subaccount_.LastGenerated(parent_.subchain_);

    if (generated.has_value()) {
        const auto target = generated.value();

        if ((false == current.has_value()) || (current.value() != target)) {
            const auto actual = current.has_value() ? current.value() + 1u : 0u;
            log_(OT_PRETTY_CLASS())(name_)(" has ")(target + 1)(
                " keys generated, but only ")(actual)(" have been indexed.")
                .Flush();

            return target;
        } else {
            log_(OT_PRETTY_CLASS())(name_)(" all ")(target + 1)(
                " generated keys have been indexed.")
                .Flush();
        }
    } else {
        log_(OT_PRETTY_CLASS())(name_)(" no generated keys present").Flush();
    }

    return std::nullopt;
}

auto DeterministicIndex::process(
    const std::optional<Bip32Index>& current,
    Bip32Index target,
    allocator_type monotonic) noexcept -> void
{
    auto elements = database::ElementMap{monotonic};
    auto postcondition = ScopeGuard{[&] { done(std::move(elements)); }};
    const auto& subchain = parent_.subchain_;
    const auto first =
        current.has_value() ? current.value() + 1u : Bip32Index{0u};
    const auto last = subaccount_.LastGenerated(subchain).value_or(0u);

    if (last > first) {
        log_(OT_PRETTY_CLASS())(name_)(" indexing elements from ")(
            first)(" to ")(last)
            .Flush();
    }

    for (auto i{first}; i <= last; ++i) {
        const auto& element = subaccount_.BalanceElement(subchain, i);
        parent_.IndexElement(parent_.filter_type_, element, i, elements);
    }

    log_(OT_PRETTY_CLASS())(name_)(" subchain is fully indexed to item ")(last)
        .Flush();
}
}  // namespace opentxs::blockchain::node::wallet
