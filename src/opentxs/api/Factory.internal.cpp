// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/Factory.internal.hpp"  // IWYU pragma: associated

#include "opentxs/api/Factory.hpp"

namespace opentxs::api::internal
{
auto Factory::Detach(api::Factory& self) noexcept -> void
{
    self.imp_ = nullptr;
}
}  // namespace opentxs::api::internal
