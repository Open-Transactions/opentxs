# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-common OBJECT "")

if(OT_QT_EXPORT)
  add_library(opentxs-moc OBJECT "")
endif()

target_link_libraries(opentxs-common PRIVATE Boost::headers opentxs::messages)
target_include_directories(
  opentxs-common SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
add_dependencies(opentxs-common generated_code)

if(OT_PCH)
  target_precompile_headers(
    opentxs-common
    PUBLIC
    "$<$<COMPILE_LANGUAGE:CXX>:<algorithm$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<array$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<atomic$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cassert$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cctype$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cerrno$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<chrono$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cmath$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<condition_variable$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<csignal$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstdarg$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstddef$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstdint$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstdio$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstdlib$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstring$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<ctime$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<deque$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<exception$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<filesystem$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<forward_list$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<fstream$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<functional$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<future$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<iomanip$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<ios$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<iosfwd$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<iostream$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<iterator$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<limits$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<list$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<locale$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<map$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<memory$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<mutex$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<new$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<numeric$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<optional$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<ostream$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<queue$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<random$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<ratio$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<regex$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<set$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<shared_mutex$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<sstream$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<stdexcept$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<string$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<string_view$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<system_error$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<thread$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<tuple$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<typeindex$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<typeinfo$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<type_traits$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<unordered_map$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<unordered_set$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<utility$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<variant$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<vector$<ANGLE-R>>"
  )

  if(APPLE OR ANDROID)
    target_precompile_headers(
      opentxs-common
      PUBLIC
      "$<$<COMPILE_LANGUAGE:CXX>:<experimental/deque$<ANGLE-R>>"
      "$<$<COMPILE_LANGUAGE:CXX>:<experimental/forward_list$<ANGLE-R>>"
      "$<$<COMPILE_LANGUAGE:CXX>:<experimental/list$<ANGLE-R>>"
      "$<$<COMPILE_LANGUAGE:CXX>:<experimental/map$<ANGLE-R>>"
      "$<$<COMPILE_LANGUAGE:CXX>:<experimental/memory_resource$<ANGLE-R>>"
      "$<$<COMPILE_LANGUAGE:CXX>:<experimental/set$<ANGLE-R>>"
      "$<$<COMPILE_LANGUAGE:CXX>:<experimental/string$<ANGLE-R>>"
      "$<$<COMPILE_LANGUAGE:CXX>:<experimental/unordered_map$<ANGLE-R>>"
      "$<$<COMPILE_LANGUAGE:CXX>:<experimental/unordered_set$<ANGLE-R>>"
      "$<$<COMPILE_LANGUAGE:CXX>:<experimental/vector$<ANGLE-R>>"
    )
  else()
    target_precompile_headers(
      opentxs-common
      PUBLIC
      "$<$<COMPILE_LANGUAGE:CXX>:<memory_resource$<ANGLE-R>>"
    )
  endif()
endif()

set(OPENTXS_QT_PUBLIC_HEADERS "")
set(OPENTXS_QT_PRIVATE_HEADERS "")

add_subdirectory(api)
add_subdirectory(blockchain)
add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(identity)
add_subdirectory(interface)
add_subdirectory(network)
add_subdirectory(otx)
add_subdirectory(serialization)
add_subdirectory(std)
add_subdirectory(util)

if(OT_QT_EXPORT)
  qt_wrap_cpp(
    OPENTXS_MOC_SOURCES
    ${OPENTXS_QT_PUBLIC_HEADERS}
    ${OPENTXS_QT_PRIVATE_HEADERS}
  )
  target_sources(
    opentxs-moc
    PRIVATE
      ${OPENTXS_QT_PUBLIC_HEADERS}
      ${OPENTXS_QT_PRIVATE_HEADERS}
      ${OPENTXS_MOC_SOURCES}
  )

  if(NOT MSVC)
    target_compile_options(
      opentxs-moc
      PRIVATE
        -Wno-extra-semi-stmt
        -Wno-redundant-parens
        -Wno-reserved-identifier
    )
  endif()

  set_target_properties(
    opentxs-moc
    PROPERTIES
      C_INCLUDE_WHAT_YOU_USE ""
      CXX_INCLUDE_WHAT_YOU_USE ""
      C_CLANG_TIDY ""
      CXX_CLANG_TIDY ""
  )
  target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-moc>)
