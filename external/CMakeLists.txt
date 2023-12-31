# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(argon2)
add_subdirectory(base64)
add_subdirectory(irrxml)

if(PACKETCRYPT_EXPORT)
  add_subdirectory(packetcrypt)
endif()

if(CASH_LUCRE_EXPORT)
  add_library(opentxs-lucre OBJECT "lucre/src/bankimp.cpp")
  libopentxs_configure_cxx_target(opentxs-lucre)
  target_link_libraries(opentxs-lucre PRIVATE OpenSSL::Crypto)
  target_include_directories(
    opentxs-lucre SYSTEM
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/lucre/"
  )
  target_include_directories(
    opentxs-lucre SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/"
  )

  if(NOT MSVC)
    target_compile_options(
      opentxs-lucre
      PRIVATE
        -Wno-effc++
        -Wno-format
        -Wno-old-style-cast
        -Wno-shorten-64-to-32
        -Wno-sign-conversion
        -Wno-string-conversion
        -Wno-unused-variable
        -Wno-zero-as-null-pointer-constant
    )
  endif()

  set_target_properties(
    opentxs-lucre
    PROPERTIES
      C_INCLUDE_WHAT_YOU_USE ""
      CXX_INCLUDE_WHAT_YOU_USE ""
      CXX_CLANG_TIDY ""
  )
  libopentxs_link_internal(opentxs-lucre)
  libopentxs_link_external(OpenSSL::Crypto)
  libopentxs_add_sources($<TARGET_OBJECTS:opentxs-lucre>)
endif(CASH_LUCRE_EXPORT)

add_library(
  opentxs-external-hashes OBJECT "smhasher/src/MurmurHash3.cpp"
                                 "smhasher/src/MurmurHash3.h"
)
libopentxs_configure_cxx_target(opentxs-external-hashes)

if(NOT MSVC)
  target_compile_options(
    opentxs-external-hashes
    PRIVATE
      -Wno-cast-align
      -Wno-effc++
      -Wno-extra-semi-stmt
      -Wno-implicit-fallthrough
      -Wno-old-style-cast
      -Wno-shadow
      -Wno-sign-conversion
      -Wno-switch-default
      -Wno-zero-as-null-pointer-constant
  )
endif()

if(${CMAKE_CXX_COMPILER_ID}
   MATCHES
   Clang
)
  target_compile_options(
    opentxs-external-hashes PRIVATE -Wno-reserved-id-macro
                                    -Wno-shadow-field-in-constructor
  )
endif()

libopentxs_add_sources($<TARGET_OBJECTS:opentxs-external-hashes>)

add_library(
  opentxs-keccak OBJECT "keccack/keccak-tiny.c" "keccack/keccak-tiny.h"
)
libopentxs_configure_c_target(opentxs-keccak)
set_target_properties(
  opentxs-keccak
  PROPERTIES
    C_STANDARD 11
    C_INCLUDE_WHAT_YOU_USE ""
    CXX_INCLUDE_WHAT_YOU_USE ""
)

if(NOT MSVC)
  target_compile_options(
    opentxs-keccak
    PRIVATE
      -Wno-declaration-after-statement
      -Wno-extra-semi-stmt
      -Wno-reserved-macro-identifier
  )
endif()

libopentxs_add_sources($<TARGET_OBJECTS:opentxs-keccak>)

add_library(
  opentxs-bech32 OBJECT
  "bech32/ref/c++/bech32.cpp"
  "bech32/ref/c++/bech32.h"
  "bech32/ref/c++/segwit_addr.cpp"
  "bech32/ref/c++/segwit_addr.h"
)
libopentxs_configure_cxx_target(opentxs-bech32)
target_include_directories(
  opentxs-bech32 SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/bech32/ref/c++"
)

if(NOT MSVC)
  target_compile_options(
    opentxs-bech32 PRIVATE -Wno-effc++ -Wno-unused-variable
  )
endif()

set_target_properties(
  opentxs-bech32
  PROPERTIES
    C_INCLUDE_WHAT_YOU_USE ""
    CXX_INCLUDE_WHAT_YOU_USE ""
    CXX_CLANG_TIDY ""
)
libopentxs_add_sources($<TARGET_OBJECTS:opentxs-bech32>)

if(OT_BOOST_JSON_HEADER_ONLY)
  target_sources(opentxs-common PRIVATE "Json.cpp")
else()
  libopentxs_link_external(Boost::json)
endif()

target_sources(opentxs-common PRIVATE "BoostAsio.cpp" "BoostAsio.hpp")
libopentxs_link_internal(Boost::headers)
libopentxs_link_external(Boost::system)

libopentxs_include_directories("${opentxs_SOURCE_DIR}/external/")
libopentxs_include_directories(
  "${opentxs_SOURCE_DIR}/external/cs_libguarded/src"
)
libopentxs_include_directories("${opentxs_SOURCE_DIR}/external/frozen/include")
libopentxs_link_internal(Boost::headers)
