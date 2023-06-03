# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.26.0)
cmake_policy(
  SET
  CMP0091
  NEW
)
cmake_policy(
  SET
  CMP0126
  NEW
)
set(CMAKE_OSX_DEPLOYMENT_TARGET 11)
project(opentxs)
list(
  APPEND
  CMAKE_MODULE_PATH
  "${${PROJECT_NAME}_SOURCE_DIR}/cmake"
  "${${PROJECT_NAME}_SOURCE_DIR}/deps/opentxs-cmake"
)

# -----------------------------------------------------------------------------
# Setup

include(otcommon-cmake)
otcommon_version_from_git()
otcommon_set_build_type()
otcommon_require_out_of_source_build()
otcommon_update_submodules()
otcommon_set_rpath()

# -----------------------------------------------------------------------------
# Set option defaults

if(CMAKE_BUILD_TYPE
   STREQUAL
   "Debug"
   OR CMAKE_BUILD_TYPE
      STREQUAL
      "RelWithDebInfo"
)
  set(OPENTXS_DEBUG_BUILD ON)
  set(OPENTXS_PEDANTIC_DEFAULT ON)
  set(OPENTXS_BUILD_TESTS_DEFAULT ${PROJECT_IS_TOP_LEVEL})
  set(CMAKE_VERBOSE_MAKEFILE ON)
else()
  set(OPENTXS_DEBUG_BUILD OFF)
  set(OPENTXS_PEDANTIC_DEFAULT OFF)
  set(OPENTXS_BUILD_TESTS_DEFAULT OFF)
endif()

if(DEFINED VCPKG_TARGET_TRIPLET)
  set(OT_USE_VCPKG_TARGETS_DEFAULT ON)
else()
  set(OT_USE_VCPKG_TARGETS_DEFAULT OFF)
endif()

# -----------------------------------------------------------------------------
# Options

option(
  OPENTXS_STANDALONE
  "Configure as standalone library instead of a subproject"
  ${PROJECT_IS_TOP_LEVEL}
)
option(
  OPENTXS_BUILD_TESTS
  "Build the unit tests."
  ${OPENTXS_BUILD_TESTS_DEFAULT}
)
option(
  OPENTXS_PEDANTIC_BUILD
  "Treat compiler warnings as errors."
  ${OPENTXS_PEDANTIC_DEFAULT}
)
option(
  OT_VALGRIND
  "Use Valgrind annotations."
  OFF
)
option(
  OT_PCH
  "Use precompiled headers"
  ON
)
option(
  OT_IWYU
  "Enable include-what-you-use static analysis"
  OFF
)
option(
  OT_CLANG_TIDY
  "Enable clang-tidy"
  OFF
)

if(OT_IWYU OR OT_CLANG_TIDY)
  set(OT_PCH OFF)
endif()

if(NOT OT_PCH)
  set(CMAKE_UNITY_BUILD OFF)
endif()

option(
  OT_STORAGE_FS
  "Enable filesystem backend for storage"
  OFF
)
option(
  OT_STORAGE_SQLITE
  "Enable sqlite backend for storage"
  OFF
)
option(
  OT_STORAGE_LMDB
  "Enable LMDB backend for storage"
  ON
)
option(
  OT_CRYPTO_SUPPORTED_KEY_ED25519
  "Enable ed25519 key support"
  ON
)
option(
  OT_CRYPTO_SUPPORTED_KEY_RSA
  "Enable RSA key support"
  OFF
)
option(
  OT_CRYPTO_SUPPORTED_KEY_SECP256K1
  "Enable secp256k1 key support"
  ON
)
option(
  OT_CRYPTO_USING_LIBSECP256K1
  "Use libsecp256k1 crypto library"
  ON
)
option(
  OT_CRYPTO_USING_PACKETCRYPT
  "Enable packetcrypt validation library"
  OFF
)
option(
  OT_WITH_BLOCKCHAIN
  "Enable blockchain client support"
  ON
)
option(
  OT_CASH_USING_LUCRE
  "Enable Lucre support"
  OFF
)
option(
  OT_LUCRE_DEBUG
  "Output Lucre debug info"
  OFF
)
option(
  OT_SCRIPT_USING_CHAI
  "Use chaiscript"
  ON
)
option(
  OT_WITH_QT
  "Enable Qt model support for ui classes"
  ON
)
option(
  OT_WITH_QML
  "Enable Qt QML model support for ui classes"
  ${OT_WITH_QT}
)
option(
  OT_USE_VCPKG_TARGETS
  "Assume dependencies are managed by vcpkg"
  ${OT_USE_VCPKG_TARGETS_DEFAULT}
)
option(
  OT_STATIC_DEPENDENCIES
  "Link against static version of dependencies"
  ${OT_USE_VCPKG_TARGETS}
)
option(
  OT_BUNDLED_SECP256K1
  "Compile secp256k1 library as part of opentxs instead of linking to an externally compiled version"
  ${OT_CRYPTO_USING_LIBSECP256K1}
)
option(
  OT_BOOST_JSON_HEADER_ONLY
  "Compile boost::json library as part of opentxs instead of linking to an externally compiled version"
  ON
)
option(
  OT_INSTALL_HEADERS
  "Packing option to control whether or not headers are installed"
  ON
)
option(
  OT_INSTALL_CMAKE
  "Packing option to control whether or not cmake package configuration is installed"
  ON
)
option(
  OT_INSTALL_LICENSE
  "Packing option to control whether or not license file is installed"
  ON
)

