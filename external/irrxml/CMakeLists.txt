# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-irrxml OBJECT
  "CXMLReaderImpl.hpp"
  "fast_atof.hpp"
  "heapsort.hpp"
  "irrArray.hpp"
  "irrString.hpp"
  "irrTypes.hpp"
  "irrXML.cpp"
  "irrXML.hpp"
)
libopentxs_configure_cxx_target(opentxs-irrxml)

if(NOT MSVC)
  target_compile_options(
    opentxs-irrxml
    PRIVATE
      -Wno-cast-align
      -Wno-cast-qual
      -Wno-conversion
      -Wno-documentation
      -Wno-documentation-unknown-command
      -Wno-effc++
      -Wno-extra-semi
      -Wno-implicit-int-conversion
      -Wno-old-style-cast
      -Wno-reserved-id-macro
      -Wno-shorten-64-to-32
      -Wno-sign-conversion
      -Wno-suggest-destructor-override
      -Wno-suggest-override
      -Wno-zero-as-null-pointer-constant
  )
endif()

libopentxs_add_sources($<TARGET_OBJECTS:opentxs-irrxml>)
