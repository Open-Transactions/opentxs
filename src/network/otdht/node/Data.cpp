// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "network/otdht/node/Shared.hpp"  // IWYU pragma: associated

namespace opentxs::network::otdht
{
Node::Shared::Data::Data(allocator_type alloc) noexcept
    : state_(alloc)
{
}

auto Node::Shared::Data::get_allocator() const noexcept -> allocator_type
{
    return state_.get_allocator();
}

Node::Shared::Data::~Data() = default;
}  // namespace opentxs::network::otdht
