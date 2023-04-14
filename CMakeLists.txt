# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.21.0)
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
)
list(
  APPEND
  CMAKE_MODULE_PATH
  "${${PROJECT_NAME}_SOURCE_DIR}"
)

if("${CMAKE_PROJECT_NAME}"
   STREQUAL
   "${PROJECT_NAME}"
)
  set(OPENTXS_STANDALONE ON)
  message(STATUS "Configuring Open-Transactions as a library")
else()
  set(OPENTXS_STANDALONE OFF)
  message(STATUS "Configuring Open-Transactions as a subproject")
endif()

# -----------------------------------------------------------------------------
# Setup

include(open-transactions-common)

set_version_from_git()
set_build_type()
set_out_of_source_build()
update_git_submodules()
set_rpath()

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
  set(OPENTXS_BUILD_TESTS_DEFAULT ON)
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

# -----------------------------------------------------------------------------
# Set compiler options

set_compiler_flags(20 ${OPENTXS_PEDANTIC_BUILD})

# temporarily disabled warnings
if(MSVC)
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} /wd4244 /wd4251 /wd4267 /wd4275 /wd4996"
  )
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4217")
  set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4217")
else()
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wno-c++20-compat -Wno-deprecated-declarations -Wno-implicit-int-conversion -Wno-noexcept -Wno-return-std-move-in-c++11 -Wno-sign-conversion -Wno-switch-enum -Wno-unused-parameter"
  )
endif()

if(CMAKE_CXX_COMPILER_ID
   MATCHES
   GNU
)
  # too many false positives, mostly in standard library and third party headers
  # we can't do anything about anyway
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wno-error=null-dereference -Wno-error=stringop-overflow= -Wno-error=stringop-overread"
  )
endif()

if(CMAKE_CXX_COMPILER_ID
   MATCHES
   Clang
   AND CMAKE_CXX_COMPILER_VERSION
       VERSION_GREATER_EQUAL
       16.0.0
)
  # clang-16 erroneously tags system headers with this warning on some platforms
  # ~~~
  # In file included from <built-in>:1:
  # /usr/include/gentoo/fortify.h:3:11: error: macro name is a reserved identifier [-Werror,-Wreserved-macro-identifier]
  # #  define __GENTOO_HAS_FEATURE(x) __has_feature(x)
  #           ^
  # /usr/include/gentoo/fortify.h:13:9: error: macro name is a reserved identifier [-Werror,-Wreserved-macro-identifier]
  # # undef __GENTOO_HAS_FEATURE
  #         ^
  # ~~~
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reserved-macro-identifier")
  # too many false positives and hits in 3rd party & generated code
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unsafe-buffer-usage")
endif()

if(CMAKE_CXX_COMPILER_ID
   MATCHES
   Clang
   AND CMAKE_CXX_COMPILER_VERSION
       VERSION_LESS
       14.0.0
)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reserved-identifier")
endif()

if(CMAKE_CXX_COMPILER_ID
   MATCHES
   AppleClang
)
  # NOTE when AppleClang rewrites operator<=> it generates code that causes this
  # warning to trigger
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-zero-as-null-pointer-constant")
endif()

# -----------------------------------------------------------------------------
# Print system information and build options

print_build_details(OPENTXS_PEDANTIC_BUILD OPENTXS_BUILD_TESTS)

message(STATUS "Storage backends-----------------------------")
message(STATUS "filesystem:               ${OT_STORAGE_FS}")
message(STATUS "sqlite                    ${OT_STORAGE_SQLITE}")
message(STATUS "LMDB                      ${OT_STORAGE_LMDB}")

message(STATUS "Key algorithms-------------------------------")
message(STATUS "ed25519:                  ${OT_CRYPTO_SUPPORTED_KEY_ED25519}")
message(STATUS "RSA:                      ${OT_CRYPTO_SUPPORTED_KEY_RSA}")
message(STATUS "secp256k1                 ${OT_CRYPTO_SUPPORTED_KEY_SECP256K1}")

message(STATUS "Crypto library providers---------------------")
message(STATUS "OpenSSL:                  ${OT_CRYPTO_USING_OPENSSL}")
message(STATUS "libsecp256k1:             ${OT_CRYPTO_USING_LIBSECP256K1}")
message(STATUS "packetcrypt:              ${OT_CRYPTO_USING_PACKETCRYPT}")

message(STATUS "Blockchain-----------------------------------")
message(STATUS "Blockchain client:        ${OT_WITH_BLOCKCHAIN}")

message(STATUS "Cash library providers-----------------------")
message(STATUS "Lucre:                    ${OT_CASH_USING_LUCRE}")
message(STATUS "Lucre debug info:         ${OT_LUCRE_DEBUG}")

message(STATUS "Script engines-------------------------------")
message(STATUS "Chai:                     ${OT_SCRIPT_USING_CHAI}")

