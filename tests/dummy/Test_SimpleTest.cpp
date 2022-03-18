// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>

// test at global namespace
TEST(SimpleTestAtGlobalNamespace, simpleTest) { ASSERT_TRUE(true); }

// test at unnamed namespace
namespace
{
TEST(SimpleTestAtUnNamedNamespace, simpletest) { ASSERT_TRUE(true); }
}  // namespace

// test at named namespace
namespace ottest
{
TEST(SimpleTestAtNamedNamespace, simpleTest) { ASSERT_TRUE(true); }
}  // namespace ottest
