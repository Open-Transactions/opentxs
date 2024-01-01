// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::CashType

#pragma once

#include <opentxs/protobuf/CashEnums.pb.h>

#include "opentxs/otx/blind/Types.hpp"

namespace opentxs
{
auto translate(const otx::blind::CashType in) noexcept -> protobuf::CashType;
auto translate(const otx::blind::PurseType in) noexcept -> protobuf::PurseType;
auto translate(const otx::blind::TokenState in) noexcept
    -> protobuf::TokenState;
auto translate(const protobuf::CashType in) noexcept -> otx::blind::CashType;
auto translate(const protobuf::PurseType in) noexcept -> otx::blind::PurseType;
auto translate(const protobuf::TokenState in) noexcept
    -> otx::blind::TokenState;
}  // namespace opentxs
