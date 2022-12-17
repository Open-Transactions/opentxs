// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <memory>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class User;
struct TXOState;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
namespace ot = opentxs;

struct OPENTXS_EXPORT TXOs {
    auto AddConfirmed(
        const ot::blockchain::block::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool;
    auto AddGenerated(
        const ot::blockchain::block::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner,
        const ot::blockchain::block::Height position) noexcept -> bool;
    auto AddUnconfirmed(
        const ot::blockchain::block::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool;
    auto Confirm(
        const ot::blockchain::block::TransactionHash& transaction) noexcept
        -> bool;
    auto Mature(const ot::blockchain::block::Height position) noexcept -> bool;
    auto Orphan(
        const ot::blockchain::block::TransactionHash& transaction) noexcept
        -> bool;
    auto OrphanGeneration(
        const ot::blockchain::block::TransactionHash& transaction) noexcept
        -> bool;
    auto SpendUnconfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
        -> bool;
    auto SpendConfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
        -> bool;

    auto Extract(TXOState& output) const noexcept -> void;

    TXOs(const User& owner) noexcept;
    TXOs() = delete;
    TXOs(const TXOs&) = delete;
    TXOs(TXOs&&) = delete;
    auto operator=(const TXOs&) -> TXOs& = delete;
    auto operator=(TXOs&&) -> TXOs& = delete;

    ~TXOs();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace ottest
