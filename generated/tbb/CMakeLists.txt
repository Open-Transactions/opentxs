# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_WITH_TBB)
  configure_file(
    "tbb.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/TBB.hpp"
    @ONLY
  )
else()
  configure_file(
    "no-tbb.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/TBB.hpp"
    @ONLY
  )
endif()

target_include_directories(opentxs-common PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