if(NOT
   DEFINED
   OT_HEADER_INSTALL_PATH
)
  set(OT_HEADER_INSTALL_PATH "${CMAKE_INSTALL_INCLUDEDIR}")
endif()

if(NOT
   DEFINED
   OT_LIBRARY_INSTALL_PATH
)
  set(OT_LIBRARY_INSTALL_PATH "${CMAKE_INSTALL_LIBDIR}")
endif()

if(NOT
   DEFINED
   OT_CMAKE_INSTALL_PATH
)
  set(OT_CMAKE_INSTALL_PATH
      "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake"
  )
endif()

if(NOT
   DEFINED
   OT_LICENSE_FILE_NAME
)
  set(OT_LICENSE_FILE_NAME "LICENSE")
endif()

if(OT_SCRIPT_USING_CHAI
   AND (${CMAKE_CXX_COMPILER_ID}
        MATCHES
        Clang)
   AND (CMAKE_CXX_COMPILER_VERSION
        VERSION_GREATER
        13.99)
)
  message(
    WARNING
      "clang-14 and above is not capable of compiling the ChaiScript library in c++20 mode."
  )
  set(OT_SCRIPT_USING_CHAI OFF)
endif()

if(OT_CRYPTO_SUPPORTED_KEY_RSA
   OR OT_CASH_USING_LUCRE
   OR OT_WITH_BLOCKCHAIN
)
  set(OT_CRYPTO_USING_OPENSSL_DEFAULT ON)
else()
  set(OT_CRYPTO_USING_OPENSSL_DEFAULT OFF)
endif()

option(
  OT_CRYPTO_USING_OPENSSL
  "Use OpenSSL crypto library"
  ${OT_CRYPTO_USING_OPENSSL_DEFAULT}
)

if(NOT OT_WITH_QT)
  if(OT_WITH_QML)
    message(WARNING "QML support requires Qt")
  endif()

  set(OT_WITH_QML OFF)
endif()

if(MSVC)
  set(OT_CRYPTO_USING_PACKETCRYPT OFF)
endif()

if(ANDROID)
  set(OT_INSTALL_LIBRARY_DEPENDENCIES_DEFAULT ON)
elseif(OT_USE_VCPKG_TARGETS)
  if("${VCPKG_TARGET_TRIPLET}"
     STREQUAL
     "x64-windows"
  )
    set(OT_INSTALL_LIBRARY_DEPENDENCIES_DEFAULT ON)
  endif()
else()
  set(OT_INSTALL_LIBRARY_DEPENDENCIES_DEFAULT OFF)
endif()

option(
  OT_INSTALL_LIBRARY_DEPENDENCIES
  "Install dependent libraries along with libopentxs"
  ${OT_INSTALL_LIBRARY_DEPENDENCIES_DEFAULT}
)
option(
  OT_ENABLE_RPC
  "Build RPC server"
  ${OT_ENABLE_RPC_DEFAULT}
)

