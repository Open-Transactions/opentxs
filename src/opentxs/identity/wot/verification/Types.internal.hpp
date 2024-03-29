// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::VerificationKind
// IWYU pragma: no_include "opentxs/identity/wot/verification/VerificationType.hpp"

#pragma once

#include <opentxs/protobuf/VerificationKind.pb.h>

#include "opentxs/identity/wot/verification/Types.hpp"

namespace opentxs::protobuf
{
auto translate(VerificationKind) noexcept -> identity::wot::verification::Type;
}  // namespace opentxs::protobuf

namespace opentxs::identity::wot::verification
{
auto translate(Type) noexcept -> protobuf::VerificationKind;
}  // namespace opentxs::identity::wot::verification
