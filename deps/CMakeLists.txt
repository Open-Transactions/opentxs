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
  add_library(lucre OBJECT "lucre/src/bankimp.cpp")
  target_link_libraries(lucre PRIVATE OpenSSL::Crypto)
  target_include_directories(
    lucre SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/lucre/"
  )
  target_include_directories(
    lucre SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/"
  )

  if(NOT MSVC)
    target_compile_options(
      lucre
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
    lucre PROPERTIES C_INCLUDE_WHAT_YOU_USE "" CXX_INCLUDE_WHAT_YOU_USE ""
  )
  opentxs_link_internal(lucre)
  opentxs_link_external(OpenSSL::Crypto)
  opentxs_add_sources($<TARGET_OBJECTS:lucre>)
endif(CASH_LUCRE_EXPORT)

add_library(
  external-hashes OBJECT "smhasher/src/MurmurHash3.cpp"
                         "smhasher/src/MurmurHash3.h"
)

if(NOT MSVC)
  target_compile_options(
    external-hashes
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
    external-hashes PRIVATE -Wno-reserved-id-macro
                            -Wno-shadow-field-in-constructor
  )
endif()

opentxs_add_sources($<TARGET_OBJECTS:external-hashes>)

add_library(
  bech32 OBJECT
  "bech32/ref/c++/bech32.cpp"
  "bech32/ref/c++/bech32.h"
  "bech32/ref/c++/segwit_addr.cpp"
  "bech32/ref/c++/segwit_addr.h"
)
target_include_directories(
  bech32 SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/bech32/ref/c++"
)

if(NOT MSVC)
  target_compile_options(bech32 PRIVATE -Wno-effc++ -Wno-unused-variable)
endif()

set_target_properties(
  bech32 PROPERTIES C_INCLUDE_WHAT_YOU_USE "" CXX_INCLUDE_WHAT_YOU_USE ""
)
opentxs_add_sources($<TARGET_OBJECTS:bech32>)

if(OT_BUNDLED_SECP256K1 AND OT_CRYPTO_USING_LIBSECP256K1)
  set(SECP256K1_ENABLE_COVERAGE
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_ENABLE_BRANCH_COVERAGE
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_BUILD_TEST
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_BUILD_BENCH
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_ENABLE_MODULE_ECDH
      ON
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_ECMULT_STATIC_PRECOMPUTATION
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  set(SECP256K1_ENABLE_JNI
      OFF
      CACHE
        BOOL
        ""
        FORCE
  )
  add_subdirectory(secp256k1)
  set_target_properties(
    secp256k1-obj PROPERTIES C_INCLUDE_WHAT_YOU_USE "" CXX_INCLUDE_WHAT_YOU_USE
                                                       ""
  )
  set_target_properties(
    secp256k1-lib PROPERTIES C_INCLUDE_WHAT_YOU_USE "" CXX_INCLUDE_WHAT_YOU_USE
                                                       ""
  )

  if(MSVC)
    target_compile_options(
      secp256k1-obj
      PRIVATE
        /wd4127
        /wd4132
        /wd4146
        /wd4242
        /wd4244
        /wd4267
        /wd4310
        /wd4334
        /wd4706
        /wd4710
        /wd4711
        /wd4820
        /wd5045
    )
  endif()
endif()

if(OT_BOOST_JSON_HEADER_ONLY)
  target_sources(opentxs-common PRIVATE "Json.cpp")
else()
  opentxs_link_external(Boost::json)
endif()

target_sources(opentxs-common PRIVATE "BoostAsio.hpp")

target_sources(opentxs-common PRIVATE "TBB.hpp")
opentxs_link_internal(TBB::tbb)
opentxs_link_external(TBB::tbb)

opentxs_include_directories("${opentxs_SOURCE_DIR}/deps/")
opentxs_include_directories("${opentxs_SOURCE_DIR}/deps/cs_libguarded/src")
opentxs_include_directories("${opentxs_SOURCE_DIR}/deps/frozen/include")
opentxs_include_directories("${opentxs_SOURCE_DIR}/deps/robin-hood/src/include")
opentxs_include_directories(
  "${opentxs_SOURCE_DIR}/deps/unordered_dense/include"
)
opentxs_link_internal(Boost::headers)
