// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/Wallet.hpp"  // IWYU pragma: associated

#include "TBB.hpp"
#include "internal/blockchain/crypto/Account.hpp"
#include "internal/util/P0330.hpp"

namespace opentxs::blockchain::crypto::implementation
{
auto Wallet::get(std::span<std::pair<const crypto::Account*, Notifications*>>
                     in) const noexcept -> void
{
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, in.size()}, [&](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                auto& [account, out] = in[i];
                account->Internal().Get(*out);
            }
        });
}
}  // namespace opentxs::blockchain::crypto::implementation
