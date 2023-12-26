// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/crypto/BIP32.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "ottest/data/crypto/Bip32.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;

auto BIP32::make_path(const Child::Path& path) const noexcept -> Path
{
    auto output = Path{};
    static constexpr auto hard =
        static_cast<ot::crypto::Bip32Index>(ot::crypto::Bip32Child::HARDENED);

    for (const auto& item : path) {
        if (item.hardened_) {
            output.emplace_back(item.index_ | hard);
        } else {
            output.emplace_back(item.index_);
        }
    }

    return output;
}

BIP32::BIP32()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , reason_(api_.Factory().PasswordPrompt(__func__))
{
}
}  // namespace ottest
