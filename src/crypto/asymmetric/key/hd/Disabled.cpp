// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "crypto/asymmetric/key/hd/Imp.hpp"  // IWYU pragma: associated

namespace opentxs::crypto::asymmetric::key::implementation
{
auto HD::ChildKey(
    const Bip32Index index,
    const PasswordPrompt& reason,
    allocator_type alloc) const noexcept -> asymmetric::key::HD
{
    return HDPrivate::ChildKey(index, reason, alloc);
}
}  // namespace opentxs::crypto::asymmetric::key::implementation
