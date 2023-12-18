// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/ClaimPrivate.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>

namespace opentxs::identity::wot
{
ClaimPrivate::Data::Data(
    std::span<const claim::Attribute> attributes,
    VersionNumber version,
    allocator_type alloc) noexcept
    : Allocated(alloc)
    , attributes_([&] {
        auto out = decltype(attributes_){alloc};
        std::ranges::copy(attributes, std::inserter(out, out.end()));

        return out;
    }())
    , version_(version)
{
}

ClaimPrivate::Data::Data(const Data& rhs, allocator_type alloc) noexcept
    : Allocated(alloc)
    , attributes_(rhs.attributes_, alloc)
    , version_(rhs.version_)
{
}
}  // namespace opentxs::identity::wot
