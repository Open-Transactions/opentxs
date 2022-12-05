// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/interface/ui/UI.hpp"  // IWYU pragma: associated

namespace opentxs::factory
{
auto BlockchainActivityThreadItem(
    const ui::implementation::ActivityThreadInternalInterface&,
    const api::session::Client&,
    const identifier::Nym&,
    const ui::implementation::ActivityThreadRowID&,
    const ui::implementation::ActivityThreadSortKey&,
    ui::implementation::CustomData&) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>
{
    return std::make_shared<ui::implementation::ActivityThreadRowBlank>();
}
}  // namespace opentxs::factory
