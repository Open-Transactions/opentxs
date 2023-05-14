# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include(otcommon-configure-target)
include(otcommon-define-signed-overflow)

function(libopentxs_configure_c_target target_name)
  otcommon_configure_c_target(${target_name})
  otcommon_define_signed_overflow(${target_name})

  if(CMAKE_CXX_COMPILER_ID
     MATCHES
     Clang
     AND CMAKE_CXX_COMPILER_VERSION
         VERSION_GREATER_EQUAL
         16.0.0
  )
    # NOTE too many false positives and hits in 3rd party & generated code
    target_compile_options(${target_name} PRIVATE "-Wno-unsafe-buffer-usage")
  endif()
endfunction()

function(libopentxs_configure_cxx_target target_name)
  otcommon_configure_cxx_target(${target_name})
  otcommon_define_signed_overflow(${target_name})

  if(MSVC)
    target_compile_options(
      ${target_name}
      PRIVATE
        "/wd4244"
        "/wd4251"
        "/wd4267"
        "/wd4275"
        "/wd4996"
    )
  else()
    target_compile_options(
      ${target_name}
      PRIVATE
        "-Wno-c++20-compat"
        "-Wno-deprecated-declarations"
        "-Wno-implicit-int-conversion"
        "-Wno-noexcept"
        "-Wno-return-std-move-in-c++11"
        "-Wno-sign-conversion"
        "-Wno-switch-enum"
        "-Wno-unused-parameter"
    )
  endif()

  if(CMAKE_CXX_COMPILER_ID
     MATCHES
     GNU
  )
    # NOTE too many false positives, mostly in standard library and third party
    # headers we can't do anything about anyway
    target_compile_options(
      ${target_name}
      PRIVATE
        "-Wno-error=null-dereference"
        "-Wno-error=stringop-overflow="
        "-Wno-error=stringop-overread"
    )
  endif()

  if(CMAKE_CXX_COMPILER_ID
     MATCHES
     Clang
     AND CMAKE_CXX_COMPILER_VERSION
         VERSION_GREATER_EQUAL
         16.0.0
  )
    # NOTE clang-16 erroneously tags system headers with this warning on some
    # platforms
    # ~~~
    # In file included from <built-in>:1:
    # /usr/include/gentoo/fortify.h:3:11: error: macro name is a reserved identifier [-Werror,-Wreserved-macro-identifier]
    # #  define __GENTOO_HAS_FEATURE(x) __has_feature(x)
    #           ^
    # /usr/include/gentoo/fortify.h:13:9: error: macro name is a reserved identifier [-Werror,-Wreserved-macro-identifier]
    # # undef __GENTOO_HAS_FEATURE
    #         ^
    # ~~~
    target_compile_options(
      ${target_name} PRIVATE "-Wno-reserved-macro-identifier"
    )
    # NOTE too many false positives and hits in 3rd party & generated code
    target_compile_options(${target_name} PRIVATE "-Wno-unsafe-buffer-usage")
  endif()

  if(CMAKE_CXX_COMPILER_ID
     MATCHES
     Clang
     AND CMAKE_CXX_COMPILER_VERSION
         VERSION_LESS
         14.0.0
  )
    target_compile_options(${target_name} PRIVATE "-Wno-reserved-identifier")
  endif()

  if(CMAKE_CXX_COMPILER_ID
     MATCHES
     AppleClang
  )
    # NOTE when AppleClang rewrites operator<=> it generates code that causes
    # this warning to trigger
    target_compile_options(
      ${target_name} PRIVATE "-Wno-zero-as-null-pointer-constant"
    )
  endif()
endfunction()
