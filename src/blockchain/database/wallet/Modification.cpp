// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/wallet/Modification.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "opentxs/blockchain/node/TxoState.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"

namespace opentxs::blockchain::database::wallet
{
using namespace std::literals;
using enum node::TxoState;

Modification::Modification(node::TxoState state, allocator_type alloc) noexcept
    : Allocated({alloc})
    , final_state_(state)
    , initial_state_()
    , output_()
    , proposals_(alloc)
{
}

Modification::Modification(
    const Modification& rhs,
    allocator_type alloc) noexcept
    : Allocated({alloc})
    , final_state_(rhs.final_state_)
    , initial_state_(rhs.initial_state_)
    , output_([&]() -> decltype(output_) {
        if (rhs.output_.has_value()) {

            return protocol::bitcoin::base::block::Output{*rhs.output_, alloc};
        } else {

            return std::nullopt;
        }
    }())
    , proposals_(rhs.proposals_, alloc)
{
}

Modification::Modification(Modification&& rhs) noexcept
    : Modification(std::move(rhs), rhs.get_allocator())
{
}

Modification::Modification(Modification&& rhs, allocator_type alloc) noexcept
    : Allocated({alloc})
    , final_state_(std::move(rhs.final_state_))
    , initial_state_(std::move(rhs.initial_state_))
    , output_([&]() -> decltype(output_) {
        if (rhs.output_.has_value()) {

            return protocol::bitcoin::base::block::Output{
                std::move(*rhs.output_), alloc};
        } else {

            return std::nullopt;
        }
    }())
    , proposals_(std::move(rhs.proposals_), alloc)
{
}

auto Modification::CreatesGeneration() const noexcept -> bool
{
    return Immature == final_state_;
}

auto Modification::operator=(const Modification& rhs) noexcept -> Modification&
{
    final_state_ = rhs.final_state_;
    initial_state_ = rhs.initial_state_;
    output_ = rhs.output_;
    proposals_ = rhs.proposals_;

    return *this;
}
}  // namespace opentxs::blockchain::database::wallet
