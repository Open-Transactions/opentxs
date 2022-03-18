# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(opentxs-test-helpers PRIVATE "Helpers.hpp" "Helpers.cpp")

add_opentx_test(unittests-opentxs-integration Test_Basic.cpp)
add_opentx_test(unittests-opentxs-integration-addcontact Test_AddContact.cpp)
add_opentx_test(unittests-opentxs-integration-deposit Test_DepositCheques.cpp)
add_opentx_test(unittests-opentxs-integration-pair Test_Pair.cpp)

set_tests_properties(unittests-opentxs-integration PROPERTIES DISABLED TRUE)
set_tests_properties(
  unittests-opentxs-integration-addcontact PROPERTIES DISABLED TRUE
)
set_tests_properties(
  unittests-opentxs-integration-deposit PROPERTIES DISABLED TRUE
)
set_tests_properties(
  unittests-opentxs-integration-pair PROPERTIES DISABLED TRUE
)