message(STATUS "Interface------------------------------------")
message(STATUS "Qt:                       ${OT_WITH_QT}")
message(STATUS "QML:                      ${OT_WITH_QML}")
message(STATUS "RPC:                      ${OT_ENABLE_RPC}")

message(STATUS "Developer -----------------------------------")
message(STATUS "Valgrind support:         ${OT_VALGRIND}")
message(STATUS "precompiled headers:      ${OT_PCH}")
message(STATUS "iwyu:                     ${OPENTXS_IWYU_ARGS}")
message(STATUS "fix_includes:             ${OPENTXS_FIX_INCLUDES_ARGS}")
message(STATUS "clang-tidy:               ${OT_CLANG_TIDY}")

if(OT_IWYU)
  message(STATUS "")
  message(STATUS "iwyu instructions:")
  message(
    STATUS
      "  1. Save build output when compiling: cmake --build . -- -k 0 | tee iwyu.txt"
  )
  message(
    STATUS
      "  2. Run the fix_includes target to apply automatic fixes: cmake --build . --target fix_includes"
  )
  message(STATUS "")
  message(STATUS "fix_includes caveats (requires manual editing to fix):")
  message(
    STATUS
      "  1. iwyu always suggests pre-c++11 standard library paths. (<string.h> vs <cstring>)"
  )
  message(
    STATUS
      "  2. iwyu will sometimes suggest bracket includes when we want quote includes."
  )
  message(
    STATUS
      "  3. iwyu will sometimes suggest platform-specific headers instead of standard headers. (<bits/exception.h> vs <stdexcept>) Add new cases to iwyu.imp when they occur."
  )
  message(
    STATUS
      "  4. iwyu will suggest forward declarations simultaneously in source files and their associated headers. cpp files should not have forward declarations."
  )
  message(
    STATUS
      "  5. The way iwyu sorts forward declarations is cursed. Sort namespaces before types and add one blank line between two namespaces at the same level or between the last namespace and the types."
  )
  message(STATUS "  6. Run clang-format on any files that fix_includes edited.")
  message(STATUS "")
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

find_system_libraries()

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

find_package(Threads REQUIRED)

if(WIN32)
  find_package(pthread REQUIRED)
endif()

find_package(unofficial-sodium REQUIRED)
find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)
find_package(
  Boost
  1.75.0
  REQUIRED
  COMPONENTS
    iostreams
    json
    program_options
    system
    thread
  OPTIONAL_COMPONENTS stacktrace_basic
)

if(OT_USE_VCPKG_TARGETS)
  find_package(ZeroMQ REQUIRED)
else()
  find_package(unofficial-zeromq REQUIRED)
endif()

if(OT_WITH_BLOCKCHAIN)
  if("${CMAKE_SIZEOF_VOID_P}"
     EQUAL
     "8"
  )
    if(ANDROID OR WIN32)
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

if(OT_STORAGE_SQLITE)
  find_package(SQLite3 REQUIRED)
endif()

if(OT_STORAGE_LMDB)
  find_package(lmdb REQUIRED)
endif()

if(CMAKE_MSVC_RUNTIME_LIBRARY
   STREQUAL
   "MultiThreaded"
)
  set(OPENSSL_MSVC_STATIC_RT ON)
endif()

find_package(
  OpenSSL
  1.0.1
  REQUIRED
)

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

if(OT_CRYPTO_USING_LIBSECP256K1 AND NOT OT_BUNDLED_SECP256K1)
  find_package(unofficial-secp256k1 REQUIRED)
endif()

if(OT_WITH_QT)
  find_package(
    QT
    NAMES
    Qt6
    Qt5
    COMPONENTS Core Gui
    REQUIRED
  )
  find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Core Gui
    REQUIRED
  )

  if(OT_WITH_QML)
    find_package(
      Qt${QT_VERSION_MAJOR}
      COMPONENTS Qml
      REQUIRED
    )
  endif()
endif()

find_package(TBB REQUIRED)

# -----------------------------------------------------------------------------
# Source Definitions

set_common_defines()

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

if(APPLE)
  add_definitions(-DBOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED)
endif()

if(OT_VALGRIND)
  add_definitions(-DOT_VALGRIND=1)
else()
  add_definitions(-DOT_VALGRIND=0)
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

if(WIN32)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# -----------------------------------------------------------------------------
# Build source

