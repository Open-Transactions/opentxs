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
list(
  APPEND
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake"
)
include(libopentxs-set-osx-deployment-target)
libopentxs_set_osx_deployment_target(13.4)
project(opentxs)

# -----------------------------------------------------------------------------
# Setup

find_package(
  otcommon
  2
  CONFIG
  REQUIRED
)
otcommon_set_version_from_git()
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

if(WIN32)
  set(OT_USE_PSTL_DEFAULT ON)
  set(OT_PSTL_NEEDS_TBB_DEFAULT OFF)
  set(OT_WITH_TBB_DEFAULT OFF)
  set(OT_BOOST_STACKTRACE_ADDR2LINE_DEFAULT OFF)
elseif(
  UNIX
  AND NOT APPLE
  AND NOT ANDROID
)
  set(OT_BOOST_STACKTRACE_ADDR2LINE_DEFAULT ON)

  if(CMAKE_CXX_COMPILER_ID
     MATCHES
     Clang
     AND CMAKE_CXX_COMPILER_VERSION
         VERSION_LESS
         16.0.0
  )
    set(OT_USE_PSTL_DEFAULT OFF)
    set(OT_PSTL_NEEDS_TBB_DEFAULT OFF)
    set(OT_WITH_TBB_DEFAULT OFF)
  else()
    set(OT_USE_PSTL_DEFAULT ON)
    set(OT_PSTL_NEEDS_TBB_DEFAULT ON)
    set(OT_WITH_TBB_DEFAULT OFF)
  endif()
else()
  set(OT_USE_PSTL_DEFAULT OFF)
  set(OT_PSTL_NEEDS_TBB_DEFAULT OFF)
  set(OT_WITH_TBB_DEFAULT ON)
endif()

if(NOT
   DEFINED
   BOOST_STACKTRACE_ADDR2LINE_LOCATION
)
  set(BOOST_STACKTRACE_ADDR2LINE_LOCATION "/usr/bin/addr2line")
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
  OT_USE_PSTL
  "Use standard library concurrent algorithms"
  ${OT_USE_PSTL_DEFAULT}
)
option(
  OT_WITH_TBB
  "Use TBB concurrent algorithms"
  ${OT_WITH_TBB_DEFAULT}
)
option(
  OT_PSTL_NEEDS_TBB
  "Set to ON if standard library must link TBB for parallel algorithms (libstdc++)"
  ${OT_PSTL_NEEDS_TBB_DEFAULT}
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
  OT_BOOST_JSON_HEADER_ONLY
  "Compile boost::json library as part of opentxs instead of linking to an externally compiled version"
  ON
)
option(
  OT_BOOST_STACKTRACE_ADDR2LINE
  "Configure boost::stacktrace to use addr2line for line numbers"
  ${OT_BOOST_STACKTRACE_ADDR2LINE_DEFAULT}
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

if(OT_USE_PSTL AND OT_WITH_TBB)
  message(
    FATAL_ERROR "At most one of OT_USE_PSTL or OT_WITH_TBB can be enabled"
  )
endif()

if(OT_PSTL_NEEDS_TBB AND NOT OT_USE_PSTL)
  message(
    WARNING
      "Not linking TBB for standard library parallel algorithms since OT_USE_PSTL is set to OFF"
  )
  set(OT_PSTL_NEEDS_TBB OFF)
endif()

if(CMAKE_CXX_COMPILER_ID
   MATCHES
   Clang
   AND CMAKE_CXX_COMPILER_VERSION
       VERSION_LESS
       16.0.0
)
  if(OT_USE_PSTL)
    message(
      FATAL_ERROR
        "OT_USE_PSTL can not be used with versions of clang less than 16 due to missing ranges support"
    )
  endif()
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

option(
  OT_ENABLE_NONFREE
  "Enable optional features whose use may be subject to additional terms and conditions by third parties"
  OFF
)

option(
  OT_ENABLE_MATTERFI
  "Enable optional features contributed by MatterFi, Inc."
  OFF
)

if(OT_ENABLE_MATTERFI AND NOT OT_ENABLE_NONFREE)
  message(
    FATAL_ERROR
      "You must enable OT_ENABLE_NONFREE in order to enable OT_ENABLE_MATTERFI"
  )
endif()

if(OT_IWYU)
  otcommon_make_iwyu_target()
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
        "${CLANG_TIDY_COMMAND};-header-filter=(${CMAKE_CURRENT_SOURCE_DIR}|${CMAKE_CURRENT_BINARY_DIR})/(generated|include|src|tests)/.*.(h|t)pp;-fix-errors;-extra-arg=-Wno-unknown-warning-option;--warnings-as-errors=*"
    )
  else()
    message(FATAL_ERROR "clang-tidy not found")
  endif()
endif()

# -----------------------------------------------------------------------------
# Source Definitions

if(OPENTXS_BUILD_TESTS)
  set(OPENTXS_HIDDEN_SYMBOLS OFF)
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

  if(WIN32 AND BUILD_SHARED_LIBS)
    message(
      FATAL_ERROR
        "Building as a shared library without hidden symbols is impossible on windows due to linker limitations"
    )
  endif()
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
define_property(TARGET PROPERTY OPENTXS_PROTOBUF_FILES)

if(OPENTXS_STANDALONE)
  # library type controlled by BUILD_SHARED_LIBS
  add_library(libopentxs "")
else()
  add_library(libopentxs STATIC "")
  target_include_directories(
    libopentxs INTERFACE $<BUILD_INTERFACE:${opentxs_BINARY_DIR}/src>
                         $<BUILD_INTERFACE:${opentxs_SOURCE_DIR}/external>
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
    OPENTXS_PROTOBUF_FILES ""
)
set_property(
  TARGET libopentxs
  APPEND
  PROPERTY EXPORT_PROPERTIES OPENTXS_PROTOBUF_FILES
)

define_property(TARGET PROPERTY QT_MOC_PUBLIC)
define_property(TARGET PROPERTY QT_MOC_PRIVATE)
add_library(opentxs-common OBJECT "")
set_target_properties(
  opentxs-common PROPERTIES QT_MOC_PUBLIC "" QT_MOC_PRIVATE ""
)
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

include(libopentxs-add-enum-header)
include(libopentxs-add-platform-specific)
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
add_subdirectory(data)
add_subdirectory(external)
add_subdirectory(src)

if(OPENTXS_BUILD_TESTS AND OPENTXS_STANDALONE)
  include(CTest)
  add_subdirectory(tests)
endif()

otcommon_print_target_compiler_flags(opentxs-common)

# -----------------------------------------------------------------------------
# Package

if(OPENTXS_STANDALONE)
  include(libopentxs-generate-pkgconfig)
  include(libopentxs-generate-uninstall-target)
  include(libopentxs-configure-cpack)
  libopentxs_generate_pkgconfig("-lopentxs")
  libopentxs_generate_uninstall_target()
  libopentxs_configure_cpack()
endif()
