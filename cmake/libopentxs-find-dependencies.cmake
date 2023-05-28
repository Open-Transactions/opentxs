# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(Boost_NO_WARN_NEW_VERSIONS 1)
find_package(
  Boost
  1.82.0
  QUIET
  REQUIRED
  COMPONENTS
    iostreams
    json
    program_options
    system
    thread
  OPTIONAL_COMPONENTS
    align
    any
    array
    assert
    atomic
    bind
    chrono
    circular_buffer
    concept_check
    config
    container_hash
    context
    core
    coroutine
    describe
    detail
    exception
    function
    fusion
    integer
    io
    iterator
    lexical_cast
    logic
    math
    mp11
    mpl
    numeric-conversion
    optional
    parameter
    predef
    preprocessor
    random
    range
    regex
    serialization
    smart_ptr
    stacktrace_basic
    static_assert
    static_string
    throw_exception
    tokenizer
    tuple
    type_index
    type_traits
    typeof
    ublas
    unordered
    variant2
    winapi
)
find_package(OpenSSL REQUIRED)

if(OT_USE_VCPKG_TARGETS)
  find_package(
    Protobuf
    CONFIG
    REQUIRED
  )
else()
  find_package(Protobuf REQUIRED)
endif()

find_package(
  TBB
  CONFIG
  REQUIRED
)
find_package(Threads REQUIRED)

if(OT_USE_VCPKG_TARGETS)
  find_package(
    unofficial-sodium
    CONFIG
    REQUIRED
  )
else()
  find_package(unofficial-sodium REQUIRED)
endif()

find_package(ZLIB REQUIRED)

if(WIN32)
  find_package(pthread REQUIRED)
endif()

if(OT_USE_VCPKG_TARGETS)
  find_package(
    ZeroMQ
    CONFIG
    REQUIRED
  )
else()
  find_package(unofficial-zeromq REQUIRED)
endif()

if(OT_STORAGE_SQLITE)
  find_package(SQLite3 REQUIRED)
endif()

if(OT_STORAGE_LMDB)
  if(OT_USE_VCPKG_TARGETS)
    find_package(
      lmdb
      CONFIG
      REQUIRED
    )
  else()
    find_package(lmdb REQUIRED)
  endif()
endif()

if(OT_CRYPTO_USING_LIBSECP256K1 AND NOT OT_BUNDLED_SECP256K1)
  find_package(unofficial-secp256k1 REQUIRED)
endif()

if(OT_WITH_QT)
  if(NOT
     DEFINED
     QT_VERSION_MAJOR
  )
    find_package(
      QT
      REQUIRED
      NAMES
      Qt6
      Qt5
    )
  endif()

  find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS
      Core
      Gui
      CONFIG
    REQUIRED
  )

  if(OT_WITH_QML)
    find_package(
      Qt${QT_VERSION_MAJOR}
      COMPONENTS Qml CONFIG
      REQUIRED
    )
  endif()
endif()
