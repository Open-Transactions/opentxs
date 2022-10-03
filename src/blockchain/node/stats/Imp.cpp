// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "blockchain/node/stats/Imp.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/node/stats/Shared.hpp"

namespace opentxs::blockchain::node
{
Stats::Imp::Imp(std::shared_ptr<stats::Shared> shared) noexcept
    : shared_(std::move(shared))
{
}

Stats::Imp::Imp() noexcept
    : Imp(std::make_shared<stats::Shared>())
{
}

auto Stats::Imp::clone() const noexcept -> std::unique_ptr<Imp>
{
    return std::make_unique<Imp>(shared_);
}

Stats::Imp::~Imp() = default;
}  // namespace opentxs::blockchain::node
