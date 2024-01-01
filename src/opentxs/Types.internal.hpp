// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::AddressType

#pragma once

#include <opentxs/protobuf/ContractEnums.pb.h>
#include <cstddef>
#include <functional>
#include <mutex>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
using VersionConversionMap = UnallocatedMap<VersionNumber, VersionNumber>;

// local ID, remote ID
using ContextID = std::pair<UnallocatedCString, UnallocatedCString>;
using ContextLockCallback =
    std::function<std::recursive_mutex&(const ContextID&)>;

auto check_subset(
    std::size_t available,
    std::size_t requested,
    std::size_t position) noexcept -> bool;
auto translate(AddressType) noexcept -> protobuf::AddressType;
}  // namespace opentxs

namespace opentxs::protobuf
{
auto translate(AddressType) noexcept -> opentxs::AddressType;
}  // namespace opentxs::protobuf
