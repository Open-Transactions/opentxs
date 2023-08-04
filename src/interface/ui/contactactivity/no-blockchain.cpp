// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/interface/ui/UI.hpp"  // IWYU pragma: associated

namespace opentxs::factory
{
auto BlockchainContactActivityItem(
    const ui::implementation::ContactActivityInternalInterface&,
    const api::session::Client&,
    const identifier::Nym&,
    const ui::implementation::ContactActivityRowID&,
    const ui::implementation::ContactActivitySortKey&,
    ui::implementation::CustomData&) noexcept
    -> std::shared_ptr<ui::implementation::ContactActivityRowInternal>
{
    return std::make_shared<ui::implementation::ContactActivityRowBlank>();
}
}  // namespace opentxs::factory
