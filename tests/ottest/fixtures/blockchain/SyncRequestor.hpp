// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <atomic>
#include <cstddef>
#include <memory>

#include "ottest/Basic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class MinedBlocks;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
struct OPENTXS_EXPORT SyncRequestor {
    std::atomic_int checked_;
    std::atomic_int expected_;

    auto check(
        const ot::network::otdht::State& state,
        const ot::blockchain::block::Position& pos) const noexcept -> bool;
    auto check(const ot::network::otdht::State& state, const std::size_t index)
        const -> bool;
    auto check(const ot::network::otdht::Block& block, const std::size_t index)
        const noexcept -> bool;

    auto get(const std::size_t index) const
        -> const ot::network::zeromq::Message&;
    auto request(const ot::blockchain::block::Position& pos) const noexcept
        -> bool;
    auto request(const ot::network::otdht::Base& command) const noexcept
        -> bool;
    auto wait(const bool hard = true) noexcept -> bool;

    SyncRequestor(
        const ot::api::session::Client& api,
        const MinedBlocks& cache) noexcept;

    ~SyncRequestor();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace ottest
