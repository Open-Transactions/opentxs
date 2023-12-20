# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(libopentxs_configure_constexpr_target target_name)
  include(libopentxs-configure-cxx-target)
  libopentxs_configure_cxx_target(${target_name})
  set_target_properties(${target_name} PROPERTIES UNITY_BUILD OFF)
  target_link_libraries(${target_name} PRIVATE opentxs-common)
  target_compile_definitions(${target_name} PRIVATE "${OT_EXPORT_MACRO}")
  include(libopentxs-add-sources)
  libopentxs_add_sources($<TARGET_OBJECTS:${target_name}>)

  set(MAX_CONSTEXPR_STEPS "268435456")

  if(MSVC)
    target_compile_options(
      ${target_name} PRIVATE "/constexpr:steps${MAX_CONSTEXPR_STEPS}"
    )
  elseif(
    CMAKE_CXX_COMPILER_ID
    MATCHES
    GNU
  )
    target_compile_options(
      ${target_name} PRIVATE "-fconstexpr-ops-limit=${MAX_CONSTEXPR_STEPS}"
    )
  elseif(
    CMAKE_CXX_COMPILER_ID
    MATCHES
    Clang
    OR CMAKE_CXX_COMPILER_ID
       MATCHES
       AppleClang
  )
    target_compile_options(
      ${target_name} PRIVATE "-fconstexpr-steps=${MAX_CONSTEXPR_STEPS}"
    )
  endif()
endfunction()
