// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT CreateNymHD : public ::testing::Test
{
public:
    static constexpr auto alice_expected_id_{
        "ot2xuVaYSXY9rZsm9oFHHPjy9uDrCJPG3DNyExF4uUHV2aCLuCvS8f3"};
    static constexpr auto bob_expected_id_{
        "ot2xuVSVeLyKCRrEpjPPwdra3UgwrZVLfhvxnZnA8oiy5FwiDDEW1KU"};
    static constexpr auto eve_expected_id_{
        "ot2xuVVxcrbQ8SsU3YQ2K6uEsTrF3xbLfwNq6f3BH5aUEXWSPuaANe9"};
    static constexpr auto frank_expected_id_{
        "ot2xuVYn8io5LpjK7itnUT7ujx8n5Rt3GKs5xXeh9nfZja2SwB5jEq6"};

    const ot::api::session::Client& api_;
    ot::PasswordPrompt reason_;
    ot::crypto::SeedID seed_a_;
    ot::crypto::SeedID seed_b_;
    ot::crypto::SeedID seed_c_;
    ot::crypto::SeedID seed_d_;
    ot::UnallocatedCString alice_, bob_;

    CreateNymHD();
};

}  // namespace ottest
