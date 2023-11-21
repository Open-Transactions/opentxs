// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/internal.factory.hpp"  // IWYU pragma: associated

#include "opentxs/api/FactoryPrivate.hpp"

namespace opentxs::factory
{
auto FactoryAPI(const api::Crypto& crypto) noexcept
    -> std::shared_ptr<api::internal::Factory>
{
    using ReturnType = api::FactoryPrivate;

    return std::make_shared<ReturnType>(crypto);
}
}  // namespace opentxs::factory
