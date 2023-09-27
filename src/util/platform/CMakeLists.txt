# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OPENTXS_DEBUG_BUILD)
  add_subdirectory(debug)
else()
  add_subdirectory(release)
endif()

if(LMDB_EXPORT)
  add_subdirectory(lmdb)
endif()

if(WIN32)
  target_sources(opentxs-common PRIVATE "Windows.cpp")
  libopentxs_link_internal(unofficial-sodium::sodium)
  libopentxs_link_external(unofficial-sodium::sodium)
  libopentxs_link_external(PThreads4W::PThreads4W)
elseif(UNIX)
  # Use this file for functions which are the same between all non-Windows
  # platforms
  target_sources(opentxs-common PRIVATE "Posix.cpp")
  libopentxs_link_internal(unofficial-sodium::sodium)
  libopentxs_link_external(unofficial-sodium::sodium)

  if(APPLE)
    target_sources(opentxs-common PRIVATE "Apple.cpp")

    if(CMAKE_CXX_COMPILER_ID
       MATCHES
       AppleClang
       AND CMAKE_CXX_COMPILER_VERSION
           VERSION_LESS
           15
    )
      target_sources(opentxs-common PRIVATE "libc++_pmr_workaround.cpp")
    endif()

    if(IOS)
      target_sources(opentxs-common PRIVATE "iOS.cpp")
    else()
      target_sources(opentxs-common PRIVATE "MacOS.cpp")
    endif()
  else()
    # Use this file for functions which do not need different behavior on
    # Android.
    target_sources(opentxs-common PRIVATE "Unix.cpp")

    if(ANDROID)
      target_sources(opentxs-common PRIVATE "Android.cpp")
      target_sources(opentxs-common PRIVATE "libc++_pmr_workaround.cpp")
    else()
      target_sources(opentxs-common PRIVATE "Linux.cpp")
    endif()
  endif()
endif()

if(NOT ANDROID)
  target_sources(opentxs-common PRIVATE "non-Android.cpp")
endif()

libopentxs_link_internal("${OT_ZMQ_TARGET}")
libopentxs_link_external("${OT_ZMQ_TARGET}")
