# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 2.8.12)
project(opentxs)


set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM LAST)
find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND git "describe" OUTPUT_VARIABLE GIT_VERSION WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "^([0-9]+)\\..*" "\\1" VERSION_MAJOR "${GIT_VERSION}")
    string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${GIT_VERSION}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_BUGFIX "${GIT_VERSION}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+-([0-9]+)-.*" "\\1" VERSION_NEW_COMMITS "${GIT_VERSION}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+-[0-9]+-(.*)" "\\1" VERSION_SHA1 "${GIT_VERSION}")
    if("${VERSION_SHA1}" STREQUAL "")
        set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUGFIX}")
    else()
        set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUGFIX}-${VERSION_NEW_COMMITS}-${VERSION_SHA1}")
    endif()
    if ("${VERSION_STRING}" STREQUAL "..")
        message(FATAL_ERROR "Version string missing. Did you git clone using the --depth parameter?")
    endif()
else()
  message(FATAL_ERROR "Git not found.")
endif()


#-----------------------------------------------------------------------------
# Options for building

option(BUILD_VERBOSE       "Verbose build output." ON)
option(BUILD_DOCUMENTATION "Build the Doxygen documentation." ON)
option(BUILD_TESTS         "Build the unit tests." ON)
option(USE_CCACHE          "Use ccache." OFF)
option(OT_STRICT           "Use pedantic compiler options." ON)

option(BUILD_SHARED_LIBS   "Build shared libraries." ON)

option(RPM                 "Build a RPM" OFF)
option(DEB                 "Build a DEB" OFF)
set(PACKAGE_CONTACT        ""                             CACHE <TYPE>  "Package Maintainer")
set(PACKAGE_VENDOR         "Open Transactions Developers" CACHE <TYPE>  "Package Vendor")
option(AUTOCOMPLETION      "CL autocompletion to opentxs CL client" OFF)

option(SIGNAL_HANLDER      "Enable Signal Handler" OFF)

option(KEYRING_WINDOWS     "Build with Windows Keyring" OFF)
option(KEYRING_MAC         "Build with Mac OSX Keyring" OFF)
option(KEYRING_IOS         "Build with iOS Keyring" OFF)
option(KEYRING_GNOME       "Build with Gnome Keyring" OFF)
option(KEYRING_KWALLET     "Build with KWallet Keyring" OFF)
option(KEYRING_FLATFILE    "Build with Flatfile Keyring" OFF)
option(NO_PASSWORD         "Use default password for all prompts" OFF)

option(OT_DHT    "Enable OpenDHT support" OFF)

option(OT_STORAGE_FS       "Use filesystem backend for storage" OFF)
option(OT_STORAGE_SQLITE   "Use sqlite backend for storage" ON)

option(OT_CRYPTO_SUPPORTED_ALGO_AES     "Enable AES encryption algorithm" ON)

option(OT_CRYPTO_SUPPORTED_SOURCE_BIP47     "Enable support for BIP-47 nyms" ON)

option(OT_CRYPTO_SUPPORTED_KEY_HD     "Enable HD key derivation support" ON)
option(OT_CRYPTO_SUPPORTED_KEY_ED25519     "Enable ed25519 key support" ON)
option(OT_CRYPTO_SUPPORTED_KEY_RSA     "Enable RSA key support" OFF)
option(OT_CRYPTO_SUPPORTED_KEY_SECP256K1 "Enable secp256k1 key support" ON)

option(OT_CRYPTO_USING_OPENSSL       "Use OpenSSL crypto implementation" ON)
option(OT_CRYPTO_USING_LIBSECP256K1  "Use libsecp256k1 crypto implementation" ON)
option(OT_CRYPTO_USING_TREZOR  "Use trezor crypto implementation" ON)

option(OT_CRYPTO_SHA2_PREFER_OPENSSL       "Use OpenSSL SHA2 implementation when available" OFF)

option(OT_SCRIPT_USING_CHAI  "Use chaiscript" ON)

option(OT_INTERNAL_ZMQ       "Use bundled version of ZeroMQ and czmq" OFF)
option(OT_INTERNAL_LIBSECP   "Use bundled version of libsecp256k1" OFF)

