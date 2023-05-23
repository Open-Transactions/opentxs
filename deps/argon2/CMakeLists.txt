# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-argon2 OBJECT
  "argon2.c"
  "argon2.h"
  "blake2-impl.h"
  "blake2.h"
  "blake2b.c"
  "core.c"
  "core.h"
  "encoding.c"
  "encoding.h"
)
libopentxs_configure_c_target(opentxs-argon2)

if(MSVC)
  target_compile_options(opentxs-argon2 PRIVATE "/wd4996")
endif()

if(${CMAKE_CXX_COMPILER_ID}
   MATCHES
   GNU
)
  target_compile_options(
    opentxs-argon2
    PRIVATE
      "-Wno-pedantic"
      "-Wno-switch-default"
      "-Wno-type-limits"
      "-Wno-unused-macros"
  )
endif()

if(${CMAKE_CXX_COMPILER_ID}
   MATCHES
   Clang
)
  target_compile_options(
    opentxs-argon2
    PRIVATE
      "-Wno-c99-extensions"
      "-Wno-cast-align"
      "-Wno-cast-qual"
      "-Wno-documentation-unknown-command"
      "-Wno-implicit-fallthrough"
      "-Wno-long-long"
      "-Wno-reserved-identifier"
      "-Wno-sign-conversion"
      "-Wno-unused-macros"
  )
endif()

if(ANDROID)
  target_sources(opentxs-argon2 PRIVATE "blamka-round-ref.h" "ref.c")
elseif(
  APPLE
  AND CMAKE_SYSTEM_PROCESSOR
      MATCHES
      "arm64"
)
  target_sources(opentxs-argon2 PRIVATE "blamka-round-ref.h" "ref.c")
else()
  target_sources(opentxs-argon2 PRIVATE "blamka-round-opt.h" "opt.c")
endif()

target_include_directories(
  opentxs-argon2 SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_compile_definitions(
  opentxs-argon2 PRIVATE "ARGON2_NO_THREADS" "NO_WARN_X86_INTRINSICS"
)
set_target_properties(
  opentxs-argon2
  PROPERTIES
    C_INCLUDE_WHAT_YOU_USE ""
    C_STANDARD 90
    UNITY_BUILD OFF
)
libopentxs_link_internal(opentxs-argon2)
libopentxs_add_sources($<TARGET_OBJECTS:opentxs-argon2>)
