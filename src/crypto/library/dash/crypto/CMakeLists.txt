# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-dashcrypto OBJECT
  "blake.c"
  "bmw.c"
  "cubehash.c"
  "echo.c"
  "groestl.c"
  "jh.c"
  "keccak.c"
  "luffa.c"
  "shavite.c"
  "simd.c"
  "skein.c"
  "sph_blake.h"
  "sph_bmw.h"
  "sph_cubehash.h"
  "sph_echo.h"
  "sph_groestl.h"
  "sph_jh.h"
  "sph_keccak.h"
  "sph_luffa.h"
  "sph_shavite.h"
  "sph_simd.h"
  "sph_skein.h"
  "sph_types.h"
)
set_target_properties(
  opentxs-dashcrypto
  PROPERTIES
    MSVC_RUNTIME_LIBRARY "${MSVC_RUNTIME_DEFAULT}"
    C_INCLUDE_WHAT_YOU_USE ""
    CXX_INCLUDE_WHAT_YOU_USE ""
    C_CLANG_TIDY ""
    CXX_CLANG_TIDY ""
    UNITY_BUILD OFF
)

libopentxs_configure_c_target(opentxs-dashcrypto)

if(MSVC)
  target_compile_options(opentxs-dashcrypto PRIVATE "/wd4267")
else()
  target_compile_options(
    opentxs-dashcrypto
    PRIVATE
      "-Wno-extra-semi-stmt"
      "-Wno-implicit-int-conversion"
      "-Wno-newline-eof"
      "-Wno-shadow"
      "-Wno-shorten-64-to-32"
      "-Wno-sign-conversion"
      "-Wno-switch-default"
      "-Wno-undef"
      "-Wno-unused-but-set-variable"
      "-Wno-unused-const-variable"
      "-Wno-unused-macros"
  )
endif()

libopentxs_add_sources($<TARGET_OBJECTS:opentxs-dashcrypto>)
