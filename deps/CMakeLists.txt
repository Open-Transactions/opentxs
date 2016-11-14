# Copyright (c) Monetas AG, 2014

add_subdirectory(gtest)
add_subdirectory(irrxml)
add_subdirectory(base64)

### Build lucre as library
set(lucre-sources
  ${CMAKE_CURRENT_SOURCE_DIR}/lucre/src/bankimp.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/lucre/ ${OPENSSL_INCLUDE_DIR})

if(WIN32)
  # suppress sscanf/sprintf being unsafe warnings
  add_definitions("/wd4996")
endif()

add_library(lucre OBJECT
  ${lucre-sources}
)

set_property(TARGET lucre PROPERTY POSITION_INDEPENDENT_CODE 1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# -----------------------------------------------
if (OT_INTERNAL_ZMQ)

  SET (EXTERNAL_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE})

#  if (ANDROID)
#    SET (EXTERNAL_TOOLCHAIN_FILE ${ANDROID_TOOLCHAIN_CMAKE})
#  endif()

  include(ExternalProject)
  ExternalProject_Add(
    internal_zeromq
    BUILD_IN_SOURCE 0
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_TOOLCHAIN_FILE=${EXTERNAL_TOOLCHAIN_FILE} -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH} -DZMQ_BUILD_TESTS=OFF -DZMQ_BUILD_FRAMEWORK=OFF
  )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib
          DESTINATION ${CMAKE_INSTALL_LIBDIR})

  ExternalProject_Add(
    internal_czmq
    BUILD_IN_SOURCE 0
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/czmq
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_TOOLCHAIN_FILE=${EXTERNAL_TOOLCHAIN_FILE} -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH} -DZEROMQ_INCLUDE_DIRS=${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4/include -DZEROMQ_LIBRARIES=${CMAKE_CURRENT_BINARY_DIR}/lib/libzmq${CMAKE_SHARED_LIBRARY_SUFFIX}
    DEPENDS internal_zeromq
  )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib
          DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
# -----------------------------------------------

if (LIBSECP256K1_EXPORT AND OT_INTERNAL_LIBSECP)
    include(ExternalProject)
    ExternalProject_Add(
        libsecp256k1
        BUILD_IN_SOURCE 0
        EP_BASE ${CMAKE_CURRENT_BINARY_DIR}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin-secp256k1
        CONFIGURE_COMMAND CFLAGS=-fpic ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin-secp256k1/configure --prefix ${CMAKE_CURRENT_BINARY_DIR} --enable-shared=no --enable-static=yes
        BUILD_COMMAND make
    )

    ExternalProject_Add_Step(
        libsecp256k1
        generate
        COMMAND autoreconf -if --warnings=all ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin-secp256k1
        DEPENDERS configure
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libsecp256k1
    )

    ExternalProject_Add_Step(
        libsecp256k1
        make_build
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/libsecp256k1
        DEPENDERS generate
    )
    set_property(TARGET libsecp256k1 PROPERTY EXCLUDE_FROM_ALL TRUE)
endif()

if (TREZORCRYPTO_EXPORT)
    set(trezor-sources
        trezor-crypto/aes_modes.c
        trezor-crypto/aescrypt.c
        trezor-crypto/aeskey.c
        trezor-crypto/aestab.c
        trezor-crypto/base58.c
        trezor-crypto/bignum.c
        trezor-crypto/bip32.c
        trezor-crypto/bip39.c
        trezor-crypto/curves.c
        trezor-crypto/ecdsa.c
        trezor-crypto/hmac.c
        trezor-crypto/nist256p1.c
        trezor-crypto/pbkdf2.c
        trezor-crypto/rand.c
        trezor-crypto/ripemd160.c
        trezor-crypto/script.c
        trezor-crypto/secp256k1.c
        trezor-crypto/sha2.c
        trezor-crypto/sha3.c
        trezor-crypto/ed25519-donna/ed25519.c
    )
    set(trezor-headers
        trezor-crypto/aes.h
        trezor-crypto/aesopt.h
        trezor-crypto/aestab.h
        trezor-crypto/base58.h
        trezor-crypto/bignum.h
        trezor-crypto/bip32.h
        trezor-crypto/bip39.h
        trezor-crypto/bip39_english.h
        trezor-crypto/curves.h
        trezor-crypto/ecdsa.h
        trezor-crypto/hmac.h
        trezor-crypto/macros.h
        trezor-crypto/nist256p1.h
        trezor-crypto/options.h
        trezor-crypto/pbkdf2.h
        trezor-crypto/rand.h
        trezor-crypto/ripemd160.h
        trezor-crypto/script.h
        trezor-crypto/secp256k1.h
        trezor-crypto/sha2.h
        trezor-crypto/sha3.h
        trezor-crypto/ed25519-donna/ed25519.h
    )
    include_directories(trezor-crypto/ed25519-donna)
    set(CMAKE_C_FLAGS "-std=c99")
    set_source_files_properties(
        trezor-crypto/aeskey.c
        PROPERTIES
        COMPILE_FLAGS
        -Wno-sequence-point
    )
    add_library(TrezorCrypto OBJECT ${trezor-sources} ${trezor-headers})
    set_property(TARGET TrezorCrypto PROPERTY POSITION_INDEPENDENT_CODE 1)
endif()
