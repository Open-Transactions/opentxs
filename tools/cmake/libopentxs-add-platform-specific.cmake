# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_use_if_exists filename)
  if(EXISTS "${filename}")
    target_sources(opentxs-common PRIVATE "${filename}")
  endif()
endmacro()

macro(libopentxs_add_platform_specific basename)
  if(WIN32)
    libopentxs_use_if_exists(
      "${CMAKE_CURRENT_SOURCE_DIR}/${basename}.windows.cpp"
    )
  elseif(UNIX)
    libopentxs_use_if_exists(
      "${CMAKE_CURRENT_SOURCE_DIR}/${basename}.posix.cpp"
    )

    if(APPLE)
      libopentxs_use_if_exists(
        "${CMAKE_CURRENT_SOURCE_DIR}/${basename}.apple.cpp"
      )

      if(IOS)
        libopentxs_use_if_exists(
          "${CMAKE_CURRENT_SOURCE_DIR}/${basename}.ios.cpp"
        )
      else()
        libopentxs_use_if_exists(
          "${CMAKE_CURRENT_SOURCE_DIR}/${basename}.macos.cpp"
        )
      endif()
    else()
      libopentxs_use_if_exists(
        "${CMAKE_CURRENT_SOURCE_DIR}/${basename}.unix.cpp"
      )

      if(ANDROID)
        libopentxs_use_if_exists(
          "${CMAKE_CURRENT_SOURCE_DIR}/${basename}.android.cpp"
        )
      else()
        libopentxs_use_if_exists(
          "${CMAKE_CURRENT_SOURCE_DIR}/${basename}.linux.cpp"
        )
      endif()
    endif()
  endif()

  if(NOT ANDROID)
    libopentxs_use_if_exists(
      "${CMAKE_CURRENT_SOURCE_DIR}/${basename}.non-android.cpp"
    )
  endif()
endmacro()