if(OT_IWYU)
  find_program(opentxs_iwyu_path NAMES include-what-you-use REQUIRED)
  find_program(opentxs_fix_includes_path NAMES fix_includes.py REQUIRED)

  if(opentxs_iwyu_path)
    set(OPENTXS_IWYU_ARGS
        "${opentxs_iwyu_path}"
        "-Xiwyu"
        "--mapping_file=${CMAKE_CURRENT_SOURCE_DIR}/iwyu.imp"
    )
    set(CMAKE_C_INCLUDE_WHAT_YOU_USE "${OPENTXS_IWYU_ARGS}")
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${OPENTXS_IWYU_ARGS}")
  endif()

  if(opentxs_fix_includes_path)
    set(OPENTXS_FIX_INCLUDES_ARGS
        "${opentxs_fix_includes_path}"
        "-b"
        "--nocomments"
        "--nosafe_headers"
        "--reorder"
        "<"
        "${CMAKE_CURRENT_BINARY_DIR}/iwyu.txt"
    )
  else()
    set(OPENTXS_FIX_INCLUDES_ARGS "OFF")
  endif()

  add_custom_target(
    fix_includes
    COMMAND "${OPENTXS_FIX_INCLUDES_ARGS}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND_EXPAND_LISTS
  )
else()
  set(OPENTXS_IWYU_ARGS "OFF")
  set(OPENTXS_FIX_INCLUDES_ARGS "OFF")
endif()

if(OPENTXS_BUILD_TESTS
   AND NOT
       (CMAKE_BUILD_TYPE
        STREQUAL
        "Debug"
        OR CMAKE_BUILD_TYPE
           STREQUAL
           "RelWithDebInfo")
)
  message(FATAL_ERROR "opentxs unit tests require CMAKE_BUILD_TYPE=Debug")
endif()

if(OT_WITH_BLOCKCHAIN)
  if("${CMAKE_SIZEOF_VOID_P}"
     EQUAL
     "8"
  )
    if(ANDROID)
      set(OPENTXS_BLOCK_STORAGE_ENABLED 0)
      set(OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY 0)
    else()
      set(OPENTXS_BLOCK_STORAGE_ENABLED 1)
      set(OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY 1)
    endif()
  else()
    set(OPENTXS_BLOCK_STORAGE_ENABLED 0)
    set(OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY 0)
  endif()
endif()

# -----------------------------------------------------------------------------
# Set compiler options

include(libopentxs-configure-target)
otcommon_set_project_language_standard(
  99
  20
  ${OPENTXS_PEDANTIC_BUILD}
)

if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4217")
  set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4217")
endif()

# -----------------------------------------------------------------------------
# Print system information and build options

include(libopentxs-print-build-details)
libopentxs_print_build_details()

# -----------------------------------------------------------------------------
# Dependencies

if(OT_STATIC_DEPENDENCIES)
  set(Protobuf_USE_STATIC_LIBS ON)
  set(OT_ZMQ_TARGET "libzmq-static")
  set(OPENSSL_USE_STATIC_LIBS ON)
else()
  set(Protobuf_USE_STATIC_LIBS OFF)
  set(OT_ZMQ_TARGET "libzmq")
  set(OPENSSL_USE_STATIC_LIBS OFF)
endif()

otcommon_find_system_libraries()

if(OPENTXS_BUILD_TESTS)
  if(OT_USE_VCPKG_TARGETS)
    find_package(
      GTest
      1.11.0
      CONFIG
      REQUIRED
    )
  else()
    find_package(
      GTest
      1.11.0
      REQUIRED
    )
  endif()
  enable_testing()
endif()

include(libopentxs-find-dependencies)

string(
  REGEX
    MATCH
    "[0-9]*"
    OPENSSL_MAJOR
    "${OPENSSL_VERSION}"
)

if("${OPENSSL_MAJOR}"
   GREATER_EQUAL
   3
)
  if(OT_CASH_USING_LUCRE)
    message(
      WARNING
        "Lucre is not compatible with OpenSSL 3. You will receive runtime errors when attempting to use blinded instruments"
    )
  endif()
endif()

