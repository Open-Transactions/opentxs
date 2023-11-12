# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(SCRIPT_CHAI_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/external/ChaiScript.hpp"
      "OTScriptChai.cpp"
      "OTScriptChai.hpp"
  )
  libopentxs_include_directories(
    "${opentxs_SOURCE_DIR}/external/ChaiScript/include"
  )
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()
