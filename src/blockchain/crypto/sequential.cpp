// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/Wallet.hpp"  // IWYU pragma: associated

#include <algorithm>

#include "internal/blockchain/crypto/Account.hpp"

namespace opentxs::blockchain::crypto::implementation
{
auto Wallet::get(std::span<std::pair<const crypto::Account*, Notifications*>>
                     in) const noexcept -> void
{
    const auto cb = [](const auto& i) { i.first->Internal().Get(*i.second); };
    std::for_each(in.begin(), in.end(), cb);
}
}  // namespace opentxs::blockchain::crypto::implementation
