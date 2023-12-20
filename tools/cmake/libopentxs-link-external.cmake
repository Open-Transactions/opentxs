# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_link_external LIB)
  target_link_libraries(libopentxs PUBLIC "${LIB}")
  target_link_libraries(opentxs-common PUBLIC "${LIB}")

  if(OPENTXS_BUILD_TESTS)
    target_link_libraries(opentxs-testlib PUBLIC "${LIB}")
  endif()
endmacro()
