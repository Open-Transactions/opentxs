// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs::contract::blank
{
struct Signable : virtual public contract::Signable {
    auto Alias() const noexcept -> UnallocatedCString final { return {}; }
    auto ID() const noexcept -> identifier::Generic final { return {}; }
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto Nym() const noexcept -> Nym_p final { return {}; }
    auto Terms() const noexcept -> const UnallocatedCString& final
    {
        static const auto blank = UnallocatedCString{};

        return blank;
    }
    auto Serialize() const noexcept -> ByteArray final { return {}; }
    auto Validate() const noexcept -> bool final { return {}; }
    auto Version() const noexcept -> VersionNumber final { return {}; }

    auto SetAlias(const UnallocatedCString& alias) noexcept -> bool final
    {
        return {};
    }
};
}  // namespace opentxs::contract::blank
