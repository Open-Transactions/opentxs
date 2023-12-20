# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(libopentxs_configure_c_target target_name)
  include(otcommon-configure-target)
  include(otcommon-define-signed-overflow)
  otcommon_configure_c_target(${target_name})
  otcommon_define_signed_overflow(${target_name})

  if(CMAKE_CXX_COMPILER_ID
     MATCHES
     Clang
     AND CMAKE_CXX_COMPILER_VERSION
         VERSION_GREATER_EQUAL
         16.0.0
  )
    # NOTE too many false positives and hits in 3rd party & generated code
    target_compile_options(${target_name} PRIVATE "-Wno-unsafe-buffer-usage")
  endif()
endfunction()
