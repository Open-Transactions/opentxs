# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OPENTXS_DEBUG_BUILD)
  target_sources(opentxs-common PRIVATE "Debug.cpp")
else()
  target_sources(opentxs-common PRIVATE "Release.cpp")
endif()

if(WIN32)
  target_sources(opentxs-common PRIVATE "Windows.cpp")

  if(OPENTXS_DEBUG_BUILD)
    target_sources(opentxs-common PRIVATE "Windows_debug.cpp")
  else()
    target_sources(opentxs-common PRIVATE "Windows_release.cpp")
  endif()

  if(LMDB_EXPORT)
    target_sources(opentxs-common PRIVATE "Windows_lmdb.cpp")
  endif()

  opentxs_link_internal(unofficial-sodium::sodium)
  opentxs_link_external(unofficial-sodium::sodium)
  opentxs_link_external(PThreads4W::PThreads4W)
elseif(UNIX)
  # Use this file for functions which are the same between all non-Windows
  # platforms
  target_sources(opentxs-common PRIVATE "Posix.cpp")

  if(OPENTXS_DEBUG_BUILD)
    # TODO
  else()
    target_sources(opentxs-common PRIVATE "Posix_release.cpp")
  endif()

  opentxs_link_internal(unofficial-sodium::sodium)
  opentxs_link_external(unofficial-sodium::sodium)

  if(APPLE)
    target_sources(opentxs-common PRIVATE "Apple.cpp")

    if(OPENTXS_DEBUG_BUILD)
      target_sources(opentxs-common PRIVATE "Apple_debug.cpp")
    endif()

    if(LMDB_EXPORT)
      target_sources(opentxs-common PRIVATE "Apple_lmdb.cpp")
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

    if(OPENTXS_DEBUG_BUILD)
      target_sources(opentxs-common PRIVATE "Unix_debug.cpp")
    endif()

    if(LMDB_EXPORT)
      target_sources(opentxs-common PRIVATE "Unix_lmdb.cpp")
    endif()

    if(ANDROID)
      target_sources(opentxs-common PRIVATE "Android.cpp")
    else()
      target_sources(opentxs-common PRIVATE "Linux.cpp")
    endif()
  endif()
endif()

if(NOT ANDROID)
  target_sources(opentxs-common PRIVATE "non-Android.cpp")
endif()

opentxs_link_internal("${OT_ZMQ_TARGET}")
opentxs_link_external("${OT_ZMQ_TARGET}")