# SWIG Bindings
option(JAVA    "Build with Java binding" OFF)
option(PERL    "Build with Perl binding" OFF)
option(PHP     "Build with PHP binding" OFF)
option(PYTHON3 "Build with Python3 binding" OFF)
option(PYTHON2 "Build with Python2 binding" OFF)
option(PYTHON  "Build with default Python binding" OFF)


if(BUILD_VERBOSE)
  set(CMAKE_VERBOSE_MAKEFILE true)
endif()

if(RPM OR DEB)
    if(NOT PACKAGE_CONTACT)
        message(FATAL_ERROR "Specify a PACKAGE_CONTACT to build packages.")
    endif()
endif()

if(ANDROID)
    #set(OT_INTERNAL_ZMQ ON CACHE BOOL "Use bundled version of ZeroMQ and czmq" FORCE)
endif()

#-----------------------------------------------------------------------------
# Print system information and build options

message(STATUS "General -------------------------------------")
message(STATUS "opentxs version:        ${VERSION_STRING}")
message(STATUS "CMake version:          ${CMAKE_VERSION}")
message(STATUS "System:                 ${CMAKE_SYSTEM}")
message(STATUS "Processor:              ${CMAKE_SYSTEM_PROCESSOR}")

message(STATUS "Verbose:                ${BUILD_VERBOSE}")
message(STATUS "Testing:                ${BUILD_TESTS}")
message(STATUS "Documentation:          ${BUILD_DOCUMENTATION}")
message(STATUS "Using ccache            ${USE_CCACHE}")
message(STATUS "Pedantic compilation:   ${OT_STRICT}")

message(STATUS "Packaging -----------------------------------")
message(STATUS "Build RPM:              ${RPM}")
message(STATUS "Build DEB:              ${DEB}")
message(STATUS "Package Contact:        ${PACKAGE_CONTACT}")
message(STATUS "Package Vendor:         ${PACKAGE_VENDOR}")

message(STATUS "CL autocompletion:      ${AUTOCOMPLETION}")

message(STATUS "Keyrings ------------------------------------")
message(STATUS "Keyring Windows:        ${KEYRING_WINDOWS}")
message(STATUS "Keyring MAC:            ${KEYRING_MAC}")
message(STATUS "Keyring iOS:            ${KEYRING_IOS}")
message(STATUS "Keyring Gnome:          ${KEYRING_GNOME}")
message(STATUS "Keyring KWallet:        ${KEYRING_KWALLET}")
message(STATUS "Keyring flatfile:       ${KEYRING_FLATFILE}")
message(STATUS "No password:            ${NO_PASSWORD}")

message(STATUS "Network plugins------------------------------")
message(STATUS "DHT:                    ${OT_DHT}")

message(STATUS "Storage backends-----------------------------")
message(STATUS "filesystem:             ${OT_STORAGE_FS}")
message(STATUS "sqlite                  ${OT_STORAGE_SQLITE}")

message(STATUS "Nym ID sources------------------------------")
message(STATUS "BIP-47:                 ${OT_CRYPTO_SUPPORTED_SOURCE_BIP47}")

message(STATUS "Encryption algorithms------------------------")
message(STATUS "AES:                    ${OT_CRYPTO_SUPPORTED_ALGO_AES}")

message(STATUS "Key algorithms-------------------------------")
message(STATUS "HD derivation:          ${OT_CRYPTO_SUPPORTED_KEY_HD}")
message(STATUS "ed25519:                ${OT_CRYPTO_SUPPORTED_KEY_ED25519}")
message(STATUS "RSA:                    ${OT_CRYPTO_SUPPORTED_KEY_RSA}")
message(STATUS "secp256k1               ${OT_CRYPTO_SUPPORTED_KEY_SECP256K1}")

message(STATUS "Crypto library providers---------------------")
message(STATUS "OpenSSL:                ${OT_CRYPTO_USING_OPENSSL}")
message(STATUS "libsecp256k1:           ${OT_CRYPTO_USING_LIBSECP256K1}")
message(STATUS "trezor-crypto:          ${OT_CRYPTO_USING_TREZOR}")

