// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/PathsPrivate.hpp"  // IWYU pragma: associated

namespace opentxs::api::internal
{
auto PathsPrivate::prepend() noexcept -> UnallocatedCString
{
    return "Documents/";
}
}  // namespace opentxs::api::internal
