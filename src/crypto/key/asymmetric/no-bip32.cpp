// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "crypto/key/asymmetric/HD.hpp"  // IWYU pragma: associated

#include "internal/util/LogMacros.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::crypto::key::implementation
{
auto HD::ChildKey(const Bip32Index, const PasswordPrompt&) const noexcept
    -> std::unique_ptr<key::HD>
{
    LogError()(OT_PRETTY_CLASS())("HD key support missing").Flush();

    return {};
}
}  // namespace opentxs::crypto::key::implementation
