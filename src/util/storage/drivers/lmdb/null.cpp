// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"
#include "opentxs/util/storage/Driver.hpp"

namespace opentxs::factory
{
auto StorageLMDB(const api::Crypto&, const storage::Config&) noexcept
    -> std::unique_ptr<storage::Driver>
{
    return {};
}
}  // namespace opentxs::factory
