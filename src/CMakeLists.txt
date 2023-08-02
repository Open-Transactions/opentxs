# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_include_directories(
  opentxs-common PUBLIC "${CMAKE_CURRENT_BINARY_DIR}"
                        "${CMAKE_CURRENT_SOURCE_DIR}"
)

if(OT_QT_EXPORT)
  add_library(opentxs-moc OBJECT "")
  libopentxs_configure_cxx_target(opentxs-moc)
  target_include_directories(
    opentxs-moc PRIVATE "${opentxs_BINARY_DIR}/include"
                        "${opentxs_SOURCE_DIR}/include"
  )
endif()

include(libopentxs-add-public-header-qt)

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
    "$<$<COMPILE_LANGUAGE:CXX>:<concepts$<ANGLE-R>>"
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
    "$<$<COMPILE_LANGUAGE:CXX>:<span$<ANGLE-R>>"
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

  if(OT_USE_PSTL)
    target_precompile_headers(
      opentxs-common
      PUBLIC
      "$<$<COMPILE_LANGUAGE:CXX>:<execution$<ANGLE-R>>"
      "$<$<COMPILE_LANGUAGE:CXX>:<ranges$<ANGLE-R>>"
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

libopentxs_add_public_header("" "Export.hpp")
libopentxs_add_public_header("" "OT.hpp")
libopentxs_add_public_header("" "opentxs.hpp")

if(OT_QT_EXPORT)
  libopentxs_add_public_header("" "Qt.hpp")

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
        -Wno-unsafe-buffer-usage
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
  target_compile_definitions(
    opentxs-moc PRIVATE "${OT_EXPORT_MACRO}" "QT_NO_KEYWORDS"
  )
  target_compile_definitions(opentxs-common PRIVATE "QT_NO_KEYWORDS")
  libopentxs_add_sources($<TARGET_OBJECTS:opentxs-moc>)
endif()

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_BINARY_DIR}/src/Version.cpp"
    "2_Factory.hpp"
    "OT.cpp"
)

if(OT_WITH_TBB OR OT_PSTL_NEEDS_TBB)
  target_sources(opentxs-common PRIVATE "tbb.cpp")
  libopentxs_link_internal(TBB::tbb)
  libopentxs_link_external(TBB::tbb)
else()
  target_sources(opentxs-common PRIVATE "no-tbb.cpp")
endif()

libopentxs_add_sources($<TARGET_OBJECTS:opentxs-common>)

if(OPENTXS_BUILD_TESTS)
  target_sources(opentxs-testlib PRIVATE $<TARGET_OBJECTS:opentxs-common>)
endif()

if(Threads_FOUND)
  libopentxs_link_external(Threads::Threads)
endif()

if(CMAKE_DL_LIBS)
  libopentxs_link_external(${CMAKE_DL_LIBS})
endif()

if(LIB_RT)
  libopentxs_link_external(${LIB_RT})
endif()

if(LIB_ANDROID_LOG)
  libopentxs_link_external(${LIB_ANDROID_LOG})
endif()

if(Boost_exception_FOUND)
  libopentxs_link_external(Boost::exception)
endif()

if(Boost_stacktrace_basic_FOUND)
  libopentxs_link_external(Boost::stacktrace_basic)
endif()

include(libopentxs-install)
libopentxs_install()
