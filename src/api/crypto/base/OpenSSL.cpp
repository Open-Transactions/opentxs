// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "api/crypto/base/Crypto.hpp"  // IWYU pragma: associated

#include "internal/crypto/library/OpenSSL.hpp"
#include "internal/util/LogMacros.hpp"

namespace opentxs::api::imp
{
auto Crypto::Init_OpenSSL() noexcept -> void { OT_ASSERT(ssl_); }

auto Crypto::OpenSSL() const noexcept -> const opentxs::crypto::OpenSSL&
{
    return *ssl_;
}
}  // namespace opentxs::api::imp
