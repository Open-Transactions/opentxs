// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::otx::blind::CashType
// IWYU pragma: no_forward_declare opentxs::otx::blind::PurseType
// IWYU pragma: no_forward_declare opentxs::otx::blind::TokenState
// IWYU pragma: no_forward_declare opentxs::proto::CashType
// IWYU pragma: no_include "opentxs/otx/blind/CashType.hpp"
// IWYU pragma: no_include "opentxs/otx/blind/PurseType.hpp"
// IWYU pragma: no_include "opentxs/otx/blind/TokenState.hpp"

#pragma once

#include "opentxs/otx/blind/Types.hpp"

#include <CashEnums.pb.h>

namespace opentxs
{
auto translate(const otx::blind::CashType in) noexcept -> proto::CashType;
auto translate(const otx::blind::PurseType in) noexcept -> proto::PurseType;
auto translate(const otx::blind::TokenState in) noexcept -> proto::TokenState;
auto translate(const proto::CashType in) noexcept -> otx::blind::CashType;
auto translate(const proto::PurseType in) noexcept -> otx::blind::PurseType;
auto translate(const proto::TokenState in) noexcept -> otx::blind::TokenState;
}  // namespace opentxs
