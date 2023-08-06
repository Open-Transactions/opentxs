# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(PROTO_SOURCES Generics.proto Markets.proto)

protobuf_generate_cpp(
  PROTO_SRC
  PROTO_HEADER
  ${PROTO_SOURCES}
)

add_library(opentxs-otprotob OBJECT ${PROTO_SRC} ${PROTO_HEADER})
libopentxs_configure_cxx_target(opentxs-otprotob)
target_link_libraries(opentxs-otprotob PUBLIC protobuf::libprotobuf-lite)
target_include_directories(
  opentxs-otprotob SYSTEM PUBLIC "${CMAKE_CURRENT_BINARY_DIR}"
)

if(MSVC)
  target_compile_options(opentxs-otprotob PRIVATE "/wd4244" "/wd4267")
else()
  target_compile_options(
    opentxs-otprotob
    PRIVATE
      -Wno-c++20-compat
      -Wno-effc++
      -Wno-extra-semi
      -Wno-extra-semi-stmt
      -Wno-missing-declarations
      -Wno-reserved-identifier
      -Wno-shorten-64-to-32
      -Wno-suggest-destructor-override
      -Wno-switch-default
      -Wno-switch-enum
      -Wno-undef
      -Wno-unsafe-buffer-usage
      -Wno-unused-macros
  )
endif()

if(WIN32)
  set_target_properties(opentxs-otprotob PROPERTIES UNITY_BUILD OFF)
endif()

set_target_properties(
  opentxs-otprotob
  PROPERTIES
    C_INCLUDE_WHAT_YOU_USE ""
    CXX_INCLUDE_WHAT_YOU_USE ""
    C_CLANG_TIDY ""
    CXX_CLANG_TIDY ""
)

libopentxs_link_internal(opentxs-otprotob)
libopentxs_link_external(protobuf::libprotobuf-lite)
libopentxs_link_external($<BUILD_INTERFACE:opentxs-otprotob>)
libopentxs_add_sources($<TARGET_OBJECTS:opentxs-otprotob>)
add_dependencies(generated_code opentxs-otprotob)
