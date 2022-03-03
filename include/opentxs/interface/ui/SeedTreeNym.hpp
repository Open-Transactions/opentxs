// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>

#include "opentxs/interface/ui/ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs::ui
{
class OPENTXS_EXPORT SeedTreeNym : virtual public ListRow
{
public:
    virtual auto Index() const noexcept -> std::size_t = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto NymID() const noexcept -> UnallocatedCString = 0;

    ~SeedTreeNym() override = default;

protected:
    SeedTreeNym() noexcept = default;

private:
    SeedTreeNym(const SeedTreeNym&) = delete;
    SeedTreeNym(SeedTreeNym&&) = delete;
    auto operator=(const SeedTreeNym&) -> SeedTreeNym& = delete;
    auto operator=(SeedTreeNym&&) -> SeedTreeNym& = delete;
};
}  // namespace opentxs::ui
