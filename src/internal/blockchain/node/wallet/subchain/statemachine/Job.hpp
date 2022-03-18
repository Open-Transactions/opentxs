// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"

namespace opentxs::blockchain::node::wallet
{
class Job
{
public:
    using State = JobState;

    [[nodiscard]] virtual auto ChangeState(const State state) noexcept
        -> bool = 0;
    virtual auto ProcessReorg(const block::Position& parent) noexcept
        -> void = 0;

    Job(const Job&) = delete;
    Job(Job&&) = delete;
    Job& operator=(const Job&) = delete;
    Job& operator=(Job&&) = delete;

    virtual ~Job() = default;

protected:
    Job() = default;
};
}  // namespace opentxs::blockchain::node::wallet
