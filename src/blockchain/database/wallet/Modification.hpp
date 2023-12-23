// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>

#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::database::wallet
{
struct Modification final : public pmr::Allocated {
    node::TxoState final_state_;
    std::optional<node::TxoState> initial_state_;
    std::optional<protocol::bitcoin::base::block::Output> output_;
    Set<identifier::Generic> proposals_;

    auto CreatesGeneration() const noexcept -> bool;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Modification() = delete;
    Modification(node::TxoState state, allocator_type alloc = {}) noexcept;
    Modification(const Modification& rhs, allocator_type alloc = {}) noexcept;
    Modification(Modification&& rhs) noexcept;
    Modification(Modification&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Modification& rhs) noexcept -> Modification&;
};
}  // namespace opentxs::blockchain::database::wallet
