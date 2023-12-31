// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/protobuf/syntax/Types.internal.hpp"

namespace opentxs::protobuf::inline syntax
{
auto LucreTokenDataAllowedCiphertext() noexcept -> const VersionMap&;
auto PurseAllowedCiphertext() noexcept -> const VersionMap&;
auto PurseAllowedEnvelope() noexcept -> const VersionMap&;
auto PurseAllowedSymmetricKey() noexcept -> const VersionMap&;
auto PurseAllowedToken() noexcept -> const VersionMap&;
auto PurseExchangeAllowedPurse() noexcept -> const VersionMap&;
auto TokenAllowedLucreTokenData() noexcept -> const VersionMap&;
}  // namespace opentxs::protobuf::inline syntax
