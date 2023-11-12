# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_generate_uninstall_target)
  configure_file(
    "${${PROJECT_NAME}_SOURCE_DIR}/external/opentxs-cmake/uninstall.cmake.in"
    "${${PROJECT_NAME}_BINARY_DIR}/uninstall.cmake"
    IMMEDIATE
    @ONLY
  )

  add_custom_target(
    uninstall COMMAND ${CMAKE_COMMAND} -P
                      ${${PROJECT_NAME}_BINARY_DIR}/uninstall.cmake
  )
endmacro()
