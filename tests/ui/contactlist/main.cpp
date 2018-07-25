// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include "OTTestEnvironment.hpp"

int main(int argc, char** argv)
{
    system("rm -r $HOME/.ot/");
    ::testing::AddGlobalTestEnvironment(new OTTestEnvironment());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
