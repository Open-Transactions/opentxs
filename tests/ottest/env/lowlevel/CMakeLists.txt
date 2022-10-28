# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(ottest-lowlevel PRIVATE "OTTestEnvironment.cpp")
target_link_libraries(ottest-lowlevel PRIVATE opentxs-testlib)
set_target_properties(
  ottest-lowlevel PROPERTIES POSITION_INDEPENDENT_CODE 1 UNITY_BUILD OFF
)
