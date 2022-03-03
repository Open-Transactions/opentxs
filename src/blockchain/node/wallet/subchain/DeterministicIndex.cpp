// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"  // IWYU pragma: associated

#include "blockchain/node/wallet/subchain/statemachine/Progress.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
DeterministicStateData::Index::Index(
    const crypto::Deterministic& subaccount,
    SubchainStateData& parent,
    Scan& scan,
    Rescan& rescan,
    Progress& progress) noexcept
    : wallet::Index(parent, scan, rescan, progress)
    , subaccount_(subaccount)
{
}

auto DeterministicStateData::Index::Do(
    std::optional<Bip32Index> current,
    Bip32Index target) noexcept -> void
{
    auto elements = WalletDatabase::ElementMap{};
    const auto original = progress_.Get();
    auto postcondition = ScopeGuard{[&] { done(original, elements); }};
    const auto& name = parent_.name_;
    const auto& subchain = parent_.subchain_;
    const auto first =
        current.has_value() ? current.value() + 1u : Bip32Index{0u};
    const auto last = subaccount_.LastGenerated(subchain).value_or(0u);

    if (last > first) {
        LogVerbose()(OT_PRETTY_CLASS())(name)(" indexing elements from ")(
            first)(" to ")(last)
            .Flush();
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(
            name)(" subchain is fully indexed to item ")(last)
            .Flush();
    }

    for (auto i{first}; i <= last; ++i) {
        const auto& element = subaccount_.BalanceElement(subchain, i);
        index_element(parent_.filter_type_, element, i, elements);
    }
}

auto DeterministicStateData::Index::need_index(
    const std::optional<Bip32Index>& current) const noexcept
    -> std::optional<Bip32Index>
{
    const auto& name = parent_.name_;
    const auto generated = subaccount_.LastGenerated(parent_.subchain_);

    if (generated.has_value()) {
        const auto target = generated.value();

        if ((false == current.has_value()) || (current.value() != target)) {
            LogVerbose()(OT_PRETTY_CLASS())(name)(" has ")(target + 1)(
                " keys generated, but only ")(current.value_or(0))(
                " have been indexed.")
                .Flush();

            return target;
        } else {
            LogTrace()(OT_PRETTY_CLASS())(name)(" all ")(target + 1)(
                " generated keys have been indexed.")
                .Flush();
        }
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(name)(" no generated keys present")
            .Flush();
    }

    return std::nullopt;
}
}  // namespace opentxs::blockchain::node::wallet
