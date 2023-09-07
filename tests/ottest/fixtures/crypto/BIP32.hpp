// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "ottest/data/crypto/Bip32.hpp"

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT BIP32 : public ::testing::Test
{
protected:
    using Path = ot::UnallocatedVector<ot::Bip32Index>;

    const ot::api::session::Client& api_;
    const ot::PasswordPrompt reason_;

    auto make_path(const Child::Path& path) const noexcept -> Path;

    BIP32();
};
}  // namespace ottest
