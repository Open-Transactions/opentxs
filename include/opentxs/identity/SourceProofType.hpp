// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/identity/Types.hpp"  // IWYU pragma: keep

namespace opentxs::identity
{
enum class SourceProofType : std::underlying_type_t<SourceProofType> {
    Error = 0,
    SelfSignature = 1,
    Signature = 2,
};
}  // namespace opentxs::identity
