// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/blockchain/node/Stats.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace node
{
namespace stats
{
class Shared;
}  // namespace stats
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class Stats::Imp
{
public:
    std::shared_ptr<stats::Shared> shared_;

    auto clone() const noexcept -> std::unique_ptr<Imp>;

    Imp(std::shared_ptr<stats::Shared> shared) noexcept;
    Imp() noexcept;
    Imp(const Imp&) noexcept;
    Imp(Imp&&) noexcept;
    auto operator=(const Imp&) noexcept -> Imp&;
    auto operator=(Imp&&) noexcept -> Imp&;

    ~Imp();
};
}  // namespace opentxs::blockchain::node
