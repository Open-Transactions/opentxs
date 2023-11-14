# Copyright (c) 2020-2023 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_set_osx_deployment_target TARGET_VERSION)
  if(CMAKE_OSX_DEPLOYMENT_TARGET
     VERSION_LESS
     "${TARGET_VERSION}"
  )
    if(DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
      message(
        WARNING
          "Increasing previously defined CMAKE_OSX_DEPLOYMENT_TARGET to ${TARGET_VERSION}"
      )
    endif()

    set(CMAKE_OSX_DEPLOYMENT_TARGET "${TARGET_VERSION}")
  endif()
endmacro()
