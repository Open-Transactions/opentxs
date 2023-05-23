# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-base64 OBJECT "base64.c" "base64.h")
libopentxs_configure_c_target(opentxs-base64)

if(MSVC)
  target_compile_options(opentxs-base64 PRIVATE "/wd4244")
endif()

if(${CMAKE_CXX_COMPILER_ID}
   MATCHES
   Clang
)
  target_compile_options(
    opentxs-base64 PRIVATE "-Wno-reserved-macro-identifier"
                           "-Wno-shorten-64-to-32"
  )
endif()

libopentxs_add_sources($<TARGET_OBJECTS:opentxs-base64>)