message(STATUS "Bindings ------------------------------------")
message(STATUS "Java binding:           ${JAVA}")
message(STATUS "Perl binding:           ${PERL}")
message(STATUS "PHP binding:            ${PHP}")
message(STATUS "Python3 binding:        ${PYTHON3}")
message(STATUS "Python2 binding:        ${PYTHON2}")
message(STATUS "Default Python binding: ${PYTHON}")

message(STATUS "Script engines-------------------------------")
message(STATUS "Chai:                   ${OT_SCRIPT_USING_CHAI}")

message(STATUS "Dependencies---------------------------------")
message(STATUS "Internal ZeroMQ:        ${OT_INTERNAL_ZMQ}")
message(STATUS "Internal Libsecp256k1:  ${OT_INTERNAL_LIBSECP}")

#-----------------------------------------------------------------------------
# Collect all binaries into bin subdirectory

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${CMAKE_CURRENT_BINARY_DIR}/lib)


#-----------------------------------------------------------------------------
# Check which compiler to use

if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
  INCLUDE(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG(-std=c++11 HAVE_STD11)
  if(HAVE_STD11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
    set(GCC_WARN_FLAGS "-Wnoexcept -Wstrict-null-sentinel -Wlogical-op")
  else()
    message(FATAL_ERROR "No advanced standard C++ support (-std=c++11 not defined).")
  endif()
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  set(CMAKE_COMPILER_IS_CLANGXX 1)
  INCLUDE(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG(-std=c++11 HAVE_STD11)
  if(HAVE_STD11)
    if(APPLE)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
      set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -stdlib=libc++")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
  else()
    message(FATAL_ERROR "No C++11 support for Clang version. Please upgrade Clang to a version supporting C++11.")
  endif()
endif()

if(BUILD_SHARED_LIBS)
  # Windows would complain about unknown flag -fPIC
  if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif()
endif()


if(WIN32)
  add_definitions(-D_UNICODE)
else()
  if(OT_STRICT)
    set(PEDANTIC_CXX_FLAGS "-Wall -Werror -Wextra -pedantic -Wno-missing-braces -Wcast-align -Wctor-dtor-privacy -Wdisabled-optimization -Winit-self -Wmissing-include-dirs -Wswitch-default -Wold-style-cast -Woverloaded-virtual -Wmissing-declarations -Wunused-macros -Wundef -Wformat=2 -Wno-format-nonliteral ${GCC_WARN_FLAGS}")
  else()
    set(PEDANTIC_CXX_FLAGS "")
  endif()
endif()

if(ANDROID)
  add_definitions(-D__STDC_LIMIT_MACROS)
endif()

message(STATUS "C++ Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Flags:       ${CMAKE_CXX_FLAGS}")
message(STATUS "C++ link flags:  ${CMAKE_CXX_LINK_FLAGS}")

#-----------------------------------------------------------------------------
# Get submodules

execute_process(COMMAND git "submodule" "update" "--init" "--recursive" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

#-----------------------------------------------------------------------------
# Third party libraries

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_definitions(-DPROTOBUF_INLINE_NOT_IN_HEADERS=0)
find_package(opentxs-proto REQUIRED)

if(APPLE)
message(STATUS "On Apple, you may have to use: brew install openssl ; brew link --force openssl")
  find_package(OpenSSL 1.0.1 REQUIRED)
else()
  find_package(OpenSSL 1.0.1 REQUIRED)
endif()
find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Sodium REQUIRED)
if(NOT OT_INTERNAL_ZMQ)
  find_package(czmq REQUIRED)
endif()
if(OT_CRYPTO_USING_LIBSECP256K1 AND OT_INTERNAL_LIBSECP)
  find_package(GMP REQUIRED)
endif()
if(OT_DHT)
  find_package(OPENDHT 0.6 REQUIRED)
  find_package(GnuTLS REQUIRED)
endif()
if(OT_STORAGE_SQLITE)
  find_package(SQLite3 REQUIRED)
endif()
if(OT_STORAGE_FS)
  find_package(Boost REQUIRED system)
  find_package(Boost REQUIRED filesystem)
endif()
if (NOT OT_INTERNAL_LIBSECP)
  find_package(Libsecp256k1 REQUIRED)
endif()

#-----------------------------------------------------------------------------
# System libraries used for linking.

if(UNIX AND NOT APPLE)
  list(APPEND OPENTXS_SYSTEM_LIBRARIES dl)
  if(ANDROID)
    #list(APPEND OPENTXS_SYSTEM_LIBRARIES log)
  else()
    list(APPEND OPENTXS_SYSTEM_LIBRARIES rt)
  endif()
endif()


#-----------------------------------------------------------------------------
# Force out-of-source build

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
message(FATAL_ERROR "CMake generation is not allowed within the source directory!
Remove the CMakeCache.txt file and try again from another folder, e.g.:

   rm CMakeCache.txt
   mkdir build
   cd build
   cmake ..
")
endif()

#-----------------------------------------------------------------------------
# Use ccache

if (USE_CCACHE)
  find_program(CCACHE "ccache")
  if (CCACHE)
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "CCACHE_CPP2=yes ${CCACHE}")
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE})
  endif(CCACHE)
endif (USE_CCACHE)

#-----------------------------------------------------------------------------
# Default to Release build

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()


#-----------------------------------------------------------------------------
# Build Unit tests

if(BUILD_TESTS AND NOT ANDROID)
  set(GTEST_ROOT ${opentxs_SOURCE_DIR}/deps/gtest)
  set(GTEST_FOUND ON)
  set(GTEST_INCLUDE_DIRS ${GTEST_ROOT}/include)
  set(GTEST_LIBRARY gtest)
  set(GTEST_MAIN_LIBRARY gtest_main)
  set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})

  enable_testing()