if(OT_CLANG_TIDY)
  find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)

  if(CLANG_TIDY_COMMAND)
    set(CMAKE_CXX_CLANG_TIDY
        "${CLANG_TIDY_COMMAND};-header-filter=${CMAKE_CURRENT_SOURCE_DIR}/(include|src|tests)/.*.(h|t)pp;-fix-errors;-extra-arg=-Wno-unknown-warning-option"
    )
  else()
    message(FATAL_ERROR "clang-tidy not found")
  endif()
endif()

# -----------------------------------------------------------------------------
# Source Definitions

if(CMAKE_BUILD_TYPE
   STREQUAL
   "Debug"
)
  set(OPENTXS_HIDDEN_SYMBOLS OFF)

  if(WIN32 AND BUILD_SHARED_LIBS)
    message(
      FATAL_ERROR
        "Building as a shared library in debug mode is impossible on windows due to linker limitations"
    )
  endif()
else()
  set(OPENTXS_HIDDEN_SYMBOLS ON)
endif()

if(OPENTXS_HIDDEN_SYMBOLS)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
else()
  set(CMAKE_VISIBILITY_INLINES_HIDDEN OFF)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Storage backends

if(OT_STORAGE_FS)
  set(FS_EXPORT 1)
else()
  set(FS_EXPORT 0)
endif()

if(OT_STORAGE_SQLITE)
  set(SQLITE_EXPORT 1)
else()
  set(SQLITE_EXPORT 0)
endif()

if(OT_STORAGE_LMDB)
  set(LMDB_EXPORT 1)
else()
  set(LMDB_EXPORT 0)
endif()

if((NOT OT_STORAGE_FS)
   AND (NOT OT_STORAGE_SQLITE)
   AND (NOT OT_STORAGE_LMDB)
)
  message(FATAL_ERROR "At least one storage backend must be defined.")
endif()

# Key types

