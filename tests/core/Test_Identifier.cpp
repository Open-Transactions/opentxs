// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>

#include "internal/api/session/FactoryAPI.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/Identifier.pb.h"

namespace ot = opentxs;

namespace ottest
{
struct Default_Identifier : public ::testing::Test {
    ot::OTIdentifier identifier_;

    Default_Identifier()
        : identifier_(ot::Identifier::Factory())
    {
    }
};

struct Random_Identifier : public Default_Identifier {
    const ot::api::session::Client& api_;

    Random_Identifier()
        : Default_Identifier()
        , api_(ot::Context().StartClientSession(0))
    {
        identifier_->Randomize();
    }
};

TEST_F(Default_Identifier, default_accessors)
{
    ASSERT_EQ(identifier_->data(), nullptr);
    ASSERT_EQ(identifier_->size(), 0);
}

TEST_F(Default_Identifier, serialize_empty)
{
    const auto str = identifier_->str();
    const auto recovered = ot::Identifier::Factory(str);

    EXPECT_EQ(identifier_, recovered);
}

TEST_F(Random_Identifier, serialize_non_empty)
{
    const auto str = identifier_->str();
    const auto recovered = ot::Identifier::Factory(str);

    EXPECT_EQ(identifier_, recovered);
}

TEST_F(Random_Identifier, serialize_protobuf)
{
    auto proto = ot::proto::Identifier{};

    EXPECT_TRUE(identifier_->Serialize(proto));

    const auto recovered = api_.Factory().InternalSession().Identifier(proto);

    EXPECT_EQ(identifier_, recovered);
}
}  // namespace ottest
