// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "opentxs/util/Allocator.hpp"  // IWYU pragma: associated

#include <boost/container/pmr/global_resource.hpp>

#include "internal/util/BoostPMR.hpp"

namespace opentxs::alloc
{
auto System() noexcept -> Resource*
{
    // TODO replace with std::pmr::new_delete_resource once Android and Apple
    // catch up
    static auto resource =
        BoostWrap{boost::container::pmr::new_delete_resource()};

    return &resource;
}

auto Null() noexcept -> Resource*
{
    // TODO replace with std::pmr::null_memory_resource once Android and Apple
    // catch up
    static auto resource =
        BoostWrap{boost::container::pmr::null_memory_resource()};

    return &resource;
}
}  // namespace opentxs::alloc
