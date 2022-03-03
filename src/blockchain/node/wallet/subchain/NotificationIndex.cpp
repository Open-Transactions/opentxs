// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/NotificationStateData.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <iterator>
#include <utility>

#include "blockchain/node/wallet/subchain/statemachine/Progress.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
NotificationStateData::Index::Index(
    SubchainStateData& parent,
    Scan& scan,
    Rescan& rescan,
    Progress& progress,
    PaymentCode&& code) noexcept
    : wallet::Index(parent, scan, rescan, progress)
    , code_(std::move(code))
{
}

auto NotificationStateData::Index::Do(
    std::optional<Bip32Index>,
    Bip32Index) noexcept -> void
{
    auto elements = WalletDatabase::ElementMap{};
    const auto original = progress_.Get();
    auto postcondition = ScopeGuard{[&] { done(original, elements); }};

    for (auto i{code_.Version()}; i > 0; --i) {
        auto& vector = elements[i];

        switch (i) {
            case 1:
            case 2: {
                code_.Locator(writer(vector.emplace_back()), i);
            } break;
            case 3:
            default: {
                vector.reserve(2);
                auto b = std::array<std::byte, 33>{};
                auto& prefix = b[0];
                auto* start = std::next(b.data(), 1);
                auto* stop = std::next(b.data(), b.size());
                code_.Locator(preallocated(32, start), i);
                prefix = std::byte{0x02};
                vector.emplace_back(b.data(), stop);
                prefix = std::byte{0x03};
                vector.emplace_back(b.data(), stop);
            }
        }
    }

    LogTrace()(OT_PRETTY_CLASS())("Payment code ")(code_.asBase58())(" indexed")
        .Flush();
}

auto NotificationStateData::Index::need_index(
    const std::optional<Bip32Index>& current) const noexcept
    -> std::optional<Bip32Index>
{
    const auto version = code_.Version();

    if (current.value_or(0) < version) {
        LogVerbose()(OT_PRETTY_CLASS())("Payment code ")(code_.asBase58())(
            " notification elements not yet indexed for version ")(version)
            .Flush();

        return static_cast<Bip32Index>(version);
    } else {
        LogTrace()(OT_PRETTY_CLASS())("Payment code ")(code_.asBase58())(
            " already indexed")
            .Flush();

        return std::nullopt;
    }
}
}  // namespace opentxs::blockchain::node::wallet
