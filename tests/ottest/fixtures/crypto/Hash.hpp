// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT Test_Hash : public ::testing::Test
{
public:
    const ot::api::Crypto& crypto_;

    auto GetHash(
        const ot::api::Session& api,
        const ot::crypto::symmetric::Key& key,
        const ot::PasswordPrompt& reason,
        std::size_t bytes) const noexcept -> ot::ByteArray;

    Test_Hash();
};
}  // namespace ottest
