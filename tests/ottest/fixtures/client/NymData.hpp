// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT NymData : public ::testing::Test
{
public:
    const ot::api::session::Client& client_;
    ot::PasswordPrompt reason_;
    ot::NymData nym_data_;

    static auto ExpectedStringOutput(const std::uint32_t version)
        -> ot::UnallocatedCString;

    NymData();
};

}  // namespace ottest
