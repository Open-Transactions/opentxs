# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(crypto)

add_opentx_test(unittests-opentxs-core-amount Test_Amount.cpp)
add_opentx_test(unittests-opentxs-core-data Test_Data.cpp)
add_opentx_test(unittests-opentxs-core-fixed_byte_array Test_FixedByteArray.cpp)
add_opentx_test(unittests-opentxs-core-identifier Test_Identifier.cpp)
add_opentx_test(unittests-opentxs-core-ledger Test_Ledger.cpp)
add_opentx_test(unittests-opentxs-core-nym Test_Nym.cpp)
add_opentx_test(unittests-opentxs-core-statemachine Test_StateMachine.cpp)
add_opentx_test(unittests-opentxs-core-display Test_DisplayScale.cpp)
add_opentx_test(unittests-opentxs-core-contact Test_Contact.cpp)
