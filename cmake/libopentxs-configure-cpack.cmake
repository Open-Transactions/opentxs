# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_configure_cpack)
  include(InstallRequiredSystemLibraries)

  set(CPACK_NSIS_MODIFY_PATH "ON")

  if(WIN32)
    set(CPACK_PACKAGE_EXECUTABLES
        "..\\\\Uninstall;Uninstall ${PROJECT_NAME}  v${PROJECT_VERSION}"
    )

    set(CPACK_NSIS_MENU_LINKS "https://opentransactions.org"
                              "Open Transactions Link"
    )
  endif()

  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL, "ON")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
  set(CPACK_PACKAGE_VENDOR "http://www.OpenTransactions.org")
  set(CPACK_PACKAGE_VERSION_MAJOR "${${PROJECT_NAME}_VERSION_MAJOR}")
  set(CPACK_PACKAGE_VERSION_MINOR "${${PROJECT_NAME}_VERSION_MINOR}")
  set(CPACK_PACKAGE_VERSION_PATCH "${${PROJECT_NAME}_VERSION_PATCH}")

  # set( CPACK_PACKAGE_ICON " ")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
      "The Open-Transactions project is a collaborative effort to develop a robust, commercial-grade, fully-featured, free-software toolkit implementing the OTX protocol as well as a full-strength financial cryptography library, API, CLI, and prototype server. The project is managed by a worldwide community of volunteers that use the Internet to communicate, plan, and develop the Open-Transactions toolkit and its related documentation. ${${PROJECT_NAME}_GIT_VERSION}"
  )
  # set( CPACK_PACKAGE_CHECKSUM "" )
  set(CPACK_PACKAGE_HOMEPAGE_URL "https://opentransactions.org/wiki/Main_Page")
  set(CPACK_THREADS 0)
  include(CPack)
endmacro()