# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-identity-nym Test_Nym.cpp)
# NOTE tests files must not access un-exported symbols, such as any class
# defined in a .pb.h header or internal/ header
# add_opentx_test(ottest-identity-source Test_Source.cpp)
# add_opentx_test(ottest-identity-authority Test_Authority.cpp)