if(NOT OT_CRYPTO_SUPPORTED_KEY_ED25519)
  set(ED25519_EXPORT 0)
  if(NOT OT_CRYPTO_SUPPORTED_KEY_RSA)
    if(NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
      message(FATAL_ERROR "At least one type of key must be enabled")
    endif()
  endif()
else()
  set(ED25519_EXPORT 1)
endif()

if(OT_CRYPTO_SUPPORTED_KEY_RSA)
  if(NOT OT_CRYPTO_USING_OPENSSL)
    message(FATAL_ERROR "RSA support requires OpenSSL")
  endif()
  set(RSA_EXPORT 1)
else()
  set(RSA_EXPORT 0)
endif()

if(OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
  if(NOT OT_CRYPTO_USING_LIBSECP256K1)
    message(FATAL_ERROR "secp256k1 key support requires libsecp256k1")
  endif()
  set(SECP256K1_EXPORT 1)
else()
  set(SECP256K1_EXPORT 0)
endif()

# Crypto libraries

if(OT_CRYPTO_USING_OPENSSL)
  set(OPENSSL_EXPORT 1)
else()
  set(OPENSSL_EXPORT 0)
endif()

if(OT_CRYPTO_USING_LIBSECP256K1)
  if(OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    set(LIBSECP256K1_EXPORT 1)
  else()
    set(LIBSECP256K1_EXPORT 0)
    message(WARNING "Libsecp256k1 is pointless without secp256k1 keys.")
  endif()
else()
  set(LIBSECP256K1_EXPORT 0)
endif()

if(OT_CRYPTO_USING_PACKETCRYPT)
  if(OT_WITH_BLOCKCHAIN)
    set(PACKETCRYPT_EXPORT 1)
  else()
    set(PACKETCRYPT_EXPORT 0)
    message(WARNING "packetcrypt is pointless without blockchain support.")
  endif()
else()
  set(PACKETCRYPT_EXPORT 0)
endif()

if(OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
  set(BIP32_EXPORT 1)
else()
  set(BIP32_EXPORT 0)
endif()

if(OT_WITH_BLOCKCHAIN)
  if(NOT OT_STORAGE_LMDB)
    message(FATAL_ERROR "Blockchain support requires LMDB")
  endif()

  if(NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    message(FATAL_ERROR "Blockchain support requires secp256k1 key support")
  endif()

  if(NOT BIP32_EXPORT)
    message(FATAL_ERROR "Blockchain support requires BIP32 support")
  endif()

  if(NOT OT_CRYPTO_USING_OPENSSL)
    message(FATAL_ERROR "Blockchain support requires OpenSSL")
  endif()

  set(OT_BLOCKCHAIN_EXPORT 1)
else()
  set(OT_BLOCKCHAIN_EXPORT 0)
endif()

if(OT_CASH_USING_LUCRE)
  if(NOT OT_CRYPTO_USING_OPENSSL)
    message(FATAL_ERROR "Lucre support requires OpenSSL")
  endif()

  set(CASH_LUCRE_EXPORT 1)
else()
  set(CASH_LUCRE_EXPORT 0)
endif()

if(FALSE)
  set(CASH_MM_EXPORT 1)
else()
  set(CASH_MM_EXPORT 0)
endif()

# Script engines
if(OT_SCRIPT_USING_CHAI)
  set(SCRIPT_CHAI_EXPORT 1)
else()
  set(SCRIPT_CHAI_EXPORT 0)
endif()

if(OT_WITH_QT)
  set(OT_QT_EXPORT 1)
else()
  set(OT_QT_EXPORT 0)
endif()

if(OT_WITH_QML)
  set(OT_QML_EXPORT 1)
else()
  set(OT_QML_EXPORT 0)
endif()

# -----------------------------------------------------------------------------
# Build source

include(libopentxs-generate-export-macro)
include(libopentxs-target-helpers)
libopentxs_generate_export_macro()

add_custom_target(generated_code)

if(OPENTXS_STANDALONE)
  # library type controlled by BUILD_SHARED_LIBS
  add_library(libopentxs "")
else()
  add_library(libopentxs STATIC "")
  target_include_directories(
    libopentxs INTERFACE $<BUILD_INTERFACE:${opentxs_BINARY_DIR}/src>
                         $<BUILD_INTERFACE:${opentxs_SOURCE_DIR}/deps>
  )
  target_compile_definitions(libopentxs INTERFACE "${OT_IMPORT_MACRO}")

  if(OPENTXS_BUILD_TESTS)
    message(WARNING "tests can only be built in standalone mode")
  endif()
endif()

target_sources(
  libopentxs
  PUBLIC
    FILE_SET
    HEADERS
    BASE_DIRS
    "${opentxs_SOURCE_DIR}/include"
    "${opentxs_BINARY_DIR}/include"
)
add_library(
  opentxs::libopentxs
  ALIAS
  libopentxs
)
set_target_properties(
  libopentxs
  PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    VERSION
    "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}"
    SOVERSION 1
)

add_library(opentxs-common OBJECT "")
otcommon_pch_pic_workaround(libopentxs)
otcommon_pch_pic_workaround(opentxs-common)
libopentxs_configure_cxx_target(libopentxs)
libopentxs_configure_cxx_target(opentxs-common)
target_include_directories(
  opentxs-common PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include"
                        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_compile_definitions(opentxs-common PRIVATE "${OT_EXPORT_MACRO}")
add_dependencies(opentxs-common generated_code)

include(libopentxs-add-public-header)

if(OPENTXS_BUILD_TESTS)
  add_library(opentxs-testlib "")
  otcommon_pch_pic_workaround(opentxs-testlib)
  libopentxs_configure_cxx_target(opentxs-testlib)
  target_compile_definitions(opentxs-testlib PRIVATE "${OT_EXPORT_MACRO}")
  target_compile_definitions(opentxs-testlib INTERFACE "${OT_IMPORT_MACRO}")
  target_link_libraries(opentxs-testlib PUBLIC opentxs-common)
endif()

add_subdirectory(generated)
add_subdirectory(deps)
add_subdirectory(src)

if(OPENTXS_BUILD_TESTS AND OPENTXS_STANDALONE)
  include(CTest)
  add_subdirectory(tests)
endif()

otcommon_print_target_compiler_flags(opentxs-common)

# -----------------------------------------------------------------------------
# Package

if(OPENTXS_STANDALONE)
  include(libopentxs-configure-cpack)
  otcommon_generate_pkgconfig("-lopentxs")
  otcommon_generate_uninstall_target()
  libopentxs_configure_cpack()
endif()
