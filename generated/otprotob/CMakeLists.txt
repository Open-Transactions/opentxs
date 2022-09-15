# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

protobuf_generate_cpp(
  cxx-sources
  cxx-headers
  Bitcoin.proto
  Generics.proto
  Markets.proto
  Moneychanger.proto
)

add_library(opentxs-core-otprotob OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-core-otprotob PUBLIC protobuf::libprotobuf-lite)
target_link_libraries(opentxs PUBLIC protobuf::libprotobuf-lite)
target_include_directories(
  opentxs-core-otprotob SYSTEM PUBLIC "${CMAKE_CURRENT_BINARY_DIR}"
)

if(MSVC)
  target_compile_options(opentxs-core-otprotob PRIVATE "/wd4244" "/wd4267")
else()
  target_compile_options(
    opentxs-core-otprotob
    PRIVATE
      -Wno-c++20-compat
      -Wno-effc++
      -Wno-extra-semi
      -Wno-missing-declarations
      -Wno-reserved-identifier
      -Wno-suggest-destructor-override
      -Wno-switch-default
      -Wno-switch-enum
      -Wno-undef
      -Wno-unused-macros
  )
endif()

if(WIN32)
  set_target_properties(messages PROPERTIES UNITY_BUILD OFF)
endif()

set_target_properties(
  opentxs-core-otprotob
  PROPERTIES
    C_INCLUDE_WHAT_YOU_USE ""
    CXX_INCLUDE_WHAT_YOU_USE ""
    C_CLANG_TIDY ""
    CXX_CLANG_TIDY ""
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-core-otprotob>)
add_dependencies(generated_code opentxs-core-otprotob)
