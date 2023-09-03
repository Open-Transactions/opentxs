// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/crypto/blockchain/Imp.hpp"  // IWYU pragma: associated

#include <algorithm>  // IWYU pragma: keep

#include "internal/blockchain/crypto/Account.hpp"  // IWYU pragma: keep

namespace opentxs::api::crypto::imp
{
auto Blockchain::Imp::get(
    std::span<std::pair<
        const opentxs::blockchain::crypto::Account*,
        opentxs::blockchain::crypto::Notifications*>> in) const noexcept -> void
{
    const auto cb = [](const auto& i) { i.first->Internal().Get(*i.second); };
    std::for_each(in.begin(), in.end(), cb);
}
}  // namespace opentxs::api::crypto::imp
