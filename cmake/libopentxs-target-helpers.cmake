# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_include_directories DIR)
  target_include_directories(opentxs-common SYSTEM PRIVATE "${DIR}")

  if(OPENTXS_BUILD_TESTS)
    target_include_directories(opentxs-testlib SYSTEM PRIVATE "${DIR}")
  endif()
endmacro()

macro(libopentxs_link_internal LIB)
  target_link_libraries(opentxs-common PRIVATE "${LIB}")

  if(OPENTXS_BUILD_TESTS)
    target_link_libraries(opentxs-testlib PRIVATE "${LIB}")
  endif()
endmacro()

macro(libopentxs_link_external LIB)
  target_link_libraries(opentxs PUBLIC "${LIB}")

  if(OPENTXS_BUILD_TESTS)
    target_link_libraries(opentxs-testlib PUBLIC "${LIB}")
  endif()
endmacro()

macro(libopentxs_add_sources SOURCES)
  target_sources(opentxs PRIVATE "${SOURCES}")

  if(OPENTXS_BUILD_TESTS)
    target_sources(opentxs-testlib PRIVATE "${SOURCES}")
  endif()
endmacro()
