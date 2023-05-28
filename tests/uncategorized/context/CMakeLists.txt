# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_low_level_test(ottest-context-client-session Test_ClientSession.cpp)
add_opentx_low_level_test(ottest-context-options Test_Options.cpp)

if(NOT WIN32)
  # NOTE: can not build due to LNK2019 error related to
  # ::testing::Matcher<std::string_view>
  add_opentx_low_level_test(ottest-context-ot Test_OT.cpp)
endif()

add_opentx_low_level_test(
  ottest-context-startup-shutdown Test_StartupShutdown.cpp
)
add_opentx_low_level_test(ottest-context-server-session Test_ServerSession.cpp)
add_opentx_low_level_test(
  ottest-context-password-callback Test_PasswordCallback.cpp
)

add_opentx_test(ottest-utils-standard-file-names Test_Legacy.cpp)
