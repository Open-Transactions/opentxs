# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_install)
  if(OPENTXS_STANDALONE OR OT_INSTALL_LIBRARY_DEPENDENCIES)
    install(
      TARGETS libopentxs
      DESTINATION "${OT_LIBRARY_INSTALL_PATH}"
      EXPORT opentxs
      COMPONENT opentxs_library
      FILE_SET HEADERS
      DESTINATION "${OT_HEADER_INSTALL_PATH}"
      INCLUDES
      DESTINATION "${OT_HEADER_INSTALL_PATH}"
    )
  endif()

  if(OPENTXS_STANDALONE)
    set(PACKAGE_INIT_STRING "@PACKAGE_INIT@")
    set(MODULES_DIR_STRING "@PACKAGE_MODULES_DIR@")

    install(
      FILES "${opentxs_SOURCE_DIR}/cmake/libopentxs-find-dependencies.cmake"
      DESTINATION "${OT_CMAKE_INSTALL_PATH}"
      COMPONENT opentxs_cmake_modules
    )

    configure_file(
      "${opentxs_SOURCE_DIR}/cmake/opentxsConfig.cmake.in"
      "${opentxs_BINARY_DIR}/cmake/opentxsConfig.cmake.in"
      @ONLY
    )

    otcommon_generate_cmake_files(
      "${opentxs_BINARY_DIR}/cmake/opentxsConfig.cmake.in"
      "cmake"
      "${OT_CMAKE_INSTALL_PATH}"
    )

    if(OT_INSTALL_LICENSE)
      install(
        FILES "${opentxs_SOURCE_DIR}/LICENSE"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}"
        COMPONENT opentxs_documentation
        RENAME "${OT_LICENSE_FILE_NAME}"
      )
    endif()
  endif()

  if(OT_INSTALL_LIBRARY_DEPENDENCIES)
    add_library(libopentxs-deps SHARED "${opentxs_SOURCE_DIR}/tests/pch.cpp")
    libopentxs_configure_cxx_target(libopentxs-deps)
    target_link_libraries(libopentxs-deps PUBLIC opentxs::libopentxs)

    install(
      FILES $<TARGET_RUNTIME_DLLS:libopentxs-deps>
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT opentxs_library
    )
  endif()
endmacro()
