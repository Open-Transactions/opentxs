# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_include_directories DIR)
  target_include_directories(opentxs-common SYSTEM PRIVATE "${DIR}")

  if(OPENTXS_BUILD_TESTS)
    target_include_directories(opentxs-testlib SYSTEM PRIVATE "${DIR}")
  endif()
endmacro()