endif()

include(GenerateExportHeader)
generate_export_header(
  opentxs
  EXPORT_FILE_NAME
  "${opentxs_BINARY_DIR}/src/opentxs/Export.hpp"
  EXPORT_MACRO_NAME
  OPENTXS_EXPORT
)

set(cxx-install-headers "${opentxs_BINARY_DIR}/src/opentxs/Export.hpp"
                        "${opentxs_SOURCE_DIR}/include/opentxs/OT.hpp"
)
target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_BINARY_DIR}/src/Version.cpp"
    "0_stdafx.hpp"
    "2_Factory.hpp"
    "OT.cpp"
    "Proto.cpp"
    "Proto.hpp"
    "Proto.tpp"
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-common>)
set_target_properties(
  opentxs
  PROPERTIES
    VERSION
    "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}"
    SOVERSION 1
    POSITION_INDEPENDENT_CODE 1
)

if(Threads_FOUND)
  target_link_libraries(opentxs PUBLIC Threads::Threads)
endif()

if(CMAKE_DL_LIBS)
  target_link_libraries(opentxs PUBLIC ${CMAKE_DL_LIBS})
endif()

if(LIB_RT)
  target_link_libraries(opentxs PUBLIC ${LIB_RT})
endif()

if(LIB_ANDROID_LOG)
  target_link_libraries(opentxs PUBLIC ${LIB_ANDROID_LOG})
endif()

if(Boost_stacktrace_basic_FOUND)
  target_link_libraries(opentxs PUBLIC Boost::stacktrace_basic)
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs
    COMPONENT dev
  )
  install(
    FILES ${OPENTXS_QT_PUBLIC_HEADERS}
    DESTINATION include/opentxs/interface/qt
    COMPONENT dev
  )
endif()

if(OPENTXS_STANDALONE OR OT_INSTALL_LIBRARY_DEPENDENCIES)
  install(
    TARGETS opentxs
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    EXPORT opentxs-targets
    COMPONENT main
  )
endif()

if(OT_INSTALL_LIBRARY_DEPENDENCIES)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_SUFFIX)
  get_target_property(
    Boost_system_LIBRARY
    Boost::system
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Boost_thread_LIBRARY
    Boost::thread
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Protobuf_LITE_LIBRARY
    protobuf::libprotobuf-lite
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    ZLIB_LIBRARY
    ZLIB::ZLIB
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )

  if(OT_USE_VCPKG_TARGETS)
    get_target_property(
      ZMQ_LIBRARY_PATH
      "${OT_ZMQ_TARGET}"
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )
  else()
    set(ZMQ_LIBRARY_PATH "${ZMQ_LIBRARY}")
  endif()

  if(OPENTXS_BUILD_TESTS)
    install(
      FILES "${GTEST_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif(OPENTXS_BUILD_TESTS)

  install(
    FILES
      "${Boost_system_LIBRARY}"
      "${Boost_thread_LIBRARY}"
      "${Protobuf_LITE_LIBRARY}"
      "${SODIUM_LIBRARY}"
      "${ZLIB_LIBRARY}"
      "${ZMQ_LIBRARY_PATH}"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT main
  )

  if(Boost_stacktrace_basic_FOUND)
    get_target_property(
      Boost_stacktrace_basic_LIBRARY
      Boost::stacktrace_basic
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )
    install(
      FILES "${Boost_stacktrace_basic_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(Boost_iostreams_FOUND)
    get_target_property(
      Boost_iostreams_LIBRARY
      Boost::iostreams
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )
    install(
      FILES "${Boost_iostreams_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(SQLITE_EXPORT)
    get_target_property(
      SQLITE3_LIBRARY
      SQLite::SQLite3
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )

    install(
      FILES "${SQLITE3_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif(SQLITE_EXPORT)

  if(LMDB_EXPORT)
    install(
      FILES "${LMDB_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(OPENSSL_EXPORT)
    install(
      FILES "${OPENSSL_CRYPTO_LIBRARY}" "${OPENSSL_SSL_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(LIBSECP256K1_EXPORT AND NOT OT_BUNDLED_SECP256K1)
    install(
      FILES "${SECP256K1_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()
endif()
