// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "internal/crypto/library/Factory.hpp"  // IWYU pragma: associated

#include "internal/crypto/library/OpenSSL.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto OpenSSL() noexcept -> std::unique_ptr<crypto::OpenSSL> { return {}; }
}  // namespace opentxs::factory