include_directories(
  "${opentxs_BINARY_DIR}/include"
  "${opentxs_BINARY_DIR}/src"
  "${opentxs_SOURCE_DIR}/include"
  "${opentxs_SOURCE_DIR}/src"
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_custom_target(generated_code)

if(OPENTXS_STANDALONE)
  # library type controlled by BUILD_SHARED_LIBS
  add_library(opentxs "")

  if(BUILD_SHARED_LIBS)
    set(OT_EXPORT_MACRO "opentxs_EXPORTS")
    set(OT_IMPORT_MACRO "")
  else()
    set(OT_EXPORT_MACRO "OPENTXS_STATIC_DEFINE")
    set(OT_IMPORT_MACRO "OPENTXS_STATIC_DEFINE")
  endif()
else()
  add_library(opentxs STATIC "")
  target_include_directories(
    opentxs
    INTERFACE
      $<BUILD_INTERFACE:${opentxs_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${opentxs_BINARY_DIR}/src>
      $<BUILD_INTERFACE:${opentxs_SOURCE_DIR}/deps>
      $<BUILD_INTERFACE:${opentxs_SOURCE_DIR}/include>
  )
  set(OT_EXPORT_MACRO "OPENTXS_STATIC_DEFINE")
  set(OT_IMPORT_MACRO "OPENTXS_STATIC_DEFINE")
  target_compile_definitions(opentxs INTERFACE "${OT_IMPORT_MACRO}")

  if(OPENTXS_BUILD_TESTS)
    message(WARNING "tests can only be built in standalone mode")
  endif()
endif()

add_library(
  opentxs::libopentxs
  ALIAS
  opentxs
)
add_library(opentxs-common OBJECT "")
target_compile_definitions(opentxs-common PRIVATE "${OT_EXPORT_MACRO}")

# defined signed overflow
if(MSVC)
  # TODO target_compile_options(opentxs-common PRIVATE "/d2UndefIntOverflow-")
else()
  target_compile_options(opentxs-common PRIVATE "-fwrapv")
endif()

if(OPENTXS_BUILD_TESTS)
  add_library(opentxs-testlib "")
  target_compile_definitions(opentxs-testlib PRIVATE "${OT_EXPORT_MACRO}")
  target_compile_definitions(opentxs-testlib INTERFACE "${OT_IMPORT_MACRO}")
endif()

macro(opentxs_include_directories DIR)
  target_include_directories(opentxs-common SYSTEM PRIVATE "${DIR}")

  if(OPENTXS_BUILD_TESTS)
    target_include_directories(opentxs-testlib SYSTEM PRIVATE "${DIR}")
  endif()
endmacro()

macro(opentxs_link_internal LIB)
  target_link_libraries(opentxs-common PRIVATE "${LIB}")

  if(OPENTXS_BUILD_TESTS)
    target_link_libraries(opentxs-testlib PRIVATE "${LIB}")
  endif()
endmacro()

macro(opentxs_link_external LIB)
  target_link_libraries(opentxs PUBLIC "${LIB}")

  if(OPENTXS_BUILD_TESTS)
    target_link_libraries(opentxs-testlib PUBLIC "${LIB}")
  endif()
endmacro()

macro(opentxs_add_sources SOURCES)
  target_sources(opentxs PRIVATE "${SOURCES}")

  if(OPENTXS_BUILD_TESTS)
    target_sources(opentxs-testlib PRIVATE "${SOURCES}")
  endif()
endmacro()

add_subdirectory(generated)
add_subdirectory(deps)
add_subdirectory(src)

if(OPENTXS_BUILD_TESTS AND OPENTXS_STANDALONE)
  include(CTest)
  add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Package

if(OPENTXS_STANDALONE)
  generate_cmake_files()
  generate_pkgconfig("-lopentxs")
  generate_uninstall_target()
endif()

if(OPENTXS_STANDALONE)
  include(InstallRequiredSystemLibraries)

  set(CPACK_NSIS_MODIFY_PATH "ON")

  if(WIN32)
    set(CPACK_PACKAGE_EXECUTABLES
        "..\\\\Uninstall;Uninstall ${PROJECT_NAME}  v${PROJECT_VERSION}"
    )

    set(CPACK_NSIS_MENU_LINKS "https://opentransactions.org"
                              "Open Transactions Link"
    )
  endif()

  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL, "ON")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
  set(CPACK_PACKAGE_VENDOR "http://www.OpenTransactions.org")
  set(CPACK_PACKAGE_VERSION_MAJOR "${${PROJECT_NAME}_VERSION_MAJOR}")
  set(CPACK_PACKAGE_VERSION_MINOR "${${PROJECT_NAME}_VERSION_MINOR}")
  set(CPACK_PACKAGE_VERSION_PATCH "${${PROJECT_NAME}_VERSION_PATCH}")

  # set( CPACK_PACKAGE_ICON " ")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
      "The Open-Transactions project is a collaborative effort to develop a robust, commercial-grade, fully-featured, free-software toolkit implementing the OTX protocol as well as a full-strength financial cryptography library, API, CLI, and prototype server. The project is managed by a worldwide community of volunteers that use the Internet to communicate, plan, and develop the Open-Transactions toolkit and its related documentation. ${${PROJECT_NAME}_GIT_VERSION}"
  )
  # set( CPACK_PACKAGE_CHECKSUM "" )
  set(CPACK_PACKAGE_HOMEPAGE_URL "https://opentransactions.org/wiki/Main_Page")
  set(CPACK_THREADS 0)
  include(CPack)
endif()