endif()


#-----------------------------------------------------------------------------
# Build Documentation

if(BUILD_DOCUMENTATION)
  set(DOC_GRAPHS "YES" CACHE STRING "Create dependency graphs (needs graphviz)")
  set(DOC_FULLGRAPHS "NO" CACHE STRING "Create call/callee graphs (large)")

  find_program(DOT_PATH dot)

  if (DOT_PATH STREQUAL "DOT_PATH-NOTFOUND")
    message("Doxygen: graphviz not found - graphs disabled")
    set(DOC_GRAPHS "NO")
  endif()

  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    configure_file("cmake/Doxyfile.in" "Doxyfile" @ONLY)
    configure_file("cmake/Doxygen.extra.css.in" "Doxygen.extra.css" @ONLY)
    add_custom_target(doc
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen.." VERBATIM)
  endif()
endif()


#-----------------------------------------------------------------------------
# Source Definitions

if(WIN32)
    if(NOT IDC_STATIC)
        add_definitions(-DIDC_STATIC=-1)
    endif()

    add_definitions(-DOT_COMPANYNAME="Open Transactions Community")
    add_definitions(-DOT_PRODUCTNAME="Open Transactions")
    add_definitions(-DOT_PRODUCTVERSION="${VERSION_MAJOR},${VERSION_MINOR},${VERSION_BUGFIX},0")
    add_definitions(-DOT_API_NAME="Open Transactions API")
    add_definitions(-DOT_LIBRARY_NAME="Open Transactions Library")
    add_definitions(-DOT_PROMPT_NAME="Open Transactions Prompt")
endif()

if(SIGNAL_HANLDER)
  add_definitions(-DOT_SIGNAL_HANDLING)
endif()

add_definitions(-DCHAISCRIPT_NO_THREADS)

if(ANDROID)
  add_definitions(-DOT_ANDROID)
endif()

#Network

if(OT_DHT)
  add_definitions(-DOT_DHT=1)
endif()

#Storage backends

if(OT_STORAGE_FS)
  add_definitions(-DOT_STORAGE_FS=1)
endif()

if(OT_STORAGE_SQLITE)
  add_definitions(-DOT_STORAGE_SQLITE=1)
endif()

if (OT_STORAGE_FS AND OT_STORAGE_SQLITE)
  message(FATAL_ERROR "Only one storage backend may be defined.")
endif()

if ((NOT OT_STORAGE_FS) AND (NOT OT_STORAGE_SQLITE))
  message(FATAL_ERROR "At least one storage backend must be defined.")
endif()

#Nym sources

if(OT_CRYPTO_SUPPORTED_SOURCE_BIP47)
  if (NOT OT_CRYPTO_SUPPORTED_KEY_HD)
    message(FATAL_ERROR "BIP-47 support requires HD key support")
  endif()
  if (NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    message(FATAL_ERROR "BIP-47 support requires secp256k1 key support")
  endif()
  set(BIP47_EXPORT 1)
else()
  set(BIP47_EXPORT 0)
endif()

#Encryption algorithms

if (OT_CRYPTO_SUPPORTED_ALGO_AES)
  if (NOT OT_CRYPTO_USING_OPENSSL)
    message(FATAL_ERROR "AES support requires OpenSSL")
  endif()
  set(AES_EXPORT 1)
else()
  set(AES_EXPORT 0)
endif()

#Key types

if(OT_CRYPTO_SUPPORTED_KEY_HD)
  if (NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    message(FATAL_ERROR "HD key support requires secp256k1 key support")
  endif()
  if (NOT OT_CRYPTO_USING_TREZOR)
    message(FATAL_ERROR "HD key support requires trezor-crypto")
  endif()
  set(HD_EXPORT 1)
else()
  set(HD_EXPORT 0)
endif()

if (NOT OT_CRYPTO_SUPPORTED_KEY_ED25519)
  set(ED25519_EXPORT 0)
  if (NOT OT_CRYPTO_SUPPORTED_KEY_RSA)
    if (NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
        message(FATAL_ERROR "At least one type of key must be enabled")
    endif()
  endif()
else()
  set(ED25519_EXPORT 1)
endif()

if (OT_CRYPTO_SUPPORTED_KEY_RSA)
  if (NOT OT_CRYPTO_USING_OPENSSL)
    message(FATAL_ERROR "RSA support requires OpenSSL")
  endif()
  set(RSA_EXPORT 1)
else()
  set(RSA_EXPORT 0)
endif()

if (OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
  if (NOT OT_CRYPTO_USING_LIBSECP256K1)
    message(FATAL_ERROR "secp256k1 key support requires libsecp256k1")
  endif()
  if (NOT OT_CRYPTO_USING_TREZOR)
    message(FATAL_ERROR "secp256k1 key support requires trezor-crypto")
  endif()
  set(SECP256K1_EXPORT 1)
else()
  set(SECP256K1_EXPORT 0)
endif()

#Crypto libraries

if(OT_CRYPTO_USING_OPENSSL)
  set(OPENSSL_EXPORT 1)
  if(OT_CRYPTO_SHA2_PREFER_OPENSSL)
    set(SHA2_VIA_OPENSSL_EXPORT 1)
  else()
    set(SHA2_VIA_OPENSSL_EXPORT 0)
  endif()
else()
  set(OPENSSL_EXPORT 0)
  set(SHA2_VIA_OPENSSL_EXPORT 0)
endif()

if(OT_CRYPTO_USING_LIBSECP256K1)
  if (OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    set(LIBSECP256K1_EXPORT 1)
  else()
    set(LIBSECP256K1_EXPORT 0)
    message(WARNING "Libsecp256k1 is pointless without secp256k1 keys.")
  endif()
else()
  set(LIBSECP256K1_EXPORT 0)
endif()

if(OT_CRYPTO_USING_TREZOR)
    set(TREZORCRYPTO_EXPORT 1)
  if (OT_CRYPTO_SUPPORTED_KEY_HD OR OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    set(BIP32_EXPORT 1)
    set(BIP39_EXPORT 1)
  else()
    set(BIP32_EXPORT 0)
    set(BIP39_EXPORT 0)
  endif()
else()
  set(TREZORCRYPTO_EXPORT 0)
  set(BIP32_EXPORT 0)
  set(BIP39_EXPORT 0)
  message(FATAL_ERROR "trezor-crypto support is required for base58 encoding")
endif()

add_definitions(-DOT_CRYPTO_SUPPORTED_SOURCE_BIP47=${BIP47_EXPORT})
add_definitions(-DOT_CRYPTO_SUPPORTED_ALGO_AES=${AES_EXPORT})
add_definitions(-DOT_CRYPTO_SUPPORTED_KEY_HD=${HD_EXPORT})
add_definitions(-DOT_CRYPTO_WITH_BIP32=${BIP32_EXPORT})
add_definitions(-DOT_CRYPTO_WITH_BIP39=${BIP39_EXPORT})
add_definitions(-DOT_CRYPTO_SUPPORTED_KEY_ED25519=${ED25519_EXPORT})
add_definitions(-DOT_CRYPTO_SUPPORTED_KEY_RSA=${RSA_EXPORT})
add_definitions(-DOT_CRYPTO_SUPPORTED_KEY_SECP256K1=${SECP256K1_EXPORT})
add_definitions(-DOT_CRYPTO_USING_OPENSSL=${OPENSSL_EXPORT})
add_definitions(-DOT_CRYPTO_USING_LIBSECP256K1=${LIBSECP256K1_EXPORT})
add_definitions(-DOT_CRYPTO_USING_TREZOR=${TREZORCRYPTO_EXPORT})
add_definitions(-DOT_CRYPTO_SHA2_VIA_OPENSSL=${SHA2_VIA_OPENSSL_EXPORT})
add_definitions(-DOT_CASH_USING_LUCRE)

#Keyrings
if(KEYRING_WINDOWS AND WIN32)
  add_definitions(-DOT_KEYRING_WINDOWS)
endif()

if(KEYRING_MAC AND APPLE)
  add_definitions(-DOT_KEYRING_MAC)
endif()

if(KEYRING_IOS)
  add_definitions(-DOT_KEYRING_IOS)
endif()

if(KEYRING_GNOME)
  add_definitions(-DOT_KEYRING_GNOME)
endif()

if(KEYRING_KWALLET)
  add_definitions(-DOT_KEYRING_KWALLET)
endif()

if(KEYRING_FLATFILE)
  add_definitions(-DOT_KEYRING_FLATFILE)
endif()

if(NO_PASSWORD)
  add_definitions(-DOT_NO_PASSWORD)
endif()

if(WIN32)
  add_definitions("-DEXPORT=__declspec(dllexport)")
else()
  add_definitions(-DEXPORT=)
endif()

#Script engines
if(OT_SCRIPT_USING_CHAI)
  add_definitions(-DOT_USE_SCRIPT_CHAI)
endif()

add_definitions(-DOPENTXS_INTERNAL_BUILD=1)

#-----------------------------------------------------------------------------
# Set in-source Version
configure_file("include/opentxs/core/Version.hpp" "${CMAKE_CURRENT_BINARY_DIR}/include/opentxs/core/Version.hpp" @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/opentxs/core/Version.hpp
        DESTINATION include/opentxs/core/
        COMPONENT dev)

#-----------------------------------------------------------------------------
# Build source

#Fix RPATH
if(APPLE)
  set(CMAKE_MACOSX_RPATH 0)
  set(CMAKE_INSTALL_NAME_DIR @rpath)
endif()
if(NOT RPM)
  SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
  if(NOT WIN32)
    include(GNUInstallDirs)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
  endif()
endif()

if(UNIX)
  add_definitions(-DOT_PREFIX_PATH=\"${CMAKE_INSTALL_PREFIX}\")
endif()

function(set_lib_property lib)
  if (NOT ANDROID)
    set_target_properties(${lib} PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${VERSION_MAJOR})
  endif()
endfunction(set_lib_property)

add_subdirectory(deps)

if (OT_CRYPTO_USING_LIBSECP256K1 AND OT_INTERNAL_LIBSECP)
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/deps/include)
    link_directories(${CMAKE_CURRENT_BINARY_DIR}/deps/lib)
endif()

if (OT_INTERNAL_ZMQ)
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/deps/include)
endif()

add_subdirectory(src)
add_subdirectory(wrappers)

if (OT_CRYPTO_USING_LIBSECP256K1 AND OT_INTERNAL_LIBSECP)
    add_dependencies(opentxs-cron libsecp256k1)
    add_dependencies(opentxs-recurring libsecp256k1)
    add_dependencies(opentxs-script libsecp256k1)
    add_dependencies(opentxs-trade libsecp256k1)
    add_dependencies(opentxs-storage libsecp256k1)
    add_dependencies(opentxs-network libsecp256k1)
endif()

if (OT_INTERNAL_ZMQ)
  add_dependencies(opentxs-core internal_czmq internal_zeromq)
endif()

if (NOT ANDROID)
  add_subdirectory(tests)
endif()

if (NOT ANDROID)
#-----------------------------------------------------------------------------
# Produce a cmake-package

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_DEST "share/cmake/Modules")
set(OPENTXS_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")

export(TARGETS opentxs opentxs-core FILE "${CMAKE_BINARY_DIR}/opentxsTargets.cmake")

configure_package_config_file(
       "cmake/opentxsConfig.cmake.in"
       "${CMAKE_BINARY_DIR}/opentxsConfig.cmake"
       INSTALL_DESTINATION ${CMAKE_CONFIG_DEST}
       PATH_VARS OPENTXS_INCLUDE_DIRS)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/opentxsConfigVersion.cmake"
  VERSION ${VERSION_STRING}
  COMPATIBILITY AnyNewerVersion)

install(EXPORT opentxsTargets
        DESTINATION ${CMAKE_CONFIG_DEST} FILE "opentxsTargets.cmake")

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/opentxsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/opentxsConfigVersion.cmake"
        DESTINATION ${CMAKE_CONFIG_DEST}
        COMPONENT dev)

#-----------------------------------------------------------------------------
# Produce a pkg-config file

if(NOT WIN32)
	configure_file("cmake/opentxs.pc.in" "opentxs.pc" @ONLY)
	include(GNUInstallDirs)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/opentxs.pc"
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
		COMPONENT main)
endif()

#-----------------------------------------------------------------------------
# Install

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include
        COMPONENT dev
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "Version.hpp" EXCLUDE)

install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/simple_ptr.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/containers_fixes.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/exceptions.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/copy_functors.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/simple_ptr.tpp
	DESTINATION include/opentxs/core/containers
	COMPONENT dev)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/irrxml/
        DESTINATION include/irrxml
        COMPONENT dev
        FILES_MATCHING PATTERN "*.hpp")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ot/
        DESTINATION lib/opentxs
        COMPONENT dev
        FILES_MATCHING PATTERN "*.ot")

