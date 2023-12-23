// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
template <typename I>
struct HDIndex {
    crypto::Bip32Index value_{};

    operator crypto::Bip32Index() const { return value_; }

    HDIndex(const I in)
        : value_(static_cast<crypto::Bip32Index>(in))
    {
    }

    HDIndex(const I lhs, const crypto::Bip32Child rhs)
        : value_(
              static_cast<crypto::Bip32Index>(lhs) |
              static_cast<crypto::Bip32Index>(rhs))
    {
    }
};

template <typename Bip43Purpose>
HDIndex(const Bip43Purpose, const crypto::Bip32Child) -> HDIndex<Bip43Purpose>;
}  // namespace opentxs
