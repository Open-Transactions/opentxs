# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-blockchain-activity-labels Test_Labels.cpp)
add_opentx_test(ottest-blockchain-activity-merge Test_Merge.cpp)
add_opentx_test(ottest-blockchain-activity-threads Test_Threads.cpp)

set_tests_properties(ottest-blockchain-activity-merge PROPERTIES DISABLED TRUE)
set_tests_properties(
  ottest-blockchain-activity-threads PROPERTIES DISABLED TRUE
)