# opentxs CL client autocompletion
if(AUTOCOMPLETION)
  install(FILES
          ${CMAKE_CURRENT_SOURCE_DIR}/scripts/bash_completion.d/opentxs-bash-completion.sh
	  DESTINATION /etc/bash_completion.d/
          COMPONENT main)
endif()

#-----------------------------------------------------------------------------
# Uninstal
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake)

#-----------------------------------------------------------------------------
# Packaging
set(CPACK_GENERATOR "TGZ")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

set(CPACK_PACKAGE_VERSION ${VERSION_STRING})
set(CPACK_PACKAGE_NAME "opentxs")
set(CPACK_PACKAGE_CONTACT ${PACKAGE_CONTACT})
set(CPACK_PACKAGE_VENDOR ${PACKAGE_VENDOR})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Transactions library and command line client")
set(CPACK_PACKAGE_DESCRIPTION "
Open-Transactions democratizes financial and monetary actions.
You can use it for issuing currencies/stock, paying dividends, creating asset accounts, sending/receiving digital cash, writing/depositing cheques, cashier's cheques, creating basket currencies, trading on markets, scripting custom agreements, recurring payments, escrow, etc.
")

if(RPM)
  set(CPACK_GENERATOR "RPM")
  set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
  set(CPACK_RPM_COMPONENT_INSTALL ON)
  set(CPACK_RPM_CHANGELOG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog")
endif()

if(DEB)
  set(CPACK_GENERATOR "DEB")
  set(CPACK_DEB_COMPONENT_INSTALL ON)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "openssl, libprotobuf8")
  set(CPACK_DEBIAN_PACKAGE_PRIORITY "extra")
  set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${OPEN_TRANSACTIONS_WEBSITE})
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${PACKAGE_CONTACT})
endif()

include(CPack)
endif (NOT ANDROID)
