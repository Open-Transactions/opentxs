// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/crypto/HD.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::crypto::internal
{
auto HD::Blank() noexcept -> HD&
{
    static auto blank = HD{};

    return blank;
}

auto HD::Standard() const noexcept -> HDProtocol { return {}; }
}  // namespace opentxs::blockchain::crypto::internal
